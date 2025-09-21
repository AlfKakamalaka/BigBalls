#include "player.h"
#include "raymath.h"
#include "stdlib.h"
#include "food.h"

int cmp_ball_desc(const void *a, const void *b) {//для quicksort
    float ra = ((const Ball*)a)->radius;
    float rb = ((const Ball*)b)->radius;
    return (ra < rb) - (ra > rb); // -1 если ra>rb, 1 если ra<rb, 0 если равно
}

void InitPlayer(Player *player, Vector2 mapSize) {
    player->currentBallsCount = 1;
    player->mergeDuration = 3;
    player->splitDuration = 0.5f;
    player->balls = malloc(sizeof(Ball) * player->currentBallsCount);
    player->balls->radius = 20;
    player->balls->color = WHITE;
    player->balls->pos = (Vector2){mapSize.x / 2, mapSize.y / 2};
    player->balls->merge = false;
    player->balls->collision = false;
    player->balls->lifetime = 0;
    player->balls->targetRadius = 20;

    player->balls->targetPos = (Vector2){mapSize.x / 2, mapSize.y / 2};
    player->balls->mergeTimer = 0;
}

void EatingFood(Player *player, Food *food, int foodCount) {//
    for (int i = 0; i < player->currentBallsCount; i++) {
        for (int j = 0; j < foodCount; j++) {
            float dist = Vector2Distance(player->balls[i].pos, food[j].pos);
            if (dist < player->balls[i].radius + food[j].radius) {
                if (player->balls[i].radius > food[j].radius) {
                    player->balls[i].targetRadius = sqrtf(
                        player->balls[i].targetRadius * player->balls[i].targetRadius +
                        food[j].radius * food[j].radius
                    );
                    food[j].radius = 0;
                }
            }
        }
    }
}

float MovementNormalize(Player player, Vector2 mousePos) {
    float length = 0;
    const float maxDistance = 250;
    for (int i = 0; i < player.currentBallsCount; i++) {
        player.balls[i].dir = (Vector2){mousePos.x - player.balls[i].pos.x, mousePos.y - player.balls[i].pos.y};
        length = sqrtf(player.balls[i].dir.x * player.balls[i].dir.x + player.balls[i].dir.y * player.balls[i].dir.y);
        if (length > 0) {
            player.balls[i].dir.x /= length;
            player.balls[i].dir.y /= length;
        }
    }
    float normalizeDistance = Clamp(length / maxDistance,0,1);
    return normalizeDistance;
}

void SplitPlayer(Player* player, Vector2 mousePos,float minRadius, int maxBallsCount, float speed) {
    int oldCount = player->currentBallsCount;
    int count = 0;

    qsort(player->balls, oldCount, sizeof(Ball), cmp_ball_desc);
    for (int i = 0 ; i < oldCount; i++) {
        if (player->currentBallsCount + 1 > maxBallsCount) return;
        if (minRadius > player->balls[i].radius)continue;
        player->currentBallsCount += 1;
        player->balls = realloc(player->balls, sizeof(Ball) * player->currentBallsCount);
        player->balls[i].radius /= 2;
        player->balls[i].targetRadius /= 2;

        Ball newBall = player->balls[i];
        newBall.splitDir = (Vector2){mousePos.x - newBall.pos.x, mousePos.y - newBall.pos.y};
        float length = sqrtf(newBall.splitDir.x*newBall.splitDir.x + newBall.splitDir.y*newBall.splitDir.y);
        if (length > 0) {
            newBall.splitDir.x /= length;
            newBall.splitDir.y /= length;
        }

        newBall.splitSpeed = speed * sqrtf(newBall.radius / 7);
        newBall.splitTimer = 0;
        newBall.mergeTimer = 0;
        newBall.merge = false;
        newBall.lifetime = 0;
        newBall.collision = false;

        player->balls[oldCount + count] = newBall;
        count++;
    }
}

void ResolveCircleCollision(Player *player) {
    for (int i = 0; i < player->currentBallsCount; i++) {
        for (int j = i + 1; j < player->currentBallsCount; j++) {
            Vector2 pi = player->balls[i].pos;
            Vector2 pj = player->balls[j].pos;
            float ri = player->balls[i].radius;
            float rj = player->balls[j].radius;
            float minCollisionDist = ri + rj;

            if (player->balls[i].lifetime > player->splitDuration)
                player->balls[i].collision = true;
            if (player->balls[j].lifetime > player->splitDuration)
                player->balls[j].collision = true;

            if (player->balls[i].collision && player->balls[j].collision) {
                Vector2 d = Vector2Subtract(pj, pi);
                float dist = Vector2Length(d);
                dist = Clamp(dist, 0.00001f, minCollisionDist);

                float overlap = minCollisionDist - dist;
                if (overlap > 0.0f) {
                    Vector2 n = Vector2Normalize(d);
                    Vector2 corr = Vector2Scale(n, overlap * 0.5f);
                    player->balls[i].targetPos = Vector2Subtract(player->balls[i].targetPos, corr);
                    player->balls[j].targetPos = Vector2Add(player->balls[j].targetPos, corr);
                }
            }

            if (player->balls[i].merge || player->balls[j].merge){
                float dist = Vector2Distance(player->balls[i].pos, player->balls[j].pos);
                if (dist < minCollisionDist) {
                    if (player->balls[i].lifetime < 0.5)break;
                    if (player->balls[j].lifetime < 0.5)continue;
                    if (player->balls[i].radius > player->balls[j].radius) {
                        player->balls[i].targetRadius += player->balls[j].targetRadius;
                        player->balls[j] = player->balls[player->currentBallsCount-1];
                    }
                    else {
                        player->balls[j].targetRadius += player->balls[i].targetRadius;
                        player->balls[i] = player->balls[player->currentBallsCount-1];
                    }
                    player->currentBallsCount--;

                    player->balls[i].mergeTimer = 0;
                    player->balls[i].merge = false;
                }
            }
        }
    }
    player->balls = realloc(player->balls, sizeof(Ball) * player->currentBallsCount);
}