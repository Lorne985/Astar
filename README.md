# Astar
Astar study and visualizer

## Clone and build

Clone with submodules:

```powershell
git clone --recurse-submodules https://github.com/Lorne985/Astar.git
cd Astar
cmake -S . -B build
cmake --build build --config Debug
```

If the repository was already cloned, restore the dependencies with:

```powershell
git submodule update --init --recursive
```

## clangd

Generate a local compilation database after cloning or changing computers:

```powershell
.\tools\configure-clangd.ps1 -Build
```

The script automatically uses an active MSVC environment, MinGW from `PATH`
or MSYS2, or an installed Visual Studio toolchain. You can override detection
with `-Toolchain MSVC` or `-Toolchain MinGW`.
