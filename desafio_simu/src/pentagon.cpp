#include "pentagon.h"

void DrawPentagon(Vector2 center, float radius, Color color) {
    DrawPoly(center, 5, radius, -90, color);
}