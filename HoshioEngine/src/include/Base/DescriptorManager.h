#ifndef _DESCRIPTOR_MANAGER_H_
#define _DESCRIPTOR_MANAGER_H_

#include "Base/MemoryManager.h"

namespace HoshioEngine {
	class DescriptorSetLayout {
	private:
		VkDescriptorSetLayout handle = VK_NULL_HANDLE;
	public:
		DescriptorSetLayout() = default;
		DescriptorSetLayout(VkDescriptorSetLayoutCreateInfo& createInfo);
		DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
		~DescriptorSetLayout();
		operator VkDescriptorSetLayout() const;
		const VkDescriptorSetLayout* Address() const;

		void Create(VkDescriptorSetLayoutCreateInfo& createInfo);
	};

	class DescriptorSet {
	private:
		friend class DescriptorPool;
		VkDescriptorSet handle = VK_NULL_HANDLE;
	public:
		DescriptorSet() = default;
		DescriptorSet(DescriptorSet&& descriptor) noexcept;

		operator VkDescriptorSet() const;
		const VkDescriptorSet* Address() const;

		void Write(ArrayRef<const VkDescriptorImageInfo> descriptorInfos,
			VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const;

		void Write(ArrayRef<const VkDescriptorBufferInfo> descriptorInfos, 
			VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const;

		void Write(ArrayRef<const VkBufferView> descriptorInfos,
			VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const;

		void Write(ArrayRef<const BufferView> descriptorInfos,
			VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const;

		static void Update(ArrayRef<VkWriteDescriptorSet> write_infos, ArrayRef<VkCopyDescriptorSet> copy_infos = {});
	};

	class DescriptorPool {
	private:
		VkDescriptorPool handle = VK_NULL_HANDLE;
	public:
		DescriptorPool() = default;
		DescriptorPool(VkDescriptorPoolCreateInfo& createInfo);
		DescriptorPool(uint32_t maxSets, ArrayRef<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags = 0);
		DescriptorPool(DescriptorPool&& other) noexcept;
		~DescriptorPool();

		operator VkDescriptorPool () const;
		const VkDescriptorPool* Address() const;
		
		void AllocateDescriptorSets(ArrayRef<VkDescriptorSet> descriptorSets, ArrayRef<const VkDescriptorSetLayout> setLayouts) const;

		void AllocateDescriptorSets(ArrayRef<DescriptorSet> descriptorSets, ArrayRef<const VkDescriptorSetLayout> setLayouts) const;

		void AllocateDescriptorSets(ArrayRef<VkDescriptorSet> descriptorSets, ArrayRef<const DescriptorSetLayout> setLayouts) const;

		void AllocateDescriptorSets(ArrayRef<DescriptorSet> descriptorSets, ArrayRef<const DescriptorSetLayout> setLayouts) const;

		void FreeDescriptorSets(ArrayRef<VkDescriptorSet> descriptorSets) const;

		void FreeDescriptorSets(ArrayRef<DescriptorSet> descriptorSets) const;

		void Create(VkDescriptorPoolCreateInfo& createInfo);
		void Create(uint32_t maxSets, ArrayRef<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags = 0);

	};

	class DescriptorManager {
	private:
		int m_descriptor_pool_id = 0;
		int m_descriptor_set_layout_id = 0;

		std::unordered_map<std::string, int> mDescriptorPoolIDs;
		std::unordered_map<int, DescriptorPool> mDescriptorPools;
		std::unordered_map<std::string, int> mDescriptorSetLayoutIDs;
		std::unordered_map<int, DescriptorSetLayout> mDescriptorSetLayouts;

		std::pair<int, std::span<DescriptorPool>> RecreateDescriptorPool(int id, VkDescriptorPoolCreateInfo& createInfo);
		std::pair<int, std::span<DescriptorSetLayout>> RecreateDescriptorSetLayout(int id, VkDescriptorSetLayoutCreateInfo& createInfo);

	public:
		std::pair<int, std::span<DescriptorPool>> CreateDescriptorPool(std::string name, VkDescriptorPoolCreateInfo& createInfo);
		std::pair<int, std::span<DescriptorPool>> GetDescriptorPool(std::string name);
		std::pair<int, std::span<DescriptorPool>> GetDescriptorPool(int id);
		bool HasDescriptorPool(std::string name);
		bool HasDescriptorPool(int id);
		size_t GetDescriptorPoolCount() const;

		std::pair<int, std::span<DescriptorSetLayout>> CreateDescriptorSetLayout(std::string name, VkDescriptorSetLayoutCreateInfo& createInfo);
		std::pair<int, std::span<DescriptorSetLayout>> GetDescriptorSetLayout(std::string name);
		std::pair<int, std::span<DescriptorSetLayout>> GetDescriptorSetLayout(int id);
		bool HasDescriptorSetLayout(std::string name);
		bool HasDescriptorSetLayout(int id);
		size_t GetDescriptorSetLayoutCount() const; 

	};


}



#endif // !_DESCRIPTOR_MANAGER_H_
