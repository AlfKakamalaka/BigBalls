#ifndef FOOD_H
#define FOOD_H
#include "raylib.h"

typedef struct {
    Vector2 pos;
    Color color;
    float radius;
} Food;

void InitFood(Food *food, int foodCount, Vector2 mapSize);
#endif //FOOD_H
