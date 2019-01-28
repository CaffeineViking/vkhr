#include <vkpp/render_pass.hh>

#include <vkpp/device.hh>
#include <vkpp/swap_chain.hh>
#include <vkpp/debug_marker.hh>
#include <vkpp/exception.hh>

#include <vkhr/rasterizer/depth_map.hh>

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

        create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());

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

        create_info.subpassCount = static_cast<std::uint32_t>(subpasses.size());

        this->subpasses.resize(subpasses.size());

        subpass_color_references.resize(subpasses.size());
        subpass_depth_references.resize(subpasses.size());
        subpass_input_references.resize(subpasses.size());

        for (std::size_t i { 0 }; i < subpasses.size(); ++i) {
            this->subpasses[i].flags = 0;
            this->subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            for (const auto& attachment : subpasses[i]) {
                if (attachment.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                    subpass_input_references[i].push_back(attachment);
                if (attachment.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                    attachment.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                    subpass_color_references[i].push_back(attachment);
                if (attachment.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    attachment.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
                    subpass_depth_references[i].push_back(attachment);
            }

            this->subpasses[i].inputAttachmentCount = static_cast<std::uint32_t>(subpass_input_references[i].size());

            if (subpass_input_references[i].size() != 0) {
                this->subpasses[i].pInputAttachments = subpass_input_references[i].data();
            } else {
                this->subpasses[i].pInputAttachments = nullptr;
            }

            this->subpasses[i].colorAttachmentCount = static_cast<std::uint32_t>(subpass_color_references[i].size());

            if (subpass_color_references[i].size() != 0) {
                this->subpasses[i].pColorAttachments = subpass_color_references[i].data();
            } else {
                this->subpasses[i].pColorAttachments = nullptr;
            }

            this->subpasses[i].pResolveAttachments = nullptr;

            if (subpass_depth_references[i].size() != 0) {
                VkAttachmentReference& depth = subpass_depth_references[i][0];
                depth_attachment_binding = depth.attachment;
                this->subpasses[i].pDepthStencilAttachment = &depth;
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

        create_info.dependencyCount = static_cast<std::uint32_t>(dependencies.size());

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

    RenderPass::RenderPass(Device& logical_device,
                           Attachment& attachment,
                           Subpass& subpass)
                          : RenderPass { logical_device,
                                         std::vector<Attachment> { attachment },
                                         subpass } {  }

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

        swap(lhs.subpass_color_references, rhs.subpass_color_references);
        swap(lhs.subpass_depth_references, rhs.subpass_depth_references);
        swap(lhs.subpass_input_references, rhs.subpass_input_references);
        swap(lhs.depth_attachment_binding, rhs.depth_attachment_binding);

        swap(lhs.device, rhs.device);
        swap(lhs.handle, rhs.handle);
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

    bool RenderPass::has_depth_attachment() const {
        return depth_attachment_binding != -1;
    }

    void RenderPass::create_standard_color_pass(RenderPass& color_pass, Device& device, SwapChain& swap_chain) {
        std::vector<RenderPass::Attachment> attachments {
            {
                swap_chain.get_color_attachment_format(),
                swap_chain.get_khr_presentation_layout()
            },
            {
                swap_chain.get_depth_attachment_format(),
                swap_chain.get_depth_attachment_layout()
            }
        };

        std::vector<RenderPass::Subpass> subpasses {
            {
                { 0, swap_chain.get_color_attachment_layout() },
                { 1, swap_chain.get_depth_attachment_layout() }
            }
        };

        std::vector<RenderPass::Dependency> dependencies {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            }
        };

        color_pass = RenderPass {
             device,
             attachments,
             subpasses,
             dependencies
        };

        DebugMarker::object_name(device, color_pass, VK_OBJECT_TYPE_RENDER_PASS, "Color Pass");
    }

    void RenderPass::create_modified_color_pass(RenderPass& color_pass, Device& device, SwapChain& swap_chain) {
        std::vector<RenderPass::Attachment> attachments {
            {
                swap_chain.get_color_attachment_format(),
                swap_chain.get_khr_presentation_layout()
            },
            {
                swap_chain.get_depth_attachment_format(),
                swap_chain.get_depth_attachment_layout()
            }
        };

        std::vector<RenderPass::Subpass> subpasses {
            {
                { 0, swap_chain.get_color_attachment_layout() },
                { 1, swap_chain.get_depth_attachment_layout() }
            },
            {
                { 0, swap_chain.get_color_attachment_layout() },
                { 1, swap_chain.get_shader_read_only_layout() }
            },
        };

        std::vector<RenderPass::Dependency> dependencies {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            },
            {
                0,
                1,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            }
        };

        color_pass = RenderPass {
             device,
             attachments,
             subpasses,
             dependencies
        };

        DebugMarker::object_name(device, color_pass, VK_OBJECT_TYPE_RENDER_PASS, "Color Pass");
    }

    void RenderPass::create_standard_depth_pass(RenderPass& depth_pass, Device& device) {
        std::vector<RenderPass::Attachment> attachments {
            {
                vkhr::vulkan::DepthMap::get_attachment_format(),
                vkhr::vulkan::DepthMap::get_read_depth_layout()
            }
        };

        std::vector<RenderPass::Subpass> subpasses {
            {
                { 0, vkhr::vulkan::DepthMap::get_attachment_layout() }
            }
        };

        std::vector<RenderPass::Dependency> dependencies {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                0,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
            },
            {
                0,
                VK_SUBPASS_EXTERNAL,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT
            }
        };

        depth_pass = RenderPass {
             device,
             attachments,
             subpasses,
             dependencies
        };

        DebugMarker::object_name(device, depth_pass, VK_OBJECT_TYPE_RENDER_PASS, "Depth Pass");
    }
}
