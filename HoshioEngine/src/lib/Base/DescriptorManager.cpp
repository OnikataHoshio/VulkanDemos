#include "Base/DescriptorManager.h"

namespace HoshioEngine {

#pragma region DescriptorSetLayout

	DescriptorSetLayout::DescriptorSetLayout(VkDescriptorSetLayoutCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		if (handle) {
			vkDestroyDescriptorSetLayout(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	DescriptorSetLayout::operator VkDescriptorSetLayout() const
	{
		return handle;
	}

	const VkDescriptorSetLayout* DescriptorSetLayout::Address() const
	{
		return &handle;
	}

	void DescriptorSetLayout::Create(VkDescriptorSetLayoutCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		if (vkCreateDescriptorSetLayout(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a descriptor set layout");
	}

#pragma endregion

#pragma region DescriptorSet

	DescriptorSet::DescriptorSet(DescriptorSet&& descriptor) noexcept
	{
		handle = descriptor.handle;
		descriptor.handle = VK_NULL_HANDLE;
	}

	DescriptorSet::operator VkDescriptorSet() const
	{
		return handle;
	}

	const VkDescriptorSet* DescriptorSet::Address() const
	{
		return &handle;
	}

	void DescriptorSet::Write(ArrayRef<const VkDescriptorImageInfo> descriptorInfos,
		VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement) const
	{
		VkWriteDescriptorSet writeDescriptorSet = {
			.dstSet = handle,
			.dstBinding = dstBinding,
			.dstArrayElement = dstArrayElement,
			.descriptorCount = static_cast<uint32_t>(descriptorInfos.size()),
			.descriptorType = descriptorType,
			.pImageInfo = descriptorInfos.data()
		};
		Update(writeDescriptorSet);
	}

	void DescriptorSet::Write(ArrayRef<const VkDescriptorBufferInfo> descriptorInfos,
		VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement) const {
		VkWriteDescriptorSet writeDescriptorSet = {
			.dstSet = handle,
			.dstBinding = dstBinding,
			.dstArrayElement = dstArrayElement,
			.descriptorCount = static_cast<uint32_t>(descriptorInfos.size()),
			.descriptorType = descriptorType,
			.pBufferInfo = descriptorInfos.data()
		};
		Update(writeDescriptorSet);
	}

	void DescriptorSet::Write(ArrayRef<const VkBufferView> descriptorInfos,
		VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement) const {
		VkWriteDescriptorSet writeDescriptorSet = {
			.dstSet = handle,
			.dstBinding = dstBinding,
			.dstArrayElement = dstArrayElement,
			.descriptorCount = static_cast<uint32_t>(descriptorInfos.size()),
			.descriptorType = descriptorType,
			.pTexelBufferView = descriptorInfos.data()
		};
		Update(writeDescriptorSet);
	}

	void DescriptorSet::Write(ArrayRef<const BufferView> descriptorInfos,
		VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement) const {
		Write({descriptorInfos[0].Address(), descriptorInfos.size()}, descriptorType, dstBinding, dstArrayElement);
	}

	void DescriptorSet::Update(ArrayRef<VkWriteDescriptorSet> write_infos, ArrayRef<VkCopyDescriptorSet> copy_infos)
	{
		for (auto& info : write_infos)
			info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		for (auto& info : copy_infos)
			info.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
		vkUpdateDescriptorSets(VulkanBase::Base().Device(), static_cast<uint32_t>(write_infos.size()), write_infos.data(), static_cast<uint32_t>(copy_infos.size()), copy_infos.data());
	}

#pragma endregion

#pragma region DescriptorPool

	DescriptorPool::DescriptorPool(VkDescriptorPoolCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	DescriptorPool::DescriptorPool(uint32_t maxSets, ArrayRef<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags)
	{
		Create(maxSets, poolSizes, flags);
	}

	DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	DescriptorPool::~DescriptorPool()
	{
		if (handle) {
			vkDestroyDescriptorPool(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	DescriptorPool::operator VkDescriptorPool() const
	{
		return handle;
	}

	const VkDescriptorPool* DescriptorPool::Address() const
	{
		return &handle;
	}

	void DescriptorPool::AllocateDescriptorSets(ArrayRef<VkDescriptorSet> descriptorSets, ArrayRef<const VkDescriptorSetLayout> setLayouts) const
	{
		if (descriptorSets.size() != setLayouts.size()) {
			if (descriptorSets.size() > descriptorSets.size())
				throw std::runtime_error("For each descriptor set, must provide a corresponding layout!");
			else
				std::cout << "[ DescriptorPool ] WARNING \n Provided layouts are more than sets!\n";
		}

		VkDescriptorSetAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = handle,
			.descriptorSetCount = static_cast<uint32_t>(descriptorSets.size()),
			.pSetLayouts = setLayouts.data()
		};

		if (vkAllocateDescriptorSets(VulkanBase::Base().Device(), &allocateInfo, descriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	void DescriptorPool::AllocateDescriptorSets(ArrayRef<DescriptorSet> descriptorSets, ArrayRef<const VkDescriptorSetLayout> setLayouts) const
	{
		AllocateDescriptorSets({ &descriptorSets[0].handle, descriptorSets.size() }, setLayouts);
	}

	void DescriptorPool::AllocateDescriptorSets(ArrayRef<VkDescriptorSet> descriptorSets, ArrayRef<const DescriptorSetLayout> setLayouts) const
	{
		AllocateDescriptorSets(descriptorSets, { setLayouts[0].Address(), setLayouts.size() });
	}

	void DescriptorPool::AllocateDescriptorSets(ArrayRef<DescriptorSet> descriptorSets, ArrayRef<const DescriptorSetLayout> setLayouts) const
	{
		AllocateDescriptorSets({ &descriptorSets[0].handle, descriptorSets.size() }, { setLayouts[0].Address(), setLayouts.size() });
	}

	void DescriptorPool::FreeDescriptorSets(ArrayRef<VkDescriptorSet> descriptorSets) const
	{
		vkFreeDescriptorSets(VulkanBase::Base().Device(), handle, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
		memset(descriptorSets.data(), 0, descriptorSets.size() * sizeof(VkDescriptorSet));
	}

	void DescriptorPool::FreeDescriptorSets(ArrayRef<DescriptorSet> descriptorSets) const
	{
		FreeDescriptorSets({&descriptorSets[0].handle, descriptorSets.size()});
	}

	void DescriptorPool::Create(VkDescriptorPoolCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		if (vkCreateDescriptorPool(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a descriptor pool.");
	}

	void DescriptorPool::Create(uint32_t maxSets, ArrayRef<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags)
	{
		VkDescriptorPoolCreateInfo createInfo = {
			.flags = flags,
			.maxSets = maxSets,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data(),
		};
		Create(createInfo);
	}

#pragma endregion

#pragma region DescriptorManager

	std::pair<int, std::span<DescriptorPool>> DescriptorManager::RecreateDescriptorPool(int id, VkDescriptorPoolCreateInfo& createInfo)
	{
		if (auto it = mDescriptorPools.find(id); it != mDescriptorPools.end()) {
			it->second.~DescriptorPool();
			it->second.Create(createInfo);
			return { id, std::span<DescriptorPool>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] DescriptorManager: DescriptorPool with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<DescriptorSetLayout>> DescriptorManager::RecreateDescriptorSetLayout(int id, VkDescriptorSetLayoutCreateInfo& createInfo)
	{
		if (auto it = mDescriptorSetLayouts.find(id); it != mDescriptorSetLayouts.end()) {
			it->second.~DescriptorSetLayout();
			it->second.Create(createInfo);
			return { id, std::span<DescriptorSetLayout>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] DescriptorManager: DescriptorSetLayout with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<DescriptorPool>> DescriptorManager::CreateDescriptorPool(std::string name, VkDescriptorPoolCreateInfo& createInfo)
	{
		if (auto it = mDescriptorPoolIDs.find(name); it != mDescriptorPoolIDs.end())
			return RecreateDescriptorPool(it->second, createInfo);

		const int id = m_descriptor_pool_id++;

		auto [it1, ok1] = mDescriptorPools.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] DescriptorManager: Emplace descriptorPool for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mDescriptorPoolIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] DescriptorPassManager: DescriptorPool '{}' has not been recorded!\n", name);
			mDescriptorPools.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<DescriptorPool>(&vec, 1) };
	}

	std::pair<int, std::span<DescriptorPool>> DescriptorManager::GetDescriptorPool(std::string name)
	{
		if (auto it = mDescriptorPoolIDs.find(name); it != mDescriptorPoolIDs.end())
			return GetDescriptorPool(it->second);
		std::cerr << std::format("[ERROR] DescriptorManager: DescriptorPool with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<DescriptorPool>> DescriptorManager::GetDescriptorPool(int id)
	{
		if (auto it = mDescriptorPools.find(id); it != mDescriptorPools.end())
			return { id, std::span<DescriptorPool>(&it->second, 1) };
		std::cerr << std::format("[ERROR] DescriptorManager: DescriptorPool with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool DescriptorManager::HasDescriptorPool(std::string name)
	{
		if (auto it = mDescriptorPoolIDs.find(name); it != mDescriptorPoolIDs.end())
			return HasDescriptorPool(it->second);
		return false;
	}

	bool DescriptorManager::HasDescriptorPool(int id)
	{
		return mDescriptorPools.contains(id);
	}

	size_t DescriptorManager::GetDescriptorPoolCount() const
	{
		return mDescriptorPools.size();
	}

	std::pair<int, std::span<DescriptorSetLayout>> DescriptorManager::CreateDescriptorSetLayout(std::string name, VkDescriptorSetLayoutCreateInfo& createInfo)
	{
		if (auto it = mDescriptorSetLayoutIDs.find(name); it != mDescriptorSetLayoutIDs.end())
			return RecreateDescriptorSetLayout(it->second, createInfo);

		const int id = m_descriptor_pool_id++;

		auto [it1, ok1] = mDescriptorSetLayouts.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] DescriptorManager: Emplace descriptorSetLayout for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mDescriptorSetLayoutIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] DescriptorPassManager: DescriptorSetLayout '{}' has not been recorded!\n", name);
			mDescriptorSetLayouts.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<DescriptorSetLayout>(&vec, 1) };
	}

	std::pair<int, std::span<DescriptorSetLayout>> DescriptorManager::GetDescriptorSetLayout(std::string name)
	{
		if (auto it = mDescriptorSetLayoutIDs.find(name); it != mDescriptorSetLayoutIDs.end())
			return GetDescriptorSetLayout(it->second);
		std::cerr << std::format("[ERROR] DescriptorManager: DescriptorSetLayout with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<DescriptorSetLayout>> DescriptorManager::GetDescriptorSetLayout(int id)
	{
		if (auto it = mDescriptorSetLayouts.find(id); it != mDescriptorSetLayouts.end())
			return { id, std::span<DescriptorSetLayout>(&it->second, 1) };
		std::cerr << std::format("[ERROR] DescriptorManager: DescriptorSetLayout with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool DescriptorManager::HasDescriptorSetLayout(std::string name)
	{
		if (auto it = mDescriptorSetLayoutIDs.find(name); it != mDescriptorSetLayoutIDs.end())
			return HasDescriptorSetLayout(it->second);
		return false;
	}

	bool DescriptorManager::HasDescriptorSetLayout(int id)
	{
		return false;
	}

	size_t DescriptorManager::GetDescriptorSetLayoutCount() const
	{
		return mDescriptorSetLayouts.size();
	}

#pragma endregion

}