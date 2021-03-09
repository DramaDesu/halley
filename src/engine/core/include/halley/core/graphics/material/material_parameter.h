#pragma once

#include <halley/maths/colour.h>
#include <halley/maths/vector2.h>
#include <halley/maths/vector3.h>
#include <halley/maths/vector4.h>
#include <halley/maths/matrix4.h>
#include <memory>

namespace Halley
{
	enum class TextureSamplerType : uint8_t;
	enum class ShaderType;
	class Texture;
	class VideoAPIInternal;
	class Material;
	class MaterialPass;
	enum class ShaderParameterType : uint8_t;

	class MaterialTextureParameter
	{
	public:
		MaterialTextureParameter(Material& material, const String& name, TextureSamplerType samplerType);
		unsigned int getAddress(int pass, ShaderType stage) const;
		TextureSamplerType getSamplerType() const { return samplerType; }

	private:
		String name;
		Vector<int> addresses;
		TextureSamplerType samplerType;
	};

	class MaterialParameter
	{
		friend class Material;
		friend class MaterialPass;

	public:
		bool set(Colour colour);
		bool set(float p);
		bool set(Vector2f p);
		bool set(Vector3f p);
		bool set(Vector4f p);
		bool set(int p);
		bool set(Vector2i p);
		bool set(const Matrix4f& m);

		const String& getName() const { return name; }
		ShaderParameterType getType() const { return type; }

	private:
		MaterialParameter(Material& material, String name, ShaderParameterType type, int blockNumber, size_t offset);

		void rebind(Material& material) noexcept;
		
		Material* material;
		String name;
		size_t offset;
		ShaderParameterType type;
		int blockNumber;
	};

}
