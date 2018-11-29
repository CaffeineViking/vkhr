#ifndef VKPP_RENDER_PASS_HH
#define VKPP_RENDER_PASS_HH

#include <vulkan/vulkan.h>

#include <vector>
#include <utility>
#include <cstdint>

namespace vkhr::vulkan { class DepthMap; }

namespace vkpp {
    class Device;
    class SwapChain;

    class RenderPass final {
    public:
        struct Attachment {
            VkFormat format;
            VkImageLayout final_layout   { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
            VkAttachmentStoreOp store_operation { VK_ATTACHMENT_STORE_OP_STORE };
            VkAttachmentLoadOp load_operation   { VK_ATTACHMENT_LOAD_OP_CLEAR };
            VkSampleCountFlagBits samples { VK_SAMPLE_COUNT_1_BIT };
            VkImageLayout initial_layout { VK_IMAGE_LAYOUT_UNDEFINED };
        };

        using Subpass = std::vector<VkAttachmentReference>;

        struct Dependency {
            std::uint32_t source_subpass;
            std::uint32_t destination_subpass;
            VkPipelineStageFlags source_stages;
            VkAccessFlags source_mask;
            VkPipelineStageFlags destination_stages;
            VkAccessFlags destination_mask;
        };

        RenderPass() = default;

        RenderPass(Device& logical_device,
                   const std::vector<Attachment>& attachments,
                   const std::vector<Subpass>& subpasses,
                   const std::vector<Dependency>& dependencies = { });

        RenderPass(Device& logical_device,
                   const std::vector<Attachment>& attachments,
                   Subpass& subpass);
        RenderPass(Device& logical_device,
                   Attachment& attachment,
                   Subpass& subpass);
        RenderPass(Device& logical_device,
                   const std::vector<Attachment>& attachments,
                   Subpass& subpass, Dependency& dependency);
        RenderPass(Device& logical_device, Attachment& attachment,
                   Subpass& subpass, Dependency& dependency);

        ~RenderPass() noexcept;

        RenderPass(RenderPass&& render_pass) noexcept;
        RenderPass& operator=(RenderPass&& render_pass) noexcept;

        friend void swap(RenderPass& lhs, RenderPass& rhs);

        VkRenderPass& get_handle();

        const std::vector<VkSubpassDescription>& get_subpasses() const;
        const std::vector<VkSubpassDependency>& get_subpass_dependencies() const;
        const std::vector<VkAttachmentDescription>& get_attachments() const;

        bool has_depth_attachment() const;

        static void mk_color_pass(RenderPass& color_pass, Device& device, SwapChain& window_chain);
        static void mk_depth_pass(RenderPass& depth_pass, Device& device, vkhr::vulkan::DepthMap&);

    private:
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription>    subpasses;
        std::vector<VkSubpassDependency>     dependencies;

        std::vector<Subpass> subpass_color_references;
        std::vector<Subpass> subpass_depth_references;

        std::int32_t depth_attachment_binding { -1 };

        VkDevice     device { VK_NULL_HANDLE };
        VkRenderPass handle { VK_NULL_HANDLE };
    };
}

#endif
