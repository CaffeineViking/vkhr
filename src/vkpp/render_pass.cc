#include <vkpp/render_pass.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    RenderPass::RenderPass(Device& logical_device,
                           const std::vector<Attachment>& attachments,
                           const std::vector<Subpass>& subpasses,
                           const std::vector<Dependency>& dependencies)
                          : device { logical_device.get_handle() } {
        VkRenderPassCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.attachmentCount = attachments.size();

        this->attachments.reserve(attachments.size());

        for (const auto& attachment : attachments) {
            this->attachments.push_back({
                0,
                attachment.format,
                attachment.samples,
                attachment.load_operation,
                attachment.store_operation,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                attachment.initial_layout,
                attachment.final_layout
            });
        }

        if (this->attachments.size() != 0) {
            create_info.pAttachments = this->attachments.data();
        } else {
            create_info.pAttachments = nullptr;
        }

        create_info.subpassCount = subpasses.size();

        this->subpasses.resize(subpasses.size());

        for (std::size_t i { 0 }; i < subpasses.size(); ++i) {
            const auto& subpass = subpasses[i];

            this->subpasses[i].flags = 0;
            this->subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            this->subpasses[i].inputAttachmentCount = subpass.input_attachments.size();

            if (subpass.input_attachments.size() != 0) {
                this->subpasses[i].pInputAttachments = subpass.input_attachments.data();
            } else {
                this->subpasses[i].pInputAttachments = nullptr;
            }

            this->subpasses[i].colorAttachmentCount = subpass.color_attachments.size();

            if (subpass.color_attachments.size() != 0) {
                this->subpasses[i].pColorAttachments = subpass.color_attachments.data();
            } else {
                this->subpasses[i].pColorAttachments = nullptr;
            }

            this->subpasses[i].pResolveAttachments = nullptr;

            if (subpass.depth_attachment.first) {
                const auto& depth = &subpass.depth_attachment.second;
                this->subpasses[i].pDepthStencilAttachment = depth;
            } else {
                this->subpasses[i].pDepthStencilAttachment = nullptr;
            }

            this->subpasses[i].preserveAttachmentCount = 0;
            this->subpasses[i].pPreserveAttachments = nullptr;
        }

        if (this->subpasses.size() != 0) {
            create_info.pSubpasses = this->subpasses.data();
        } else {
            create_info.pSubpasses = nullptr;
        }

        create_info.dependencyCount = this->dependencies.size();

        this->dependencies.reserve(dependencies.size());

        for (const auto& dependency : dependencies) {
            this->dependencies.push_back({
                dependency.source_subpass,
                dependency.destination_subpass,
                dependency.source_stages,
                dependency.destination_stages,
                dependency.source_mask,
                dependency.destination_mask,
                0
            });
        }

        if (this->dependencies.size() != 0) {
            create_info.pDependencies = this->dependencies.data();
        } else {
            create_info.pDependencies = nullptr;
        }

        if (VkResult error = vkCreateRenderPass(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create the render pass!" };
        }
    }

    RenderPass::RenderPass(Device& device,
                           const std::vector<Attachment>& attachments,
                           Subpass& subpass)
                          : RenderPass { device, attachments,
                            std::vector<Subpass> { subpass } } {  }

    RenderPass::RenderPass(Device& device,
                           const std::vector<Attachment>& attachments,
                           Subpass& subpass, Dependency& dependency)
                          : RenderPass { device, attachments,
                            std::vector<Subpass> { subpass },
                            std::vector<Dependency> { dependency } } {  }

    RenderPass::RenderPass(Device& device, Attachment& attachment,
                           Subpass& subpass, Dependency& dependency)
                          : RenderPass { device,
                            std::vector<Attachment> { attachment },
                            std::vector<Subpass> { subpass },
                            std::vector<Dependency> { dependency } } {  }

    RenderPass::~RenderPass() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, handle, nullptr);
        }
    }

    RenderPass::RenderPass(RenderPass&& render_pass) noexcept {
        swap(*this, render_pass);
    }

    RenderPass& RenderPass::operator=(RenderPass&& render_pass) noexcept {
        swap(*this, render_pass);
        return *this;
    }

    void swap(RenderPass& lhs, RenderPass& rhs) {
        using std::swap;

        swap(lhs.attachments,  rhs.attachments);
        swap(lhs.subpasses,    rhs.subpasses);
        swap(lhs.dependencies, rhs.dependencies);

        swap(lhs.device, rhs.device);
        swap(lhs.handle, rhs.handle);

        rhs.device = VK_NULL_HANDLE;
        rhs.handle = VK_NULL_HANDLE;
    }

    VkRenderPass& RenderPass::get_handle() {
        return handle;
    }

    const std::vector<VkSubpassDescription>& RenderPass::get_subpasses() const {
        return subpasses;
    }

    const std::vector<VkAttachmentDescription>& RenderPass::get_attachments() const {
        return attachments;
    }

    const std::vector<VkSubpassDependency>& RenderPass::get_subpass_dependencies() const {
        return dependencies;
    }
}
