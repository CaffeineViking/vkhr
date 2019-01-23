#ifndef VKHR_RASTERIZER_HH
#define VKHR_RASTERIZER_HH

#include <vkhr/vkhr.hh>

#include <vkhr/rasterizer/model.hh>
#include <vkhr/rasterizer/hair_style.hh>
#include <vkhr/rasterizer/billboard.hh>
#include <vkhr/rasterizer/volume.hh>

#include <vkhr/rasterizer/depth_map.hh>
#include <vkhr/rasterizer/pipeline.hh>

#include <vkpp/vkpp.hh>

#include <vector>
#include <unordered_map>
#include <string>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer final : public Renderer {
    public:
        Rasterizer(Window& window, const SceneGraph& scene_graph);

        void build_render_passes();
        void recreate_swapchain(Window& window);
        void build_pipelines();

        void load(const SceneGraph& scene) override;
        void update(const SceneGraph& scene_graphs);

        std::uint32_t fetch_next_frame();

        void draw(const SceneGraph& scene) override;
        void draw(Image& fullscreen_image);

        void draw_depth(const SceneGraph& scene_graph, vk::CommandBuffer& command_buffer);
        void draw_model(const SceneGraph& scene_graph, Pipeline& pipeline, vk::CommandBuffer& command_buffer, glm::mat4 = glm::mat4 { 1.0f });
        void draw_color(const SceneGraph& scene_graph, vk::CommandBuffer& command_buffer);
        void draw_hairs(const SceneGraph& scene_graph, Pipeline& pipeline, vk::CommandBuffer& command_buffer, glm::mat4 = glm::mat4 { 1.0f });

        // Direct Volume Render (DVR) the hair strands. This needs to be done after drawing models and styles.
        void strand_dvr(const SceneGraph& scene_graph, Pipeline& pipeline, vk::CommandBuffer& command_buffer);

        bool recompile_pipeline_shaders(Pipeline& pipeline);
        void recompile();

        Interface& get_imgui();
        Image get_screenshot(SceneGraph& scene_graph,
                             Raytracer& ray_tracer);

    private:
        vk::Instance instance;
        vk::PhysicalDevice physical_device;
        vk::Device device;

        vk::CommandPool command_pool;

        vk::Surface window_surface;
        vk::SwapChain swap_chain;

        vk::RenderPass depth_pass;
        vk::RenderPass color_pass;

        vk::DescriptorPool descriptor_pool;

        std::vector<vkpp::Framebuffer> framebuffers;
        std::vector<vk::Semaphore> image_available, render_complete;
        std::vector<vk::Fence> command_buffer_finished;

        std::uint32_t frame { 0 };
        std::uint32_t latest_drawn_frame { 0 };

        std::vector<vk::UniformBuffer> camera;
        std::vector<vk::UniformBuffer> lights;
        std::vector<vk::UniformBuffer> params;

        Pipeline hair_depth_pipeline;
        Pipeline mesh_depth_pipeline;
        Pipeline hair_voxel_pipeline;

        Pipeline strand_dvr_pipeline;

        Pipeline hair_style_pipeline;
        Pipeline model_mesh_pipeline;
        Pipeline billboards_pipeline;

        std::vector<vulkan::DepthMap> shadow_maps;
        std::unordered_map<const HairStyle*, vulkan::HairStyle> hair_styles;
        std::unordered_map<const Model*, vulkan::Model> models;
        vulkan::Billboard fullscreen_billboard;

        Interface imgui;

        std::vector<vk::QueryPool> query_pools;

        std::vector<vk::CommandBuffer> command_buffers;

        friend class vulkan::HairStyle;
        friend class vulkan::Model;
        friend class vulkan::Volume;
        friend class vulkan::Billboard;

        friend class vulkan::DepthMap;

        friend class ::vkhr::Interface;
    };
}

#endif
