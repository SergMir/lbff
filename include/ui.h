#ifndef UI_H
#define UI_H

#define UI_MAX_TEXT_LEN 50

typedef enum
{
  UI_ACT_PRESSED,
  UI_ACT_RELEASED
} UI_action_t;

typedef void (*UI_buttonAction)(UI_action_t action);

typedef struct
{
  lb_float x;
  lb_float y;
  lb_float height;
  lb_float width;
  char text[UI_MAX_TEXT_LEN];
  UI_buttonAction callback;
} UI_button_t, *UI_button_p;

typedef struct
{
  lb_float x;
  lb_float y;
  lb_float height;
  lb_float width;
  char text[UI_MAX_TEXT_LEN];
} UI_label_t, *UI_label_p;

int UI_Init(void);
UI_button_p UI_CreateButton(lb_float x, lb_float y, lb_float width, lb_float height, const char *text, UI_buttonAction callback);
UI_label_p UI_CreateLabel(lb_float x, lb_float y, lb_float width, lb_float height, const char *text);
void UI_ChangeTextLabel(UI_label_p label, char *text);
void UI_Draw(void);

#endif /* UI_H */
