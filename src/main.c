#include "raylib.h"

int main(int argc, char *argv[]) {
    TraceLog(LOG_INFO, "Hello from C raylib built with Zig");

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Axelisation - basic window");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        { // this scope just to make the linter indent drawing context
            ClearBackground(RAYWHITE);
            DrawText("Text inside window!", 280, 200, 20, DARKGRAY);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}