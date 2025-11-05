#ifndef _SAMPLER_MANAGER_H_
#define _SAMPLER_MANAGER_H_

#include "Base/VulkanBase.h"

namespace HoshioEngine {
	class Sampler {
	private:
		VkSampler handle = VK_NULL_HANDLE;
	public:
		Sampler() = default;
		Sampler(VkSamplerCreateInfo& createInfo);
		Sampler(Sampler&& other) noexcept;
		~Sampler() noexcept;

		operator VkSampler() const;
		const VkSampler* Address() const;

		void Create(VkSamplerCreateInfo& createInfo);

		static VkSamplerCreateInfo& SamplerCreateInfo(VkFilter magFilter = VK_FILTER_LINEAR,
			VkFilter minFilter = VK_FILTER_LINEAR,
			VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR);

	};

	class SamplerManager {
	private:
		int m_sampler_id = 0;

		std::unordered_map<std::string, int> mSamplerIDs;
		std::unordered_map<int, Sampler> mSamplers;

		std::pair<int, std::span<Sampler>> RecreateSampler(int id, VkSamplerCreateInfo& createInfo);

	public:
		std::pair<int, std::span<Sampler>> CreateSampler(std::string name, VkSamplerCreateInfo& createInfo);
		std::pair<int, std::span<Sampler>> GetSampler(std::string name);
		std::pair<int, std::span<Sampler>> GetSampler(int id);
		bool HasSampler(std::string name);
		bool HasSampler(int id);
		size_t GetSamplerCount() const;
	};

}

#endif // !_SAMPLER_MANAGER_H_
