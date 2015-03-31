#ifndef _UI_H
#define _UI_H

#define KEY_ESC		27
#define KEY_SPACE	32
#define KEY_ENTER	13

#define UI_MAX_TEXT_LEN	50

struct ui_label;
struct ui_ctx;

typedef void (*ui_keypress_callback)(int key);

struct ui_ctx* ui_init(ui_keypress_callback keypress_callback);

struct ui_label* ui_create_label(struct ui_ctx *ctx,
				 int x, int y,
				 int width, int height,
				 const char *text);

void ui_set_label_text(struct ui_label *label, const char *text);

void ui_render(struct ui_ctx *ctx);

#endif /* _UI_H */
