/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include <SDL_image.h>

#include "pixmap.h"
#include "util.h"

// NOTE: this is pretty stupid and not at all optimized, patches welcome

#define _CONV_FUNCNAME	convert_x8_to_x8
#define _CONV_IN_MAX	UINT8_MAX
#define _CONV_IN_TYPE	uint8_t
#define _CONV_OUT_MAX	UINT8_MAX
#define _CONV_OUT_TYPE	uint8_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x8_to_x16
#define _CONV_IN_MAX	UINT8_MAX
#define _CONV_IN_TYPE	uint8_t
#define _CONV_OUT_MAX	UINT16_MAX
#define _CONV_OUT_TYPE	uint16_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x8_to_x32
#define _CONV_IN_MAX	UINT8_MAX
#define _CONV_IN_TYPE	uint8_t
#define _CONV_OUT_MAX	UINT32_MAX
#define _CONV_OUT_TYPE	uint32_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x16_to_x8
#define _CONV_IN_MAX	UINT16_MAX
#define _CONV_IN_TYPE	uint16_t
#define _CONV_OUT_MAX	UINT8_MAX
#define _CONV_OUT_TYPE	uint8_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x16_to_x16
#define _CONV_IN_MAX	UINT16_MAX
#define _CONV_IN_TYPE	uint16_t
#define _CONV_OUT_MAX	UINT16_MAX
#define _CONV_OUT_TYPE	uint16_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x16_to_x32
#define _CONV_IN_MAX	UINT16_MAX
#define _CONV_IN_TYPE	uint16_t
#define _CONV_OUT_MAX	UINT32_MAX
#define _CONV_OUT_TYPE	uint32_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x32_to_x8
#define _CONV_IN_MAX	UINT32_MAX
#define _CONV_IN_TYPE	uint32_t
#define _CONV_OUT_MAX	UINT8_MAX
#define _CONV_OUT_TYPE	uint8_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x32_to_x16
#define _CONV_IN_MAX	UINT32_MAX
#define _CONV_IN_TYPE	uint32_t
#define _CONV_OUT_MAX	UINT16_MAX
#define _CONV_OUT_TYPE	uint16_t
#include "pixmap_conversion.inc.h"

#define _CONV_FUNCNAME	convert_x32_to_x32
#define _CONV_IN_MAX	UINT32_MAX
#define _CONV_IN_TYPE	uint32_t
#define _CONV_OUT_MAX	UINT32_MAX
#define _CONV_OUT_TYPE	uint32_t
#include "pixmap_conversion.inc.h"

typedef void (*convfunc_t)(
	size_t in_elements,
	size_t out_elements,
	size_t num_pixels,
	void *vbuf_in,
	void *vbuf_out,
	void *default_pixel
);

#define CONV(in, out) { convert_x##in##_to_x##out, in, out }

struct conversion_def {
	convfunc_t func;
	uint depth_in;
	uint depth_out;
};

struct conversion_def conversion_table[] = {
	CONV(8,   8),
	CONV(8,  16),
	CONV(8,  32),

	CONV(16,  8),
	CONV(16, 16),
	CONV(16, 32),

	CONV(32,  8),
	CONV(32, 16),
	CONV(32, 32),

	{ 0 }
};

static struct conversion_def* find_conversion(uint depth_in, uint depth_out) {
	for(struct conversion_def *cv = conversion_table; cv->func; ++cv) {
		if(cv->depth_in == depth_in && cv->depth_out == depth_out) {
			return cv;
		}
	}

	log_fatal("Pixmap conversion for %upbc -> %upbc undefined, please add", depth_in, depth_out);
}

static void* default_pixel(uint depth) {
	static uint8_t  default_u8[]  = { 0, 0, 0, UINT8_MAX  };
	static uint16_t default_u16[] = { 0, 0, 0, UINT16_MAX };
	static uint32_t default_u32[] = { 0, 0, 0, UINT32_MAX };

	switch(depth) {
		case 8:  return default_u8;
		case 16: return default_u16;
		case 32: return default_u32;
		default: UNREACHABLE;
	}
}

