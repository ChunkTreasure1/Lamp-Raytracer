#pragma once

#include "Lamp/Rendering/Texture/ImageCommon.h"
#include "Lamp/Core/Base.h"

#include <vulkan/vulkan.h>

#include <filesystem>
#include <map>
#include <unordered_map>

namespace Lamp
{
	// Descriptor sets:
	// 0 - Per frame
	// 1 - Per pass -- Uniform Buffers are dynamic
	// 2 - Per object -- Unused for now
	// 3 - Per material -- All textures for now
	// 4 - Mesh data -- Shader Buffers are dynamic

	enum class DescriptorSetType : uint32_t
	{
		PerFrame = 0,
		PerPass = 1,
		PerObject = 2,
		PerMaterial = 3
	};

	class Shader
	{
	public:
		enum class Language
		{
			Invalid,
			GLSL,
			HLSL
		};

		struct ShaderStorageBuffer
		{
			VkDescriptorBufferInfo info{};
			bool writeable = true;
			bool isDynamic = false;
		};

		struct UniformBuffer
		{
			VkDescriptorBufferInfo info{};
			bool isDynamic = false;
		};

		struct StorageImage
		{
			VkDescriptorImageInfo info{};
			bool writeable = true;
		};

		struct SampledImage
		{
			VkDescriptorImageInfo info{};
			ImageDimension dimension{};
		};

		struct DynamicOffset
		{
			uint32_t offset;
			uint32_t binding;
		};

		struct ShaderResources
		{
			std::unordered_map<uint32_t, std::string> shaderTextureDefinitions; // binding -> name

			std::vector<VkDescriptorSetLayout> paddedSetLayouts;
			std::vector<VkDescriptorSetLayout> realSetLayouts;

			std::vector<VkPushConstantRange> pushConstantRanges;
			std::vector<VkDescriptorPoolSize> poolSizes;
			
			std::map<uint32_t, std::map<uint32_t, UniformBuffer>> uniformBuffersInfos; // set -> binding -> infos
			std::map<uint32_t, std::map<uint32_t, ShaderStorageBuffer>> storageBuffersInfos; // set -> binding -> infos
			std::map<uint32_t, std::map<uint32_t, StorageImage>> storageImagesInfos; // set -> binding -> infos
			std::map<uint32_t, std::map<uint32_t, SampledImage>> imageInfos; // set -> binding -> infos
			std::map<uint32_t, std::map<uint32_t, VkWriteDescriptorSet>> writeDescriptors; // set -> binding -> write

			std::map<uint32_t, std::vector<DynamicOffset>> dynamicBufferOffsets; // set -> offsets

			VkDescriptorSetAllocateInfo setAllocInfo{};
			
			void Clear();
		};

		Shader(const std::string& name, std::initializer_list<std::filesystem::path> paths, bool forceCompile);
		Shader(const std::string& name, std::vector<std::filesystem::path> paths, bool forceCompile);
		Shader() = default;
		~Shader();

		bool Reload(bool forceCompile);
	
		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetStageInfos() const { return m_pipelineShaderStageInfos; }
		inline const ShaderResources& GetResources() const { return m_resources; }
		inline const std::string& GetName() const { return m_name; }
		inline const size_t GetHash() const { return m_hash; }

		static Ref<Shader> Create(const std::string& name, std::initializer_list<std::filesystem::path> paths, bool forceCompile = false);
		static Ref<Shader> Create(const std::string& name, std::vector<std::filesystem::path> paths, bool forceCompile = false);
	
	private:
		struct TypeCount
		{
			uint32_t count = 0;
		};

		void GenerateHash();
		void LoadShaderFromFiles();
		void Release();

		bool CompileOrGetBinary(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData, bool forceCompile);
		void LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void ReflectAllStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void ReflectStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& data, std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>& outSetLayoutBindings);
		
		void SetupDescriptors(const std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>& setLayoutBindings);

		std::unordered_map<VkShaderStageFlagBits, std::string> m_shaderSources;
		std::vector<VkPipelineShaderStageCreateInfo> m_pipelineShaderStageInfos;
		std::vector<std::filesystem::path> m_shaderPaths;

		ShaderResources m_resources;
		std::string m_name;
		size_t m_hash{};

		std::unordered_map<VkShaderStageFlagBits, TypeCount> m_perStageUBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> m_perStageDynamicUBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> m_perStageSSBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> m_perStageDynamicSSBOCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> m_perStageStorageImageCount;
		std::unordered_map<VkShaderStageFlagBits, TypeCount> m_perStageImageCount;
	};
}