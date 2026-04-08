# OpenGL Learning Test W.I.P. (C++ / OpenGL / GLFW / GLEW / glm)

Project to learn OpenGL, written in c++ rendered with OpenGL libraries



## Features
- **C++20 Standard**
- **OpenGL** for graphic and input handling.
- **CPM.cmake**: Automatic dependency management (no manual installation required).

## Controls
- [W,A,S,D] to move
- [CTRL] to run
- [SPACE] to jump
- [LEFT CLICK] to break block
- [RIGHT CLICK] to place block
- [1,2...9] to select block type

## Build & Run

### Prerequisites
- C++20 compatible compiler
- CMake 3.20+

### Instructions (Terminal)
```bash
git clone https://github.com/zekgio/OpenGL3DGame.git
cd OpenGL3DGame
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/OpenGL3DGame.exe  (or .\out\build\x64-debug\OpenGL3DGame.exe if on Windows)
```

### Credits
- Inspired by Minecraft