void* pixmap_alloc_buffer(PixmapFormat format, size_t width, size_t height) {
	assert(width >= 1);
	assert(height >= 1);
	size_t pixel_size = PIXMAP_FORMAT_PIXEL_SIZE(format);
	assert(pixel_size >= 1);
	return calloc(width * height, pixel_size);
}

void* pixmap_alloc_buffer_for_copy(const Pixmap *src) {
	return pixmap_alloc_buffer(src->format, src->width, src->height);
}

void* pixmap_alloc_buffer_for_conversion(const Pixmap *src, PixmapFormat format) {
	return pixmap_alloc_buffer(format, src->width, src->height);
}

static void pixmap_copy_meta(const Pixmap *src, Pixmap *dst) {
	dst->format = src->format;
	dst->width = src->width;
	dst->height = src->height;
	dst->origin = src->origin;
}

void pixmap_convert(const Pixmap *src, Pixmap *dst, PixmapFormat format) {
	size_t num_pixels = src->width * src->height;
	size_t pixel_size = PIXMAP_FORMAT_PIXEL_SIZE(format);

	assert(dst->data.untyped != NULL);
	pixmap_copy_meta(src, dst);

	if(src->format == format) {
		memcpy(dst->data.untyped, src->data.untyped, num_pixels * pixel_size);
		return;
	}

	dst->format = format;

	struct conversion_def *cv = find_conversion(
		PIXMAP_FORMAT_DEPTH(src->format),
		PIXMAP_FORMAT_DEPTH(dst->format)
	);

	cv->func(
		PIXMAP_FORMAT_LAYOUT(src->format),
		PIXMAP_FORMAT_LAYOUT(dst->format),
		num_pixels,
		src->data.untyped,
		dst->data.untyped,
		default_pixel(PIXMAP_FORMAT_DEPTH(dst->format))
	);
}

void pixmap_convert_alloc(const Pixmap *src, Pixmap *dst, PixmapFormat format) {
	dst->data.untyped = pixmap_alloc_buffer_for_conversion(src, format);
	pixmap_convert(src, dst, format);
}

void pixmap_convert_inplace_realloc(Pixmap *src, PixmapFormat format) {
	assert(src->data.untyped != NULL);

	if(src->format == format) {
		return;
	}

	Pixmap tmp;
	pixmap_copy_meta(src, &tmp);
	pixmap_convert_alloc(src, &tmp, format);

	free(src->data.untyped);
	*src = tmp;
}

void pixmap_copy(const Pixmap *src, Pixmap *dst) {
	assert(dst->data.untyped != NULL);
	pixmap_copy_meta(src, dst);
	memcpy(dst->data.untyped, src->data.untyped, pixmap_data_size(src));
}

void pixmap_copy_alloc(const Pixmap *src, Pixmap *dst) {
	dst->data.untyped = pixmap_alloc_buffer_for_copy(src);
	pixmap_copy(src, dst);
}

size_t pixmap_data_size(const Pixmap *px) {
	return px->width * px->height * PIXMAP_FORMAT_PIXEL_SIZE(px->format);
}

void pixmap_flip_y(const Pixmap *src, Pixmap *dst) {
	assert(dst->data.untyped != NULL);
	pixmap_copy_meta(src, dst);

	size_t rows = src->height;
	size_t row_length = src->width * PIXMAP_FORMAT_PIXEL_SIZE(src->format);

	char *cdst = dst->data.untyped;
	const char *csrc = src->data.untyped;

	for(size_t row = 0, irow = rows - 1; row < rows; ++row, --irow) {
		memcpy(cdst + irow * row_length, csrc + row * row_length, row_length);
	}
}

void pixmap_flip_y_alloc(const Pixmap *src, Pixmap *dst) {
	dst->data.untyped = pixmap_alloc_buffer_for_copy(src);
	pixmap_flip_y(src, dst);
}

