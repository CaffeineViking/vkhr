#ifndef VKPP_PIPELINE_HH
#define VKPP_PIPELINE_HH

#include <vkpp/device.hh>
#include <vkpp/render_pass.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/shader_module.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class Pipeline {
    public:
        Pipeline() = default;

        virtual ~Pipeline() noexcept;

        Pipeline(Pipeline&& pipeline) noexcept;
        Pipeline& operator=(Pipeline&& pipeline) noexcept;

        friend void swap(Pipeline& lhs, Pipeline& rhs);

        VkPipeline& get_handle();

        virtual VkPipelineBindPoint get_bind_point() const = 0;

        class Layout final {
        public:
            Layout() = default;

            Layout(Device& logical_device, // TODO: better way to do this.
                   std::vector<DescriptorSet::Layout>& descriptor_layouts,
                   const std::vector<VkPushConstantRange>& push_constants);

            Layout(Device& logical_device,
                   const std::vector<VkPushConstantRange>& push_constants = {  });

            ~Layout() noexcept;

            Layout(Layout&& layout) noexcept;
            Layout& operator=(Layout&& layout) noexcept;

            friend void swap(Layout& lhs, Layout& rhs);

            VkPipelineLayout& get_handle();

            const std::vector<VkPushConstantRange>& get_push_constants() const;
            std::vector<DescriptorSet::Layout>&  get_descriptor_layouts() const;

        private:
            VkPipelineLayoutCreateInfo create_partial_info();

            void create(VkPipelineLayoutCreateInfo create_info);

            std::vector<VkPushConstantRange> push_constants;

            std::vector<VkDescriptorSetLayout> descriptor_layouts;

            VkDevice         device { VK_NULL_HANDLE };
            VkPipelineLayout handle { VK_NULL_HANDLE };
        };

        Pipeline(Device& logical_device,
                 Layout& pipeline_layout);

        Layout& get_layout();

    protected:
        VkDevice   device { VK_NULL_HANDLE };
        VkPipeline handle { VK_NULL_HANDLE };

        Layout* layout { nullptr };

    };

    class GraphicsPipeline final : public Pipeline {
    public:
        GraphicsPipeline() = default;

        VkPipelineBindPoint get_bind_point() const override;

        struct FixedFunction {
            VkPipelineVertexInputStateCreateInfo   vertex_input_state {
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                nullptr,
                0,
                0, // vertexBindingDescriptionCount
                nullptr, // pVertexBindingDescriptions
                0, // vertexAttributeDescriptionCount
                nullptr // pVertexAttributeDescriptions
            };

            void set_topology(VkPrimitiveTopology topology);
            VkPrimitiveTopology get_topology() const;

            VkPipelineInputAssemblyStateCreateInfo input_assembly_state {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // topology
                VK_FALSE // primitiveRestartEnable
            };

            std::uint32_t get_patch_control_points() const;
            void set_patch_control_points(std::uint32_t n);

            VkPipelineTessellationStateCreateInfo  tessellation_state {
                VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                nullptr,
                0,
                3 // patchControlPoints
            };

            const VkViewport& get_viewport() const;
            void set_viewport(const VkViewport& viewport);
            void set_scissor(const VkRect2D& scissor);
            const VkRect2D& get_scissor() const;

            VkRect2D   scissor;
            VkViewport viewport;

            VkPipelineViewportStateCreateInfo      viewport_state {
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                nullptr,
                0,
                0, // viewportCount
                nullptr, // pViewports
                0, // scissorCount
                nullptr // pScissors
            };

            float get_line_width() const;
            void set_line_width(float line_width);
            VkPolygonMode get_polygon_mode() const;
            void set_polygon_mode(VkPolygonMode polygon_mode);
            void set_culling_mode(VkCullModeFlags culling_mode);
            VkCullModeFlags get_culling_mode() const;
            void set_front_face(VkFrontFace front_face);
            VkFrontFace get_front_face() const;

            VkPipelineRasterizationStateCreateInfo rasterization_state {
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_FALSE, // depthClampEnable
                VK_FALSE, // rasterizerDiscardEnable
                VK_POLYGON_MODE_FILL, // polygonMode
                VK_CULL_MODE_BACK_BIT, // cullMode
                VK_FRONT_FACE_CLOCKWISE, // frontFace
                VK_FALSE, // depthBiasEnable
                0.0, // depthBiasConstantFactor
                0.0, // depthBiasClamp
                0.0, // depthBiasSlopeFactor
                1.0 // lineWidth
            };

            VkPipelineMultisampleStateCreateInfo   multisample_state {
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_SAMPLE_COUNT_1_BIT, // rasterizationSamples
                VK_FALSE, // sampleShadingEnable
                1.0, // minSampleShading
                nullptr, // pSampleMask
                VK_FALSE, // alphaToCoverageEnable
                VK_FALSE // alphaToOneEnable
            };

            void enable_depth_testing();
            void disable_depth_testing();

            VkPipelineDepthStencilStateCreateInfo  depth_stencil_state {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_FALSE, // depthTestEnable
                VK_FALSE, // depthWriteEnable
                VK_COMPARE_OP_LESS, // depthCompareOp
                VK_FALSE, // depthBoundsTestEnable
                VK_FALSE, // stencilTestEnable
                { }, // front
                { }, // back
                0.0, // minDepthBounds
                1.0 // maxDepthBounds
            };

            void disable_alpha_blending_for(std::uint32_t attachment);
            void enable_alpha_blending_for(std::uint32_t  attachment);

            std::vector<VkPipelineColorBlendAttachmentState> attachments;

            VkPipelineColorBlendStateCreateInfo    color_blending_state {
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_FALSE, // logicOpEnable
                VK_LOGIC_OP_COPY, // logicOp
                0, // attachmentCount
                nullptr, // pAttachments
                { 0.0,
                  0.0,
                  0.0,
                  0.0 } // blendConstants
            };

            void add_dynamic_state(VkDynamicState dynamic_state);

            std::vector<VkDynamicState> dynamic_states;

            VkPipelineDynamicStateCreateInfo       dynamic_state {
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                nullptr,
                0,
                0, // dynamicStateCount
                nullptr // pDynamicStates
            };
        };

        GraphicsPipeline(Device& device,
                         std::vector<ShaderModule>& shader_modules,
                         const FixedFunction& fixed_functions,
                         Pipeline::Layout& pipeline_layout,
                         RenderPass& renderpass,
                         std::uint32_t subpass = 0);

        GraphicsPipeline(GraphicsPipeline&& pipeline) noexcept;
        GraphicsPipeline& operator=(GraphicsPipeline&& pipeline) noexcept;

        friend void swap(GraphicsPipeline& lhs, GraphicsPipeline& rhs);

        const FixedFunction& get_fixed_functions() const;

        std::uint32_t get_subpass() const;

        RenderPass& get_render_pass() const;

        const std::vector<VkPipelineShaderStageCreateInfo>& get_shader_stages() const;

    private:
        void set_shader_stages(std::vector<ShaderModule>& modules);

        FixedFunction fixed_functions;

        RenderPass* render_pass { nullptr };

        std::uint32_t subpass_index { 0 };

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    };

    class ComputePipeline final : public Pipeline {
    public:
        ComputePipeline() = default;

        VkPipelineBindPoint get_bind_point() const override;

        ComputePipeline(Device& device,
                        ShaderModule& shader_module,
                        Pipeline::Layout& layout);

        ComputePipeline(ComputePipeline&& pipeline) noexcept;
        ComputePipeline& operator=(ComputePipeline&& pipeline) noexcept;

        friend void swap(ComputePipeline& lhs, ComputePipeline& rhs);

        void set_shader(ShaderModule& compute_shader);

    private:
        VkPipelineShaderStageCreateInfo shader_stage;
    };
}

#endif
