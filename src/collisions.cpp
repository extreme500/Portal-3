// ===========================================================================
//  Colisões (Portal-3) — implementação.
//
//  Ver include/collisions.h para a visão geral do módulo.
// ===========================================================================

#include "collisions.h"

#include <limits>
#include <glm/glm.hpp> // glm::clamp, glm::dot, glm::min, glm::max

// Testa se um cilindro vertical (centrado em pos_xz, com raio fixo e faixa de
// altura [y_min, y_max]) sobrepõe a CollisionAABB 'box'. O teste é decomposto
// em (1) overlap do intervalo vertical e (2) distância do centro do círculo
// (projeção do cilindro no plano XZ) ao retângulo formado pela AABB em XZ.
bool CylinderIntersectsAABB(glm::vec2 pos_xz, float raio, float y_min, float y_max, const CollisionAABB& box)
{
    // Overlap no eixo vertical (Y)
    if (y_max < box.min.y || y_min > box.max.y)
        return false;

    // Ponto do retângulo (projeção da AABB em XZ) mais próximo do centro do círculo
    float closest_x = glm::clamp(pos_xz.x, box.min.x, box.max.x);
    float closest_z = glm::clamp(pos_xz.y, box.min.z, box.max.z);

    float dx = pos_xz.x - closest_x;
    float dz = pos_xz.y - closest_z;

    return (dx*dx + dz*dz) < (raio * raio);
}

bool CylinderIntersectsLine(glm::vec2 pos_xz, float raio, float y_min_jogador, float y_max_jogador, const CollisionLine& line)
{
    // 1. Testa o overlap no eixo vertical (Y)
    if (y_max_jogador < line.y_min || y_min_jogador > line.y_max)
        return false;

    // 2. Matemática vetorial no plano XZ
    glm::vec2 ab = line.p2 - line.p1;       // Vetor da parede
    glm::vec2 ap = pos_xz - line.p1;        // Vetor do início da parede até o jogador

    // Calcula a projeção escalar de AP sobre AB
    float proj = glm::dot(ap, ab);
    float ab_len_sq = glm::dot(ab, ab);     // Comprimento da parede ao quadrado

    // Calcula 't', que é a porcentagem da projeção ao longo da parede
    float t = proj / ab_len_sq;

    // Grampeia 't' entre 0 e 1 para garantir que não estamos checando um ponto além da parede
    t = glm::clamp(t, 0.0f, 1.0f);

    // Encontra a coordenada exata do ponto mais próximo na parede
    glm::vec2 closest_point = line.p1 + t * ab;

    // Calcula a distância do jogador até esse ponto
    glm::vec2 distance_vec = pos_xz - closest_point;

    // Se a distância ao quadrado for menor que o raio ao quadrado, é colisão!
    return glm::dot(distance_vec, distance_vec) < (raio * raio);
}

CollisionAABB ComputeWorldAABB(glm::vec3 local_min, glm::vec3 local_max, const glm::mat4& model)
{
    glm::vec3 corners[8] = {
        glm::vec3(local_min.x, local_min.y, local_min.z),
        glm::vec3(local_max.x, local_min.y, local_min.z),
        glm::vec3(local_min.x, local_max.y, local_min.z),
        glm::vec3(local_max.x, local_max.y, local_min.z),
        glm::vec3(local_min.x, local_min.y, local_max.z),
        glm::vec3(local_max.x, local_min.y, local_max.z),
        glm::vec3(local_min.x, local_max.y, local_max.z),
        glm::vec3(local_max.x, local_max.y, local_max.z),
    };

    glm::vec3 world_min( std::numeric_limits<float>::max());
    glm::vec3 world_max(-std::numeric_limits<float>::max());
    for (const glm::vec3& corner : corners)
    {
        glm::vec3 world_corner = glm::vec3(model * glm::vec4(corner, 1.0f));
        world_min = glm::min(world_min, world_corner);
        world_max = glm::max(world_max, world_corner);
    }

    return CollisionAABB{ world_min, world_max };
}
