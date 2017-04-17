#pragma once
#include "api/halley_api_internal.h"
#include "graphics/texture.h"
#include "graphics/render_target/render_target_texture.h"
#include "graphics/shader.h"
#include "graphics/painter.h"

namespace Halley {
	class DummyVideoAPI : public VideoAPIInternal {
	public:
		explicit DummyVideoAPI(SystemAPI& system);

		void startRender() override;
		void finishRender() override;
		void setWindow(WindowDefinition&& windowDescriptor, bool vsync) override;
		const Window& getWindow() const override;
		std::unique_ptr<Texture> createTexture(Vector2i size) override;
		std::unique_ptr<Shader> createShader(const ShaderDefinition& definition) override;
		std::unique_ptr<TextureRenderTarget> createRenderTarget() override;
		std::unique_ptr<MaterialConstantBuffer> createConstantBuffer() override;
		void init() override;
		void deInit() override;
		std::unique_ptr<Painter> makePainter() override;

	private:
		std::shared_ptr<Window> window;
	};

	class DummyTexture : public Texture
	{
	public:
		explicit DummyTexture(Vector2i size);
		void bind(int textureUnit) const override;
		void load(TextureDescriptor&& descriptor) override;
	};

	class DummyTextureRenderTarget : public TextureRenderTarget
	{
	public:
		bool isScreen() const override;
		void bind() override;
		void unbind() override;
	};

	class DummyShader : public Shader
	{
	public:
		int getUniformLocation(const String& name) override;
		int getBlockLocation(const String& name) override;
	};

	class DummyMaterialConstantBuffer : public MaterialConstantBuffer
	{
	public:
		void update(const MaterialDataBlock& dataBlock) override;
	};

	class DummyPainter : public Painter
	{
	public:
		void clear(Colour colour) override;
		void setMaterialPass(const Material& material, int pass) override;
		void doStartRender() override;
		void doEndRender() override;
		void setVertices(const MaterialDefinition& material, size_t numVertices, void* vertexData, size_t numIndices, unsigned short* indices) override;
		void drawTriangles(size_t numIndices) override;
		void setViewPort(Rect4i rect, Vector2i renderTargetSize, bool isScreen) override;
		void setClip(Rect4i clip, Vector2i renderTargetSize, bool enable, bool isScreen) override;
	};
}
