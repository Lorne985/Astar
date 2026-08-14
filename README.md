# Astar
Astar study and visualizer

## Clone and build

Clone with submodules:

```powershell
git clone --recurse-submodules https://github.com/Lorne985/Astar.git
cd Astar
.\tools\build.ps1
```

If the repository was already cloned, restore the dependencies with:

```powershell
git submodule update --init --recursive
```

The script uses the installed MSVC toolchain and writes both the executable
and clangd compilation database to `build`.
