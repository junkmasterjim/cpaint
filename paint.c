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
 *  save:                 'ctrl-s'
 *  undo:                 'ctrl-z'
 *  TODO: redo: 'ctrl-shift-z'
 *
 *  --- compile & run --
 *  clang -o cpaint paint.c -lraylib && ./cpaint
 */

// TODO: Choose background color in settings
// TODO: maybe paint bucket too

// FIX: undo looks and works better but still has some bugs
// FIX: when selecting multiple colors, the stroke seems to lose track of where
// we are deleting.
// step 1: draw a bg in a color
// step 2: draw an accent in any other color
// step 3: write 1 to 10 in a different color from the first 2
// step 4: press 'ctrl-z' and you should see the first few numbers removed, and
//         both backgrounds deleted

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-Definitions-&-Constants--------------------------------------------------------
#define MAX_UNDOS 10         // Max undo steps allowed
#define INITIAL_CAPACITY 100 // Starting capacity for points in a stroke
#define NUM_COLORS 23        // The amount of colors available for use

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
int selected_color;
Rectangle color_rectangles[NUM_COLORS] = {};
bool is_saving = false;
int save_message_counter = 0;
//--------------------------------------------------------------------------------

// Struct to store a Point as a (Vector2){int x, y}
typedef struct Vector2 Point;

// Struct to store brush strokes
typedef struct {
  Point *points;   // Dynamic array of points in this stroke
  int point_count; // Number of points currently stored
  int max_points;  // Max capacity of *points array
  char *tool;      // Tool being used 'pencil' or 'brush'
  int color;       // Color used
  int radius;      // Radius used
} Stroke;

// Struct to store undo history
typedef struct {
  Stroke undos[MAX_UNDOS]; // Array to store undo steps up to defined max
  int current_undo_index;  // Current position in undo history
  int undo_count;          // Total amount of undo steps
} UndoHistory;

// Init a new Stroke with intiial capacity
void initStroke(Stroke *stroke) {
  stroke->points = (Point *)malloc(sizeof(Point) * INITIAL_CAPACITY);
  stroke->point_count = 0;
  stroke->max_points = INITIAL_CAPACITY;
  stroke->color = selected_color;
  stroke->radius = cursor_radius;
  stroke->tool = tool;
}

// Init undo history
void initHistory(UndoHistory *history) {
  history->current_undo_index = -1;
  history->undo_count = 0;
}

// Adds a point to the current stroke, resizing if necessary
void addToStroke(Stroke *stroke, int x, int y, int color, int radius,
                 char *stroke_tool) {
  // Check if we need to resize points array
  if (stroke->point_count >= stroke->max_points) {
    stroke->max_points *= 2;
    stroke->points =
        (Point *)realloc(stroke->points, stroke->max_points * sizeof(Point));
  }
  // Add point to the stroke
  stroke->points[stroke->point_count].x = x;
  stroke->points[stroke->point_count].y = y;
  stroke->point_count++;
  stroke->color = color;
  stroke->radius = radius;
  stroke->tool = stroke_tool;
}

// Add a completed stroke to the undo history
void addUndoStep(UndoHistory *history, Stroke *stroke) {
  // Advance to next undo step
  history->current_undo_index = (history->current_undo_index + 1) % MAX_UNDOS;
  if (history->undo_count < MAX_UNDOS) {
    history->undo_count++;
  }

  // Free any existing stroke points in the current undo slot
  free(history->undos[history->current_undo_index].points);

  // Copy the completed stroke into the undo history
  history->undos[history->current_undo_index] = *stroke;

  // Reinitialize stroke for the next pass
  initStroke(stroke);
}

// Free memory in undo history
void freeUndoHistory(UndoHistory *history) {
  for (int i = 0; i < history->undo_count; i++) {
    free(history->undos[i].points);
  }
}
//--------------------------------------------------------------------------------

//-MAIN-PROGRAM-ENTRY-POINT-------------------------------------------------------
int main(void) {

  char *filename = (char *)malloc(16 * sizeof(char));

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

    // Draw window dimensions selection
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

  int color_hovered = -1;

  UndoHistory history;
  Stroke stroke;
  initStroke(&stroke);
  initHistory(&history);
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

    // Update cursor size on scroll for brush
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
    // Update cursor size on scroll for pencil
    if (strcmp(tool, "pencil") == 0) {
      if ((mouse_wheel.y > 0 || IsKeyPressed(KEY_EQUAL)) &&
          cursor_radius <= 6) {
        cursor_radius += 2;
      } else if ((mouse_wheel.y < 0 || IsKeyPressed(KEY_MINUS)) &&
                 cursor_radius > 2) {
        cursor_radius -= 2;
      } else if ((mouse_wheel.y < 0 || IsKeyPressed(KEY_MINUS)) &&
                 cursor_radius <= 2) {
        cursor_radius = 2;
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

    if (canvas_mouse.x > 0 && canvas_prev_mouse.x &&
        IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      BeginTextureMode(canvas);
      addToStroke(&stroke, canvas_mouse.x, canvas_mouse.y, selected_color,
                  cursor_radius, tool);

      if (strcmp(tool, "pencil") == 0) {
        DrawLineEx(canvas_prev_mouse, canvas_mouse, cursor_radius,
                   colors[selected_color]);
      } else if (strcmp(tool, "brush") == 0) {
        DrawCircleV(canvas_mouse, cursor_radius, colors[selected_color]);
      }
      EndTextureMode();
    } else if (stroke.point_count > 0) {
      addUndoStep(&history, &stroke);
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

    // Handle undo with 'ctrl-z'
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)) {
      if (history.undo_count > 0) {
        // Remove the last stroke
        history.undo_count--;
        history.current_undo_index =
            (history.current_undo_index - 1 + MAX_UNDOS) % MAX_UNDOS;

        // Redraw the canvas
        BeginTextureMode(canvas);
        ClearBackground(colors[0]);
        for (int i = 0; i < history.undo_count; i++) {
          int index = (history.current_undo_index - i + MAX_UNDOS) % MAX_UNDOS;
          Stroke *s = &history.undos[index];

          if (strcmp(s->tool, "brush") == 0) {
            for (int j = 1; j < s->point_count; j++) {
              DrawCircleV((Vector2){s->points[j - 1].x, s->points[j - 1].y},
                          s->radius, colors[s->color]);
            }
          } else if (strcmp(s->tool, "pencil") == 0) {
            for (int j = 1; j < s->point_count; j++) {
              DrawLineEx(s->points[j - 1], s->points[j], cursor_radius,
                         colors[s->color]);
            }
          }
        }
        EndTextureMode();
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
  freeUndoHistory(&history);
  UnloadRenderTexture(canvas);
  CloseWindow();
  //--------------------------------------------------------------------------------

  return 0;
}
