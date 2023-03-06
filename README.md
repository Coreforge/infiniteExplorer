# infiniteExplorer
A GUI frontend for libInfinite

libInfinite is based on HIMU by MontagueM (https://github.com/MontagueM/HaloInfiniteModuleUnpacker).

Making the contents of the module available by mounting them requires FUSE3 on linux or [WinFSP](https://winfsp.dev/rel) on windows. This feature is optional though.

# Credits

[HIMU](https://github.com/MontagueM/HaloInfiniteModuleUnpacker) and [HIME](https://github.com/MontagueM/HaloInfiniteModelExtractor) by MontagueM   
detex by Harm Hanemaaijer and Contributors (see [here](https://github.com/hglm/detex/blob/master/LICENSE) for the license)  
[stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h) used for texture export, see source for authors

# Download
## Stable
The releases on github should be somewhat stable. 
To use them, download and unpack the archive and run the executable. The windows releases contain all required libraries, on linux, GTKmm3 has to be installed (at least version 3.24).
## Nightly
Nightly builds are currently only available for windows (if you'd like nighly builds for linux as well, please open an issue and I can add a job for linux too).

You can download the latest nightly build [here](https://nightly.link/Coreforge/infiniteExplorer/workflows/cmake/master/infiniteExplorer.zip) with or [here](https://nightly.link/Coreforge/infiniteExplorer/workflows/cmake/master/infiniteExplorer_noFUSE.zip) without fuse.

To run a nightly build, download the latest release, unpack it, and replace infiniteExplorer.exe with the exe from the nightly build. 
<br>Resource files likely won't be added very often as most things are done in code, including UI layout, but if the program crashes with an error saying some file could not be opened, download the missing file from this repository and place it in the location where the program is looking for it. 

# Building
## Linux
Building for linux is rather simple. 
Dependencies are 
- gtkmm-3.0-dev
- [linoodle](https://github.com/McSimp/linoodle) liblinoodle.so needs to be copied into libInfinite/, as does oo2core_8_win64.dll
- libfuse3-dev (optional)

to compile, run
```
mkdir build
cd build
cmake ..
make
```

## Windows
InfiniteExplorer can be built on Windows using mingw64. Since at least GTK3.24 is needed, not all mingw64 packages will work, as some only provide older versions (Cygwin only has 3.22, which is too old). I recommend MSYS2.

The packages which have to be installed from MSYS2 are
```
mingw-w64-x86_64-gcc
mingw-w64-x86_64-gtkmm3
mingw-w64-x86_64-cmake
```

The build process is like it is on linux, except that oodle has to be specified manually, and pkg-config doesn't work as nicely for WinFSP
```
mkdir build
cd build
env PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/c/Program Files (x86)/WinFSP/lib" cmake -DOODLE_LIBRARY=<path/to/oodle.dll> ..
make
```

Use `cmake -DOODLE_LIBRARY=<path/to/oodle.dll> .. -DFUSE=OFF` to build without fuse.

You can run the resulting infiniteExplorer.exe from within the mingw64 shell, but if you want to use it outside of the mingw64 environment, you'll need to copy the required DLLs into the folder that the exe is in (I don't know for certain which ones are needed, I just copied all except the ones that didn't look like they were needed, so there are probably some unneeded ones in the releases).
The gdk-pixbuf loaders have to be copied from the lib/ folder into a lib/ folder in the same directory as infiniteExplorer.exe, and the GSettings schema and icons have to be copied into a share/ folder.
The libraries, pixbuf-loaders, GSettings schema and icon theme can just be copied from the mingw64 root.
