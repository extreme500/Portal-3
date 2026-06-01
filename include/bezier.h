#ifndef BEZIER_H
#define BEZIER_H

#include <glm/vec4.hpp>

// Retorna um ponto numa curva de Bézier cúbica para t ∈ [0,1].
// Os pontos de controle devem ser fornecidos com w=1 (pontos homogêneos).
inline glm::vec4 BezierCubic(glm::vec4 p0, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, float t)
{
    float u = 1.0f - t;
    return u*u*u*p0
         + 3.0f*u*u*t*p1
         + 3.0f*u*t*t*p2
         + t*t*t*p3;
}

#endif // BEZIER_H
