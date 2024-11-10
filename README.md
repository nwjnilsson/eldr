# 🔥Eldr: Physically Based Renderer
## Introduction
_Eldr_ ("fire" in Old Norse) is my new hobby project. After working with
[_Mitsuba 2_](https://github.com/mitsuba-renderer/mitsuba2) in my master's
thesis [project](https://github.com/nwjnilsson/PPG-quadtree-reconstruction),
I was inspired to create something similar on my own to learn more about
computer graphics. In contrast to Mitsuba, I intend to make Eldr more GUI
oriented, making it possible to set up scenes as you go, and provide more
immediate visual feedback like most commercial renderers do. However, I want to
keep the "research spirit" of Mitsuba, making it as modular as I can to allow
swapping out components and trying new rendering techniques.

I will try to keep cross platform compatibility in mind while developing Eldr,
but it is not a priority in the beginning. I will be developing on Linux and the
build instructions etc. will therefore be Linux(debian)-oriented to begin with.

## Requirements
The following software is available on Windows as well, so it should be possible
to build there as well. MSVC 2013 is not compatible with
[cxxopts](https://github.com/jarro2783/cxxopts#requirements), however.
```
$ sudo apt-get install build-essential ninja-build meson pkg-config python3 \
    python3-pip python3-setuptools python3-wheel libspdlog-dev libglfw3-dev \
    libglm-dev libvulkan-dev libjpeg62-turbo-dev libpng-dev
```
For debugging:
```
sudo apt-get install vulkan-validationlayers-dev spirv-tools
```

Some other notes:
- [spdlog](https://github.com/gabime/spdlog/tree/v1.13.0) is a header only
library but I have been using a compiled library for faster compilation of Eldr.
- For initial testing, I've been using the [Viking room](https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38)
model by [nigelgoh](https://sketchfab.com/nigelgoh)[CC BY 4.0](https://web.archive.org/web/20200428202538/https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38)


## Build
### App
```
$ meson setup build
$ cd build
$ ninja
```
### Shaders
I have been using [glslc](https://github.com/google/shaderc) to compile some
basic shaders. Eldr will look for the in `assets/shaders/`.
```
glslc src/shaders/main.vert -o assets/shaders/main.vert.spv
glslc src/shaders/main.frag -o assets/shaders/main.frag.spv
```
