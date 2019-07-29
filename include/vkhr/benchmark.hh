#ifndef VKHR_BENCHMARK_HH
#define VKHR_BENCHMARK_HH

#include <vkhr/rasterizer.hh>

namespace vkhr {
    class Benchmark final {
    public:
        static void construct(Rasterizer& rasterizer);
    };

    void Benchmark::construct(Rasterizer& rasterizer) {
        vkhr::Rasterizer::Benchmark default_parameter {
            "Benchmark Scenario", // Description
            SCENE("ponytail.vkhr"), // Scene
            1280, 720, // Resolution
            vkhr::Renderer::Rasterizer, // Renderer
            226, // Distance to hair
            1.0, // Strand "reduction" ratio
            512 // Raymarching steps
        };

        rasterizer.append_benchmark({ "Time (ms)", SCENE("ponytail.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Rasterizer, 226, default_parameter.strand_reduction, default_parameter.raymarch_steps });

        for (float distance { 200.0f }; distance < 2000.0f; distance += 1800.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Distance",
                                          SCENE("ponytail.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Rasterizer,
                                          distance,
                                          default_parameter.strand_reduction,
                                          default_parameter.raymarch_steps });
        }

        for (float strands { 1.0f }; strands > 0.0f; strands -= 1.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Strands",
                                          SCENE("ponytail.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Rasterizer,
                                          226,
                                          strands,
                                          default_parameter.raymarch_steps });
        }

        rasterizer.append_benchmark({ "Time (ms)", SCENE("ponytail.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Raymarcher, 226, default_parameter.strand_reduction, default_parameter.raymarch_steps });

        for (float distance { 200.0f }; distance < 2000.0f; distance += 1800.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Distance",
                                          SCENE("ponytail.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Raymarcher,
                                          distance,
                                          default_parameter.strand_reduction,
                                          default_parameter.raymarch_steps });
        }

        for (int samples { 512 }; samples >= 64; samples -= 448 / 64) {
            rasterizer.append_benchmark({ "Time (ms) vs. Samples",
                                          SCENE("ponytail.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Raymarcher,
                                          226,
                                          default_parameter.strand_reduction,
                                          samples });
        }

        for (float strands { 1.0f }; strands > 0.0f; strands -= 1.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Strands",
                                          SCENE("ponytail.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Raymarcher,
                                          226,
                                          strands,
                                          default_parameter.raymarch_steps });
        }

        rasterizer.append_benchmark({ "Time (ms)", SCENE("bear.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Rasterizer, 385, default_parameter.strand_reduction, default_parameter.raymarch_steps });

        for (float distance { 300.0f }; distance < 3000.0f; distance += 2700.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Distance",
                                          SCENE("bear.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Rasterizer,
                                          distance,
                                          default_parameter.strand_reduction,
                                          default_parameter.raymarch_steps });
        }

        for (float strands { 1.0f }; strands > 0.0f; strands -= 1.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Strands",
                                          SCENE("bear.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Rasterizer,
                                          385,
                                          strands,
                                          default_parameter.raymarch_steps });
        }

        rasterizer.append_benchmark({ "Time (ms)", SCENE("bear.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Raymarcher, 385, default_parameter.strand_reduction, default_parameter.raymarch_steps });

        for (float distance { 300.0f }; distance < 3000.0f; distance += 2700.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Distance",
                                          SCENE("bear.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Raymarcher,
                                          distance,
                                          default_parameter.strand_reduction,
                                          default_parameter.raymarch_steps });
        }

        for (int samples { 512 }; samples >= 64; samples -= 448 / 64) {
            rasterizer.append_benchmark({ "Time (ms) vs. Samples",
                                          SCENE("bear.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Raymarcher,
                                          385,
                                          default_parameter.strand_reduction,
                                          samples });
        }

        for (float strands { 1.0f }; strands > 0.0f; strands -= 1.0f / 64.0f) {
            rasterizer.append_benchmark({ "Time (ms) vs. Strands",
                                          SCENE("bear.vkhr"),
                                          default_parameter.width,
                                          default_parameter.height,
                                          vkhr::Renderer::Raymarcher,
                                          385,
                                          strands,
                                          default_parameter.raymarch_steps });
        }
    }
}

#endif
