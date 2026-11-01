# 🤝 Contributing Guidelines

Contributions that improve binary inspection accuracy, documentation clarity, or defensive telemetry coverage are welcome.

## Engineering Standards
1. **Strict C11**: All C code must compile cleanly with `-std=c11 -O2 -Wall -Wextra -Werror -Wpedantic` using `x86_64-w64-mingw32-clang`.
2. **Win64 ABI Compliance**: Any x86-64 assembly in `src/abi_demo/` must strictly follow the Microsoft x64 Calling Convention (`RCX, RDX, R8, R9`, 32-byte caller shadow space, and preservation of non-volatile registers including `RDI` and `RSI`).
3. **Verification**: Run `make clean all` and `make inspect` before submitting a pull request.
