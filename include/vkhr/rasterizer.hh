#ifndef VKHR_RASTERIZER_HH
#define VKHR_RASTERIZER_HH

#include <vkhr/vkhr.hh>

#include <vkhr/vulkan/model.hh>
#include <vkhr/vulkan/hair_style.hh>
#include <vkhr/vulkan/billboard.hh>
#include <vkhr/vulkan/pipeline.hh>

#include <vkpp/vkpp.hh>

#include <vector>
#include <unordered_map>
#include <string>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer final : public Renderer {
    public:
        Rasterizer(Window& window, const SceneGraph& scene_graph, bool vsync);

        void build_pipelines();

        void load(const SceneGraph& scene) override;
        void update(const SceneGraph& scene_graphs);
        void draw(const SceneGraph& scene) override;

        void draw(const SceneGraph::Node* hair_node,
                  vk::CommandBuffer& command_buffer,
                  std::size_t fbi, MVP& mvp_matrix);

        Interface& get_imgui();

    private:
        vk::Instance instance;
        vk::PhysicalDevice physical_device;
        vk::Device device;

        vk::CommandPool command_pool;

        vk::Surface window_surface;
        vk::SwapChain swap_chain;
        vk::RenderPass default_render_pass();
        vk::RenderPass render_pass;

        vk::DescriptorPool descriptor_pool;

        std::vector<vkpp::Framebuffer> framebuffers;
        vk::Semaphore image_available, render_complete;
        vk::Fence command_buffer_done;

        vulkan::Pipeline hair_style_pipeline;

        std::unordered_map<const HairStyle*, vulkan::HairStyle> hair_styles;

        Interface imgui;

        std::vector<vk::CommandBuffer> command_buffers;

        friend class vulkan::HairStyle;
        friend class ::vkhr::Interface;
    };
}

#endif
