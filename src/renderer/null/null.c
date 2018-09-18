/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include "../api.h"
#include "resource/shader_object.h"
#include "../common/backend.h"

static char placeholder;
static Color dummycolor;

SDL_Window* null_create_window(const char *title, int x, int y, int w, int h, uint32_t flags) {
	return SDL_CreateWindow(title, x, y, w, h, flags);
}

void null_init(void) { }
void null_post_init(void) { }
void null_shutdown(void) { }

bool null_supports(RendererFeature feature) {
	return true;
}

void null_capabilities(r_capability_bits_t capbits) { }
r_capability_bits_t null_capabilities_current(void) { return (r_capability_bits_t)-1; }

void null_color4(float r, float g, float b, float a) { }
const Color* null_color_current(void) { return &dummycolor; }

void null_blend(BlendMode mode) { }
BlendMode null_blend_current(void) { return BLEND_NONE; }

void null_cull(CullFaceMode mode) { }
CullFaceMode null_cull_current(void) { return CULL_BACK; }

void null_depth_func(DepthTestFunc func) { }
DepthTestFunc null_depth_func_current(void) { return DEPTH_LESS; }

bool null_shader_language_supported(const ShaderLangInfo *lang, ShaderLangInfo *out_alternative) { return true; }

ShaderObject* null_shader_object_compile(ShaderSource *source) { return (void*)&placeholder; }
void null_shader_object_destroy(ShaderObject *shobj) { }
void null_shader_object_set_debug_label(ShaderObject *shobj, const char *label) { }
const char* null_shader_object_get_debug_label(ShaderObject *shobj) { return "Null shader object"; }

ShaderProgram* null_shader_program_link(uint num_objects, ShaderObject *shobjs[num_objects]) { return (void*)&placeholder; }
void null_shader_program_destroy(ShaderProgram *prog) { }
void null_shader_program_set_debug_label(ShaderProgram *prog, const char *label) { }
const char* null_shader_program_get_debug_label(ShaderProgram *prog) { return "Null shader program"; }

void null_shader(ShaderProgram *prog) { }
ShaderProgram* null_shader_current(void) { return (void*)&placeholder; }

Uniform* null_shader_uniform(ShaderProgram *prog, const char *uniform_name) {
	return NULL;
}

void null_uniform(Uniform *uniform, uint offset, uint count, const void *data) { }

UniformType null_uniform_type(Uniform *uniform) { return UNIFORM_FLOAT; }

void null_draw(Primitive prim, uint first, uint count, uint32_t *indices, uint instances, uint base_instance) { }

Texture* null_texture_create(const TextureParams *params) {
	return (void*)&placeholder;
}

void null_texture_get_size(Texture *tex, uint mipmap, uint *width, uint *height) {
	if(width) *width = 1;
	if(height) *height = 1;
}

void null_texture_get_params(Texture *tex, TextureParams *params) {
	memset(params, 0, sizeof(*params));
	params->width = 1;
	params->height = 1;
	params->type = TEX_TYPE_RGBA;
}

void null_texture_set_debug_label(Texture *tex, const char *label) { }
const char* null_texture_get_debug_label(Texture *tex) { return "null texture"; }
void null_texture_set_filter(Texture *tex, TextureFilterMode fmin, TextureFilterMode fmag) { }
void null_texture_set_wrap(Texture *tex, TextureWrapMode fmin, TextureWrapMode fmag) { }
void null_texture_fill(Texture *tex, uint mipmap, const Pixmap *image_data) { }
void null_texture_fill_region(Texture *tex, uint mipmap, uint x, uint y, const Pixmap *image_data) { }
void null_texture_invalidate(Texture *tex) { }
void null_texture_destroy(Texture *tex) { }
void null_texture_clear(Texture *tex, const Color *color) { }

static IntRect default_fb_viewport;

