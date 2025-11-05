#include"Base/SamplerManager.h"

namespace HoshioEngine {
#pragma region Sampler

	Sampler::Sampler(VkSamplerCreateInfo& createInfo)
	{
		Create(createInfo);
	}
	Sampler::Sampler(Sampler&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}
	Sampler::~Sampler() noexcept
	{
		if (handle) {
			vkDestroySampler(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}
	Sampler::operator VkSampler() const
	{
		return handle;
	}
	const VkSampler* Sampler::Address() const
	{
		return &handle;
	}
	void Sampler::Create(VkSamplerCreateInfo& createInfo)
	{
		if (vkCreateSampler(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a sampler.");
	}
	VkSamplerCreateInfo& Sampler::SamplerCreateInfo(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode)
	{
		static VkSamplerCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = VulkanBase::Base().PhysicalDeviceProperties().limits.maxSamplerAnisotropy,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.f,
			.maxLod = VK_LOD_CLAMP_NONE,
			.borderColor = {},
			.unnormalizedCoordinates = VK_FALSE
		};

		createInfo.magFilter = magFilter;
		createInfo.minFilter = minFilter;
		createInfo.mipmapMode = mipmapMode;

		return createInfo;
	}

#pragma endregion

#pragma region SamplerManager

	std::pair<int, std::span<Sampler>> SamplerManager::RecreateSampler(int id, VkSamplerCreateInfo& createInfo)
	{
		if (auto it = mSamplers.find(id); it != mSamplers.end()) {
			it->second.~Sampler();
			it->second.Create(createInfo);
			return { id, std::span<Sampler>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] SamplerManager: Sampler with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}
	std::pair<int, std::span<Sampler>> SamplerManager::CreateSampler(std::string name, VkSamplerCreateInfo& createInfo)
	{
		if (auto it = mSamplerIDs.find(name); it != mSamplerIDs.end())
			return RecreateSampler(it->second, createInfo);

		const int id = m_sampler_id++;

		auto [it1, ok1] = mSamplers.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] SamplerManager: Emplace sampler for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mSamplerIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] SamplerManager: Sampler '{}' has not been recorded!\n", name);
			mSamplers.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Sampler>(&vec, 1) };
	}

	std::pair<int, std::span<Sampler>> SamplerManager::GetSampler(std::string name)
	{
		if (auto it = mSamplerIDs.find(name); it != mSamplerIDs.end())
			return GetSampler(it->second);
		std::cerr << std::format("[ERROR] SamplerManager: Sampler with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Sampler>> SamplerManager::GetSampler(int id)
	{
		if (auto it = mSamplers.find(id); it != mSamplers.end())
			return { id, std::span<Sampler>(&it->second, 1) };
		std::cerr << std::format("[ERROR] SamplerManager: Sampler with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}
	bool SamplerManager::HasSampler(std::string name)
	{
		if (auto it = mSamplerIDs.find(name); it != mSamplerIDs.end())
			return HasSampler(it->second);
		return false;
	}
	bool SamplerManager::HasSampler(int id)
	{
		return mSamplers.contains(id);
	}
	size_t SamplerManager::GetSamplerCount() const
	{
		return mSamplers.size();
	}

#pragma endregion

}