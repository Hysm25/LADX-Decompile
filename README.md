# LADX-Decompile

C/C++ decompilation of **The Legend of Zelda: Link's Awakening DX** (Game Boy Color), aiming for a 1:1 behavior-accurate, readable, and verifiable modern codebase.

## Project Structure

- `include/`: Header files, hardware constants, memory map addresses, and function declarations.
  - `constants/`: Game Boy hardware registers, memory maps, gameplay enums, and graphics definitions.
  - `home/`: Headers for bank 0 / home bank routines.
  - `bank1/`, `bank2/`: Public headers grouped by subsystem, matching the source filenames. `bank2/bank2.h` remains a compatibility umbrella.
- `src/`: Decompiled C source implementations.
  - `gb.c`: Game Boy hardware, memory mapping, and DMA emulation state.
  - `home/`: Bank 0 routines grouped by subsystem (memory, banking, graphics, entities, Link, rooms, audio, and UI).
  - `bank1/`: Save/file menus, room transitions, world handling/map, and intro/cutscene subsystems.
  - `bank2/`: Link motion/animation, items, ocarina, shovel, swimming, falling, revolving doors, room events/transitions, audio, and transient VFX.
- `tests/`: Comprehensive unit and verification test suite validating memory alterations byte-for-byte against original assembly behavior.
  - `bank1/`, `bank2/`: Subsystem tests, called in the original order by `test_bank1.c` and `test_bank2.c`. Shared Bank 2 mocks live in `bank2/test_support.c`.
- `LADX-Disassembly/`: Git submodule pointing to the source of truth disassembly ([zladx/LADX-Disassembly](https://github.com/zladx/LADX-Disassembly)).
- `DECOMPILATION_PROGRESS.md`: Detailed tracking document of decompiled functions, verification status, and technical findings.

## Building and Testing

### Prerequisites

- CMake 3.16 or newer
- C11 compatible compiler (e.g. GCC, Clang)
- Git

### Build Instructions

```bash
# Clone with submodules
git clone --recursive git@github.com:Hysm111/LADX-Decompile.git
cd LADX-Decompile

# Configure and build with assertions enabled (Debug)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run the full unit verification suite
ctest --test-dir build --output-on-failure
# Or run all test suites in parallel
ctest --test-dir build -j4 --output-on-failure

# Run the unified test runner directly
./build/ladx_tests

# Run focused tests via unified runner CLI filtering
./build/ladx_tests home    # Bank 0 / Core Engine tests
./build/ladx_tests bank1   # Bank 1 tests
./build/ladx_tests bank2   # Bank 2 tests
./build/ladx_tests bank3   # Bank 3 tests

# Build and run standalone focused test targets directly
cmake --build build --target ladx_test_home  && ./build/ladx_test_home
cmake --build build --target ladx_test_bank1 && ./build/ladx_test_bank1
cmake --build build --target ladx_test_bank2 && ./build/ladx_test_bank2
cmake --build build --target ladx_test_bank3 && ./build/ladx_test_bank3

# Production-only build (excludes all test compilation and test dependencies)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF
cmake --build build --target ladx_core
```

Use Debug builds for verification: many tests use standard C `assert()`, which is disabled by `NDEBUG` in normal Release builds. Some legacy suites report failures only in their output, so inspect the individual results as well as the process exit status.

## Code Organization

- Keep related routines and their lookup tables in matching `src/<bank>/<subsystem>.c` and `include/<bank>/<subsystem>.h` files. Keep private helpers local; declare shared functions and tables in their owning header.
- Include the source file's own header first, followed by dependencies at the top of the file. Prefer subsystem headers over the Bank 2 compatibility umbrella for new code.
- Use guarded headers (`LADX_<BANK>_<SUBSYSTEM>_H`) with all declarations and constants inside the guard. Use four-space indentation and the existing same-line opening-brace style for new C code.
- Preserve assembly-derived function/table names, address comments, and verification notes. Organizational changes must not rewrite decompiled behavior or remove verified routines.
- Register new source and test files explicitly in `CMakeLists.txt`; keep lists grouped by bank and ordered by filename. Preserve test execution order in the runners.

## Progress and Verification

Progress is tracked function-by-function with strict verification against original assembly logic. See [DECOMPILATION_PROGRESS.md](DECOMPILATION_PROGRESS.md) for details.
