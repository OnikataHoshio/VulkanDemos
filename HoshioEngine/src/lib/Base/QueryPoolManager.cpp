#include "Base/QueryPoolManager.h"

namespace HoshioEngine {
#pragma region QueryPool
	QueryPool::QueryPool(VkQueryPoolCreateInfo& createInfo)
	{
		Create(createInfo);
	}
	QueryPool::QueryPool(VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics, VkQueryPoolCreateFlags flags)
	{
		Create(queryType, queryCount, pipelineStatistics, flags);
	}
	QueryPool::QueryPool(QueryPool&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}
	QueryPool::operator VkQueryPool() const
	{
		return handle;
	}
	const VkQueryPool* QueryPool::Address() const
	{
		return &handle;
	}
	QueryPool::~QueryPool()
	{
		vkDestroyQueryPool(VulkanBase::Base().Device(), handle, nullptr);
	}
	void QueryPool::Reset(VkCommandBuffer commandBuffer, uint32_t firstQueryIndex, uint32_t queryCount) const
	{
		vkCmdResetQueryPool(commandBuffer, handle, firstQueryIndex, queryCount);
	}
	void QueryPool::Begin(VkCommandBuffer commandBuffer, uint32_t queryIndex, VkQueryControlFlags flags) const
	{
		vkCmdBeginQuery(commandBuffer, handle, queryIndex, flags);
	}
	void QueryPool::End(VkCommandBuffer commandBuffer, uint32_t queryIndex) const
	{
		vkCmdEndQuery(commandBuffer, handle, queryIndex);
	}
	void QueryPool::WriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, uint32_t queryIndex) const
	{
		vkCmdWriteTimestamp(commandBuffer, pipelineStage, handle, queryIndex);
	}
	void QueryPool::CopyResults(VkCommandBuffer commandBuffer, uint32_t firstQueryIndex, uint32_t queryCount, VkBuffer buffer_dst, VkDeviceSize offset_dst, VkDeviceSize stride, VkQueryResultFlags flags) const
	{
		vkCmdCopyQueryPoolResults(commandBuffer, handle, firstQueryIndex, queryCount, buffer_dst, offset_dst, stride, flags);
	}
	void QueryPool::GetResults(uint32_t firstQueryIndex, uint32_t queryCount, size_t dataSize, void* pData_dst, VkDeviceSize stride, VkQueryResultFlags flags) const
	{
		VkResult result = vkGetQueryPoolResults(VulkanBase::Base().Device(), handle, firstQueryIndex, queryCount, dataSize, pData_dst, stride, flags);
		if (result) {
			if (result == VK_NOT_READY)
				std::cout << std::format("[ queryPool ] WARNING\nNot all queries are available!\nError code: {}\n", int32_t(result));
			else
				throw std::runtime_error(std::format("[ queryPool ] ERROR\nFailed to get query pool results!\nError code: {}\n", int32_t(result)));
		}
	}
	void QueryPool::Reset(uint32_t firstQueryIndex, uint32_t queryCount)
	{
		vkResetQueryPool(VulkanBase::Base().Device(), handle, firstQueryIndex, queryCount);
	}
	void QueryPool::Create(VkQueryPoolCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		if (vkCreateQueryPool(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("[ queryPool ] ERROR\nFailed to create a query pool!\n");
	}
	void QueryPool::Create(VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics, VkQueryPoolCreateFlags flags)
	{
		VkQueryPoolCreateInfo createInfo = {
			.queryType = queryType,
			.queryCount = queryCount,
			.pipelineStatistics = pipelineStatistics
		};
		Create(createInfo);
	}

#pragma endregion

#pragma region OcclusionQueries

	OcclusionQueries::OcclusionQueries(uint32_t capacity)
	{
		Create(capacity);
	}
	OcclusionQueries::operator VkQueryPool() const
	{
		return queryPool;
	}
	const VkQueryPool* OcclusionQueries::Address() const
	{
		return queryPool.Address();
	}
	uint32_t OcclusionQueries::Capacity() const
	{
		return passingSampleCounts.size();
	}
	uint32_t OcclusionQueries::PassingSampleCount(uint32_t index) const
	{
		return passingSampleCounts[index];
	}
	void OcclusionQueries::Reset(VkCommandBuffer commandBuffer) const
	{
		queryPool.Reset(commandBuffer, 0, Capacity());
	}
	void OcclusionQueries::Begin(VkCommandBuffer commandBuffer, uint32_t queryIndex, bool isPrecise) const
	{
		queryPool.Begin(commandBuffer, queryIndex, isPrecise);
	}
	void OcclusionQueries::End(VkCommandBuffer commandBuffer, uint32_t queryIndex) const
	{
		queryPool.End(commandBuffer, queryIndex);
	}
	void OcclusionQueries::CopyResults(VkCommandBuffer commandBuffer, uint32_t firstQueryIndex, uint32_t queryCount, VkBuffer buffer_dst, VkDeviceSize offset_dst, VkDeviceSize stride) const
	{
		queryPool.CopyResults(commandBuffer, firstQueryIndex, queryCount, buffer_dst, offset_dst, stride, VK_QUERY_RESULT_WAIT_BIT);
	}
	void OcclusionQueries::Create(uint32_t capacity)
	{
		passingSampleCounts.resize(capacity);
		passingSampleCounts.shrink_to_fit();
		queryPool.Create(VK_QUERY_TYPE_OCCLUSION, Capacity());
	}
	void OcclusionQueries::Recreate(uint32_t capacity)
	{
		VulkanBase::Base().WaitIdle();
		queryPool.~QueryPool();
		Create(capacity);
	}
	void OcclusionQueries::GetResults()
	{
		GetResults(Capacity());
	}
	void OcclusionQueries::GetResults(uint32_t queryCount)
	{
		queryPool.GetResults(0, queryCount, queryCount * 4, passingSampleCounts.data(), 4);
	}

#pragma endregion

#pragma region PipelineStatisticQuery
	PipelineStatisticQuery::PipelineStatisticQuery()
	{
		Create();
	}

	PipelineStatisticQuery::operator VkQueryPool() const
	{
		return queryPool;
	}

	const VkQueryPool* PipelineStatisticQuery::Address() const
	{
		return queryPool.Address();
	}

	uint32_t PipelineStatisticQuery::InputAssemblyVertices() const
	{
		return statistics[INPUT_ASSEMBLY_VERTICES];
	}

	uint32_t PipelineStatisticQuery::InputAssemblyPrimitives() const
	{
		return statistics[INPUT_ASSEMBLY_PRIMITIVES];
	}

	uint32_t PipelineStatisticQuery::VertexShaderInvocations() const
	{
		return statistics[VERTEX_SHADER_INVOCATIONS];
	}

	uint32_t PipelineStatisticQuery::GeometryShaderInvocations() const
	{
		return statistics[GEOMETRY_SHADER_INVOCATIONS];
	}

	uint32_t PipelineStatisticQuery::GeometryShaderPrimitives() const
	{
		return statistics[GEOMETRY_SHADER_PRIMITIVES];
	}

	uint32_t PipelineStatisticQuery::ClippingInvocations() const
	{
		return statistics[CLIPPING_INVOCATIONS];
	}

	uint32_t PipelineStatisticQuery::ClppingPrimitives() const
	{
		return statistics[CLIPPING_PRIMITIVES];
	}

	uint32_t PipelineStatisticQuery::FragmentShaderInvocations() const
	{
		return statistics[FRAMENT_SHADER_INVOCATIONS];
	}

	uint32_t PipelineStatisticQuery::TessellationControlShaderPatch() const
	{
		return statistics[TESSELLATION_CONTROL_SHADER_PATCH];
	}

	uint32_t PipelineStatisticQuery::TessellationEvalutionShaderInvocations() const
	{
		return statistics[TESSELLATION_EVALUTION_SHADER_INVOCATIONS];
	}

	uint32_t PipelineStatisticQuery::ComputeShaderInvocations() const
	{
		return statistics[COMPUTE_SHADER_INVOCATIONS];
	}

	void PipelineStatisticQuery::Reset(VkCommandBuffer commandBuffer) const
	{
		queryPool.Reset(commandBuffer, 0, 1);
	}

	void PipelineStatisticQuery::Begin(VkCommandBuffer commandBuffer) const
	{
		queryPool.Begin(commandBuffer, 0);
	}

	void PipelineStatisticQuery::End(VkCommandBuffer commandBuffer) const
	{
		queryPool.End(commandBuffer, 0);
	}

	void PipelineStatisticQuery::ResetAndBegin(VkCommandBuffer commandBuffer) const
	{
		Reset(commandBuffer);
		Begin(commandBuffer);
	}

	void PipelineStatisticQuery::Create()
	{
		queryPool.Create(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1, (1 << STATISTIC_COUNT) - 1);
	}

	void PipelineStatisticQuery::GetResults()
	{
		queryPool.GetResults(0, 1, sizeof statistics, statistics, sizeof statistics);
	}

#pragma endregion

#pragma region TimestampQueries

	TimestampQueries::TimestampQueries(uint32_t capacity)
	{
		Create(capacity);
	}

	TimestampQueries::operator VkQueryPool() const
	{
		return queryPool;
	}

	const VkQueryPool* TimestampQueries::Address() const
	{
		return queryPool.Address();
	}

	uint32_t TimestampQueries::Capacity() const
	{
		return timestamps.size();
	}

	uint32_t TimestampQueries::Timestamp(uint32_t index) const
	{
		return timestamps[index];
	}

	uint32_t TimestampQueries::Duration(uint32_t index) const
	{
		return timestamps[index + 1] - timestamps[index];
	}

	void TimestampQueries::Reset(VkCommandBuffer commandBuffer) const
	{
		queryPool.Reset(commandBuffer, 0, Capacity());
	}

	void TimestampQueries::WriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, uint32_t queryIndex) const
	{
		queryPool.WriteTimestamp(commandBuffer, pipelineStage, queryIndex);
	}

	void TimestampQueries::Create(uint32_t capacity)
	{
		timestamps.resize(capacity);
		timestamps.shrink_to_fit();
		queryPool.Create(VK_QUERY_TYPE_TIMESTAMP, Capacity());
	}

	void TimestampQueries::Recreate(uint32_t capacity)
	{
		VulkanBase::Base().WaitIdle();
		queryPool.~QueryPool();
		Create(capacity);
	}

	void TimestampQueries::GetResults()
	{
		return GetResults(Capacity());
	}

	void TimestampQueries::GetResults(uint32_t queryCount)
	{
		return queryPool.GetResults(0, queryCount, queryCount * 4, timestamps.data(), 4);
	}
	

#pragma endregion

#pragma region QueryPoolManager

	std::pair<int, std::span<TimestampQueries>> QueryPoolManager::RecreateTimestampQueries(int id, uint32_t count, uint32_t capacity)
	{
		if (count == 0) {
			std::cerr << std::format("[ERROR] QueryPoolManager: '{}' requested 0 TimestampQueriess\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mTimestampQueries.find(id); it != mTimestampQueries.end()) {
			it->second.clear();
			it->second.resize(count);
			for (size_t i = 0; i < it->second.size(); i++)
				it->second[i].Create(capacity);
			return { id, std::span<TimestampQueries>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] QueryPoolManager: TimestampQueriess with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<TimestampQueries>> QueryPoolManager::CreateTimestampQueries(std::string name, uint32_t count, uint32_t capacity)
	{
		if (auto it = mTimestampQueriesIDs.find(name); it != mTimestampQueriesIDs.end())
			return GetTimestampQueries(it->second);

		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 timestampQueriess\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<TimestampQueries> TimestampQueriess(count);
		for (auto& TimestampQueries : TimestampQueriess)
			TimestampQueries.Create(capacity);

		const int id = m_timestamp_queries_id++;

		auto [it1, ok1] = mTimestampQueries.emplace(id, std::move(TimestampQueriess));
		if (!ok1) {
			std::cerr << std::format("[ERROR] SyncManager: Emplace timestampQueriess for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTimestampQueriesIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] SyncManager: TimestampQueries '{}' has not been recorded!\n", name);
			mTimestampQueries.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TimestampQueries>(vec.data(), vec.size()) };
	}

	std::pair<int, std::span<TimestampQueries>> QueryPoolManager::GetTimestampQueries(std::string name)
	{
		if (auto it = mTimestampQueriesIDs.find(name); it != mTimestampQueriesIDs.end())
			return GetTimestampQueries(it->second);
		std::cerr << std::format("[ERROR] SyncManager: TimestampQueries with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<TimestampQueries>> QueryPoolManager::GetTimestampQueries(int id)
	{
		if (auto it = mTimestampQueries.find(id); it != mTimestampQueries.end())
			return { id, std::span<TimestampQueries>(it->second.data(), it->second.size()) };
		std::cerr << std::format("[ERROR] SyncManager: TimestampQueries with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool QueryPoolManager::HasTimestampQueries(std::string name)
	{
		if (auto it = mTimestampQueriesIDs.find(name); it != mTimestampQueriesIDs.end())
			return HasTimestampQueries(it->second);
		return false;
	}

	bool QueryPoolManager::HasTimestampQueries(int id)
	{
		return mTimestampQueries.contains(id);
	}

	size_t QueryPoolManager::GetTimestampQueriesCount() const
	{
		return mTimestampQueries.size();
	}

#pragma endregion

}
