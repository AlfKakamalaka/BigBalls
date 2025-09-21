#ifndef COLLIDER_H
#define COLLIDER_H
#include "raylib.h"

typedef struct {
    Vector2 pos;
    float width;
    float height;
}AABB;

typedef struct {
    Vector2 pos;
    float radius;
}Circle;

#endif //COLLIDER_H
