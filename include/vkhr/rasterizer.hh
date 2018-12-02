#ifndef VKHR_RASTERIZER_HH
#define VKHR_RASTERIZER_HH

#include <vkhr/vkhr.hh>

#include <vkhr/vulkan/model.hh>
#include <vkhr/vulkan/hair_style.hh>
#include <vkhr/vulkan/billboard.hh>
#include <vkhr/vulkan/depth_map.hh>
#include <vkhr/vulkan/pipeline.hh>

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

        std::uint32_t fetch_next_frame();

        void draw(const SceneGraph& scene) override;
        void draw(Image& fullscreen_image);

        void render_node(const SceneGraph& scene_graph, MVP& view_projection, vk::CommandBuffer& command_buffer, std::size_t frame);
        void render_node(const SceneGraph::Node* node,  MVP& view_projection, vk::CommandBuffer& command_buffer, std::size_t frame);
        void render_hair(const SceneGraph& scene_graph, MVP& view_projection, vk::CommandBuffer& command_buffer, std::size_t frame);
        void render_hair(const SceneGraph::Node* node,  MVP& view_projection, vk::CommandBuffer& command_buffer, std::size_t frame);

        void recompile();

        Interface& get_imgui();
        Image get_screenshot();

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
        std::vector<vk::Fence> command_buffer_done;

        std::uint32_t frame { 0 };

        std::vector<vk::UniformBuffer> transform;
        std::vector<vk::UniformBuffer> light_data;

        vulkan::Pipeline depth_view_pipeline;
        vulkan::Pipeline billboards_pipeline;
        vulkan::Pipeline hair_style_pipeline;

        vulkan::DepthView shadow_map;
        std::unordered_map<const HairStyle*, vulkan::HairStyle> hair_styles;
        vulkan::Billboard raytraced_image;

        Interface imgui;

        std::vector<vk::CommandBuffer> command_buffers;

        friend class vulkan::HairStyle;
        friend class vulkan::Billboard;
        friend class ::vkhr::Interface;
        friend class vulkan::DepthView;
    };
}

#endif
