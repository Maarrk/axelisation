#include "raylib.h"
#include "raymath.h"
#include <math.h>

int main(int argc, char *argv[]) {
    TraceLog(LOG_INFO, "Hello from C raylib built with Zig");

    const Vector2 targetSize = {640.0f, 360.0f};
    const float initialScale = 2.0f;
    Vector2 windowSize = Vector2Scale(targetSize, initialScale);

    const int tileSize = 32;

    InitWindow((int)windowSize.x, (int)windowSize.y, "Axelisation - basic window");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Image image = LoadImage("resources/basicguy32px.png");
    Texture2D basicguyTexture = LoadTextureFromImage(image);
    UnloadImage(image);

    Camera2D cameraWorld = {0};
    cameraWorld.target = Vector2Scale(targetSize, 0.5f);
    cameraWorld.offset = Vector2Zero();
    cameraWorld.rotation = 0.0f;
    cameraWorld.zoom = 1.0f;

    Camera2D cameraUI = {0};
    cameraUI.target = Vector2Zero();
    cameraUI.offset = Vector2Zero();
    cameraUI.rotation = 0.0f;
    cameraUI.zoom = 1.0f;

    float currentScale;
    int currentWidth;
    int currentHeight;

    Vector2 playerPosition = Vector2Zero();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F)) {
            if (!IsWindowFullscreen()) {
                windowSize.x = GetScreenWidth();
                windowSize.y = GetScreenHeight();
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
            } else {
                ToggleFullscreen();
                SetWindowSize(windowSize.x, windowSize.y);
            }
        }

        currentScale = roundf(GetScreenWidth() / targetSize.x);
        currentWidth = roundf(GetScreenWidth() / currentScale);
        currentHeight = roundf(GetScreenHeight() / currentScale);

        cameraWorld.target =
            Vector2Add(playerPosition, Vector2Scale(Vector2One(), tileSize * 0.5f));
        cameraWorld.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        cameraWorld.zoom = currentScale;

        cameraUI.zoom = currentScale;

        {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            {
                BeginMode2D(cameraWorld);
                DrawTextureV(basicguyTexture, playerPosition, WHITE);
                EndMode2D();
            }

            {
                BeginMode2D(cameraUI);
                DrawText("Top left UI", 8, 8, 20, DARKGRAY);
                DrawText("Bottom left UI", 8, currentHeight - 28, 20, DARKGRAY);
                DrawFPS(currentWidth - 96, 8);
                EndMode2D();
            }

            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}