void pixmap_flip_y_inplace(Pixmap *src) {
	size_t rows = src->height;
	size_t row_length = src->width * PIXMAP_FORMAT_PIXEL_SIZE(src->format);
	char *data = src->data.untyped;
	char swap_buffer[row_length];

	for(size_t row = 0; row < rows / 2; ++row) {
		memcpy(swap_buffer, data + row * row_length, row_length);
		memcpy(data + row * row_length, data + (rows - row - 1) * row_length, row_length);
		memcpy(data + (rows - row - 1) * row_length, swap_buffer, row_length);
	}
}

void pixmap_flip_to_origin(const Pixmap *src, Pixmap *dst, PixmapOrigin origin) {
	assert(dst->data.untyped != NULL);

	if(src->origin == origin) {
		pixmap_copy(src, dst);
	} else {
		pixmap_flip_y(src, dst);
		dst->origin = origin;
	}
}

void pixmap_flip_to_origin_alloc(const Pixmap *src, Pixmap *dst, PixmapOrigin origin) {
	dst->data.untyped = pixmap_alloc_buffer_for_copy(src);
	pixmap_flip_to_origin(src, dst, origin);
}

void pixmap_flip_to_origin_inplace(Pixmap *src, PixmapOrigin origin) {
	if(src->origin == origin) {
		return;
	}

	pixmap_flip_y_inplace(src);
	src->origin = origin;
}

static void quit_sdl_image(void) {
	IMG_Quit();
}

static void init_sdl_image(void) {
	static bool sdlimage_initialized = false;

	if(sdlimage_initialized) {
		return;
	}

	sdlimage_initialized = true;

	int want_flags = IMG_INIT_JPG | IMG_INIT_PNG;
	int init_flags = IMG_Init(want_flags);

	if((want_flags & init_flags) != want_flags) {
		log_warn(
			"SDL_image doesn't support some of the formats we want. "
			"Requested: %i, got: %i. "
			"Textures may fail to load",
			want_flags,
			init_flags
		);
	}

	atexit(quit_sdl_image);
}

static bool pixmap_load_finish(SDL_Surface *surf, Pixmap *dst) {
	SDL_Surface *cv_surf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(surf);

	if(cv_surf == NULL) {
		log_warn("SDL_ConvertSurfaceFormat() failed: %s", SDL_GetError());
		return false;
	}

	// NOTE: 32 in SDL_PIXELFORMAT_RGBA32 is bits per pixel; 8 in PIXMAP_FORMAT_RGBA8 is bits per channel.
	dst->format = PIXMAP_FORMAT_RGBA8;
	dst->width = cv_surf->w;
	dst->height = cv_surf->h;
	dst->origin = PIXMAP_ORIGIN_TOPLEFT;
	dst->data.rgba8 = pixmap_alloc_buffer_for_copy(dst);

	SDL_LockSurface(cv_surf);
	memcpy(dst->data.rgba8, cv_surf->pixels, pixmap_data_size(dst));
	SDL_UnlockSurface(cv_surf);
	SDL_FreeSurface(cv_surf);

	return true;
}

bool pixmap_load_stream_tga(SDL_RWops *stream, Pixmap *dst) {
	init_sdl_image();
	SDL_Surface *surf = IMG_LoadTGA_RW(stream);

	if(surf == NULL) {
		log_warn("IMG_LoadTGA_RW() failed: %s", IMG_GetError());
		return false;
	}

	return pixmap_load_finish(surf, dst);
}

bool pixmap_load_stream(SDL_RWops *stream, Pixmap *dst) {
	init_sdl_image();
	SDL_Surface *surf = IMG_Load_RW(stream, false);

	if(surf == NULL) {
		log_warn("IMG_Load_RW() failed: %s", IMG_GetError());
		return false;
	}

	return pixmap_load_finish(surf, dst);
}

bool pixmap_load_file(const char *path, Pixmap *dst) {
	SDL_RWops *stream = vfs_open(path, VFS_MODE_READ | VFS_MODE_SEEKABLE);

	if(!stream) {
		log_warn("VFS error: %s", vfs_get_error());
		return false;
	}

	bool result;

	if(strendswith(path, ".tga")) {
		result = pixmap_load_stream_tga(stream, dst);
	} else {
		result = pixmap_load_stream(stream, dst);
	}

	SDL_RWclose(stream);
	return result;
}
