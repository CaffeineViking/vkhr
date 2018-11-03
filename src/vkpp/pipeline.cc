#include <vkpp/pipeline.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Pipeline::~Pipeline() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, handle, nullptr);
        }
    }

    Pipeline::Pipeline(Device& device) : device { device.get_handle() } { }

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

        rhs.device = VK_NULL_HANDLE;
        rhs.handle = VK_NULL_HANDLE;
    }

    VkPipeline& Pipeline::get_handle() {
        return handle;
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

        swap(lhs.vertex_input_state, rhs.vertex_input_state);
        swap(lhs.input_assembly_state, rhs.input_assembly_state);
        swap(lhs.tessellation_state, rhs.tessellation_state);
        swap(lhs.viewport_state, rhs.viewport_state);
        swap(lhs.rasterization_state, rhs.rasterization_state);
        swap(lhs.multisample_state, rhs.multisample_state);
        swap(lhs.depth_stencil_state, rhs.depth_stencil_state);
        swap(lhs.color_blending_state, rhs.color_blending_state);
        swap(lhs.dynamic_state, rhs.dynamic_state);
    }

    VkPrimitiveTopology GraphicsPipeline::get_topology() const {
        return input_assembly_state.topology;
    }

    void GraphicsPipeline::set_topology(VkPrimitiveTopology topology) {
        input_assembly_state.topology = topology;
    }

    std::uint32_t GraphicsPipeline::get_patch_control_points() const {
        return tessellation_state.patchControlPoints;
    }

    void GraphicsPipeline::set_patch_control_points(std::uint32_t n) {
        tessellation_state.patchControlPoints = n;
    }

    const VkViewport& GraphicsPipeline::get_viewport() const {
        return viewport;
    }

    void GraphicsPipeline::set_viewport(const VkViewport& viewport) {
        this->viewport = viewport;
        viewport_state.pViewports = &(this->viewport);
        viewport_state.viewportCount = 1;
    }

    void GraphicsPipeline::set_scissor(const VkRect2D& scissor) {
        this->scissor = scissor;
        viewport_state.pScissors = &(this->scissor);
        viewport_state.scissorCount = 1;
    }

    const VkRect2D& GraphicsPipeline::get_scissor() const {
        return scissor;
    }

    void GraphicsPipeline::set_line_width(float line_width) {
        rasterization_state.lineWidth = line_width;
    }

    float GraphicsPipeline::get_line_width() const {
        return rasterization_state.lineWidth;
    }

    VkPolygonMode GraphicsPipeline::get_polygon_mode() const {
        return rasterization_state.polygonMode;
    }

    void GraphicsPipeline::set_polygon_mode(VkPolygonMode polygon_mode) {
        rasterization_state.polygonMode = polygon_mode;
    }

    void GraphicsPipeline::set_culling_mode(VkCullModeFlags culling_mode) {
        rasterization_state.cullMode = culling_mode;
    }
    VkCullModeFlags GraphicsPipeline::get_culling_mode() const {
        return rasterization_state.cullMode;
    }

    VkFrontFace GraphicsPipeline::get_front_face() const {
        return rasterization_state.frontFace;
    }

    void GraphicsPipeline::set_front_face(VkFrontFace front_face) {
        rasterization_state.frontFace = front_face;
    }

    void GraphicsPipeline::enable_depth_testing() {
        depth_stencil_state.depthWriteEnable = VK_TRUE;
        depth_stencil_state.depthTestEnable  = VK_TRUE;
    }

    void GraphicsPipeline::disable_depth_testing() {
        depth_stencil_state.depthWriteEnable = VK_FALSE;
        depth_stencil_state.depthTestEnable  = VK_FALSE;
    }

    void GraphicsPipeline::disable_alpha_blending(std::uint32_t attachment) {
        if (attachments.size() <= attachment) {
            attachments.resize(attachment);
        }

        attachments[attachment].blendEnable = VK_FALSE;

        color_blending_state.attachmentCount = attachments.size();
        color_blending_state.pAttachments    = attachments.data();
    }

    void GraphicsPipeline::enable_alpha_blending(std::uint32_t attachment) {
        if (attachments.size() <= attachment) {
            attachments.resize(attachment);
        }

        attachments[attachment].colorWriteMask =  VK_COLOR_COMPONENT_R_BIT |
                                                  VK_COLOR_COMPONENT_G_BIT |
                                                  VK_COLOR_COMPONENT_B_BIT |
                                                  VK_COLOR_COMPONENT_A_BIT;
        attachments[attachment].blendEnable = VK_TRUE;
        attachments[attachment].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        attachments[attachment].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachments[attachment].colorBlendOp = VK_BLEND_OP_ADD;
        attachments[attachment].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachments[attachment].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachments[attachment].alphaBlendOp = VK_BLEND_OP_ADD;

        color_blending_state.attachmentCount = attachments.size();
        color_blending_state.pAttachments    = attachments.data();
    }

    void GraphicsPipeline::add_dynamic_state(VkDynamicState dyn_state) {
        dynamic_states.push_back(dyn_state);
        dynamic_state.dynamicStateCount = dynamic_states.size();
        dynamic_state.pDynamicStates = dynamic_states.data();
    }

    void GraphicsPipeline::set_shader_stages(std::vector<ShaderModule>& shaders) {
        shader_stages.clear();
        add_shader_stages(shaders);
    }

    void GraphicsPipeline::add_shader_stages(std::vector<ShaderModule>& shaders) {
        for (auto& shader : shaders) {
            VkPipelineShaderStageCreateInfo shader_info;
            shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_info.pNext = nullptr;
            shader_info.flags = 0;

            shader_info.stage = static_cast<VkShaderStageFlagBits>(shader.get_stage());
            shader_info.module = shader.get_handle();
            shader_info.pName = "main";
            shader_info.pSpecializationInfo = nullptr; // TODO: add this to ShaderModule

            shader_stages.push_back(shader_info);
        }
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

        swap(lhs.shader_stage, rhs.shader_stage);
    }

    void ComputePipeline::set_shader(ShaderModule& shader) {
        VkPipelineShaderStageCreateInfo shader_info;
        shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_info.pNext = nullptr;
        shader_info.flags = 0;

        shader_info.stage = static_cast<VkShaderStageFlagBits>(shader.get_stage());

        if (shader.get_stage() != ShaderModule::Type::Compute) {
            throw Exception { "couldn't create compute pipeline!",
            "are you crazy? Don't pass other types of shaders!" };
        }

        shader_info.module = shader.get_handle();
        shader_info.pName = "main";
        shader_info.pSpecializationInfo = nullptr; // TODO: add this to ShaderModule

        shader_stage = shader_info;
    }
}
