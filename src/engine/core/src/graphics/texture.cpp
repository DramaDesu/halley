#include "halley/core/graphics/texture.h"
#include "halley/core/api/halley_api.h"
#include "halley/core/graphics/texture_descriptor.h"
#include <halley/file_formats/image.h>
#include <halley/resources/metadata.h>
#include "halley/concurrency/concurrent.h"
#include "halley/support/logger.h"

using namespace Halley;

Texture::Texture(Vector2i size)
	: size(size)
{}

void Texture::load(TextureDescriptor desc)
{
	descriptor = std::move(desc);
	doLoad(descriptor);

	if (!descriptor.retainPixelData) {
		descriptor.pixelData = TextureDescriptorImageData();
	}
}

std::optional<uint32_t> Texture::getPixel(Vector2f texPos) const
{
	if (descriptor.retainPixelData) {
		const auto* img = descriptor.pixelData.getImage();
		Vector2i pos = Vector2i((texPos * Vector2f(size)).round());
		pos = Vector2i::max(Vector2i(), Vector2i::min(size - Vector2i(1, 1), pos)); // Clamp mode
		
		if (img) {
			if (img->getFormat() == Image::Format::RGBA || img->getFormat() == Image::Format::RGBAPremultiplied) {
				return img->getPixel4BPP(pos);
			}
		}
	}

	return {};
}

void Texture::copyToTexture(Painter& painter, Texture& other) const
{
	if (getSize() != other.getSize()) {
		throw Exception("Size of source and destination texture don't match.", HalleyExceptions::Graphics);
	}
	doCopyToTexture(painter, other);
}

void Texture::copyToImage(Painter& painter, Image& image) const
{
	if (image.getSize() != getSize()) {
		throw Exception("Incompatible image and texture sizes.", HalleyExceptions::Graphics);
	}
	doCopyToImage(painter, image);
}

std::unique_ptr<Image> Texture::makeImage(Painter& painter) const
{
	auto image = std::make_unique<Image>(Image::Format::RGBA, getSize());
	doCopyToImage(painter, *image);
	return image;
}

void Texture::doLoad(TextureDescriptor& descriptor)
{
}

void Texture::doCopyToTexture(Painter& painter, Texture& other) const
{
	Logger::logWarning("Copying to texture not implemented.");
}

void Texture::doCopyToImage(Painter& painter, Image& image) const
{
	Logger::logWarning("Copying to image not implemented.");
	image.clear(Image::convertRGBAToInt(255, 0, 255));
}

std::shared_ptr<Texture> Texture::loadResource(ResourceLoader& loader)
{
	const auto& meta = loader.getMeta();

	Vector2i size(meta.getInt("width", -1), meta.getInt("height", -1));
	if (size.x == -1 && size.y == -1) {
		return {};
	}

	std::shared_ptr<Texture> texture = loader.getAPI().video->createTexture(size);
	texture->setMeta(meta);
	bool retain = loader.getResources().getOptions().retainPixelData;

	loader.getAsync(true)
	.then([texture](std::unique_ptr<ResourceDataStatic> data) -> TextureDescriptorImageData
	{
		auto& meta = texture->getMeta();
		if (meta.getString("compression") == "png") {
			return TextureDescriptorImageData(std::make_unique<Image>(*data, meta));
		} else {
			return TextureDescriptorImageData(data->getSpan());
		}
	})
	.then(Executors::getVideoAux(), [texture, retain](TextureDescriptorImageData img)
	{
		auto& meta = texture->getMeta();

		const auto imgFormat = fromString<Image::Format>(meta.getString("format", "rgba"));
		TextureFormat format = TextureFormat::RGBA;
		switch (imgFormat) {
		case Image::Format::Indexed:
			format = TextureFormat::Indexed;
			break;
		case Image::Format::RGB:
			format = TextureFormat::RGB;
			break;
		case Image::Format::RGBA:
		case Image::Format::RGBAPremultiplied:
			format = TextureFormat::RGBA;
			break;
		case Image::Format::SingleChannel:
			format = TextureFormat::Red;
			break;
		case Image::Format::Undefined:
			format = TextureFormat::RGBA; // Hmm
		}

		Vector2i size(meta.getInt("width"), meta.getInt("height"));
		TextureDescriptor descriptor(size);
		descriptor.useFiltering = meta.getBool("filtering", false);
		descriptor.useMipMap = meta.getBool("mipmap", false);
		descriptor.addressMode = fromString<TextureAddressMode>(meta.getString("addressMode", "clamp"));
		descriptor.format = format;
		descriptor.pixelData = std::move(img);
		descriptor.pixelFormat = meta.getString("compression") == "png" ? PixelDataFormat::Image : PixelDataFormat::Precompiled;
		descriptor.retainPixelData = retain;
		texture->load(std::move(descriptor));
	});

	return texture;
}
