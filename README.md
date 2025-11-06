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
build instructions etc. will therefore be Linux-oriented to begin with.

## Current status
Eldr is in early stage development and I am still working on the Vulkan backend.
I knew Vulkan would be overkill for a project like this but I want to learn it
so it will take the time it takes.

## Requirements
For the most up-to-date list of required dependencies, see `shell.nix` in the
project root. Many dependencies are provided via Meson wraps, but build tools
and certain libraries are expected to be installed in advance.

Some other notes:
- For initial testing, I've been using the [Viking room](https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38)
model by [nigelgoh](https://sketchfab.com/nigelgoh) ([CC BY 4.0](https://web.archive.org/web/20200428202538/https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38))
At the moment of writing, there are still hardcoded references to these files,
and they will probably stay until I'm more finished with the vulkan backend.
The files can also be downloaded from the the [Vulkan Tutorial](https://vulkan-tutorial.com/Loading_models).

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
