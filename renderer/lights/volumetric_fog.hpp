/* Copyright (c) 2017-2018 Hans-Kristian Arntzen
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

#pragma once

#include "lights.hpp"
#include "render_components.hpp"
#include "event.hpp"
#include "shader_manager.hpp"
#include "renderer.hpp"
#include "clusterer.hpp"

namespace Granite
{
class RenderTextureResource;
class RenderPass;

class VolumetricFog : public RenderPassCreator
{
public:
	VolumetricFog();
	void add_texture_dependency(std::string name);
	void set_resolution(unsigned width, unsigned height, unsigned depth);
	void set_z_range(float range);

	float get_slice_z_log2_scale() const;
	const Vulkan::ImageView &get_view() const;

private:
	std::vector<std::string> texture_dependencies;

	unsigned width = 160;
	unsigned height = 90;
	unsigned depth = 64;
	float z_range = 1024.0f;
	float slice_z_log2_scale;

	void add_render_passes(RenderGraph &graph) override;
	void setup_render_pass_dependencies(RenderGraph &graph, RenderPass &target) override;
	void setup_render_pass_resources(RenderGraph &graph) override;
	void set_base_renderer(Renderer *, Renderer *, Renderer *) override;
	void set_base_render_context(const RenderContext *context) override;
	void set_scene(Scene *scene) override;

	const Vulkan::ImageView *view = nullptr;
	const RenderContext *context = nullptr;
	RenderTextureResource *fog_volume = nullptr;
	RenderPass *pass = nullptr;

	void build_density(Vulkan::CommandBuffer &cmd, Vulkan::ImageView &density);
	void build_light(Vulkan::CommandBuffer &cmd, Vulkan::ImageView &light, Vulkan::ImageView &density);
	void build_fog(Vulkan::CommandBuffer &cmd, Vulkan::ImageView &fog, Vulkan::ImageView &light);

	float slice_extents[1024];
	void compute_slice_extents();
};
}