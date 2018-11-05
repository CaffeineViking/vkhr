#include <vkpp/framebuffer.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

namespace vkpp {
    Framebuffer::Framebuffer(VkDevice& device,
                             RenderPass& render_pass,
                             std::vector<ImageView>& image_views,
                             const VkExtent2D& extent)
                            : extent { extent },
                              render_pass { render_pass.get_handle() },
                              device { device } {
        auto create_info = partially_create_info();

        create_info.attachmentCount = image_views.size();

        this->image_views.reserve(image_views.size());
        for (auto& image_view : image_views) {
            this->image_views.push_back(image_view.get_handle());
        }

        create_info.pAttachments = this->image_views.data();

        if (VkResult error = vkCreateFramebuffer(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create framebuffer!" };
        }
    }

    Framebuffer::Framebuffer(Device& device,
                             RenderPass& render_pass,
                             std::vector<ImageView>& image_views,
                             const VkExtent2D& extent)
                            : Framebuffer { device.get_handle(), render_pass,
                                            image_views, extent } { }

    Framebuffer::Framebuffer(VkDevice& device,
                             RenderPass& render_pass,
                             ImageView& image_view,
                             const VkExtent2D& extent)
                            : extent { extent },
                              render_pass { render_pass.get_handle() },
                              device { device } {
        auto create_info = partially_create_info();

        create_info.attachmentCount = 1;

        this->image_views.reserve(1);

        this->image_views.push_back(image_view.get_handle());
        create_info.pAttachments = this->image_views.data();

        if (VkResult error = vkCreateFramebuffer(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create framebuffer!" };
        }
    }

    Framebuffer::~Framebuffer() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, handle, nullptr);
        }
    }

    Framebuffer::Framebuffer(Framebuffer&& framebuffer) noexcept {
        swap(*this, framebuffer);
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& framebuffer) noexcept {
        swap(*this, framebuffer);
        return *this;
    }

    void swap(Framebuffer& lhs, Framebuffer& rhs) {
        using std::swap;

        swap(lhs.extent, rhs.extent);

        swap(lhs.image_views, rhs.image_views);

        swap(lhs.handle, rhs.handle);
        swap(lhs.render_pass, rhs.render_pass);
        swap(lhs.device, rhs.device);
    }

    VkFramebuffer& Framebuffer::get_handle() {
        return handle;
    }

    VkRenderPass& Framebuffer::get_current_render_pass() {
        return render_pass;
    }

    const std::vector<VkImageView>& Framebuffer::get_attachments() const {
        return image_views;
    }

    const VkExtent2D& Framebuffer::get_extent() const {
        return extent;
    }

    VkFramebufferCreateInfo Framebuffer::partially_create_info() {
        VkFramebufferCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.renderPass = render_pass;

        create_info.width  = extent.width;
        create_info.height = extent.height;
        create_info.layers = 1;
        return create_info;
    }
}
