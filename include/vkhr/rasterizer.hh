#ifndef VKHR_RASTERIZER_HH
#define VKHR_RASTERIZER_HH

#include <vkhr/renderer.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/hair_style.hh>
#include <vkhr/window.hh>

#include <vkhr/vulkan/pipeline.hh>
#include <vkhr/vulkan/hair_style.hh>
#include <vkhr/vulkan/model.hh>

#include <vkpp/instance.hh>
#include <vkpp/physical_device.hh>
#include <vkpp/device.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/swap_chain.hh>
#include <vkpp/render_pass.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/framebuffer.hh>
#include <vkpp/semaphore.hh>
#include <vkpp/fence.hh>

#include <vector>
#include <unordered_map>
#include <string>

namespace vk = vkpp;

namespace vkhr {
    // Forward declaration for friending.
    namespace vulkan { class HairStyle; }
    namespace embree { class HairStyle; }

    class Rasterizer final : public Renderer {
    public:
        Rasterizer(const Window& window, const SceneGraph& scene_graph);

        void build_pipelines();

        void load(const SceneGraph& scene) override;
        void draw(const SceneGraph& scene) override;

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

        std::vector<vk::CommandBuffer> command_buffers;

        vulkan::Pipeline hair_style_pipeline;

        std::unordered_map<std::string, vulkan::HairStyle> hair_styles;

        friend class vulkan::HairStyle;
    };
}

#endif
