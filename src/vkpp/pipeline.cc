#include <vkpp/pipeline.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Pipeline::~Pipeline() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, handle, nullptr);
        }
    }

    Pipeline::Pipeline(Device& device, Layout& layout)
                      : device { device.get_handle() },
                        layout { &layout } { }

    Pipeline::Pipeline(Pipeline&& pipeline) noexcept {
        swap(*this, pipeline);
    }

    Pipeline& Pipeline::operator=(Pipeline&& pipeline) noexcept {
        swap(*this, pipeline);
        return *this;
    }

    void swap(Pipeline& lhs, Pipeline& rhs) {
        using std::swap;

        swap(lhs.device, rhs.device);
        swap(lhs.handle, rhs.handle);
        swap(lhs.layout, rhs.layout);
    }

    VkPipeline& Pipeline::get_handle() {
        return handle;
    }

    Pipeline::Layout& Pipeline::get_layout() {
        return *layout;
    }

    Pipeline::Layout::~Layout() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, handle, nullptr);
        }
    }

    Pipeline::Layout::Layout(Device& logical_device,
                             std::vector<DescriptorSet::Layout>& descriptor_layouts,
                             const std::vector<VkPushConstantRange>& push_constants)
                            : push_constants { push_constants },
                              device { logical_device.get_handle() } {
        auto create_info = create_partial_info();

        create_info.setLayoutCount = descriptor_layouts.size();

        this->descriptor_layouts.reserve(descriptor_layouts.size());
        for (auto& descriptor_layout : descriptor_layouts) {
            this->descriptor_layouts.push_back(descriptor_layout.get_handle());
        }

        if (descriptor_layouts.size() != 0) {
            create_info.pSetLayouts = this->descriptor_layouts.data();
        }

        create(create_info);
    }

    Pipeline::Layout::Layout(Device& logical_device,
                             DescriptorSet::Layout& descriptor_layout,
                             const std::vector<VkPushConstantRange>& push_constants)
                            : push_constants { push_constants },
                              device { logical_device.get_handle() } {
        auto create_info = create_partial_info();

        create_info.setLayoutCount = 1;

        this->descriptor_layouts.reserve(1);
        this->descriptor_layouts.push_back(descriptor_layout.get_handle());
        create_info.pSetLayouts = this->descriptor_layouts.data();

        create(create_info);
    }

    Pipeline::Layout::Layout(Device& logical_device,
                             const std::vector<VkPushConstantRange>& push_constants)
                            : push_constants { push_constants },
                              device { logical_device.get_handle() } {
        create(create_partial_info());
    }

    VkPipelineLayoutCreateInfo Pipeline::Layout::create_partial_info() {
        VkPipelineLayoutCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.setLayoutCount = 0;

        create_info.pushConstantRangeCount = push_constants.size();

        create_info.pSetLayouts = nullptr;

        if (push_constants.size() != 0) {
            create_info.pPushConstantRanges = push_constants.data();
        }

        return create_info;
    }

    void Pipeline::Layout::create(VkPipelineLayoutCreateInfo create_info) {
        if (VkResult error = vkCreatePipelineLayout(device, &create_info,
                                                    nullptr, &handle)) {
            throw Exception { error, "couldn't create pipeline layout!" };
        }
    }

    Pipeline::Layout::Layout(Layout&& layout) noexcept {
        swap(*this, layout);
    }

    Pipeline::Layout& Pipeline::Layout::operator=(Layout&& layout) noexcept {
        swap(*this, layout);
        return *this;
    }

    void swap(Pipeline::Layout& lhs, Pipeline::Layout& rhs) {
        using std::swap;

        swap(lhs.push_constants,     rhs.push_constants);
        swap(lhs.descriptor_layouts, rhs.descriptor_layouts);

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);
    }

    VkPipelineLayout& Pipeline::Layout::get_handle() {
        return handle;
    }

    GraphicsPipeline::GraphicsPipeline(Device& logical_device,
                                       std::vector<ShaderModule>& shader_modules,
                                       const FixedFunction& fixed_functions,
                                       Pipeline::Layout& pipeline_layout,
                                       RenderPass& render_pass,
                                       std::uint32_t subpass)
                                      : Pipeline { logical_device, pipeline_layout },
                                        fixed_functions { fixed_functions },
                                        render_pass { &render_pass },
                                        subpass_index { subpass } {
        set_shader_stages(shader_modules);

        VkGraphicsPipelineCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.stageCount = shader_stages.size();
        create_info.pStages = shader_stages.data();

        bool has_tessellation_stage { false };

        for (const auto& shader_module : shader_modules) {
            if (shader_module.get_stage() == ShaderModule::Type::TesselationControl ||
                shader_module.get_stage() == ShaderModule::Type::TesselationEvaluation)
                has_tessellation_stage = true;
        }

        create_info.pVertexInputState = &(fixed_functions.vertex_input_state);
        create_info.pInputAssemblyState = &(fixed_functions.input_assembly_state);

        if (has_tessellation_stage) {
            create_info.pTessellationState = &(fixed_functions.tessellation_state);
        } else {
            create_info.pTessellationState = nullptr;
        }

        create_info.pViewportState = &(fixed_functions.viewport_state);
        create_info.pRasterizationState = &(fixed_functions.rasterization_state);
        create_info.pMultisampleState = &(fixed_functions.multisample_state);
        create_info.pDepthStencilState = &(fixed_functions.depth_stencil_state);
        create_info.pColorBlendState = &(fixed_functions.color_blending_state);

        if (fixed_functions.dynamic_states.size() != 0) {
            create_info.pDynamicState = &(fixed_functions.dynamic_state);
        } else {
            create_info.pDynamicState = nullptr;
        }

        auto layout = pipeline_layout.get_handle();

        create_info.layout = layout;
        create_info.renderPass = render_pass.get_handle();
        create_info.subpass = subpass;

        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;

        if (VkResult error = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                                       &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create a graphics pipeline!" };
        }
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& pipeline) noexcept {
        swap(*this, pipeline);
    }

    GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& pipeline) noexcept {
        swap(*this, pipeline);
        return *this;
    }

    void swap(GraphicsPipeline& lhs, GraphicsPipeline& rhs) {
        using std::swap;

        swap(static_cast<Pipeline&>(lhs), static_cast<Pipeline&>(rhs));

        swap(lhs.shader_stages, rhs.shader_stages);
        swap(lhs.fixed_functions, rhs.fixed_functions);
        swap(lhs.specialization_constants, rhs.specialization_constants);
        swap(lhs.subpass_index, rhs.subpass_index);
        swap(lhs.render_pass, rhs.render_pass);
    }

    VkPipelineBindPoint GraphicsPipeline::get_bind_point() const {
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    }

    const VertexBindings& GraphicsPipeline::FixedFunction::get_vertex_bindings() const {
        return vertex_bindings;
    }

    void GraphicsPipeline::FixedFunction::set_vertex_bindings(const VertexBindings& binds) {
        vertex_bindings = binds;
        vertex_input_state.vertexBindingDescriptionCount = vertex_bindings.size();
        vertex_input_state.pVertexBindingDescriptions = vertex_bindings.data();
    }

    void GraphicsPipeline::FixedFunction::set_vertex_attributes(const VertexAttributes& a) {
        vertex_attributes = a;
        vertex_input_state.vertexAttributeDescriptionCount = vertex_attributes.size();
        vertex_input_state.pVertexAttributeDescriptions = vertex_attributes.data();
    }

    const VertexAttributes& GraphicsPipeline::FixedFunction::get_vertex_attributes() const {
        return vertex_attributes;
    }

    void GraphicsPipeline::FixedFunction::add_vertex_attribute(VertexAttribute attribute) {
        vertex_attributes.push_back(attribute);
        vertex_input_state.vertexAttributeDescriptionCount = vertex_attributes.size();
        vertex_input_state.pVertexAttributeDescriptions = vertex_attributes.data();
    }

    void GraphicsPipeline::FixedFunction::add_vertex_attributes(const VertexAttributes& a) {
        vertex_attributes.insert(vertex_attributes.end(), a.cbegin(), a.cend());
        vertex_input_state.vertexAttributeDescriptionCount = vertex_attributes.size();
        vertex_input_state.pVertexAttributeDescriptions = vertex_attributes.data();
    }

    void GraphicsPipeline::FixedFunction::add_vertex_binding(const VertexBinding binding) {
        vertex_bindings.push_back(binding);
        vertex_input_state.vertexBindingDescriptionCount = vertex_bindings.size();
        vertex_input_state.pVertexBindingDescriptions = vertex_bindings.data();
    }

    void GraphicsPipeline::FixedFunction::add_vertex_input(VertexBuffer& vertex_buffer) {
        add_vertex_attributes(vertex_buffer.get_attributes());
        add_vertex_binding(vertex_buffer.get_binding());
    }

    void GraphicsPipeline::FixedFunction::add_vertex_binding(const VertexAttributeBinding& attribute) {
        add_vertex_binding({ attribute.binding, attribute.stride, VK_VERTEX_INPUT_RATE_VERTEX });
        add_vertex_attribute(VertexAttribute { attribute.attribute,
                                               attribute.binding,
                                               attribute.format,
                                               attribute.offset });
    }

    VkPrimitiveTopology GraphicsPipeline::FixedFunction::get_topology() const {
        return input_assembly_state.topology;
    }

    void GraphicsPipeline::FixedFunction::set_topology(VkPrimitiveTopology topology) {
        input_assembly_state.topology = topology;
    }

    std::uint32_t GraphicsPipeline::FixedFunction::get_patch_vertices() const {
        return tessellation_state.patchControlPoints;
    }

    void GraphicsPipeline::FixedFunction::set_patch_vertices(std::uint32_t n) {
        tessellation_state.patchControlPoints = n;
    }

    const VkViewport& GraphicsPipeline::FixedFunction::get_viewport() const {
        return viewport;
    }

    void GraphicsPipeline::FixedFunction::set_viewport(const VkViewport& viewport) {
        this->viewport = viewport;
        viewport_state.pViewports = &(this->viewport);
        viewport_state.viewportCount = 1;
    }

    void GraphicsPipeline::FixedFunction::set_scissor(const VkRect2D& scissor) {
        this->scissor = scissor;
        viewport_state.pScissors = &(this->scissor);
        viewport_state.scissorCount = 1;
    }

    const VkRect2D& GraphicsPipeline::FixedFunction::get_scissor() const {
        return scissor;
    }

    void GraphicsPipeline::FixedFunction::set_line_width(float line_width) {
        rasterization_state.lineWidth = line_width;
    }

    float GraphicsPipeline::FixedFunction::get_line_width() const {
        return rasterization_state.lineWidth;
    }

    VkPolygonMode GraphicsPipeline::FixedFunction::get_polygon_mode() const {
        return rasterization_state.polygonMode;
    }

    void GraphicsPipeline::FixedFunction::set_polygon_mode(VkPolygonMode polygon_mode) {
        rasterization_state.polygonMode = polygon_mode;
    }

    void GraphicsPipeline::FixedFunction::set_culling_mode(VkCullModeFlags culling_mode) {
        rasterization_state.cullMode = culling_mode;
    }
    VkCullModeFlags GraphicsPipeline::FixedFunction::get_culling_mode() const {
        return rasterization_state.cullMode;
    }

    VkFrontFace GraphicsPipeline::FixedFunction::get_front_face() const {
        return rasterization_state.frontFace;
    }

    void GraphicsPipeline::FixedFunction::set_front_face(VkFrontFace front_face) {
        rasterization_state.frontFace = front_face;
    }

    void GraphicsPipeline::FixedFunction::enable_depth_clamp() {
        rasterization_state.depthClampEnable = VK_TRUE;
    }

    void GraphicsPipeline::FixedFunction::disable_depth_clamp() {
        rasterization_state.depthClampEnable = VK_FALSE;
    }

    void GraphicsPipeline::FixedFunction::disable_discarding() {
        rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    }

    void GraphicsPipeline::FixedFunction::enable_discarding() {
        rasterization_state.rasterizerDiscardEnable = VK_TRUE;
    }

    void GraphicsPipeline::FixedFunction::enable_depth_bias() {
        rasterization_state.depthBiasEnable = VK_TRUE;
    }

    void GraphicsPipeline::FixedFunction::enable_depth_bias(float constant_factor,
                                                            float clamp,
                                                            float slope_factor) {
        rasterization_state.depthBiasEnable = VK_TRUE;
        rasterization_state.depthBiasConstantFactor = constant_factor;
        rasterization_state.depthBiasClamp = clamp;
        rasterization_state.depthBiasSlopeFactor = slope_factor;
    }
    void GraphicsPipeline::FixedFunction::disable_depth_bias() {
        rasterization_state.depthBiasEnable = VK_FALSE;
        rasterization_state.depthBiasConstantFactor = 0.0;
        rasterization_state.depthBiasClamp = 0.0;
        rasterization_state.depthBiasSlopeFactor = 0.0;
    }

    void GraphicsPipeline::FixedFunction::FixedFunction::enable_depth_test() {
        depth_stencil_state.depthWriteEnable = VK_TRUE;
        depth_stencil_state.depthTestEnable  = VK_TRUE;
    }

    void GraphicsPipeline::FixedFunction::FixedFunction::enable_depth_test(bool write) {
        if (write) {
            depth_stencil_state.depthWriteEnable = VK_TRUE;
        } else {
            depth_stencil_state.depthWriteEnable = VK_FALSE;
        }

        depth_stencil_state.depthTestEnable  = VK_TRUE;
    }

    void GraphicsPipeline::FixedFunction::set_depth_test_compare(VkCompareOp operation) {
        depth_stencil_state.depthCompareOp = operation;
    }

    void GraphicsPipeline::FixedFunction::disable_depth_test(bool write) {
        if (write) {
            depth_stencil_state.depthWriteEnable = VK_TRUE;
        } else {
            depth_stencil_state.depthWriteEnable = VK_FALSE;
        }

        depth_stencil_state.depthTestEnable  = VK_FALSE;
    }

    void GraphicsPipeline::FixedFunction::disable_depth_test() {
        depth_stencil_state.depthWriteEnable = VK_FALSE;
        depth_stencil_state.depthTestEnable  = VK_FALSE;
    }

    void GraphicsPipeline::FixedFunction::set_samples(VkSampleCountFlagBits samples) {
        multisample_state.rasterizationSamples = samples;
    }

    VkSampleCountFlagBits GraphicsPipeline::FixedFunction::get_samples() const {
        return multisample_state.rasterizationSamples;
    }

    void GraphicsPipeline::FixedFunction::enable_sample_shading(float minimum) {
        multisample_state.sampleShadingEnable = VK_TRUE;
        multisample_state.minSampleShading = minimum;
    }

    void GraphicsPipeline::FixedFunction::disable_sample_shading() {
        multisample_state.sampleShadingEnable = VK_FALSE;
        multisample_state.minSampleShading = 1.0;
    }

    void GraphicsPipeline::FixedFunction::disable_blending_for(std::uint32_t a) {
        if (attachments.size() <= a) {
            attachments.resize(a + 1);
        }

        attachments[a].blendEnable = VK_FALSE;

        color_blending_state.attachmentCount = attachments.size();
        color_blending_state.pAttachments    = attachments.data();
    }

    void GraphicsPipeline::FixedFunction::enable_alpha_blending_for(std::uint32_t a) {
        if (attachments.size() <= a) {
            attachments.resize(a + 1);
        }

        attachments[a].colorWriteMask =  VK_COLOR_COMPONENT_R_BIT |
                                         VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT |
                                         VK_COLOR_COMPONENT_A_BIT;
        attachments[a].blendEnable = VK_TRUE;
        attachments[a].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        attachments[a].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachments[a].colorBlendOp = VK_BLEND_OP_ADD;
        attachments[a].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachments[a].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachments[a].alphaBlendOp = VK_BLEND_OP_ADD;

        color_blending_state.attachmentCount = attachments.size();
        color_blending_state.pAttachments    = attachments.data();
    }

    void GraphicsPipeline::FixedFunction::enable_additive_blending_for(std::uint32_t a) {
        if (attachments.size() <= a) {
            attachments.resize(a + 1);
        }

        attachments[a].colorWriteMask =  VK_COLOR_COMPONENT_R_BIT |
                                         VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT |
                                         VK_COLOR_COMPONENT_A_BIT;
        attachments[a].blendEnable = VK_TRUE;
        attachments[a].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachments[a].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachments[a].colorBlendOp = VK_BLEND_OP_ADD;
        attachments[a].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachments[a].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachments[a].alphaBlendOp = VK_BLEND_OP_ADD;

        color_blending_state.attachmentCount = attachments.size();
        color_blending_state.pAttachments    = attachments.data();
    }

    void GraphicsPipeline::FixedFunction::add_dynamic_state(VkDynamicState dynamic_state) {
        dynamic_states.push_back(dynamic_state);
        this->dynamic_state.dynamicStateCount = dynamic_states.size();
        this->dynamic_state.pDynamicStates    = dynamic_states.data();
    }

    const GraphicsPipeline::FixedFunction& GraphicsPipeline::get_fixed_functions() const {
        return fixed_functions;
    }

    std::uint32_t GraphicsPipeline::get_subpass() const {
        return subpass_index;
    }

    RenderPass& GraphicsPipeline::get_render_pass() const {
        return *render_pass;
    }

    const std::vector<VkPipelineShaderStageCreateInfo>&
    GraphicsPipeline::get_shader_stages() const {
        return shader_stages;
    }

    void GraphicsPipeline::set_shader_stages(std::vector<ShaderModule>& shaders) {
        specialization_constants.clear();
        shader_stages.clear();
        shader_stages.reserve(shaders.size());
        specialization_constants.reserve(shaders.size());

        for (auto& shader : shaders) {
            VkPipelineShaderStageCreateInfo shader_info;
            shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_info.pNext = nullptr;
            shader_info.flags = 0;

            shader_info.stage = static_cast<VkShaderStageFlagBits>(shader.get_stage());

            shader_info.module = shader.get_handle();
            shader_info.pName = "main";

            VkSpecializationInfo specialization_info;

            specialization_info.mapEntryCount = shader.get_constants().size();
            specialization_info.pMapEntries = shader.get_constants().data();
            specialization_info.pData = shader.get_constants_data();
            specialization_info.dataSize = shader.get_constants_data_size();

            specialization_constants.push_back(specialization_info);

            shader_info.pSpecializationInfo = &specialization_constants.back();

            shader_stages.push_back(shader_info);
        }
    }

    VkPipelineBindPoint ComputePipeline::get_bind_point() const {
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    ComputePipeline::ComputePipeline(ComputePipeline&& pipeline) noexcept {
        swap(*this, pipeline);
    }

    ComputePipeline& ComputePipeline::operator=(ComputePipeline&& pipeline) noexcept {
        swap(*this, pipeline);
        return *this;
    }

    void swap(ComputePipeline& lhs, ComputePipeline& rhs) {
        using std::swap;
        swap(static_cast<Pipeline&>(lhs), static_cast<Pipeline&>(rhs));
    }

    ComputePipeline::ComputePipeline(Device& logical_device,
                                     ShaderModule& shader_module,
                                     Pipeline::Layout& pipeline_layout)
                                    : Pipeline { logical_device, pipeline_layout } {
        VkPipelineShaderStageCreateInfo shader_info;
        shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_info.pNext = nullptr;
        shader_info.flags = 0;

        shader_info.stage = static_cast<VkShaderStageFlagBits>(shader_module.get_stage());

        if (shader_module.get_stage() != ShaderModule::Type::Compute) {
            throw Exception { "couldn't create compute pipeline!",
            "are you crazy? Don't pass other types of shaders!" };
        }

        std::string entry_point { shader_module.get_entry_point() };

        shader_info.module = shader_module.get_handle();
        shader_info.pName = entry_point.c_str();
        shader_info.pSpecializationInfo = nullptr;

        VkComputePipelineCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.stage = shader_info;

        auto layout = pipeline_layout.get_handle();

        create_info.layout = layout;

        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;

        if (VkResult error = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1,
                                                      &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create a compute pipeline!" };
        }
    }
}
