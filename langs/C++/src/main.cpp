#include <raylib.h>

import vector;

int main() {
  InitWindow(800, 600, "Position Based Fluids in C++23");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();
  }

  CloseWindow();
}
