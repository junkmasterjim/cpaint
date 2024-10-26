// an ms paint-esque application written in c && opengl using raylib

// NOTE: brush should not be able to draw over our menu / sidebar
// FIX: easy fix with if mosue.x < 50

// PERF: i need to render the canvas by using a RenderTexture2D and the Render
// texture mode
// NOTE: then i think i can draw the brush over top with the DrawCircle method
// and not interfere with anything
// NOTE:
// https://www.raylib.com/examples/textures/loader.html?name=textures_mouse_painting

/* to compile
clang -o cpaint paint.c -lraylib
*/

#include "raylib.h"
#include "stdio.h"

double window_width = 1280;
double window_height = 720;
int cursor_radius = 10;
int canvas_width;
int canvas_height;

// DrawSidebar will draw the sidebar which allows us to switch our tools
void DrawSidebar() {
  // sidebar
  DrawRectangle(0, 0, 48, window_height, LIGHTGRAY);
  // border
  DrawRectangle(48, 0, 2, window_height, GRAY);

  // buttons
  DrawRectangle(4, 0 + 4, 40, 40, BLACK);
  DrawRectangle(4, 48 + 8, 40, 40, BLACK);
  DrawRectangle(4, 96 + 12, 40, 40, BLACK);
  DrawRectangle(4, 144 + 16, 40, 40, BLACK);
}

void HandleBrush() {
  int mouse_x = GetMouseX();
  int mouse_y = GetMouseY();

  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) == true) {
    DrawCircle(mouse_x, mouse_y, cursor_radius, RAYWHITE);

    printf("%i %i", mouse_x, mouse_y);
  }
}

// main function to run our app
int main(void) {
  // config before running window
  canvas_width = window_width - 50;
  canvas_height = window_height;

  // open new window
  InitWindow(window_width, window_height, "cpaint");
  // HideCursor();
  SetExitKey(KEY_Q);

  // main loop
  while (!WindowShouldClose()) {
    BeginDrawing();

    DrawSidebar();

    DrawRectangle(50, 0, canvas_width, canvas_height, BLACK);
    HandleBrush();

    // display some text in the center of the screen
    char *text = "cpaint";
    Vector2 text_wh = MeasureTextEx(GetFontDefault(), text, 32, 1);
    DrawText(text, window_width / 2 - text_wh.x / 2,
             window_height / 2 - text_wh.y / 2, 32, GRAY);

    // draw sidebar
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
