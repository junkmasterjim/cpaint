/*  --- cpaint ---
 *
 *  An MSPaint style application. Written in C with Raylib.
 *
 *  --- controls ---
 *  paint:                'left mouse button'
 *  clear canvas:         'c'
 *  brush:                'b'
 *  pencil:               'p'
 *  increase brush size:  'scroll+ || + (=)'
 *  decrease brush size:  'scroll- || -'
 *  next color:           'down arrow || right arrow'
 *  previous color:       'up arrow || left arrow'
 *  save as:              'ctrl-s'
 *
 * TODO: paint bucket: 'g'
 * TODO: undo: 'ctrl-z'
 * TODO: redo: 'ctrl-shift-z'
 *
 *  --- compile & run --
 *  clang -o cpaint paint.c -lraylib && ./cpaint
 */

// TODO: Choose background color in settings
// TODO: Maybe add an undo and redo tree? would be cool

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int NUM_COLORS = 23;

int main(void) {
  //-Variables----------------------------------------------------------------------
  int window_width = 1280;
  int window_height = 720;
  int cursor_radius = 64;
  int prev_radius;
  Vector2 mouse;
  Vector2 prev_mouse;
  Vector2 mouse_wheel;
  char *tool = "brush";
  Color colors[NUM_COLORS] = {
      RAYWHITE,  YELLOW,    GOLD,   ORANGE,     PINK,    RED,
      MAROON,    GREEN,     LIME,   DARKGREEN,  SKYBLUE, BLUE,
      DARKBLUE,  PURPLE,    VIOLET, DARKPURPLE, BEIGE,   BROWN,
      DARKBROWN, LIGHTGRAY, GRAY,   DARKGRAY,   BLACK};
  Rectangle color_rectangles[NUM_COLORS] = {};
  bool is_saving = false;
  int save_message_counter = 0;
  char *filename = (char *)malloc(16 * sizeof(char));
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
  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow(window_width, window_height, "cpaint");

  RenderTexture2D canvas = LoadRenderTexture(window_width - 50, window_height);
  ClearBackground(RAYWHITE);
  SetTargetFPS(120);

  for (int i = 0; i < NUM_COLORS; i++) {
    color_rectangles[i].x = 4;
    color_rectangles[i].y = 4 + (window_height / 30.0) * i + 2 * i;
    color_rectangles[i].width = 40;
    color_rectangles[i].height = window_height / 30.0;
  }

  int selected_color = 22;
  int color_hovered = -1;

  //--------------------------------------------------------------------------------

  //-Main-Loop----------------------------------------------------------------------
  while (!WindowShouldClose()) {
    //-Update-------------------------------------------------------------------------
    mouse = GetMousePosition();
    mouse_wheel = GetMouseWheelMoveV();

    // Select tool with B (brush) or P (pencil) (default B brush)
    if (IsKeyPressed(KEY_P) && strcmp(tool, "pencil") != 0) {
      tool = "pencil";
      prev_radius = cursor_radius;
      cursor_radius = 4;
    } else if (IsKeyPressed(KEY_B) && strcmp(tool, "brush") != 0) {
      tool = "brush";
      if (prev_radius) {
        cursor_radius = prev_radius;
      }
    }

    // Clear canvas with C
    if (IsKeyPressed(KEY_C)) {
      BeginTextureMode(canvas);
      ClearBackground(colors[0]);
      EndTextureMode();
    }

    // Update cursor size on scroll as long as we are using a brush
    if (strcmp(tool, "brush") == 0) {
      if ((mouse_wheel.y > 0 || IsKeyPressed(KEY_EQUAL)) &&
          cursor_radius <= 256) {
        cursor_radius += 8;
      } else if ((mouse_wheel.y < 0 || IsKeyPressed(KEY_MINUS)) &&
                 cursor_radius > 8) {
        cursor_radius -= 8;
      } else if ((mouse_wheel.y < 0 || IsKeyPressed(KEY_MINUS)) &&
                 cursor_radius <= 8) {
        cursor_radius = 4;
      }
    }

    // Cycle through colors with arrow keys
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT)) {
      if (selected_color == NUM_COLORS - 1) {
        selected_color = 0;
      } else
        selected_color++;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_LEFT)) {
      if (selected_color == 0) {
        selected_color = NUM_COLORS - 1;
      } else
        selected_color--;
    }

    // Select colors with mouse
    for (int i = 0; i < NUM_COLORS; i++) {
      if (CheckCollisionPointRec(mouse, color_rectangles[i])) {
        color_hovered = i;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          selected_color = i;
        }
      }
    }
    if (!CheckCollisionPointRec(mouse, color_rectangles[color_hovered])) {
      color_hovered = -1;
    }

    // Update the canvas when the mouse is clicked
    Vector2 canvas_mouse = (Vector2){mouse.x - 50, mouse.y};
    Vector2 canvas_prev_mouse = (Vector2){prev_mouse.x - 50, prev_mouse.y};

    if (canvas_mouse.x > 50 && canvas_prev_mouse.x &&
        IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      BeginTextureMode(canvas);
      if (strcmp(tool, "pencil") == 0) {
        DrawLineEx(canvas_prev_mouse, canvas_mouse, cursor_radius,
                   colors[selected_color]);
      } else if (strcmp(tool, "brush") == 0) {
        DrawCircleV(canvas_mouse, cursor_radius, colors[selected_color]);
      }
      EndTextureMode();
    }
    prev_mouse = GetMousePosition();

    // Save file with 'ctrl-s'
    if (!is_saving && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
      int r = rand();
      sprintf(filename, "%d.png", r);

      Image canvas_image = LoadImageFromTexture(canvas.texture);
      ImageFlipVertical(&canvas_image);
      ExportImage(canvas_image, filename);

      is_saving = true;
    }
    if (is_saving) {
      save_message_counter++;
      if (save_message_counter >= 240) {
        is_saving = false;
        save_message_counter = 0;
      }
    }
    //--------------------------------------------------------------------------------

    //-Draw---------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw the canvas
    DrawTextureRec(canvas.texture,
                   (Rectangle){0, 0, (float)canvas.texture.width,
                               (float)-canvas.texture.height},
                   (Vector2){50, 0}, WHITE);

    // Draw the mouse guide
    // NOTE: Left shift will show the mouse
    if (mouse.x > 50 && !IsKeyDown(KEY_LEFT_SHIFT)) {
      HideCursor();
      DrawCircleV(mouse, cursor_radius, colors[selected_color]);
      if (selected_color == NUM_COLORS - 1) {
        DrawCircleLinesV(mouse, cursor_radius + 1, LIGHTGRAY);
      } else
        DrawCircleLinesV(mouse, cursor_radius + 1, BLACK);
    } else
      ShowCursor();

    // Draw the sidebar
    DrawRectangle(0, 0, 48, window_height, LIGHTGRAY);
    DrawRectangle(48, 0, 2, window_height, GRAY);
    // Draw the color selection rectangles
    for (int i = 0; i < NUM_COLORS; i++) {
      DrawRectangleRec(color_rectangles[i], colors[i]);
    }
    if (color_hovered >= 0) {
      DrawRectangleRec(color_rectangles[color_hovered], Fade(WHITE, 0.6f));
    }

    // Draw save dialog if we are saving
    if (is_saving) {

      char *saved_as = malloc(24 * sizeof(char));
      strcat(saved_as, "Image saved: ");
      strcat(saved_as, filename);

      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                    Fade(RAYWHITE, 0.8f));
      DrawRectangle(0, 150, GetScreenWidth(), 80, BLACK);
      DrawText(saved_as, (window_width / 2) - MeasureText(saved_as, 20) / 2,
               180, 20, RAYWHITE);
    }

    EndDrawing();
    //--------------------------------------------------------------------------------
  }

  //-De-Initialization--------------------------------------------------------------
  free(filename);
  UnloadRenderTexture(canvas);
  CloseWindow();
  //--------------------------------------------------------------------------------

  return 0;
}
