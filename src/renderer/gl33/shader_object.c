/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include "util.h"
#include "core.h"
#include "shader_object.h"
#include "../glcommon/debug.h"
#include "../glcommon/shaders.h"

bool gl33_shader_language_supported(const ShaderLangInfo *lang, ShaderLangInfo *out_alternative) {
	if(glcommon_shader_lang_table) {
		for(ShaderLangInfo *l = glcommon_shader_lang_table; l->lang != SHLANG_INVALID; ++l) {
			if(!memcmp(l, lang, sizeof(*l))) {
				return true;
			}
		}

		if(out_alternative) {
			*out_alternative = *glcommon_shader_lang_table;
		}

		return false;
	}

	// No table. Should never actually happen, but let's just hope it works, right?

	bool supported = lang->lang == SHLANG_GLSL;

	if(!supported && out_alternative) {
		out_alternative->lang = SHLANG_GLSL;
		out_alternative->glsl.version.version = 330;
		out_alternative->glsl.version.profile = GLSL_PROFILE_CORE;
	}

	return supported;
}

static void print_info_log(GLuint shader) {
	GLint len = 0, alen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

	if(len > 1) {
		char log[len];
		memset(log, 0, len);
		glGetShaderInfoLog(shader, len, &alen, log);

		log_warn(
			"\n== Shader object compilation log (%u) ==\n%s\n== End of shader object compilation log (%u) ==",
				 shader, log, shader
		);
	}
}

ShaderObject* gl33_shader_object_compile(ShaderSource *source) {
	assert(r_shader_language_supported(&source->lang, NULL));

	GLuint gl_handle = glCreateShader(
		source->stage == SHADER_STAGE_VERTEX
			? GL_VERTEX_SHADER
			: GL_FRAGMENT_SHADER
	);

	GLint status;

	// log_debug("Source code for %s:\n%s", path, source->content);

	glShaderSource(
		gl_handle, 1,
		(const GLchar*[]) { source->content },
		(GLint[])         { source->content_size - 1 }
	);

	glCompileShader(gl_handle);

#ifdef DEBUG
	if(GLAD_GL_ANGLE_translated_shader_source) {
		GLint srclen;
		glGetShaderiv(gl_handle, GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE, &srclen);

		if(srclen > 0) {
			char buf[srclen];
			glGetTranslatedShaderSourceANGLE(gl_handle, sizeof(buf), NULL, buf);
			log_debug("Final shader source (as translated by ANGLE):\n%s", buf);
		}
	}
#endif

	glGetShaderiv(gl_handle, GL_COMPILE_STATUS, &status);
	print_info_log(gl_handle);

	ShaderObject *shobj = NULL;

	if(status) {
		shobj = calloc(1, sizeof(*shobj));
		shobj->gl_handle = gl_handle;
		shobj->stage = source->stage;
		snprintf(shobj->debug_label, sizeof(shobj->debug_label), "Shader object #%i", gl_handle);
	} else {
		glDeleteShader(gl_handle);
	}

	return shobj;
}

void gl33_shader_object_destroy(ShaderObject *shobj) {
	glDeleteShader(shobj->gl_handle);
	free(shobj);
}

void gl33_shader_object_set_debug_label(ShaderObject *shobj, const char *label) {
	glcommon_set_debug_label(shobj->debug_label, "Shader object", GL_SHADER, shobj->gl_handle, label);
}

const char* gl33_shader_object_get_debug_label(ShaderObject *shobj) {
	return shobj->debug_label;
}
