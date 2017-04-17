#pragma once
#include <halley/maths/vector2.h>
#include <halley/maths/rect.h>
#include <halley/text/halleystring.h>

namespace Halley
{
	class ShaderDefinition;
	class WindowDefinition;
	class Painter;
	class Texture;
	class TextureDescriptor;
	class TextureRenderTarget;
	class Shader;
	class Window;
	class MaterialConstantBuffer;
	class Material;

	class VideoAPI
	{
	public:
		virtual ~VideoAPI() {}

		virtual void startRender() = 0;
		virtual void finishRender() = 0;

		virtual void setWindow(WindowDefinition&& windowDescriptor, bool vsync) = 0;
		virtual const Window& getWindow() const = 0;

		virtual std::unique_ptr<Texture> createTexture(Vector2i size) = 0;
		virtual std::unique_ptr<Shader> createShader(const ShaderDefinition& definition) = 0;
		virtual std::unique_ptr<TextureRenderTarget> createRenderTarget() = 0;
		virtual std::unique_ptr<MaterialConstantBuffer> createConstantBuffer() = 0;
	};
}
