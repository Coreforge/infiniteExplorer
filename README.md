# infiniteExplorer
A GUI frontend for libInfinite

libInfinite is based on HIMU by MontagueM (https://github.com/MontagueM/HaloInfiniteModuleUnpacker).


# Building
## Linux
Building for linux is rather simple. 
Dependencies are 
- gtkmm-3.0-dev
- (linoodle)[https://github.com/McSimp/linoodle] liblinoodle.so needs to be copied into libInfinite/, as does oo2core_8_win64.dll

to compile, run
```
mkdir build
cd build
cmake ..
make
```

## Windows
Building for windows is a bit more complicated, and I haven't been able to get it to fully work yet on windows.
Since I haven't been able to find a new enough version of gtkmm3 (at least 3.22) for msvc, I only got a fully functional build with mingw64.

First install mingw64, and then install mingw-w64-x86_64-gtkmm3

Ideally, the rest should be the same as on linux, except that oodle has to be specified manually
```
mkdir build
cd build
cmake -DOODLE_LIBRARY=<path/to/oodle.dll> ..
make
```
I have not been able to get this to work yet though, so I had to cross-compile from linux, using quasi-msys2 to download gtkmm3 and its dependencies,
and then use a cmake toolchain file to manually point cmake to gtkmm and its dependencies. The DLLs needed to run infiniteExplorer also have to be copied manually
(I just copied all from the lib folder from quasi-msys2, as I don't have other packages installed there), the gdk-pixbuf loaders have to be copied from the lib/ folder
into a lib/ folder in the same directory as infiniteExplorer.exe, and the GSettings schema and icons have to be copied into a share/ folder.

The code should be compatible with msvc though, so with a new enough version of gtkmm3 for msvc, it should compile and run successfully as well.
