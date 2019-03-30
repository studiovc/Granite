/* Copyright (c) 2017-2019 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "math.hpp"
#include "bitmap_to_mesh.hpp"
#include "mesh_util.hpp"
#include "texture_files.hpp"
#include "gltf_export.hpp"
#include <string.h>

using namespace Granite;

int main()
{
	Global::init();
	auto dino = load_texture_from_file("/tmp/dino_down.png");
	if (dino.empty())
		return 1;

	unsigned width = dino.get_layout().get_width();
	unsigned height = dino.get_layout().get_height();
	float inv_width = 1.0f / width;
	float inv_height = 1.0f / height;

	VoxelizedBitmap bitmap;
	if (!voxelize_bitmap(bitmap,
	                     dino.get_layout().data_2d<u8vec4>(0, 0, 0, 0)->data, 3, 4,
	                     width, height, width * 4))
		return 1;

	const bool flip_winding = true;
	if (flip_winding)
	{
		for (size_t i = 0; i < bitmap.indices.size(); i += 3)
			std::swap(bitmap.indices[i + 1], bitmap.indices[i + 2]);
		for (auto &n : bitmap.normals)
			n = -n;
	}

	struct Attr
	{
		vec3 normal;
		vec2 uv;
	};
	std::vector<Attr> attrs;
	attrs.reserve(bitmap.normals.size());

	for (size_t i = 0; i < bitmap.normals.size(); i++)
		attrs.push_back({ bitmap.normals[i], vec2(bitmap.positions[i].x * inv_width, bitmap.positions[i].z * inv_height) });

	const float y_scale = 4.0f;
	for (auto &pos : bitmap.positions)
		pos.y *= y_scale;

	SceneFormats::Mesh m;
	m.indices.resize(bitmap.indices.size() * sizeof(uint32_t));
	memcpy(m.indices.data(), bitmap.indices.data(), m.indices.size());
	m.positions.resize(bitmap.positions.size() * sizeof(vec3));
	memcpy(m.positions.data(), bitmap.positions.data(), m.positions.size());
	m.attributes.resize(attrs.size() * sizeof(Attr));
	memcpy(m.attributes.data(), attrs.data(), m.attributes.size());
	m.position_stride = sizeof(vec3);
	m.attribute_stride = sizeof(Attr);
	m.attribute_layout[Util::ecast(MeshAttribute::Position)].format = VK_FORMAT_R32G32B32_SFLOAT;
	m.attribute_layout[Util::ecast(MeshAttribute::Normal)].format = VK_FORMAT_R32G32B32_SFLOAT;
	m.attribute_layout[Util::ecast(MeshAttribute::Normal)].offset = offsetof(Attr, normal);
	m.attribute_layout[Util::ecast(MeshAttribute::UV)].format = VK_FORMAT_R32G32_SFLOAT;
	m.attribute_layout[Util::ecast(MeshAttribute::UV)].offset = offsetof(Attr, uv);

	m.index_type = VK_INDEX_TYPE_UINT32;
	m.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m.count = bitmap.indices.size();
	m.has_material = true;
	m.material_index = 0;
	m.static_aabb = AABB(vec3(0.0f, -0.5f * y_scale, 0.0f), vec3(width, 0.5f * y_scale, height));

	SceneFormats::MaterialInfo mat;
	mat.bandlimited_pixel = true;
	mat.base_color.path = "/tmp/dino_down.png";
	mat.uniform_metallic = 0.0f;
	mat.uniform_roughness = 1.0f;
	mat.pipeline = DrawPipeline::Opaque;

	SceneFormats::SceneInformation scene;
	SceneFormats::ExportOptions options;

	SceneFormats::Node n;
	n.meshes.push_back(0);

	scene.materials = { &mat, 1 };
	scene.meshes = { &m, 1 };
	scene.nodes = { &n, 1 };
	options.quantize_attributes = true;
	options.optimize_meshes = true;
	SceneFormats::export_scene_to_glb(scene, "/tmp/test.glb", options);
	Global::deinit();
}
