# Chip-8 Emulator and Debugger

CHIP-8 emulator "ChippyDbg" with an interactive debugger built in using ImGui.

## Features (planned)
- Full CHIP-8 state and instruction set implementation
- Interactive debugger with:
  - CPU state visualisation
  - Memory viewer
  - Dissasembly view
  - Live display rendering
  - Virtual keyboard
- Clean two-layer architecture (Core+UI)

## Building
```bash
mkdir build
cd build
cmake ..
make
```

## Running
```bash
./chip8_debugger path/to/rom.ch8
```

## Architecture
This project follows a two-layer architecture:
- **Core Layer** (`chip8.c/h`): Pure emulation logic
- **UI Layer** (`chip8_ui.c/h`): ImGui-based visualization

## Progress
- [ ] Core emulator implementation
- [ ] ImGui UI wrapper
- [ ] Input handling
- [ ] Debugging features
- [ ] Documentation

## License
MIT License (see LICENSE)
