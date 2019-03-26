#ifndef VKHR_INTERFACE_HH
#define VKHR_INTERFACE_HH

#include <vkhr/window.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/ray_tracer.hh>

#include <vkpp/instance.hh>
#include <vkpp/device.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/render_pass.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/query.hh>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace vkhr {
    class Rasterizer;
    class Interface final {
    public:
        Interface() = default;

        Interface(Window& w, Rasterizer* rasterizer);

        Interface(Interface&& interface) noexcept;
        Interface& operator=(Interface&& interface) noexcept;
        friend void swap(Interface& lhs, Interface& rhs);

        ~Interface() noexcept;

        void make_custom_style(ImGuiStyle&);

        void load(vkhr::Rasterizer& vulkan_renderer);
        void transform(SceneGraph& scene_graph, Rasterizer& rasterizer, Raytracer& raytracer);
        void draw(vkpp::CommandBuffer& command_list);

        enum SamplingMethod : int {
            Uniform = 0,
            Poisson = 1
        };

        enum ShadingModel : int {
            KajiyaKay = 0,
            Marschner = 1,
            Occlusion = 2,
            Voxelizer = 3
        };

        enum ShadowTechnique : int {
            ConventionalShadowMaps = 0,
            ApproximateDeepShadows = 1
        };

        struct Parameters {
            int shading_model;

            int adsm_kernel_size;
            SamplingMethod adsm_sampling_type;
            int adsm_stride_size;
            int adsm_on;

            int ctsm_kernel_size;
            SamplingMethod ctsm_sampling_type;
            float ctsm_bias;
            int ctsm_on;

            ShadowTechnique shadow_technique;

            float isosurface;
            float raycast_steps;
            float occlusion_radius;
            float ao_exponent;
            float ao_clamp;

            float lod_magnified_distance;
            Renderer::Type renderer;
            float lod_minified_distance;

            int benchmarking;
        } parameters {
            KajiyaKay,

            3,
            Uniform,
            3,
            true,

            5,
            Poisson,
            0.00005,
            true,

            ApproximateDeepShadows,

            0.075,
            1024,
            2.50f,
            10.0f,
            0.150,

            400.0,
            Renderer::Type::Rasterizer,
            800.0,

            false
        };

        void default_parameters();

        bool wants_focus() const;
        bool typing_text() const;

        bool hide();
        void toggle_visibility();
        void set_visibility(bool visible);
        bool show();

        void switch_scene(const std::string& scene_name, SceneGraph& scene_graph, Rasterizer& rasterizer);
        void switch_scene(SceneGraph& scene_graph,
                          Rasterizer& rasterizer,
                          Raytracer& ray_tracer);
        void set_sample_size(int raymarch_steps);

        bool rasterizer_enabled(float level_of_detail);
        bool raytracing_enabled();
        void toggle_light_rotation();
        bool raymarcher_enabled(float level_of_detail);
        void toggle_renderer();
        void make_current_renderer(Renderer::Type ren);

        void export_performance();

        std::string get_performance(const std::string& relevant_performance_parameters = "");
        std::string get_performance_header();

        int get_profile_limit() const;

        void record_performance(const std::unordered_map<std::string, float>& timestamps);

    private:
        void traverse(SceneGraph& scene_graph, Rasterizer& rasterizer, Raytracer& ray_tracer);
        void traverse(SceneGraph::Node* node,  Rasterizer& rasterizer, Raytracer& ray_tracer);

        int scene_file { 0 };
        int previous_scene_file { 0 };

        std::vector<std::string> scene_files;
        std::vector<std::string> renderers;
        std::vector<std::string> simulations;
        std::vector<std::string> shaders;
        std::vector<std::string> shadow_maps;
        std::vector<std::string> shadow_samplers;

        int simulation_effect { 0 };

        Renderer::Type current_renderer  { Renderer::Rasterizer };
        Renderer::Type previous_renderer { Renderer::Raymarcher };

        struct ProfilePair {
            std::vector<float> timestamps;
            int offset;
            std::string output { "" };
        };

        int profile_limit { 60 }; // Might change at run-time.
        std::unordered_map<std::string, ProfilePair> profiles;
        std::vector<std::string> export_profiles {
            "Bake Shadow Maps",
            "Voxelize Strands",
            "Clear PPLL Nodes",
            "Draw Mesh Models",
            "Draw Hair Styles",
            "Raymarch Strands",
            "Resolve the PPLL"
        };

        static bool get_string_from_vector(void*, int, const char**);

        bool light_debugger { false };
        bool light_rotation { false };

        bool gui_visible { true };

        ImGuiContext* ctx { nullptr };
    };
}

#endif
