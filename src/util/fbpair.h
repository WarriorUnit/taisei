/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#pragma once
#include "taisei.h"

#include "renderer/api.h"

typedef struct FBPair {
	/*
	 *  Rule of thumb:
	 *      1. Bind back buffer;
	 *      2. Draw front buffer;
	 *      3. Call fbpair_swap;
	 *      4. Rinse, repeat.
	 */

	Framebuffer *const front;
	Framebuffer *const back;

	Framebuffer framebuffers[2];
} FBPair;

typedef struct FBAttachmentConfig {
	FramebufferAttachment attachment;
	TextureParams tex_params;
} FBAttachmentConfig;

void fbpair_create(FBPair *pair, uint num_attachments, FBAttachmentConfig attachments[num_attachments]) attr_nonnull(1, 3);
void fbpair_resize(FBPair *pair, FramebufferAttachment attachment, uint width, uint height) attr_nonnull(1);
void fbpair_resize_all(FBPair *pair, uint width, uint height) attr_nonnull(1);
void fbpair_destroy(FBPair *pair) attr_nonnull(1);
void fbpair_swap(FBPair *pair) attr_nonnull(1);
void fbpair_viewport(FBPair *pair, int x, int y, int w, int h) attr_nonnull(1);
void fbpair_viewport_rect(FBPair *pair, IntRect vp) attr_nonnull(1);
