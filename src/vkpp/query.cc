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

        timestamp_buffer = new std::uint64_t[query_count];

        if (VkResult error = vkCreateQueryPool(this->device, &create_info, nullptr, &handle))
            throw Exception { error, "couldn't create query pool!" };
    }

    QueryPool::~QueryPool() noexcept {
        if (handle != VK_NULL_HANDLE)
            vkDestroyQueryPool(device, handle, nullptr);
        if (timestamp_buffer != nullptr)
            delete timestamp_buffer;
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
        swap(lhs.ns_per_unit, rhs.ns_per_unit);
        swap(lhs.query, rhs.query);

        swap(lhs.timestamps, rhs.timestamps);
        swap(lhs.timestamp_ms_time, rhs.timestamp_ms_time);
        swap(lhs.timestamp_buffer, rhs.timestamp_buffer);
    }

    std::uint32_t QueryPool::get_timestamp_query_count() const {
        return 2 * timestamps.size();
    }

    const QueryPool::TimestampPair& QueryPool::get_timestamp(const std::string& query_name) const {
        return timestamps.at(query_name);
    }

    void QueryPool::set_begin_timestamp(const std::string& name, std::uint32_t query) {
        auto timestamp = timestamps.find(name);
        if (timestamp == timestamps.end()) {
            timestamps[name] = TimestampPair { 0, 0 };
            timestamp = timestamps.find(name);
        }

        timestamp->second.begin = query;
    }

    void QueryPool::set_end_timestamp(const std::string& name,   std::uint32_t query) {
        auto timestamp = timestamps.find(name);
        if (timestamp == timestamps.end()) {
            timestamps[name] = TimestampPair { 0, 0 };
            timestamp = timestamps.find(name);
        }

        timestamp->second.end = query;
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

    std::unordered_map<std::string, float>& QueryPool::calculate_timestamp_queries() {
        get_results(0, get_query_count(),
                    sizeof(std::uint64_t) * get_query_count(),
                    timestamp_buffer, VK_QUERY_RESULT_64_BIT,
                    sizeof(std::uint64_t));

        for (const auto& timestamp : timestamps) {
            std::int64_t begin_timestamp = timestamp_buffer[timestamp.second.begin];
            std::int64_t end_timestamp   = timestamp_buffer[timestamp.second.end];
            auto duration_in_ns  = (end_timestamp - begin_timestamp) * get_ns_per_unit();
            timestamp_ms_time[timestamp.first] = duration_in_ns / 1e6;
        }

        return timestamp_ms_time;
    }

    std::vector<QueryPool> QueryPool::create(std::size_t count, Device& device, VkQueryType query_type, std::uint32_t query_count,
                                             VkQueryPipelineStatisticFlags pipeline_stats) {
        std::vector<QueryPool> query_pools;
        query_pools.reserve(count);

        for (std::size_t i { 0 }; i < count; ++i) {
            query_pools.emplace_back(device, query_type, query_count, pipeline_stats);
        }

        return query_pools;
    }
}