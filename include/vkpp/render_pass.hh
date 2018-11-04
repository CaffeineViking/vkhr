#ifndef VKPP_RENDER_PASS_HH
#define VKPP_RENDER_PASS_HH

#include <vkpp/device.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class RenderPass final {
    public:
        struct Attachment {
            VkFormat format; // Only parameter that is prob. unkown.
            VkSampleCountFlagBits samples { VK_SAMPLE_COUNT_1_BIT };
            VkImageLayout final_layout { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
            VkAttachmentLoadOp load_operation { VK_ATTACHMENT_LOAD_OP_CLEAR };
            VkAttachmentStoreOp store_operation { VK_ATTACHMENT_STORE_OP_STORE };
            VkImageLayout initial_layout { VK_IMAGE_LAYOUT_UNDEFINED };
        };

        struct Subpass {
            std::vector<VkAttachmentReference> color_attachments { };
            bool has_depth_attachment { false };
            VkAttachmentReference depth_attachment {  };
            std::vector<VkAttachmentReference> input_attachments { };
        };

        using Dependency = VkSubpassDependency;

        RenderPass() = default;
        RenderPass(Device& device,
                   const std::vector<Attachment>& attachments,
                   const std::vector<Subpass>& subpasses,
                   const std::vector<Dependency>& dependencies = {});

        // Only a single subpass!
        RenderPass(Device& device,
                   const std::vector<Attachment>& attachments,
                   Subpass& subpass);

        ~RenderPass() noexcept;

        RenderPass(RenderPass&& render_pass) noexcept;
        RenderPass& operator=(RenderPass&& render_pass) noexcept;

        friend void swap(RenderPass& lhs, RenderPass& rhs);

        VkRenderPass& get_handle();

        const std::vector<VkSubpassDescription>& get_subpasses() const;
        const std::vector<VkSubpassDependency>& get_subpass_dependencies() const;
        const std::vector<VkAttachmentDescription>& get_attachments() const;

    private:
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription>    subpasses;
        std::vector<VkSubpassDependency>     dependencies;

        VkDevice     device { VK_NULL_HANDLE };
        VkRenderPass handle { VK_NULL_HANDLE };
    };
}

#endif
