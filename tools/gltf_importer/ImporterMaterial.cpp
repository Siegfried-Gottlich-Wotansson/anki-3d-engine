// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

namespace anki
{

const char* MATERIAL_TEMPLATE = R"(<?xml version="1.0" encoding="UTF-8" ?>
<!-- This file is auto generated by ImporterMaterial.cpp -->
<material shaderProgram="shaders/GBufferGeneric.glslp">
	<mutators>
		<mutator name="DIFFUSE_TEX" value="%diffTexMutator%"/>
		<mutator name="SPECULAR_TEX" value="%specTexMutator%"/>
		<mutator name="ROUGHNESS_TEX" value="%roughnessTexMutator%"/>
		<mutator name="METAL_TEX" value="%metalTexMutator%"/>
		<mutator name="NORMAL_TEX" value="%normalTexMutator%"/>
		<mutator name="PARALLAX" value="%parallaxMutator%"/>
		<mutator name="EMISSIVE_TEX" value="%emissiveTexMutator%"/>
	</mutators>

	<inputs>
		<input shaderInput="mvp" builtin="MODEL_VIEW_PROJECTION_MATRIX"/>
		<input shaderInput="prevMvp" builtin="PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX"/>
		<input shaderInput="rotationMat" builtin="ROTATION_MATRIX"/>
		<input shaderInput="globalSampler" builtin="GLOBAL_SAMPLER"/>
		%parallaxInput%

		%diff%
		%spec%
		%roughness%
		%metallic%
		%normal%
		%emission%
		%subsurface%
		%height%
	</inputs>
</material>
)";

static std::string replaceAllString(const std::string& str, const std::string& from, const std::string& to)
{
	if(from.empty())
	{
		return str;
	}

	std::string out = str;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != std::string::npos)
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return out;
}

static CString getTextureUri(const cgltf_texture_view& view)
{
	ANKI_ASSERT(view.texture);
	ANKI_ASSERT(view.texture->image);
	ANKI_ASSERT(view.texture->image->uri);
	return view.texture->image->uri;
}

Error Importer::writeMaterial(const cgltf_material& mtl)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankimtl", m_outDir.cstr(), mtl.name);
	ANKI_GLTF_LOGI("Exporting %s", fname.cstr());

	if(!mtl.has_pbr_metallic_roughness)
	{
		ANKI_GLTF_LOGE("Expecting PBR metallic roughness");
		return Error::USER_DATA;
	}

	std::string xml = MATERIAL_TEMPLATE;

	// Diffuse
	if(mtl.pbr_metallic_roughness.base_color_texture.texture)
	{
		StringAuto uri(m_alloc);
		uri.sprintf("%s%s", "TODO", getTextureUri(mtl.pbr_metallic_roughness.base_color_texture).cstr());

		xml = replaceAllString(
			xml, "%diff%", "<input shaderInput=\"diffTex\" value=\"" + std::string(uri.cstr()) + "\"/>");
		xml = replaceAllString(xml, "%diffTexMutator%", "1");
	}
	else
	{
		const F32* diffCol = &mtl.pbr_metallic_roughness.base_color_factor[0];

		xml = replaceAllString(xml,
			"%diff%",
			"<input shaderInput=\"diffColor\" value=\"" + std::to_string(diffCol[0]) + " " + std::to_string(diffCol[1])
				+ " " + std::to_string(diffCol[2]) + "\"/>");

		xml = replaceAllString(xml, "%diffTexMutator%", "0");
	}

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));
	ANKI_CHECK(file.writeText("%s", xml.c_str()));

	return Error::NONE;
}

} // end namespace anki