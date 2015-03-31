/*
 * LBFF: User Interface, both graphical and console
 *
 * Copyright (C) 2011 LBFF Authors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation: http://www.gnu.org/licenses/
 */
#include <ui.h>
#include <graph.h>
#include <utils.h>

#include <GL/glut.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct ui_label {
	int x;
	int y;
	int height;
	int width;
	char text[UI_MAX_TEXT_LEN];
};

struct ui_labels_list {
	struct ui_label *label;
	struct ui_labels_list *next;
};

struct ui_callbacks_list {
	ui_keypress_callback keypress_callback;
	struct ui_callbacks_list *next;
};

struct ui_ctx {
	ui_keypress_callback keypress_callback;
	struct ui_labels_list *labels;
};

static struct ui_callbacks_list *callbacks_list = NULL;

static void ui_keyboard_handler(unsigned char key, int x, int y)
{
	struct ui_callbacks_list *callbacks_node = callbacks_list;

	while (NULL != callbacks_node) {
		callbacks_node->keypress_callback(key);
		callbacks_node = callbacks_node->next;
	}

	UNUSED(x);
	UNUSED(y);
}

static void ui_spec_keyboard_handler(int key, int x, int y)
{
	ui_keyboard_handler(key, x, y);
}

struct ui_label* ui_create_label(struct ui_ctx *ctx,
				 int x, int y,
				 int width, int height,
				 const char *text)
{
	struct ui_label *label = malloc(sizeof(*label));

	LIST_ADD(ctx->labels, struct ui_labels_list,
		 label, label);

	label->x = x;
	label->y = y;
	label->width = width;
	label->height = height;
	strcpy(label->text, text);

	return label;
}

void ui_set_label_text(struct ui_label *label, const char *text)
{
	strcpy(label->text, text);
}

void ui_render(struct ui_ctx *ctx)
{
	struct ui_labels_list *label_node = ctx->labels;

	for (; NULL != label_node; label_node = label_node->next) {
		struct ui_label *label = label_node->label;
		GRAPH_DrawLabel(label->x, label->y,
				label->width, label->height,
				label->text);
	}
}

struct ui_ctx* ui_init(ui_keypress_callback keypress_callback)
{
	struct ui_ctx *ctx = malloc(sizeof(*ctx));

	if (NULL != keypress_callback) {
		LIST_ADD(callbacks_list, struct ui_callbacks_list, \
			 keypress_callback, keypress_callback);
	}

	glutKeyboardFunc(ui_keyboard_handler);
	glutSpecialFunc(ui_spec_keyboard_handler);

	return ctx;
}
