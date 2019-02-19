#include <vkhr/rasterizer/linked_list.hh>

#include <vkhr/rasterizer.hh>

namespace vkhr {
    namespace vulkan {
        LinkedList::LinkedList(vkhr::Rasterizer& rasterizer, std::uint32_t width, std::uint32_t height, std::size_t node_size, std::size_t node_count) {
            create(rasterizer, width, height, node_size, node_count);
        }

        void LinkedList::create(vkhr::Rasterizer& rasterizer, std::uint32_t width, std::uint32_t height, std::size_t node_size, std::size_t node_count) {
            heads = vk::DeviceImage {
                rasterizer.device,
                width, height,
                width * height * sizeof(std::uint32_t), VK_FORMAT_R32_UINT,
                VK_IMAGE_USAGE_STORAGE_BIT
            };

            this->width  = width;
            this->height = height;

            auto command_buffer = rasterizer.command_pool.allocate_and_begin();
            heads.transition(command_buffer,
                             0,
                             VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_GENERAL,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            command_buffer.end();
            command_buffer.get_queue().submit(command_buffer)
                                      .wait_idle();

            vk::DebugMarker::object_name(rasterizer.device, heads, VK_OBJECT_TYPE_IMAGE, "PPLL Heads", id);
            vk::DebugMarker::object_name(rasterizer.device, heads.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "PPLL Heads Device Memory", id);

            heads_view = vk::ImageView {
                rasterizer.device,
                heads,
                VK_IMAGE_LAYOUT_GENERAL
            };

            vk::DebugMarker::object_name(rasterizer.device, heads_view, VK_OBJECT_TYPE_IMAGE, "PPLL Heads View", id);

            nodes = vk::StorageBuffer {
                rasterizer.device,
                node_count * node_size + 8 * sizeof(std::uint32_t) // { Atomic Counter, Size Limit, Padding, Nodes }.
            };

            vk::DebugMarker::object_name(rasterizer.device, nodes, VK_OBJECT_TYPE_BUFFER, "PPLL Nodes", id);
            vk::DebugMarker::object_name(rasterizer.device, nodes.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "PPLL Nodes Device Memory", id);

            null_value.uint32[0] = Null;
        }

        void LinkedList::clear(vk::CommandBuffer& command_buffer) {
            command_buffer.clear_color_image(heads, null_value);
            command_buffer.fill_buffer(nodes,
                                       0,
                                       sizeof(std::uint32_t),
                                       0);
            command_buffer.fill_buffer(nodes,
                                       sizeof(std::uint32_t),
                                       sizeof(std::uint32_t),
                                       node_count);
        }

        void LinkedList::resolve(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer, vk::ImageView& color_view) {
            command_buffer.bind_pipeline(pipeline);

            descriptor_set.write(5, heads_view);
            descriptor_set.write(6, nodes);

            command_buffer.bind_descriptor_set(descriptor_set, pipeline);

            command_buffer.dispatch(width / 8, height / 8);
        }

        std::size_t LinkedList::get_width() const {
            return width;
        }

        std::size_t LinkedList::get_node_count() const {
            return node_count;
        }

        std::size_t LinkedList::get_node_size() const {
            return node_size;
        }

        std::size_t LinkedList::get_heads_size_in_bytes() const {
            return width * height * sizeof(std::uint32_t);
        }

        std::size_t LinkedList::get_nodes_size_in_bytes() const {
            return node_count * node_size;
        }

        std::size_t LinkedList::get_height() const {
            return height;
        }

        vk::DeviceImage& LinkedList::get_heads() {
            return heads;
        }

        vk::ImageView& LinkedList::get_heads_view() {
            return heads_view;
        }

        vk::StorageBuffer& LinkedList::get_nodes() {
            return nodes;
        }

        void LinkedList::build_pipeline(Pipeline& pipeline, Rasterizer& rasterizer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline */ };

            pipeline.shader_stages.emplace_back(rasterizer.device, SHADER("transparency/resolve.comp"));
            vk::DebugMarker::object_name(rasterizer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "PPLL Resolve");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                rasterizer.device,
                {
                    { 5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  },
                    { 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER }
                }
            };

            vk::DebugMarker::object_name(rasterizer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "PPLL Descriptor Set Layout");
            pipeline.descriptor_sets = rasterizer.descriptor_pool.allocate(rasterizer.swap_chain.size(),
                                                                           pipeline.descriptor_set_layout,
                                                                           "PPLL Descriptor Set");

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                rasterizer.device,
                pipeline.descriptor_set_layout
            };

            vk::DebugMarker::object_name(rasterizer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "PPLL Pipeline Layout");

            pipeline.compute_pipeline = vk::ComputePipeline {
                rasterizer.device,
                pipeline.shader_stages[0],
                pipeline.pipeline_layout
            };

            vk::DebugMarker::object_name(rasterizer.device, pipeline.compute_pipeline,
                                         VK_OBJECT_TYPE_PIPELINE, "PPLL Pipeline");
        }

        int LinkedList::id { 0 };
    }
}