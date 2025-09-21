#ifndef PLAYER_H
#define PLAYER_H
#include "collider.h"
#include "food.h"

typedef struct {//
    AABB aabb;
    Circle circle;
    Vector2 pos;
    Vector2 targetPos;
    Vector2 dir;
    Vector2 splitDir;
    Vector2 velocity;

    Color color;
    float radius;
    float targetRadius;
    float lifetime;
    float splitTimer;
    float mergeTimer;
    float splitSpeed;

    bool collision;
    bool merge;
} Ball;

typedef struct Player {
    Ball *balls;
    int currentBallsCount;
    float mergeDuration;//вопрос к const в структуре
    float splitDuration;
}Player;

void InitPlayer(Player *player, Vector2 mapSize);
void EatingFood(Player *player, Food *food, int foodCount);
float MovementNormalize(Player player, Vector2 mousePos);
void SplitPlayer(Player* player, Vector2 mousePos, float minRadius, int maxBallsCount, float speed);
void ResolveCircleCollision(Player *player);
#endif //PLAYER_H
