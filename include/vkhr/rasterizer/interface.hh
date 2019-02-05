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

            int shadow_technique;

            float isosurface;
            float raycast_steps;
            float occlusion_radius;
            float ao_exponent;
            float ao_clamp;
        } parameters {
            KajiyaKay,

            3,
            Poisson,
            4,
            true,

            3,
            Uniform,
            0.00005f,
            true,

            ApproximateDeepShadows,

            0.025,
            512,
            2.50f,
            24.00f,
            0.075f
        };

        void default_parameters();

        bool wants_focus() const;
        bool typing_text() const;

        bool hide();
        void toggle_visibility();
        void set_visibility(bool visible);
        bool show();

        bool rasterizer_enabled();
        bool raytracing_enabled();
        void toggle_light_rotation();
        bool raymarcher_enabled();
        void toggle_renderer();

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
        Renderer::Type previous_renderer { Renderer::Ray_Tracer };

        struct ProfilePair {
            std::vector<float> timestamps;
            int offset;
            std::string output { "" };
        };

        static constexpr int profile_limit { 60 };
        std::unordered_map<std::string, ProfilePair> profiles;

        static bool get_string_from_vector(void*, int, const char**);

        bool light_debugger { false };
        bool light_rotation { false };

        bool gui_visible { true };

        ImGuiContext* ctx { nullptr };
    };
}

#endif
