#pragma once
#include "halley/core/graphics/painter.h"
#include "dx11_buffer.h"
#include <map>

#include "dx11_rasterizer.h"
#include "dx11_blend.h"
#include "dx11_depth_stencil.h"
#include "halley/data_structures/hash_map.h"

namespace Halley
{
	class MaterialPass;
	class DX11Video;
	class DX11Blend;
	class DX11Rasterizer;
	class DX11RasterizerOptions;
	class DX11DepthStencil;

	class DX11Painter final : public Painter
	{
	public:
		explicit DX11Painter(DX11Video& video, Resources& resources);

		void resetState() override;
		
		void doClear(std::optional<Colour> colour, std::optional<float> depth, std::optional<uint8_t> stencil) override;
		void setMaterialPass(const Material& material, int pass) override;
		void setMaterialData(const Material& material) override;

		void doStartRender() override;
		void doEndRender() override;

		void setVertices(const MaterialDefinition& material, size_t numVertices, const void* vertexData, size_t numIndices, const IndexType* indices, bool standardQuadsOnly) override;
		void drawTriangles(size_t numIndices) override;
		void setViewPort(Rect4i rect) override;
		void setClip(Rect4i clip, bool enable) override;

		void onUpdateProjection(Material& material, bool hashChanged) override;

	protected:
		void onFinishRender() override;
		bool startPerformanceMeasurement() override;
		void endPerformanceMeasurement() override;
		void doRecordTimestamp(TimestampType type, size_t id, ITimestampRecorder* snapshot) override;
		void doCheckEndOfPerformanceMeasurement();

	private:
		DX11Video& dx11Video;

		Vector<DX11Buffer> vertexBuffers;
		Vector<DX11Buffer> indexBuffers;
		ID3D11InputLayout* layout = nullptr;
		std::map<BlendType, DX11Blend> blendModes;

		std::map<DX11RasterizerOptions, std::unique_ptr<DX11Rasterizer>> rasterizers;
		DX11Rasterizer* curRaster = nullptr;

		HashMap<MaterialDepthStencil, std::unique_ptr<DX11DepthStencil>> depthStencils;
		DX11DepthStencil* curDepthStencil = nullptr;

		size_t curBuffer = 0;
		std::optional<Rect4i> clipping;
		Vector<int> renderTargetTextureUnits;

		struct PerfQuery {
			TimestampType type;
			size_t id;
			ITimestampRecorder* snapshot;
			ID3D11Query* query = nullptr;
		};
		ID3D11Query* timestampDisjointQuery = nullptr;
		Vector<PerfQuery> perfQueries;

		DX11Blend& getBlendMode(BlendType type);
		void rotateBuffers();

		DX11Rasterizer& getRasterizer(const DX11RasterizerOptions& options);
		void setRasterizer(const MaterialPass& pass);
		void setRasterizer(const DX11RasterizerOptions& options);

		DX11DepthStencil& getDepthStencil(const MaterialDepthStencil& depthStencil);
		void setDepthStencil(const MaterialDepthStencil& depthStencil);

		void unbindRenderTargetTextureUnits(size_t lastIndex, int minimumTextureUnit);
	};
}
