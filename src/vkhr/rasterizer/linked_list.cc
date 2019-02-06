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
                node_count * node_size
            };

            vk::DebugMarker::object_name(rasterizer.device, nodes, VK_OBJECT_TYPE_BUFFER, "PPLL Nodes", id);
            vk::DebugMarker::object_name(rasterizer.device, nodes.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "PPLL Nodes Device Memory", id);

            null_value.uint32[0] = Null;
        }

        void LinkedList::clear(vk::CommandBuffer& command_buffer) {
            command_buffer.clear_color_image(heads, null_value);
            // Reset the atomic counter to 0:
            command_buffer.fill_buffer(nodes,
                                       0, sizeof(std::uint32_t),
                                       0);
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

        int LinkedList::id { 0 };
    }
}