Framebuffer* null_framebuffer_create(void) { return (void*)&placeholder; }
void null_framebuffer_set_debug_label(Framebuffer *fb, const char *label) { }
const char* null_framebuffer_get_debug_label(Framebuffer *fb) { return "null framebuffer"; }
void null_framebuffer_attach(Framebuffer *framebuffer, Texture *tex, uint mipmap, FramebufferAttachment attachment) { }
Texture* null_framebuffer_attachment(Framebuffer *framebuffer, FramebufferAttachment attachment) { return (void*)&placeholder; }
uint null_framebuffer_attachment_mipmap(Framebuffer *framebuffer, FramebufferAttachment attachment) { return 0; }
void null_framebuffer_destroy(Framebuffer *framebuffer) { }
void null_framebuffer_viewport(Framebuffer *framebuffer, IntRect vp) { }
void null_framebuffer_viewport_current(Framebuffer *framebuffer, IntRect *vp) { *vp = default_fb_viewport; }
void null_framebuffer(Framebuffer *framebuffer) { }
Framebuffer* null_framebuffer_current(void) { return (void*)&placeholder; }
void null_framebuffer_clear(Framebuffer *framebuffer, ClearBufferFlags flags, const Color *colorval, float depthval) { }

static int64_t null_vertex_buffer_stream_seek(SDL_RWops *rw, int64_t offset, int whence) { return 0; }
static int64_t null_vertex_buffer_stream_size(SDL_RWops *rw) { return (1 << 16); }
static size_t null_vertex_buffer_stream_write(SDL_RWops *rw, const void *data, size_t size, size_t num) { return num; }
static int null_vertex_buffer_stream_close(SDL_RWops *rw) { return 0; }

static size_t null_vertex_buffer_stream_read(SDL_RWops *rw, void *data, size_t size, size_t num) {
	SDL_SetError("Stream is write-only");
	return 0;
}

static SDL_RWops dummy_stream = {
	.seek = null_vertex_buffer_stream_seek,
	.size = null_vertex_buffer_stream_size,
	.write = null_vertex_buffer_stream_write,
	.read = null_vertex_buffer_stream_read,
	.close = null_vertex_buffer_stream_close,
};

SDL_RWops* null_vertex_buffer_get_stream(VertexBuffer *vbuf) {
	return &dummy_stream;
}

VertexBuffer* null_vertex_buffer_create(size_t capacity, void *data) { return (void*)&placeholder; }
void null_vertex_buffer_set_debug_label(VertexBuffer *vbuf, const char *label) { }
const char* null_vertex_buffer_get_debug_label(VertexBuffer *vbuf) { return "null vertex buffer"; }
void null_vertex_buffer_destroy(VertexBuffer *vbuf) { }
void null_vertex_buffer_invalidate(VertexBuffer *vbuf) { }

VertexArray* null_vertex_array_create(void) { return (void*)&placeholder; }
void null_vertex_array_set_debug_label(VertexArray *varr, const char *label) { }
const char* null_vertex_array_get_debug_label(VertexArray *varr) { return "null vertex array"; }
void null_vertex_array_destroy(VertexArray *varr) { }
void null_vertex_array_attach_buffer(VertexArray *varr, VertexBuffer *vbuf, uint attachment) { }
VertexBuffer* null_vertex_array_get_attachment(VertexArray *varr, uint attachment) { return (void*)&placeholder; }
void null_vertex_array_layout(VertexArray *varr, uint nattribs, VertexAttribFormat attribs[nattribs]) { }
void null_vertex_array(VertexArray *varr) { }
VertexArray* null_vertex_array_current(void) { return (void*)&placeholder; }

void null_clear(ClearBufferFlags flags) { }
void null_clear_color4(float r, float g, float b, float a) { }
const Color* null_clear_color_current(void) { return &dummycolor; }

void null_vsync(VsyncMode mode) { }
VsyncMode null_vsync_current(void) { return VSYNC_NONE; }

void null_swap(SDL_Window *window) { }

bool null_screenshot(Pixmap *dest) { return false; }

