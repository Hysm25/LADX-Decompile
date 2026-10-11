#!/usr/bin/env python3
"""
Automated unit tests for LADX Authoritative Routine Inventory Generator.

Tests:
  - Opcode vs. data directive classification
  - Data macros (dialogue, entities, audio bytecode, palettes, SGB, credits)
  - Execution-expanding macros (callsb, callsw, callhl, jpsb, jp_table, farcall)
  - Alias detection (consecutive column-0 labels sharing the same address)
  - Internal label classification (jr_, ret_, jump_, _return)
  - Local label classification (.loop, .variant, etc.)
  - Comment stripping and lookahead past local labels
  - Ambiguous / unresolved symbol handling
  - Real assembly samples from multiple ROM banks
  - Determinism verification
"""

import os
import sys
import tempfile
import unittest

# Add tools directory to Python path
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
TOOLS_DIR = os.path.join(PROJECT_ROOT, "tools")
sys.path.insert(0, TOOLS_DIR)

import generate_routine_inventory as gri


class TestRoutineInventoryClassifier(unittest.TestCase):
    """Test classification rules and edge cases in symbol parsing."""

    def test_opcodes_vs_data_directives(self):
        """Verify standard CPU opcodes classify as code and data directives classify as data."""
        sample_asm = """
FunctionA::
    ld   a, [wLinkHealth]
    ret

DataTableA::
    db   $01, $02, $03, $04

DataTableB::
    dw   $4000, $4002

DataBinaryA::
    incbin "gfx/test.2bpp"
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {0: [rel_file]}
            # Patch DISASM_ROOT temporarily
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            self.assertEqual(sym_map.get("FunctionA"), "ROUTINE")
            self.assertEqual(sym_map.get("DataTableA"), "DATA_TABLE")
            self.assertEqual(sym_map.get("DataTableB"), "DATA_TABLE")
            self.assertEqual(sym_map.get("DataBinaryA"), "DATA_TABLE")
        finally:
            os.remove(temp_path)

    def test_data_macros_classification(self):
        """Ensure dialogue, entity, audio bytecode, palette, and SGB macros are DATA_TABLE, not ROUTINE."""
        sample_asm = """
DialogSample::
    dialog_text "Hello World!"

RoomEntitiesSample::
    entity 2, 4, $15
    entities_end

ColorPaletteSample::
    rgb #F8F888, #000000

AudioMusicSample::
    note D_, 4
    notelen 8
    set_waveform 2
    set_envelope_duty $30
    begin_loop 2
    next_loop
    rest 4

SgbCommandSample::
    sgb_cmd 1, 4

CreditsFadeSample::
    credits_fade 1, 2, 3, 4
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {0: [rel_file]}
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            self.assertEqual(sym_map.get("DialogSample"), "DATA_TABLE")
            self.assertEqual(sym_map.get("RoomEntitiesSample"), "DATA_TABLE")
            self.assertEqual(sym_map.get("ColorPaletteSample"), "DATA_TABLE")
            self.assertEqual(sym_map.get("AudioMusicSample"), "DATA_TABLE")
            self.assertEqual(sym_map.get("SgbCommandSample"), "DATA_TABLE")
            self.assertEqual(sym_map.get("CreditsFadeSample"), "DATA_TABLE")
        finally:
            os.remove(temp_path)

    def test_execution_macros_classification(self):
        """Ensure execution-expanding macros classify as ROUTINE."""
        sample_asm = """
FarCaller1::
    callsb TargetRoutine

FarCaller2::
    callsw TargetRoutine2

FarCaller3::
    callhl TargetRoutine3

TableJumper::
    jp_table

BankCaller::
    call_bank 3, TargetRoutine4
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {0: [rel_file]}
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            self.assertEqual(sym_map.get("FarCaller1"), "ROUTINE")
            self.assertEqual(sym_map.get("FarCaller2"), "ROUTINE")
            self.assertEqual(sym_map.get("FarCaller3"), "ROUTINE")
            self.assertEqual(sym_map.get("TableJumper"), "ROUTINE")
            self.assertEqual(sym_map.get("BankCaller"), "ROUTINE")
        finally:
            os.remove(temp_path)

    def test_alias_detection(self):
        """Ensure consecutive column-0 labels sharing the same ROM entry are classified as ALIAS."""
        sample_asm = """
