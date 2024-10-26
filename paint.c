/*  an ms paint-esque application written in c && opengl using raylib
 *  to compile
 *  clang -o cpaint paint.c -lraylib
 */

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  //-Variables----------------------------------------------------------------------
  int window_width = 1280;
  int window_height = 720;
  int cursor_radius = 32;
  Vector2 mouse;
  Vector2 mouse_wheel;
  //--------------------------------------------------------------------------------

  //-Settings-----------------------------------------------------------------------
  InitWindow(400, 300, "cpaint settings");
  SetExitKey(KEY_ENTER);

  char *window_height_string = (char *)malloc(24 * sizeof(char));
  char *window_width_string = (char *)malloc(24 * sizeof(char));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    int monitor = GetCurrentMonitor();

    sprintf(window_width_string, "document width: %d", window_width);
    if (IsKeyDown(KEY_RIGHT) &&
        window_width <= GetMonitorWidth(monitor) * 0.9) {
      window_width++;
    } else if (IsKeyDown(KEY_LEFT) && window_width >= 401) {
      window_width--;
    }

    sprintf(window_height_string, "document height: %d", window_height);
    if (IsKeyDown(KEY_UP) && window_height <= GetMonitorHeight(monitor) * 0.8) {
      window_height++;
    } else if (IsKeyDown(KEY_DOWN) && window_height >= 301) {
      window_height--;
    }

    // Escape key cancels program and frees malloc
    if (IsKeyPressed(KEY_ESCAPE)) {
      CloseWindow();
      free(window_height_string);
      free(window_width_string);
      return 0;
    }

    DrawText(window_width_string, 4, 4, 20, GRAY);
    DrawText(window_height_string, 4, 28, 20, GRAY);

    DrawText("press 'enter' to accept", 4, 256 - 4, 20, GRAY);
    DrawText("press 'esc' to cancel", 4, 280 - 4, 20, GRAY);

    EndDrawing();
  }
  CloseWindow();
  free(window_height_string);
  free(window_width_string);
  //--------------------------------------------------------------------------------

  //-Initialization-----------------------------------------------------------------
  InitWindow(window_width, window_height, "cpaint");
  HideCursor();
  SetExitKey(KEY_Q);

  RenderTexture2D canvas = LoadRenderTexture(window_width, window_height);
  ClearBackground(WHITE);
  //--------------------------------------------------------------------------------

  //-Main-Loop----------------------------------------------------------------------
  while (!WindowShouldClose()) {
    //-Update-------------------------------------------------------------------------
    mouse = GetMousePosition();
    mouse_wheel = GetMouseWheelMoveV();

    // Update cursor size on scroll
    if (mouse_wheel.y > 0 && cursor_radius <= 256) {
      cursor_radius++;
    } else if (mouse_wheel.y < 0 && cursor_radius >= 4) {
      cursor_radius--;
      printf("mouse.y = %f\n", mouse.y);
    }

    // Update the canvas when the mouse is clicked
    if (mouse.x > 50 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      BeginTextureMode(canvas);
      DrawCircleV(mouse, cursor_radius, BLACK);
      EndTextureMode();
    }
    //--------------------------------------------------------------------------------

    //-Draw---------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw the canvas
    DrawTextureRec(canvas.texture,
                   (Rectangle){0, 0, (float)canvas.texture.width,
                               (float)-canvas.texture.height},
                   (Vector2){0, 0}, WHITE);

    // Draw the mouse guide
    if (mouse.x > 50) {
      DrawCircleV(mouse, cursor_radius, BLACK);
    }

    EndDrawing();
    //--------------------------------------------------------------------------------
  }

  //-De-Initialization--------------------------------------------------------------
  UnloadRenderTexture(canvas);
  CloseWindow();
  //--------------------------------------------------------------------------------

  return 0;
}
