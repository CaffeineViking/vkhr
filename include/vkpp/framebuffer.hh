#ifndef VKPP_FRAMEBUFFER_HH
#define VKPP_FRAMEBUFFER_HH

#include <vkpp/image.hh>
#include <vkpp/render_pass.hh>

#include <vector>

#include <cstdint>

namespace vkpp {
    class Device;
    class Framebuffer final {
    public:
        Framebuffer() = default;

        Framebuffer(Device& device,
                    RenderPass& render_pass,
                    std::vector<ImageView>& attachments,
                    const VkExtent2D& extent);

        Framebuffer(VkDevice& device,
                    RenderPass& render_pass,
                    std::vector<ImageView>& attachments,
                    const VkExtent2D& extent);

        Framebuffer(VkDevice& device,
                    RenderPass& render_pass,
                    ImageView& attachment,
                    const VkExtent2D& extent);

        Framebuffer(VkDevice& device,
                    RenderPass& render_pass,
                    ImageView& color_attachment,
                    ImageView& depth_attachment,
                    const VkExtent2D& extent);

        ~Framebuffer() noexcept;

        Framebuffer(Framebuffer&& device) noexcept;
        Framebuffer& operator=(Framebuffer&& device) noexcept;

        friend void swap(Framebuffer& lhs, Framebuffer& rhs);

        VkFramebuffer& get_handle();

        VkRenderPass& get_current_render_pass();

        const std::vector<VkImageView>& get_attachments() const;

        const VkExtent2D& get_extent() const;

    private:
        VkFramebufferCreateInfo partially_create_info();

        std::vector<VkImageView> image_views;

        VkExtent2D extent { 0, 0 };

        VkRenderPass  render_pass { VK_NULL_HANDLE };
        VkDevice      device      { VK_NULL_HANDLE };
        VkFramebuffer handle      { VK_NULL_HANDLE };
    };
}

#endif
