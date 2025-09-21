#include "food.h"

void InitFood(Food *food, int foodCount, Vector2 mapSize) {
    for (int i = 0; i < foodCount; i++) {
        food[i].radius = GetRandomValue(3, 25);
        food[i].pos = (Vector2){GetRandomValue(1, mapSize.x - 1), GetRandomValue(1, mapSize.y - 1)};
        food[i].color = (Color){GetRandomValue(15, 255), GetRandomValue(5,255), GetRandomValue(1,255), 255};
    }
}
