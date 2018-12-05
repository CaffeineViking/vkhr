#include <vkpp/query.hh>

#include <vkpp/device.hh>
#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    QueryPool::QueryPool(Device& device,
                         VkQueryType query_type,
                         std::uint32_t query_count,
                         VkQueryPipelineStatisticFlags pipeline_stats)
                        : query_types { query_type },
                          query_count { query_count },
                          pipeline_statistics { pipeline_stats },
                          device { device.get_handle() } {
        VkQueryPoolCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.queryType = query_type;
        create_info.queryCount = query_count;
        create_info.pipelineStatistics = pipeline_stats;

        ns_per_unit = device.get_physical_device().get_properties().limits.timestampPeriod;

        if (VkResult error = vkCreateQueryPool(this->device, &create_info, nullptr, &handle))
            throw Exception { error, "couldn't create query pool!" };
    }

    QueryPool::~QueryPool() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyQueryPool(device, handle, nullptr);
        }
    }

    QueryPool::QueryPool(QueryPool&& command_pool) noexcept {
        swap(*this, command_pool);
    }

    QueryPool& QueryPool::operator=(QueryPool&& command_pool) noexcept {
        swap(*this, command_pool);
        return *this;
    }
    void swap(QueryPool& lhs, QueryPool& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);
        swap(lhs.pipeline_statistics, rhs.pipeline_statistics);
        swap(lhs.query_count, rhs.query_count);
        swap(lhs.query_types, rhs.query_types);
        swap(lhs.query, rhs.query);
    }

    VkQueryPool& QueryPool::get_handle() {
        return handle;
    }

    VkQueryType QueryPool::get_query_type() const {
        return query_types;
    }

    VkQueryPipelineStatisticFlags QueryPool::get_pipeline_statistics_flag() const {
        return pipeline_statistics;
    }

    std::uint32_t QueryPool::get_query_count() const {
        return query_count;
    }

    float QueryPool::get_ns_per_unit() const {
        return ns_per_unit;
    }

    VkResult QueryPool::get_results(std::uint32_t first_query, std::uint32_t query_count,
                                    VkDeviceSize size, void* buffer,
                                    VkQueryResultFlags result_flags,
                                    VkDeviceSize stride) {
        return vkGetQueryPoolResults(device, handle,
                                     first_query, query_count,
                                     size, buffer, stride,
                                     result_flags);
    }
}