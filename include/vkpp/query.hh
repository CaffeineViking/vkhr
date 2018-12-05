#ifndef VKPP_QUERY_HH
#define VKPP_QUERY_HH

#include <vulkan/vulkan.h>

#include <cstdint>

namespace vkpp {
    class Device;
    class QueryPool final {
    public:
        QueryPool() = default;
        QueryPool(Device& device,
                  VkQueryType query_type,
                  std::uint32_t query_count,
                  VkQueryPipelineStatisticFlags pipeline_stats = 0);
        ~QueryPool() noexcept;

        QueryPool(QueryPool&& command_pool) noexcept;
        QueryPool& operator=(QueryPool&& command_pool) noexcept;
        friend void swap(QueryPool& lhs, QueryPool& rhs);

        VkQueryType get_query_type() const;
        VkQueryPipelineStatisticFlags get_pipeline_statistics_flag() const;
        std::uint32_t get_query_count() const;

        float get_ns_per_unit() const;

        VkQueryPool& get_handle();

        std::uint32_t query { 0 };

        VkResult get_results(std::uint32_t first_query, std::uint32_t query_count,
                             VkDeviceSize size, void* buffer,
                             VkQueryResultFlags result_flags,
                             VkDeviceSize stride = 0);

    private:
        VkQueryType query_types;
        VkQueryPipelineStatisticFlags pipeline_statistics;
        std::uint32_t query_count;

        float ns_per_unit;

        VkDevice device    { VK_NULL_HANDLE };
        VkQueryPool handle { VK_NULL_HANDLE };
    };
}

#endif