PrimaryRoutine::
SecondaryAlias::
    ld   a, 1
    ret
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {0: [rel_file]}
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            self.assertEqual(sym_map.get("PrimaryRoutine"), "ROUTINE")
            self.assertEqual(sym_map.get("SecondaryAlias"), "ALIAS")
        finally:
            os.remove(temp_path)

    def test_internal_and_local_labels(self):
        """Ensure jr_, ret_, jump_, _return, and .local labels are classified as INTERNAL_LABEL / LOCAL_LABEL."""
        sample_asm = """
MyRoutine::
.loop
    dec  a
    jr   nz, .loop
    jr   jr_001_5A32

jr_001_5A32:
    ret

ret_001_5A34:
    ret

MyRoutine_return:
    ret
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {0: [rel_file]}
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            self.assertEqual(sym_map.get("MyRoutine"), "ROUTINE")
            self.assertEqual(sym_map.get(".loop"), "LOCAL_LABEL")
            self.assertEqual(sym_map.get("jr_001_5A32"), "INTERNAL_LABEL")
            self.assertEqual(sym_map.get("ret_001_5A34"), "INTERNAL_LABEL")
            self.assertEqual(sym_map.get("MyRoutine_return"), "INTERNAL_LABEL")
        finally:
            os.remove(temp_path)

    def test_comment_stripping_and_local_lookahead(self):
        """Ensure comments with address annotations and intermediate local labels do not break lookahead."""
        sample_asm = """
SpriteVariantsTable::
.variant0 ;; 03:69AA
    db   $36, $02
.variant1 ;; 03:69AE
    db   $36, $03
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {3: [rel_file]}
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            # Must be DATA_TABLE, NOT ROUTINE!
            self.assertEqual(sym_map.get("SpriteVariantsTable"), "DATA_TABLE")
        finally:
            os.remove(temp_path)

    def test_ambiguous_and_unresolved_symbols(self):
        """Ensure unknown tokens are tagged UNRESOLVED and never silently forced to ROUTINE."""
        sample_asm = """
AmbiguousSymbol::
    unknown_custom_directive_12345 foo, bar
"""
        with tempfile.NamedTemporaryFile("w", suffix=".asm", delete=False) as f:
            f.write(sample_asm)
            temp_path = f.name

        try:
            rel_file = os.path.basename(temp_path)
            bank_files = {0: [rel_file]}
            orig_disasm = gri.DISASM_ROOT
            gri.DISASM_ROOT = os.path.dirname(temp_path)
            try:
                symbols = gri.parse_asm_symbols(bank_files, {})
            finally:
                gri.DISASM_ROOT = orig_disasm

            sym_map = {s["name"]: s["type"] for s in symbols}
            self.assertEqual(sym_map.get("AmbiguousSymbol"), "UNRESOLVED")
        finally:
            os.remove(temp_path)


