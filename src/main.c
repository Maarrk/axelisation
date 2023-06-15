#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef enum TileId {
    TILE_PLAYER,
    TILE_BLOCK,
    TILE_COUNT,
} TileId;

typedef struct Tile {
    Texture2D texture;
    Rectangle collider;
} Tile;

Tile tiles[TILE_COUNT];

typedef struct Solid {
    TileId tileId;
    Vector2 position;
} Solid;

#define SOLID_COUNT 9

Solid solids[SOLID_COUNT];

typedef enum Action { ACTION_DO_NOTHING, ACTION_COUNT } Action;

typedef struct Actor {
    TileId tileId;
    Vector2 position;
    Vector2 moveRemainder;
    Action onCollision;
} Actor;

void ActorMove(Actor *actor, Vector2 movementAmount);

bool CollideAt(Actor actor, Vector2 position);

void HandleCollision(Actor *actor, Vector2 movementSign);

int main(int argc, char *argv[]) {
    const Vector2 targetSize = {640.0f, 360.0f};
    const float initialScale = 2.0f;
    Vector2 windowSize = Vector2Scale(targetSize, initialScale);

    const int tileSize = 32;

    InitWindow((int)windowSize.x, (int)windowSize.y, "Axelisation - basic window");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    {
        const char imagePaths[TILE_COUNT][64] = {
            "resources/basicguy.png",   // TILE_PLAYER
            "resources/basicblock.png", // TILE_BLOCK
        };
        const Rectangle colliders[TILE_COUNT] = {
            {3, 0, 26, tileSize},       // TILE_PLAYER
            {0, 0, tileSize, tileSize}, // TILE_BLOCK
        };
        for (size_t i = 0; i < TILE_COUNT; i++) {
            Image image = LoadImage(imagePaths[i]);
            tiles[i].texture = LoadTextureFromImage(image);
            tiles[i].collider = colliders[i];
            UnloadImage(image);
        }
    }

    Actor playerActor = {TILE_PLAYER, Vector2Zero(), Vector2Zero(), ACTION_DO_NOTHING};

    {
        solids[0] = (Solid){TILE_BLOCK, (Vector2){0, 1}};
        solids[1] = (Solid){TILE_BLOCK, (Vector2){1, 1}};
        solids[2] = (Solid){TILE_BLOCK, (Vector2){2, 1}};
        solids[3] = (Solid){TILE_BLOCK, (Vector2){3, 1}};
        solids[4] = (Solid){TILE_BLOCK, (Vector2){3, 0}};
        solids[5] = (Solid){TILE_BLOCK, (Vector2){-1, 1}};
        solids[6] = (Solid){TILE_BLOCK, (Vector2){-2, 1}};
        solids[7] = (Solid){TILE_BLOCK, (Vector2){-3, 1}};
        solids[8] = (Solid){TILE_BLOCK, (Vector2){-3, 0}};
        for (size_t i = 0; i < SOLID_COUNT; i++) {
            solids[i].position = Vector2Scale(solids[i].position, tileSize);
        }
    }

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

    float playerWalkVelocity = 64.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 playerVelocity = Vector2Zero();
        if (IsKeyDown(KEY_RIGHT)) {
            playerVelocity.x += playerWalkVelocity;
        }
        if (IsKeyDown(KEY_LEFT)) {
            playerVelocity.x -= playerWalkVelocity;
        }
        ActorMove(&playerActor, Vector2Scale(playerVelocity, dt));

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
            Vector2Add(playerActor.position, Vector2Scale(Vector2One(), tileSize * 0.5f));
        cameraWorld.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        cameraWorld.zoom = currentScale;

        cameraUI.zoom = currentScale;

        {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            {
                BeginMode2D(cameraWorld);

                DrawTextureV(tiles[playerActor.tileId].texture, playerActor.position, WHITE);

                for (size_t i = 0; i < SOLID_COUNT; i++) {
                    DrawTextureV(tiles[solids[i].tileId].texture, solids[i].position, WHITE);
                }

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

    for (size_t i = 0; i < TILE_COUNT; i++) {
        UnloadTexture(tiles[i].texture);
    }
    CloseWindow();

    return 0;
}

void ActorMove(Actor *actor, Vector2 movementAmount) {
    // Adapted from https://maddymakesgames.com/articles/celeste_and_towerfall_physics/index.html
    { // Move in X
        actor->moveRemainder.x += movementAmount.x;
        int move = roundf(actor->moveRemainder.x);
        if (move != 0) {
            actor->moveRemainder.x -= move;
            int sign = move > 0 ? 1 : -1;
            while (move != 0) {
                if (!CollideAt(*actor, Vector2Add(actor->position, (Vector2){sign, 0}))) {
                    // New position doesn't overlap a solid
                    actor->position.x += sign;
                    move -= sign;
                } else { // Would hit a solid in new position
                    HandleCollision(actor, (Vector2){sign, 0});
                    // Remainder can be left there, it's less than one pixel
                    break;
                }
            }
        }
    }
    { // Move in Y
        actor->moveRemainder.y += movementAmount.y;
        int move = roundf(actor->moveRemainder.y);
        if (move != 0) {
            actor->moveRemainder.y -= move;
            int sign = move > 0 ? 1 : -1;
            while (move != 0) {
                if (!CollideAt(*actor, Vector2Add(actor->position, (Vector2){0, sign}))) {
                    // New position doesn't overlap a solid
                    actor->position.y += sign;
                    move -= sign;
                } else { // Would hit a solid in new position
                    HandleCollision(actor, (Vector2){0, sign});
                    break;
                }
            }
        }
    }
}

bool CollideAt(Actor actor, Vector2 position) {
    Rectangle checkedCollider = tiles[actor.tileId].collider;
    checkedCollider.x += position.x;
    checkedCollider.y += position.y;

    for (size_t i = 0; i < SOLID_COUNT; i++) {
        Rectangle solidCollider = tiles[solids[i].tileId].collider;
        solidCollider.x += solids[i].position.x;
        solidCollider.y += solids[i].position.y;

        if (CheckCollisionRecs(checkedCollider, solidCollider))
            return true;
    }

    return false;
}

void HandleCollision(Actor *actor, Vector2 movementSign) {
    switch (actor->onCollision) {
    case ACTION_DO_NOTHING:
        return;
    }
}
