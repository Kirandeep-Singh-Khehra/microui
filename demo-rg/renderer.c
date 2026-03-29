#include "raylib.h"
#include <string.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "renderer.h"
#include <assert.h>

#define FONT_SIZE 10

#define MU_TO_R_COLOR(color)                                                   \
  (Color) { color.r, color.g, color.b, color.a }
#define MU_TO_R_RECT(rect)                                                     \
  (Rectangle) { rect.x, rect.y, rect.w, rect.h }

static int width = 800;
static int height = 600;

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) {
    len = strlen(text);
  }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) { return r_get_text_height(); }

mu_Context *r_init(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, "Hello there");
  SetTargetFPS(60);

  mu_Context *ctx = malloc(sizeof(mu_Context));
  mu_init(ctx);
  ctx->text_width = text_width;
  ctx->text_height = text_height;

  return ctx;
}

void r_draw_rect(mu_Rect rect, mu_Color color) {
  DrawRectangleRec(MU_TO_R_RECT(rect), MU_TO_R_COLOR(color));
}

void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  DrawText(text, pos.x, pos.y, FONT_SIZE, MU_TO_R_COLOR(color));
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  int x = rect.x + 1;
  int y = rect.y + 1;

  int raygui_icon_id = 0;
  switch (id) {
  case MU_ICON_CLOSE:
    raygui_icon_id = ICON_CROSS;
    break;
  case MU_ICON_CHECK:
    raygui_icon_id = ICON_OK_TICK;
    break;
  case MU_ICON_COLLAPSED:
    raygui_icon_id = ICON_ARROW_RIGHT_FILL;
    break;
  case MU_ICON_EXPANDED:
    raygui_icon_id = ICON_ARROW_DOWN_FILL;
    break;
  case MU_ICON_MAX:
    raygui_icon_id = ICON_BOX_GRID_BIG;
    break;
  }
  GuiDrawIcon(raygui_icon_id, x, y, 1, MU_TO_R_COLOR(color));
}

int r_get_text_width(const char *text, int len) {
  char text0[len + 1];
  memcpy(text0, text, len);
  text0[len] = '\0';
  return MeasureText(text0, FONT_SIZE);
}

int r_get_text_height(void) { return FONT_SIZE; }

void r_set_clip_rect(mu_Rect rect) {
  if (rect.w == 0x1000000 && rect.h == 0x1000000) {
    EndScissorMode();
  } else {
    BeginScissorMode(rect.x, rect.y, rect.w, rect.h);
  }
}

void r_clear(mu_Color clr) {
  BeginDrawing();
  ClearBackground(MU_TO_R_COLOR(clr));
}

void r_process_input(mu_Context *ctx) {

  int mouseX = GetMouseX();
  int mouseY = GetMouseY();
  mu_input_mousemove(ctx, mouseX, mouseY);

  mu_input_scroll(ctx, 0, GetMouseWheelMove() * -30);

  for (int btn = MOUSE_BUTTON_LEFT; btn < MOUSE_BUTTON_MIDDLE; btn++) {
    if (IsMouseButtonReleased(btn)) {
      mu_input_mouseup(ctx, mouseX, mouseY, 1 << btn);
    }
    if (IsMouseButtonPressed(btn)) {
      mu_input_mousedown(ctx, mouseX, mouseY, 1 << btn);
    }
  }

  // int codepoint = GetCharPressed(); // Get Unicode codepoint
  // int codepointSize = 0;
  // const char *charEncoded = CodepointToUTF8(codepoint, &codepointSize);
  // // mu_input_text(ctx, (char[]){codepoint, '\0'});
  // mu_input_text(ctx, (char[codepointSize]){charEncoded, '\0'});

  for (int key = GetCharPressed(); key != 0; key = GetCharPressed()) {
    int codepointSize = 0;
    const char *charEncoded = CodepointToUTF8(key, &codepointSize);
    mu_input_text(ctx, TextFormat("%.*s", codepointSize, charEncoded));
  }

  for (int key = GetKeyPressed(); key != 0; key = GetKeyPressed()) {
    switch (key) {
      case KEY_LEFT_SHIFT:
      case KEY_RIGHT_SHIFT:
        mu_input_keydown(ctx, MU_KEY_SHIFT);
        IsKeyUp(GetKeyPressed())?mu_input_keyup(ctx, MU_KEY_SHIFT):NULL;
        break;

      case KEY_LEFT_CONTROL:
      case KEY_RIGHT_CONTROL:
        mu_input_keydown(ctx, MU_KEY_CTRL);
        IsKeyUp(GetKeyPressed())?mu_input_keyup(ctx, MU_KEY_CTRL):NULL;
        break;

      case KEY_LEFT_ALT:
      case KEY_RIGHT_ALT:
        mu_input_keydown(ctx, MU_KEY_ALT);
        IsKeyUp(GetKeyPressed())?mu_input_keyup(ctx, MU_KEY_ALT):NULL;
        break;

      case KEY_BACKSPACE:
        mu_input_keydown(ctx, MU_KEY_BACKSPACE);
        IsKeyUp(GetKeyPressed())?mu_input_keyup(ctx, MU_KEY_BACKSPACE):NULL;
        break;

      case KEY_ENTER:
        mu_input_keydown(ctx, MU_KEY_RETURN);
        IsKeyUp(GetKeyPressed())?mu_input_keyup(ctx, MU_KEY_RETURN):NULL;
        break;
    }
  }
}

void r_present(mu_Context *ctx, float bg[3]) {
  r_clear(mu_color(bg[0], bg[1], bg[2], 255));
  mu_Command *cmd = NULL;
  while (mu_next_command(ctx, &cmd)) {
    switch (cmd->type) {
    case MU_COMMAND_TEXT:
      r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
      break;
    case MU_COMMAND_RECT:
      r_draw_rect(cmd->rect.rect, cmd->rect.color);
      break;
    case MU_COMMAND_ICON:
      r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
      break;
    case MU_COMMAND_CLIP:
      r_set_clip_rect(cmd->clip.rect);
      break;
    }
  }
  EndDrawing();
}
