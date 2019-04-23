Real-Time Hybrid Hair Renderer in Vulkan™
=========================================

<p align="center">
    <img src="/share/images/screenshots/ponytail.jpg"/>
</p>

<p align="center">
    <img src="/share/images/screenshots/ponytail-hair.jpg"/>
</p>

<p align="center">
    <img src="/share/images/screenshots/hybrid.jpg"/>
</p>

<p align="center">
    <img src="/share/images/screenshots/bear.jpg"/>
</p>

<p align="center">
    <img src="/share/images/screenshots/bear-fur.jpg"/>
</p>

Table of Contents
-----------------

* [Introduction](#real-time-hybrid-hair-renderer-in-vulkan)
* [Table of Contents](#table-of-contents)
* [Features](#features)
* [Benchmark](#benchmark)
* [Dependencies](#dependencies)
* [Compiling](#compiling)
* [System Requirements](#system-requirements)
* [Usage](#usage)
* [Documentation](#documentation)
* [Directories](#directories)
* [Reporting Bugs](#reporting-bugs)
* [Acknowledgements](#acknowledgements)
* [Legal Notice](#legal-notice)

Features
--------

Benchmark
---------

<p align="center">
    <img src="/share/images/figures/performance-memory.png"/>
</p>

<p align="center">
    <img src="/share/images/figures/pixels-strands.png"/>
</p>

Dependencies
------------

* `premake5` (pre-build)
* Any Vulkan™ 1.1 SDK
* `glfw3` (tested v3.2.1)
* `embree3` (uses v3.2.4)
* Any C++17 compiler!

All other dependencies are fetched using `git submodules`. They include the following awesome libraries: `g-truc/glm`, `ocurnut/imgui`, `syoyo/tinyobjloader`, `nothings/stb` and `nlohmann/json`. The C++17 Vulkan wrapper: `vkpp` is being developed alongside this project. It will at a later time be split into another repository: [vkpp](https://github.com/CaffeineViking/vkpp), when I have time to do it.

Compiling
---------

1. First, make sure you've changed your current directory to `vkhr`
2. Then do `git submodule update --init --recursive --depth 1`
    * **Description:** fetches submodule dependencies to `foreign`
3. Since we use [premake](https://premake.github.io/), you'll most likely need to fetch it as well:
    * **Tip:** there's pre-generated Visual Studio solutions in `build`
        * if you're happy with that, you can skip the steps below
    * **Unix-like:** just install `premake5` with your package manager
4. Now make sure you have the [glfw3](https://www.glfw.org/) external dependency solved
    * **Unix-like:** just install `glfw` with your package manager too
    * **Visual Studio:** pre-built version is already provided for you!
5. Finally, you'll also need [Embree](https://embree.github.io/) for the hair raytracing back-end:
    * **Unix-like:** just install `embree` using your package managers
    * **Visual Studio:** pre-built version is already provided for you!
6. Generate the `vkhr` project files by targeting your current setup
    * **Visual Studio:** `premake5 vs2017` or my alias `make solution`
        * then open the Visual Studio project in `build/vkhr.sln`
        * you might have to retarget the VS solution to your SDK
    * **GNU Makefiles:** `premake5 gmake` or just call `make all/run`.
7. Build as usual in your platform, and run with `bin/vkhr <scene>`.

### Distribution

**Install:** if you're on Arch Linux it's as simple as running `makepkg -i`.

For Windows just call `make distribute` for a "portable" ZIP archive.

The client then only needs a working Vulkan runtime to start `vkhr`.

System Requirements
-------------------

Platforms *must* support Vulkan™.

It has been tested on these GPUs:

* NVIDIA® GeForce® MX150,
* Radeon™ Pro WX 9100 Graphics,
* Intel® HD Graphics 620.

on Windows 10 and GNU / Linux.

Usage
-----

* `bin/vkhr`: loads the default `vkhr` scene `share/scenes/ponytail.vkhr` with the default render settings.
* `bin/vkhr <settings> <path-to-scene>`: loads the specified  `vkhr` scene, with the given render settings.
* `bin/vkhr --benchmark yes`: runs the default benchmark and saves the profiles to an `benchmarks/` CSV.
* **Default settings:** `--width 1280 --height 720 --fullscreen no --vsync on --benchmark no --ui yes`
* **Shortcuts:** `U` toggles the UI, `S` takes a screenshots, `T` switches between renderers, `L` toggles light rotation on/off, `R` recompiles the shaders by using `glslc` (needs to be set in `$PATH` to work), and `Q` / `ESC` quits the app.
* **Controls:** simply click and drag to rotate the camera, scroll to zoom, use the middle mouse button to pan.
* **UI:** all configuration happens in the ImGUI window that is documented under the `Help` button in the UI.

Documentation
-------------

You're reading part of it! Besides this [readme.md](/readme.md), you'll find that most of the important shaders are nicely documented. Two good examples are [GPAA.glsl](/share/shaders/anti-aliasing/gpaa.glsl) for the line coverage calculations, and [approximate_deep_shadows.glsl](/share/shaders/self-shadows/approximate_deep_shadows.glsl) for the self-shadowing technique. You'll notice that the quality of it varies quite a bit, feel free to open an issue if you sense something isn't clear. I haven't documented the host-side of the implementation yet as that would take too long, and isn't that interesting anyway.

If you want a high-level summary of our technique read [Real-Time Hybrid Hair Rendering](https://eriksvjansson.net/papers/rthhr.pdf), which is a short conference paper on our method (only the pre-print). You'll also find a copy of it here, which you can build by using LaTeX. If you want a more extensive and detailed version of our paper, my thesis [Scalable Strand-Based Hair Rendering](https://eriksvjansson.net/papers/ssbhr.pdf), will soon be available. Both of these also show the difference between our technique and other existing frameworks like TressFX, that only use a rasterizer.

And if you still haven't had enough, I have written a bunch of entries in the [Captain's Log](https://github.com/CaffeineViking/vkhr/wiki/Captain's-Log), that shows the progress log from day 1 to the current version. Besides having a lot of pretty pictures, it shows the problems we encountered, and how we've solved them. This gives a bit more insight into why we have chosen this approach, and not something completely different. Oh right, we also have a short [presentation](https://eriksvjansson.net/others/sshr.pptx) if you don't want to read the paper or thesis, it has everything but in less detail.

Directories
-----------

* `benchmarks`: output from the bundled benchmarks goes in here.
* `bin`: contains the built software and any other accompanying tools.
* `build`: stores intermediate object files and generated GNU Make files.
    * `obj`: has all of the generated object files given under compilation.
    * `Makefile`: automatically generated by executing `premake5 gmake`.
    * `*.make`: program specific make config for augmenting `Makefile`.
    * you'll also find the pre-generated Visual Studio '17 solution here.
* `docs`: any generated documentation for this project is over here.
* `foreign`: external headers and source for libraries and modules.
* `include`: only internal headers from this project should go here.
    * `vkhr`: internal headers for the Vulkan hair renderer project.
    * `vkpp`: headers for a minimal modern C++ Vulkan wrapper.
* `license.md`: please look through this very carefully.
* `premake5.lua`: configuration file for the build system.
* `readme.md`: this file contains information on the project.
* `share`: any extra data that needs to be bundled should go here.
    * `images`: any images on disk that should be used as textures.
    * `models`: the meshes/models/materials to be used in the project.
    * `shaders`: all of the uncompiled shaders should go over here.
    * `scenes`: any sort of scene files (e.g. in json) should go here.
    * `styles`: the hair styles compatible with the Cem Yuksel format.
* `src`: all source code for the project should be located below here.
    * `vkhr`: source code for the Vulkan hair renderer project itself.
    * `vkpp`: full implementation of an Vulkan C++ wrapper (separate).
    * `main.cc`: the primary entry point when generating the binaries.
* `utils`: any sort of helper scripts or similar should be over here.

Reporting Bugs
--------------

There are definitely no known bugs in this software at this time.

This is a proof-of-concept research prototype, and as such, I wouldn't recommend using it for something serious, at least as it is. Also, do not expect this repository to be well maintained, I will not spend too much time with it after the thesis is done.

Still, if you find anything, feel free to open an issue and I'll fix it!

Acknowledgements
----------------

First I would like to thank Matthäus Chajdas, Dominik Baumeister, and Jason Lacroix at AMD for supervising this thesis, and for always guiding me in the right direction. I'd also like to thank the fine folk at LiU for providing feedback and support, in particular, my examinator Ingemar Ragnemalm and Harald Nautsch at ISY and Stefan Gustavson from ITN. I would also like to thank AMD and RTG Game Engineering for their hospitality and friendliness, and for letting me sit in their Munich office.

Legal Notice
------------

<img width=66% src="https://www.khronos.org/assets/images/api_logos/vulkan.svg"/>

Vulkan and the Vulkan logo are registered trademarks of Khronos Group Inc.

All hair styles are courtesy of Cem Yuksel's great [HAIR model files](http://www.cemyuksel.com/research/hairmodels/) repository.

The ponytail and bear hair geometry are from the [TressFX 3.1](https://github.com/GPUOpen-Effects/TressFX/tree/3.1.1) repository, and proper rights have been granted by AMD Inc. to be used in this repository. However, you are *not* allowed to use it outside of this repository! i.e. not the MIT license for it!

The woman model was created by Murat Afshar (also for Cem Yuksel's repo).

Everything in this repository is under the MIT license *except* the assets I've used. Those fall under the license terms of their respective creators. All of the code in this repository is my own, and that you can use however you like (under the [license](/license.md)).

See: [foreign/glfw/COPYING.txt](foreign/glfw/COPYING.txt) plus [foreign/embree/LICENSE.txt](foreign/embree/LICENSE.txt) for licenses.
