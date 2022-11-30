#pragma once

#include "Lamp/Rendering/Texture/Image2D.h"

namespace Lamp
{
	class TextureCube
	{
	public:
		TextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data);
		TextureCube() = default;
		~TextureCube();

		void SetData(const void* data, uint32_t size);

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;

		inline const Ref<Image2D> GetImage() const { return m_image; }
		static Ref<TextureCube> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);

	private:
		friend class DefaultTextureImporter;
		friend class DDSTextureImporter;

		Ref<Image2D> m_image;
	};
}