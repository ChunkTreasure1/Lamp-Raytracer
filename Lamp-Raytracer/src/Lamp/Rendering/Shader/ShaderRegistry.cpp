#include "lppch.h"
#include "ShaderRegistry.h"

#include "Lamp/Log/Log.h"

#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Utility/FileSystem.h"
#include "Lamp/Utility/StringUtility.h"

namespace Lamp
{
	void ShaderRegistry::Initialize()
	{
		LoadAllShaders();
	}

	void ShaderRegistry::Shutdown()
	{
		s_registry.clear();
	}

	Ref<Shader> ShaderRegistry::Get(const std::string& name)
	{
		std::string lowName = Utility::ToLower(name);
		auto it = s_registry.find(lowName);
		if (it == s_registry.end())
		{
			LP_CORE_ERROR("Unable to find shader {0}!", name.c_str());
			return nullptr;
		}

		return it->second;
	}

	void ShaderRegistry::Register(const std::string& name, Ref<Shader> shader)
	{
		auto it = s_registry.find(name);
		if (it != s_registry.end())
		{
			LP_CORE_ERROR("Shader with that name has already been registered!");
			return;
		}

		std::string lowName = Utility::ToLower(name);
		s_registry[lowName] = shader;
	}

	std::map<std::string, Ref<Shader>> ShaderRegistry::GetAllShaders()
	{
		return s_registry;
	}

	void ShaderRegistry::LoadAllShaders()
	{
	}
}