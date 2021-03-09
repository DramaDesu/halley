#include "dx11_render_target_texture.h"
#include "dx11_video.h"
#include "dx11_texture.h"
using namespace Halley;

DX11TextureRenderTarget::DX11TextureRenderTarget(DX11Video& video)
	: video(video)
{
}

DX11TextureRenderTarget::~DX11TextureRenderTarget()
{
	clear();
}

bool DX11TextureRenderTarget::getProjectionFlipVertical() const
{
	return true;
}

bool DX11TextureRenderTarget::getViewportFlipVertical() const
{
	return false;
}

void DX11TextureRenderTarget::onBind(Painter& painter)
{
	update();
	video.getDeviceContext().OMSetRenderTargets(UINT(views.size()), views.data(), depthStencilView);
}

ID3D11RenderTargetView* DX11TextureRenderTarget::getRenderTargetView()
{
	update();
	return views.at(0);
}

ID3D11DepthStencilView* DX11TextureRenderTarget::getDepthStencilView()
{
	return depthStencilView;
}

void DX11TextureRenderTarget::update()
{
	if (dirty) {
		clear();
		createViews();
		dirty = false;
	}
}

void DX11TextureRenderTarget::clear()
{
	for (auto& view: views) {
		view->Release();
	}
	views.clear();

	if (depthStencilView) {
		depthStencilView->Release();
		depthStencilView = nullptr;
	}
}

void DX11TextureRenderTarget::createViews()
{
	views.resize(colourBuffer.size());
	for (size_t i = 0; i < colourBuffer.size(); ++i) {
		auto& texture = static_cast<DX11Texture&>(*colourBuffer[i]);
		texture.waitForLoad();

		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = texture.getFormat();
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto result = video.getDevice().CreateRenderTargetView(texture.getTexture(), &desc, &views[i]);
		if (result != S_OK) {
			throw Exception("Unable to create render target view for texture attachment #" + toString(i), HalleyExceptions::VideoPlugin);
		}
	}

	if (depthStencilBuffer) {
		auto& depthTexture = static_cast<DX11Texture&>(*depthStencilBuffer);
		depthTexture.waitForLoad();

		auto format = depthTexture.getFormat();
		if (format == DXGI_FORMAT_R24G8_TYPELESS) {
			format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Flags = 0;
		desc.Texture2D.MipSlice = 0;

		auto result = video.getDevice().CreateDepthStencilView(depthTexture.getTexture(), &desc, &depthStencilView);
		if (result != S_OK) {
			throw Exception("Unable to create render target view for depth texture.", HalleyExceptions::VideoPlugin);
		}
	}
}
