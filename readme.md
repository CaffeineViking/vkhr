Real-Time Hybrid Hair Renderer in Vulkan™
=========================================

<p align="center"><img src="/docs/figures/ponytail-hair.jpg" alt="Lara Croft's Ponytail"/></p>

<p align="center"><img src="/docs/figures/hybrid.jpg" alt="Hybrid Hair Renderer Example"/></p>

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

A real-time hybrid hair rendering pipeline suitable for video games, that scales in the performance and quality domain. It is:

* Written from scratch in **modern C++17** with minimal dependencies,
* Uses the **[Vulkan™ API](https://www.khronos.org/vulkan/)** with a lightweight wrapper: **[vkpp](https://github.com/CaffeineViking/vkpp)**, written for modern C++17, with proper lifetime management,
* Has a built-in raytracer based on **[Intel's Embree®](https://embree.github.io/)** with a **[CMJ](https://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf)** sampler to compare ground-truth global effects, like AO,
* Loads **[Cem Yuksel's](http://www.cemyuksel.com/research/hairmodels/)** free & open **[.hair file format](http://www.cemyuksel.com/research/hairmodels/)**, and has a easy human-readable **[scene graph format](/share/scenes/ponytail.vkhr)** based on JSON,
* Consists of a strand-based hair **rasterizer** and a volume **raymarcher**.

It uses this rasterized solution for close-up shots, and our raymarched solution for level-of-detail. This hybrid hair renderer:

* Models single **light scattering** in a strand with **[Kajiya-Kay's](http://www.cs.virginia.edu/~mjh7v/bib/Kajiya89.pdf)** shading,
* Estimates hair **self-shadowing** with a fast **[Approximated Deep Shadow Map (ADSM)](developer.amd.com/wordpress/media/2013/05/HairInTombRaider_FMX2013.ppsx)** method à la **[Tomb Raider (2013)](https://www.gdcvault.com/play/1017625/Advanced-Visual-Effects-with-DirectX)**,
* Produces **anti-aliased** strands by using a simple, but effective, line coverage calculation similar to Emil Persson's **[GPAA](www.humus.name/Articles/Persson_GraphicsGemsForGames.pptx)**,
* Resolves strand **transparency** with an fragment **[k-Buffer](http://www-rev.sci.utah.edu/publications/SCITechReports/UUSCI-2006-032.pdf)** **[PPLL](http://developer.amd.com/wordpress/media/2013/06/2041_final.pdf)** similar to **[TressFX's](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.231.5679&rep=rep1&type=pdf)** OIT that builds and sorts on the GPU,
* Has a scalable **level-of-detail** scheme based on volume ray casting.

This novel volumetric approximation for strand-based hair can be found once per-frame for fully simulated hair. It features:

* A very fast compute-based **[strand voxelization](https://arxiv.org/pdf/1801.01155.pdf)** technique for hairs,
* An approximation of **[Kajiya-Kay's](http://www.cs.virginia.edu/~mjh7v/bib/Kajiya89.pdf)** model by finding the **tangents** inside of a volume by **[quantized strand voxelization](https://arxiv.org/pdf/1801.01155.pdf)**,
* An **[ADSM](developer.amd.com/wordpress/media/2013/05/HairInTombRaider_FMX2013.ppsx)** equivalent, that also takes into account the varying hair spacing by using the actual **strand density** as input,
* A way to approximate the **[local ambient occlusion](http://www.diva-portal.org/smash/get/diva2:321233/FULLTEXT01.pdf)** by using the same **strand density** (a useful representation for hair),
* That can also be used to "fake" **transparency** for **low density areas**.

Our hybrid rendering solution combines the best of strand- and volume-based hair representations. Some benefits are that:

* It is **faster** than purely raster-based techniques in the far away case,
* The **performance** is more **predictable and configurable** as raymarching scales **linearly** with the hair's screen coverage,
* The **level-of-detail transition** is quite **smooth** because both the rasterizer and raymarcher **approximate similar effects**,
* The **ambient occlusion** and other **global effects** are **trivial to estimate in a volume**, but not in strand-based renderers,
* It is **automatic** as our voxelization works even with **simulated hairs**.

Benchmark
---------

Along with this project we bundle a set of benchmarks that can be run by passing the `--benchmark yes` flag. They compare the performance between the rasterized and raymarched solutions and how these perf scale (e.g. with respect to increasing distances or strands). In order for you to get an idea if our solution is good enough for your purposes, we have included the results from our paper, which were run on a Radeon™ Pro WX 9100. The results were taken with V-Sync off and without any other GPU intensive programs running in the background. The timing information was taken via Vulkan timestamp queries, and averaged over a period of 60 frames (not much variance). We have plotted the results below for your viewing pleasure.

<p align="center"><img width=96% src="/docs/figures/breakdown.png" alt="Performance and Memory Breakdown"/></p>

In the above plot to the left we see how the rasterizer and raymarcher fare at different distances, and how much time each rendering pass takes. For the near cases (e.g. a character close-up) the raymarched solution is around twice as fast, but the fidelity isn't as good, as strands appear to be clumped together because of the volume approximation. On the other hand, the rasterized solution produces high-quality output as each strand is individually distinguishable. However for far away to medium distances, these small details are not noticable, and the rasterized and raymarched solution are indistinguishable. The raymarcher on the other hand is now 5x faster in these distances! It has better scaling with distance for far away shots.

**Setup:** Rendering at 1280x720, Ponytail Scene, V-Sync Off, 1024x1024 Shadow Maps, 256³ Volume, 512 Raymarching Steps.

The raymarcher also doesn't have to produce shadow maps, which would scale linearly with the number of light sources for the scene. Finally, notice that the strand voxelization is quite cheap and does not account for much of the total render time. In the memory department, the figure to the right shows the GPU data breakdown. When comparing to the original strand-based geometry, the volume does not consume an inordinate amount of memory, and this value can also be tweaked with the volume resolution. The main culpit are the PPLL nodes that are used for our transparency solution. These scale with the resolution and also depend on how many strands are being shaded (and might lead to artifacts if memory underallocated).

<p align="center"><img width=96% src="/docs/figures/scaling.png" alt="Performance Scaling"/></p>

For the two plots above we see how performance scales for each renderer with respect to screen coverage and number of hair strands. The raymarcher has a lower intercept, making it cheap to render for low screen coverage (far away distances). Performance on the rasterizer scales linearly with the number of hair strands (as expected), and also for the raymarcher but with a very slow slope (caused by the voxelization). Our technique works especially well for realistic amounts of hair, where anything less than ~20,000 strands of hair will look bald. While the scaling on the right doesn't look very promising for the raymarcher, its performance can be tuned by changing the number of raymarch steps that moves the intercept up / down.

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

You're reading part of it! Besides this [readme.md](/readme.md), you'll find that most of the important shaders are nicely documented. Two good examples are [GPAA.glsl](/share/shaders/anti-aliasing/gpaa.glsl) for the line coverage calculations, and [approximate_deep_shadows.glsl](/share/shaders/self-shadowing/approximate_deep_shadows.glsl) for the self-shadowing technique. You'll notice that the quality of it varies quite a bit, feel free to open an issue if you sense something isn't clear. I haven't documented the host-side of the implementation yet as that would take too long, and isn't that interesting anyway.

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

`vkhr` is 100% bug-free, anything that seems like a bug is in fact a feature.

This is a proof-of-concept research prototype, and as such, I wouldn't recommend using it for something serious, at least as it is. Also, do not expect this repository to be well maintained, I will not spend too much time with it after the thesis is done.

Still, if you find anything, feel free to open an issue, I'll see what I can do :)

Acknowledgements
----------------

First I would like to thank Matthäus Chajdas, Dominik Baumeister, and Jason Lacroix at AMD for supervising this thesis, and for always guiding me in the right direction. I'd also like to thank the fine folk at LiU for providing feedback and support, in particular, my examinator Ingemar Ragnemalm and Harald Nautsch at ISY and Stefan Gustavson from ITN. I would also like to thank AMD and RTG Game Engineering for their hospitality and friendliness, and for letting me sit in their Munich office.

Legal Notice
------------

<img width=66% src="https://www.khronos.org/assets/images/api_logos/vulkan.svg" alt="The Vulkan Logo"/>

Vulkan and the Vulkan logo are registered trademarks of Khronos Group Inc.

All hair styles are courtesy of Cem Yuksel's great [HAIR model files](http://www.cemyuksel.com/research/hairmodels/) repository.

The ponytail and bear hair geometry are from the [TressFX 3.1](https://github.com/GPUOpen-Effects/TressFX/tree/3.1.1) repository, and proper rights have been granted by AMD Inc. to be used in this repository. However, you are *not* allowed to use it outside of this repository! i.e. not the MIT license for it!

The woman model was created by Murat Afshar (also for Cem Yuksel's repo).

Everything in this repository is under the MIT license *except* the assets I've used. Those fall under the license terms of their respective creators. All of the code in this repository is my own, and that you can use however you like (under the [license](/license.md)).

Both GLFW and Embree are pre-compiled to facilitate building on Windows.

See: [foreign/glfw/COPYING](foreign/glfw/COPYING.txt) and [foreign/embree/LICENSE](foreign/embree/LICENSE.txt) for these licenses.
