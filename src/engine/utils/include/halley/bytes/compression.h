#pragma once
#include "../utils/utils.h"
#include <gsl/gsl>
#include <limits>

namespace Halley {
	class Compression {
	public:
		static Bytes compress(const Bytes& bytes, int level = -1);
		static Bytes compress(gsl::span<const gsl::byte> bytes, int level = -1);
		static Bytes decompress(const Bytes& bytes, size_t maxSize = std::numeric_limits<size_t>::max());
		static Bytes decompress(gsl::span<const gsl::byte> bytes, size_t maxSize = std::numeric_limits<size_t>::max());
		static std::shared_ptr<const char> decompressToSharedPtr(gsl::span<const gsl::byte> bytes, size_t& outSize, size_t maxSize = std::numeric_limits<size_t>::max());
		static Bytes compressRaw(gsl::span<const gsl::byte> bytes, bool insertLength, int level = -1);
		static gsl::span<gsl::byte> compressRaw(gsl::span<const gsl::byte> inBytes, gsl::span<gsl::byte> outBytes, bool insertLength, int level = -1);
		static Bytes decompressRaw(gsl::span<const gsl::byte> bytes, size_t maxSize, size_t expectedSize = 0);

		static size_t lz4Compress(gsl::span<const gsl::byte> src, gsl::span<gsl::byte> dst);
		static size_t lz4Compress(gsl::span<const char> src, gsl::span<char> dst);
		static size_t lz4Compress(gsl::span<const Byte> src, gsl::span<Byte> dst);
		static std::optional<size_t> lz4Decompress(gsl::span<const gsl::byte> src, gsl::span<gsl::byte> dst);
		static std::optional<size_t> lz4Decompress(gsl::span<const char> src, gsl::span<char> dst);
		static std::optional<size_t> lz4Decompress(gsl::span<const Byte> src, gsl::span<Byte> dst);
	};
}