RendererBackend _r_backend_null = {
	.name = "null",
	.funcs = {
		.init = null_init,
		.post_init = null_post_init,
		.shutdown = null_shutdown,
		.create_window = null_create_window,
		.supports = null_supports,
		.capabilities = null_capabilities,
		.capabilities_current = null_capabilities_current,
		.draw = null_draw,
		.color4 = null_color4,
		.color_current = null_color_current,
		.blend = null_blend,
		.blend_current = null_blend_current,
		.cull = null_cull,
		.cull_current = null_cull_current,
		.depth_func = null_depth_func,
		.depth_func_current = null_depth_func_current,
		.shader_language_supported = null_shader_language_supported,
		.shader_object_compile = null_shader_object_compile,
		.shader_object_destroy = null_shader_object_destroy,
		.shader_object_set_debug_label = null_shader_object_set_debug_label,
		.shader_object_get_debug_label = null_shader_object_get_debug_label,
		.shader_program_link = null_shader_program_link,
		.shader_program_destroy = null_shader_program_destroy,
		.shader_program_set_debug_label = null_shader_program_set_debug_label,
		.shader_program_get_debug_label = null_shader_program_get_debug_label,
		.shader = null_shader,
		.shader_current = null_shader_current,
		.shader_uniform = null_shader_uniform,
		.uniform = null_uniform,
		.uniform_type = null_uniform_type,
		.texture_create = null_texture_create,
		.texture_get_params = null_texture_get_params,
		.texture_get_size = null_texture_get_size,
		.texture_get_debug_label = null_texture_get_debug_label,
		.texture_set_debug_label = null_texture_set_debug_label,
		.texture_set_filter = null_texture_set_filter,
		.texture_set_wrap = null_texture_set_wrap,
		.texture_destroy = null_texture_destroy,
		.texture_invalidate = null_texture_invalidate,
		.texture_fill = null_texture_fill,
		.texture_fill_region = null_texture_fill_region,
		.texture_clear = null_texture_clear,
		.framebuffer_create = null_framebuffer_create,
		.framebuffer_get_debug_label = null_framebuffer_get_debug_label,
		.framebuffer_set_debug_label = null_framebuffer_set_debug_label,
		.framebuffer_destroy = null_framebuffer_destroy,
		.framebuffer_attach = null_framebuffer_attach,
		.framebuffer_get_attachment = null_framebuffer_attachment,
		.framebuffer_get_attachment_mipmap = null_framebuffer_attachment_mipmap,
		.framebuffer_viewport = null_framebuffer_viewport,
		.framebuffer_viewport_current = null_framebuffer_viewport_current,
		.framebuffer = null_framebuffer,
		.framebuffer_current = null_framebuffer_current,
		.framebuffer_clear = null_framebuffer_clear,
		.vertex_buffer_create = null_vertex_buffer_create,
		.vertex_buffer_get_debug_label = null_vertex_buffer_get_debug_label,
		.vertex_buffer_set_debug_label = null_vertex_buffer_set_debug_label,
		.vertex_buffer_destroy = null_vertex_buffer_destroy,
		.vertex_buffer_invalidate = null_vertex_buffer_invalidate,
		.vertex_buffer_get_stream = null_vertex_buffer_get_stream,
		.vertex_array_create = null_vertex_array_create,
		.vertex_array_get_debug_label = null_vertex_array_get_debug_label,
		.vertex_array_set_debug_label = null_vertex_array_set_debug_label,
		.vertex_array_destroy = null_vertex_array_destroy,
		.vertex_array_layout = null_vertex_array_layout,
		.vertex_array_attach_buffer = null_vertex_array_attach_buffer,
		.vertex_array_get_attachment = null_vertex_array_get_attachment,
		.vertex_array = null_vertex_array,
		.vertex_array_current = null_vertex_array_current,
		.vsync = null_vsync,
		.vsync_current = null_vsync_current,
		.swap = null_swap,
		.screenshot = null_screenshot,
	},
};
