#ifndef VKHR_RASTERIZER_HH
#define VKHR_RASTERIZER_HH

#include <vkhr/renderer.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/window.hh>

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

#include <iostream>

namespace vkhr {
    class Rasterizer final : public Renderer {
    public:
        Rasterizer(const Window& window);

        void load(const SceneGraph& scene) override;
        void draw(const SceneGraph& scene) override;

        vkpp::Instance instance;
        vkpp::PhysicalDevice physical_device;
        vkpp::Device device;

        vkpp::CommandPool graphics_pool;

        vkpp::Surface window_surface;
        vkpp::SwapChain swap_chain;
        vkpp::RenderPass default_render_pass();
        vkpp::RenderPass render_pass;

        vkpp::DescriptorPool descriptor_pool;

        std::vector<vkpp::Framebuffer> framebuffers;
        vkpp::Semaphore image_available, render_complete;
        vkpp::Fence command_buffer_done;

        std::vector<vkpp::CommandBuffer> command_buffers;
    };
}

#endif
