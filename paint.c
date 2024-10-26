/*  an ms paint-esque application written in c && opengl using raylib
 *  to compile
 *  clang -o cpaint paint.c -lraylib && ./cpaint
 */
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

const int NUM_COLORS = 23;

int main(void) {
  //-Variables----------------------------------------------------------------------
  int window_width = 1280;
  int window_height = 720;
  int cursor_radius = 64;
  Vector2 mouse;
  Vector2 mouse_wheel;
  Color colors[NUM_COLORS] = {
      RAYWHITE,  YELLOW,    GOLD,   ORANGE,     PINK,    RED,
      MAROON,    GREEN,     LIME,   DARKGREEN,  SKYBLUE, BLUE,
      DARKBLUE,  PURPLE,    VIOLET, DARKPURPLE, BEIGE,   BROWN,
      DARKBROWN, LIGHTGRAY, GRAY,   DARKGRAY,   BLACK};
  int selected_color = 22;
  Rectangle color_rectangles[NUM_COLORS] = {};

  //--------------------------------------------------------------------------------

  //-Settings-----------------------------------------------------------------------
  InitWindow(400, 300, "cpaint settings");
  SetExitKey(KEY_ENTER);

  char *window_height_string = (char *)malloc(24 * sizeof(char));
  char *window_width_string = (char *)malloc(24 * sizeof(char));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

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
  SetExitKey(KEY_Q);

  RenderTexture2D canvas = LoadRenderTexture(window_width, window_height);
  ClearBackground(RAYWHITE);

  for (int i = 0; i < NUM_COLORS; i++) {
    color_rectangles[i].x = 4;
    color_rectangles[i].y = 4 + (window_height / 30.0) * i + 2 * i;
    color_rectangles[i].width = 40;
    color_rectangles[i].height = window_height / 30.0;
  }
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

    // Cycle through colors with arrow keys
    if (IsKeyPressed(KEY_DOWN)) {
      if (selected_color == NUM_COLORS - 1) {
        selected_color = 0;
      } else
        selected_color++;
    }
    if (IsKeyPressed(KEY_UP)) {
      if (selected_color == 0) {
        selected_color = 22;
      } else
        selected_color--;
    }

    // Select colors with mouse
    for (int i = 0; i < NUM_COLORS; i++) {
      if (CheckCollisionPointRec(mouse, color_rectangles[i])) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          selected_color = i;
        }
      }
    }

    // Update the canvas when the mouse is clicked
    if (mouse.x > 50 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      BeginTextureMode(canvas);
      DrawCircleV(mouse, cursor_radius, colors[selected_color]);
      EndTextureMode();
    }
    //--------------------------------------------------------------------------------

    //-Draw---------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw the canvas
    DrawTextureRec(canvas.texture,
                   (Rectangle){50, 0, (float)canvas.texture.width,
                               (float)-canvas.texture.height},
                   (Vector2){50, 0}, WHITE);

    // Draw the mouse guide
    // NOTE: Left shift will show the mouse
    if (mouse.x > 50 && !IsKeyDown(KEY_LEFT_SHIFT)) {
      HideCursor();
      DrawCircleV(mouse, cursor_radius, colors[selected_color]);
    } else
      ShowCursor();

    // Draw the sidebar
    DrawRectangle(0, 0, 48, window_height, LIGHTGRAY);
    DrawRectangle(48, 0, 2, window_height, GRAY);

    // Draw the color selection rectangles
    for (int i = 0; i < NUM_COLORS; i++)
      DrawRectangleRec(color_rectangles[i], colors[i]);

    EndDrawing();
    //--------------------------------------------------------------------------------
  }

  //-De-Initialization--------------------------------------------------------------
  UnloadRenderTexture(canvas);
  CloseWindow();
  //--------------------------------------------------------------------------------

  return 0;
}
