#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "player.h"
#include "food.h"
#include "reasings.h"

#define REASINGS_STATIC_INLINE

void GetAABBForPlayer(Player *player) {
    for (int i = 0; i < player->currentBallsCount; i++) {
        player->balls[i].aabb.pos.x = player->balls[i].pos.x - player->balls[i].radius * 2;//Выравниваем, так как все в raylib по дефолту рисуется от верхнего левого угла.
        player->balls[i].aabb.pos.y = player->balls[i].pos.y - player->balls[i].radius * 2;
        player->balls[i].aabb.width = player->balls[i].radius * 4;
        player->balls[i].aabb.height = player->balls[i].radius * 4;
    }
}

void GetCircleForPlayer(Player *player) {
    for (int i = 0; i < player->currentBallsCount; i++) {
        player->balls[i].circle.pos.x = player->balls[i].aabb.pos.x + player->balls[i].aabb.width / 2;//Выравниваем.
        player->balls[i].circle.pos.y = player->balls[i].aabb.pos.y + player->balls[i].aabb.height / 2;
        player->balls[i].circle.radius = player->balls[i].aabb.width / 4;
    }
}

int main(void) {
    const Vector2 screen = {800,800};
    const Vector2 map_size = {2000, 2000};
    const int cell_size = 25;
    const int food_count = 300;

    const int minBallRadius = 20;
    const int maxBallsCount = 8;
    const float minSpeed = 150;
    const float maxBallSpeed = 900;
    const float mergeDuration = 2;
    const float splitDuration = 0.6f;
    const float posSmoothSpeed = 10;
    const float radiusSmoothSpeed = 5;
    const float forceTight = 25;

    float targetZoom = 1;
    float zoomSpeed = 1;

    Player player;
    InitPlayer(&player, map_size);
    Food *food = malloc(sizeof(Food) * food_count);
    InitFood(food, food_count, map_size);
    Camera2D camera = {screen.x/2, screen.y/2, player.balls->pos, 0, targetZoom};

    InitWindow(screen.x, screen.y, "Balls");
    Image img = GenImageGradientLinear(map_size.x,map_size.y, 0, DARKPURPLE, BLACK);
    Texture2D texture = LoadTextureFromImage(img);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        float sumX = 0;
        float sumY = 0;
        float radius = 0;
        Vector2 center = {0,0};

        for (int i = 0; i < player.currentBallsCount; i++) {
            sumX += player.balls[i].pos.x;
            sumY += player.balls[i].pos.y;
            radius += player.balls[i].radius;
        }

        center.x = sumX / player.currentBallsCount;
        center.y = sumY / player.currentBallsCount;
        targetZoom = 1 / sqrtf(radius/50);
        camera.target = center;
        camera.zoom = Lerp(camera.zoom, targetZoom, deltaTime * zoomSpeed);
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

        float normalizeDistance = MovementNormalize(player, mousePos);
        for (int i = 0; i < player.currentBallsCount; i++) {
            float speedCof = normalizeDistance * maxBallSpeed;
            float speed = (minSpeed + speedCof) / sqrtf(player.balls[i].radius);

            Vector2 dirToCenter = Vector2Subtract(center, player.balls[i].pos);
            dirToCenter = Vector2Normalize(dirToCenter);
            Vector2 velocityToCenter = Vector2Scale(dirToCenter,forceTight);
            player.balls[i].velocity = Vector2Add(player.balls[i].velocity, velocityToCenter);

            Vector2 movementVelocity = Vector2Scale(player.balls[i].dir, speed);
            player.balls[i].velocity = Vector2Add(player.balls[i].velocity, movementVelocity);

            if (player.balls[i].splitTimer < splitDuration) {//Split
                float ease = EaseBackOut(player.balls[i].splitTimer, 0, player.balls[i].splitSpeed, splitDuration);
                Vector2 splitVelocity = Vector2Scale(player.balls[i].splitDir, ease);
                player.balls[i].velocity = Vector2Add(player.balls[i].velocity, splitVelocity);
                player.balls[i].splitTimer += deltaTime;
            }

            player.balls[i].targetPos = Vector2Add(player.balls[i].targetPos, Vector2Scale(player.balls[i].velocity, deltaTime));
            player.balls[i].velocity = (Vector2){0,0};
            player.balls[i].targetPos.x = Clamp(player.balls[i].targetPos.x, player.balls[i].radius, map_size.x - player.balls[i].radius);
            player.balls[i].targetPos.y = Clamp(player.balls[i].targetPos.y, player.balls[i].radius, map_size.y - player.balls[i].radius);
        }
        ////////////////////////////////////////////////////////ПОЛНОСТЬЮ РАЗОБРАТЬ БЛОК ВЫШЕ!!//////////////////////////////////////

        if (IsKeyPressed(KEY_SPACE)) {
            SplitPlayer(&player, mousePos, minBallRadius, maxBallsCount, minSpeed);
            for (int i = 0; i < player.currentBallsCount; i++) {
                player.balls[i].merge = false;
            }
        }

        for (int i = 0; i < player.currentBallsCount; i++) {//БЛОК ПЛАВНОСТИ
            player.balls[i].lifetime += deltaTime;//
            player.balls[i].pos = Vector2Lerp(player.balls[i].pos, player.balls[i].targetPos, deltaTime * posSmoothSpeed);
            player.balls[i].radius = Lerp(player.balls[i].radius, player.balls[i].targetRadius, deltaTime * radiusSmoothSpeed);
            if (!player.balls[i].merge) {
                player.balls[i].mergeTimer += deltaTime;
                if (player.balls[i].mergeTimer >= mergeDuration) {
                    player.balls[i].merge = true;
                }
            }
        }

        for (int i = 0; i < player.currentBallsCount; i++) {
            ImageDrawCircle(&img, player.balls[i].pos.x, player.balls[i].pos.y, player.balls[i].radius, WHITE);
        }

        UpdateTexture(texture, img.data);
        EatingFood(&player, food, food_count);
        GetAABBForPlayer(&player);
        GetCircleForPlayer(&player);
        ResolveCircleCollision(&player);

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(camera);
        DrawTexture(texture, 0,0, WHITE);

        for (int x = 0; x <= map_size.x; x += cell_size) {//Рисуем сетку.
            DrawLine(x,0,x, map_size.y, LIGHTGRAY);
        }
        for (int y = 0; y <= map_size.y; y += cell_size) {
            DrawLine(0,y,map_size.x, y, LIGHTGRAY);
        }

        for (int i = 0; i < player.currentBallsCount; i++) {
            DrawCircleV(player.balls[i].pos, player.balls[i].radius, player.balls[i].color);//Рисуем персонажа.
            DrawLineV(player.balls[i].pos, mousePos, RED);

            DrawText(TextFormat("distance: %0.3f", Vector2Distance(player.balls[i].pos, mousePos)), (player.balls[i].pos.x + mousePos.x)/2, (player.balls[i].pos.y + mousePos.y)/2, 20, ORANGE);
            DrawText(TextFormat("lifeTime: %0.3f", player.balls[i].lifetime), (player.balls[i].pos.x-player.balls[i].radius ), (player.balls[i].pos.y), 20, GREEN);

            DrawRectangleLines(player.balls[i].aabb.pos.x,player.balls[i].aabb.pos.y, player.balls[i].aabb.width, player.balls[i].aabb.height, RED);
            DrawCircleLinesV(player.balls[i].circle.pos, player.balls[i].circle.radius, RED);
        }

        for (int i = 0; i < food_count; i++) {//Рисуем еду.
            DrawCircleV(food[i].pos, food[i].radius,food[i].color);
        }
        EndMode2D();

        DrawText(TextFormat("Radius: %f", player.balls->radius), 30, 90, 30, BLACK);
        DrawText(TextFormat("Balls: %d", player.currentBallsCount),30, 60, 30, BLACK);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
