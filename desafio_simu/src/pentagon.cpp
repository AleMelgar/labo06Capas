#include "pentagon.h"
#include <math.h>

void DrawPentagon(Vector2 center, float radius, Color color) {
    DrawPoly(center, 5, radius, -54, color);
}

void DrawHousePentagon(Vector2 center, float radius, Color color) {
    Vector2 points[5];

    float baseY = center.y + radius * 0.6f;
    float roofY = center.y - radius;
    float halfBase = radius * 0.8f;

    points[0] = (Vector2){ center.x - halfBase, baseY };
    points[1] = (Vector2){ center.x + halfBase, baseY };
    points[2] = (Vector2){ center.x + radius, center.y };
    points[3] = (Vector2){ center.x, roofY };
    points[4] = (Vector2){ center.x - radius, center.y };

    for (int i = 0; i < 5; i++) {
        DrawLineV(points[i], points[(i + 1) % 5], color);
    }
}