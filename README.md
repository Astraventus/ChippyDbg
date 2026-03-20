# ChippyDbg

A CHIP-8 emulator with an integrated debugger, built with a clean separation between the emulation core and the UI layer.

## Features

- Full CHIP-8 instruction set (all 35 opcodes)
- Step-by-step execution and configurable cycle speed
- Disassembly view with PC tracking
- Memory viewer with live PC highlighting
- CPU state inspector (registers, stack, timers)
- Virtual keyboard with live key state display
- ROM loading via native file dialog

Dependencies

GLFW must be installed on your system. Everything else (ImGui, GLAD, tinyfiledialogs) is vendored under `libs/`.

### Unix-like systems

| Platform | Command |
|---|---|
| Ubuntu/Debian | `sudo apt install libglfw3-dev` |
| Fedora | `sudo dnf install glfw-devel` |
| Arch | `sudo pacman -S glfw` |
| macOS | `brew install glfw` |

### Windows

For Windows, building GLFW via CMake is recommended.

If you do not have CMake:
- Install CMake from [cmake.org/download](https://cmake.org/download/)
- Or with winget: `winget install Kitware.CMake`
- Or with Chocolatey: `choco install cmake`

## Building

**Using Make:**
```bash
make
make run
```

**Prerequisite**: [CMake](https://cmake.org/download/) must be installed on your system.

**Using CMake:**
```bash
mkdir build && cd build
cmake ..
make
./chip8dbg path/to/rom.ch8
```

## Running

```bash
./build/chip8dbg optional/path/to/rom.ch8
```

A ROM can also be loaded at runtime via **File → Load ROM** or the Load ROM button in the Controls panel.

## ROM Compatibility

This emulator targets the original COSMAC VIP CHIP-8 specification. ROMs written for other interpreters may behave incorrectly due to differing quirk behavior:

- **Shift (8XY6/8XYE)** — uses VY as source, not VX
- **Load/Store (FX55/FX65)** — increments I after operation
- **Logic ops (8XY1/2/3)** — resets VF after operation
- **Jump (BXNN)** — uses V0 offset, not VX

XO-CHIP ROMs (Octojam entries) are not supported — they require extended opcodes and memory this emulator does not implement.

The [Kripod's Chip-8 ROMS suite](https://github.com/kripod/chip8-roms) are recommended source for compatible ROMs.

## Architecture

The project is split into two independent layers:

**Core (`src/chip8.c`, `include/chip8.h`)**
Pure emulation logic with no platform or rendering dependencies. Exposes a C API for creating/destroying emulator instances, loading ROMs, stepping execution, querying state, and handling input. Can be used or tested independently of any UI.

**UI (`src/chip8_ui.cpp`, `include/chip8_ui.h`)**
ImGui-based debugger frontend. Owns a `Chip8*` instance and drives it each frame. All rendering and input mapping is contained here. Depends on the core layer only through the public C API.

```
ChippyDbg/
├── include/
│   ├── chip8.h          # Core public API
│   └── chip8_ui.h       # UI layer
|   └── chip8_log.h      # Logging (underimplemented for now)
├── src/
│   ├── chip8.c          # Emulator core
│   ├── chip8_log.c      # Logging (underimplemented for now)
│   ├── chip8_ui.cpp     # Debugger UI
│   └── main.cpp         # Entry point
└── libs/
    ├── imgui/           # Dear ImGui
    ├── glad/            # OpenGL loader
    └── tinyfiledialogs/ # Native file dialogs
```

## License

MIT (see LICENSE)