class TestRealAssemblyIntegration(unittest.TestCase):
    """Integration tests on actual LADX disassembly files across multiple banks."""

    @classmethod
    def setUpClass(cls):
        cls.bank_files, cls.file_banks, cls.main_sections = gri.build_include_hierarchy("main.asm")
        cls.c_symbols = gri.index_c_symbols()
        cls.test_defs, cls.tested_c_funcs = gri.index_test_symbols(cls.c_symbols)
        cls.symbols = gri.parse_asm_symbols(cls.bank_files, cls.main_sections)
        cls.reconciled = gri.reconcile_and_audit(cls.symbols, cls.c_symbols, cls.tested_c_funcs)

    def test_bank16_has_zero_code_routines(self):
        """Bank 16 contains pure entity placement lists and text scripts; it must have 0 ROUTINE symbols."""
        b16_routines = [s for s in self.reconciled if s["bank"] == 16 and s["type"] == "ROUTINE"]
        self.assertEqual(len(b16_routines), 0, f"Bank 16 has {len(b16_routines)} routines, expected 0 pure data.")

    def test_bank3_group_a_and_b_verified(self):
        """Batch 111 verified routines in Bank 3 must be marked VERIFIED with non-null C and test sources."""
        verified_names = [
            "Entity4BHandler", "LiftableRockEntityHandler", "MagicRodFireballEntityHandler",
            "setCarryAndReturn", "entitiesLoop", "forceCollision", "forceCollisionEnd",
            "checkNextEntity", "Entity13Handler", "ApplyVectorTowardsLinkAndReturn"
        ]
        rec_map = {s["name"]: s for s in self.reconciled if s["bank"] == 3}
        for name in verified_names:
            sym = rec_map.get(name)
            self.assertIsNotNone(sym, f"Routine {name} missing from Bank 3 inventory")
            self.assertEqual(sym["status"], "VERIFIED", f"Routine {name} status is {sym.get('status')}, expected VERIFIED")
            self.assertIsNotNone(sym.get("c_source"), f"Routine {name} missing C source")
            self.assertIsNotNone(sym.get("test_source"), f"Routine {name} missing test source")

    def test_deterministic_output(self):
        """Running reconciliation twice on the same repository produces identical results."""
        symbols_second = gri.parse_asm_symbols(self.bank_files, self.main_sections)
        reconciled_second = gri.reconcile_and_audit(symbols_second, self.c_symbols, self.tested_c_funcs)
        self.assertEqual(len(self.reconciled), len(reconciled_second))
        for s1, s2 in zip(self.reconciled, reconciled_second):
            self.assertEqual(s1["name"], s2["name"])
            self.assertEqual(s1["type"], s2["type"])
            self.assertEqual(s1["status"], s2["status"])
            self.assertEqual(s1["bank"], s2["bank"])
            self.assertEqual(s1["file"], s2["file"])
            self.assertEqual(s1["line"], s2["line"])

    def test_data_001_48e4_classified_as_data_table(self):
        """Data_001_48E4 in file_menus.asm is a 4-byte cursor position table and must be DATA_TABLE, not ROUTINE."""
        sym = next((s for s in self.reconciled if s["name"] == "Data_001_48E4"), None)
        self.assertIsNotNone(sym, "Data_001_48E4 missing from inventory")
        self.assertEqual(sym["type"], "DATA_TABLE")
        self.assertEqual(sym["status"], "DATA")

    def test_per_bank_and_project_wide_mathematical_consistency(self):
        """Verify that every bank and the project-wide totals reconcile mathematically without discrepancy."""
        bank_stats = {}
        for b in range(64):
            bank_stats[b] = {"routines": 0, "verified": 0, "decompiled": 0, "missing": 0, "blocked": 0}

        for s in self.reconciled:
            b = s["bank"]
            t = s["type"]
            st = s.get("status")
            if t == "ROUTINE":
                bank_stats[b]["routines"] += 1
                if st == "VERIFIED":
                    bank_stats[b]["verified"] += 1
                elif st == "DECOMPILED":
                    bank_stats[b]["decompiled"] += 1
                elif st == "MISSING":
                    bank_stats[b]["missing"] += 1
                elif st == "BLOCKED":
                    bank_stats[b]["blocked"] += 1

        total_r = 0
        total_v = 0
        total_d = 0
        total_m = 0
        total_b = 0
        for b in range(64):
            st = bank_stats[b]
            self.assertEqual(
                st["routines"],
                st["verified"] + st["decompiled"] + st["missing"] + st["blocked"],
                f"Bank ${b:02X} does not reconcile: routines={st['routines']} != v({st['verified']})+d({st['decompiled']})+m({st['missing']})+b({st['blocked']})"
            )
            total_r += st["routines"]
            total_v += st["verified"]
            total_d += st["decompiled"]
            total_m += st["missing"]
            total_b += st["blocked"]

        self.assertEqual(total_r, total_v + total_d + total_m + total_b)
        self.assertEqual(total_r, 4298)
        self.assertEqual(total_v, 906)
        self.assertEqual(total_d, 98)
        self.assertEqual(total_m, 3294)
        self.assertEqual(total_b, 0)

    def test_subsystem_scope_counts(self):
        """Verify Banks 00-03 Subsystem scope counts and Bank 3 completeness."""
        b3_routines = [s for s in self.reconciled if s["bank"] == 3 and s["type"] == "ROUTINE"]
        b3_missing = [s for s in b3_routines if s.get("status") == "MISSING"]
        b3_verified = [s for s in b3_routines if s.get("status") == "VERIFIED"]
        b3_decompiled = [s for s in b3_routines if s.get("status") == "DECOMPILED"]

        self.assertEqual(len(b3_routines), 255)
        self.assertEqual(len(b3_verified), 225)
        self.assertEqual(len(b3_decompiled), 30)
        self.assertEqual(len(b3_missing), 0, "Bank 3 must have 0 missing routines")

        sub_routines = [s for s in self.reconciled if s["bank"] in (0, 1, 2, 3) and s["type"] == "ROUTINE"]
        sub_missing = [s for s in sub_routines if s.get("status") == "MISSING"]
        sub_verified = [s for s in sub_routines if s.get("status") == "VERIFIED"]
        sub_decompiled = [s for s in sub_routines if s.get("status") == "DECOMPILED"]

        self.assertEqual(len(sub_routines), 1126)
        self.assertEqual(len(sub_verified), 904)
        self.assertEqual(len(sub_decompiled), 97)
        self.assertEqual(len(sub_missing), 125)


if __name__ == "__main__":
    unittest.main()
