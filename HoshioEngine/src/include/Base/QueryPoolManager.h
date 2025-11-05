#ifndef _QUERY_POOL_MANAGER_H_
#define _QUERY_POOL_MANAGER_H_

#include "Base/CommandManager.h"

namespace HoshioEngine {


	class QueryPool {
	private:
		VkQueryPool handle = VK_NULL_HANDLE;
	public:
		QueryPool() = default;
		QueryPool(VkQueryPoolCreateInfo& createInfo);
		QueryPool(VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics = 0, VkQueryPoolCreateFlags flags = 0);
		QueryPool(QueryPool&& other) noexcept;
		operator VkQueryPool() const;
		const VkQueryPool* Address() const;
		~QueryPool();

		void Reset(VkCommandBuffer commandBuffer, uint32_t firstQueryIndex, uint32_t queryCount) const;

		void Begin(VkCommandBuffer commandBuffer, uint32_t queryIndex, VkQueryControlFlags flags = 0) const;

		void End(VkCommandBuffer commandBuffer, uint32_t queryIndex) const;

		void WriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, uint32_t queryIndex) const;

		void CopyResults(VkCommandBuffer commandBuffer, uint32_t firstQueryIndex, uint32_t queryCount,
			VkBuffer buffer_dst, VkDeviceSize offset_dst, VkDeviceSize stride, VkQueryResultFlags flags = 0) const;

		void GetResults(uint32_t firstQueryIndex, uint32_t queryCount, size_t dataSize, void* pData_dst, VkDeviceSize stride, VkQueryResultFlags flags = 0) const;

		void Reset(uint32_t firstQueryIndex, uint32_t queryCount);

		void Create(VkQueryPoolCreateInfo& createInfo);

		void Create(VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics = 0, VkQueryPoolCreateFlags flags = 0);
	};

	class OcclusionQueries {
	protected:
		QueryPool queryPool;
		std::vector<uint32_t> passingSampleCounts;
	public:
		OcclusionQueries() = default;
		OcclusionQueries(uint32_t capacity);

		operator VkQueryPool() const;
		const VkQueryPool* Address() const;
		uint32_t Capacity() const;
		uint32_t PassingSampleCount(uint32_t index) const;

		void Reset(VkCommandBuffer commandBuffer) const;

		void Begin(VkCommandBuffer commandBuffer, uint32_t queryIndex, bool isPrecise = false) const;

		void End(VkCommandBuffer commandBuffer, uint32_t queryIndex) const;

		void CopyResults(VkCommandBuffer commandBuffer, uint32_t firstQueryIndex, uint32_t queryCount,
			VkBuffer buffer_dst, VkDeviceSize offset_dst, VkDeviceSize stride) const;

		void Create(uint32_t capacity);

		void Recreate(uint32_t capacity);

		void GetResults();

		void GetResults(uint32_t queryCount);

	};

	class PipelineStatisticQuery {
	protected:
		enum statisticName {
			INPUT_ASSEMBLY_VERTICES,
			INPUT_ASSEMBLY_PRIMITIVES,
			VERTEX_SHADER_INVOCATIONS,
			GEOMETRY_SHADER_INVOCATIONS,
			GEOMETRY_SHADER_PRIMITIVES,
			CLIPPING_INVOCATIONS,
			CLIPPING_PRIMITIVES,
			FRAMENT_SHADER_INVOCATIONS,
			TESSELLATION_CONTROL_SHADER_PATCH,
			TESSELLATION_EVALUTION_SHADER_INVOCATIONS,
			COMPUTE_SHADER_INVOCATIONS,
			STATISTIC_COUNT
		};
		QueryPool queryPool;
		uint32_t statistics[STATISTIC_COUNT] = {};
	public:
		PipelineStatisticQuery();
		operator VkQueryPool() const;
		const VkQueryPool* Address() const;
		uint32_t InputAssemblyVertices() const;
		uint32_t InputAssemblyPrimitives() const;
		uint32_t VertexShaderInvocations() const;
		uint32_t GeometryShaderInvocations() const;
		uint32_t GeometryShaderPrimitives() const;
		uint32_t ClippingInvocations() const;
		uint32_t ClppingPrimitives() const;
		uint32_t FragmentShaderInvocations() const;
		uint32_t TessellationControlShaderPatch() const;
		uint32_t TessellationEvalutionShaderInvocations() const;
		uint32_t ComputeShaderInvocations() const;

		void Reset(VkCommandBuffer commandBuffer) const;

		void Begin(VkCommandBuffer commandBuffer) const;

		void End(VkCommandBuffer commandBuffer) const;

		void ResetAndBegin(VkCommandBuffer commandBuffer) const;

		void Create();

		void GetResults();
	};

	class TimestampQueries{
	protected:
		QueryPool queryPool;
		std::vector<uint32_t> timestamps;
	public:
		TimestampQueries() = default;
		TimestampQueries(uint32_t capacity);

		operator VkQueryPool() const;
		const VkQueryPool* Address() const;

		uint32_t Capacity() const;
		uint32_t Timestamp(uint32_t index) const;
		uint32_t Duration(uint32_t index) const;

		void Reset(VkCommandBuffer commandBuffer) const;
		void WriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, uint32_t queryIndex) const;

		void Create(uint32_t capacity);
		void Recreate(uint32_t capacity);
		void GetResults();
		void GetResults(uint32_t queryCount);

	};


	class QueryPoolManager {
	private:
		int m_timestamp_queries_id = 0;

		std::unordered_map<std::string, int> mTimestampQueriesIDs;
		std::unordered_map<int, std::vector<TimestampQueries>> mTimestampQueries;

		std::pair<int, std::span<TimestampQueries>> RecreateTimestampQueries(int id, uint32_t count, uint32_t capacity);
	public:
		std::pair<int, std::span<TimestampQueries>> CreateTimestampQueries(std::string name, uint32_t count, uint32_t capacity);
		std::pair<int, std::span<TimestampQueries>> GetTimestampQueries(std::string name);
		std::pair<int, std::span<TimestampQueries>> GetTimestampQueries(int id);
		bool HasTimestampQueries(std::string name);
		bool HasTimestampQueries(int id);
		size_t GetTimestampQueriesCount() const;
	};

}

#endif // !_QUERY_POOL_MANAGER_H_

