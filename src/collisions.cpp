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

// ----------------------------------------------------------------------------
// Queries de colisão por entidade (jogador, caixa, rádio).
// ----------------------------------------------------------------------------

// Testa se o cilindro do jogador, na posição candidata 'pos' (mesma referência
// de Pos_Player — não os pés diretamente, ver PLAYER_FEET_Y_OFFSET), colide
// com alguma AABB do cenário registrada em w.aabbs.
//
// O intervalo vertical testado é "encolhido" por COLLISION_SKIN em cada ponta:
// sem isso, ao ficar exatamente apoiado no chão (pés tocando o topo da AABB do
// piso) o teste consideraria o jogador permanentemente colidindo também na
// horizontal, travando todo o movimento. Esse encolhimento permite uma
// penetração tolerável de poucos centímetros, imperceptível no jogo.
bool PlayerCollidesAt(const CollisionWorld& w, const SceneState& s, const glm::vec3& pos)
{
    glm::vec2 pos_xz(pos.x, pos.z);
    float feet_y = pos.y + PLAYER_FEET_Y_OFFSET;
    float y_min = feet_y + COLLISION_SKIN;
    float y_max = feet_y + w.player.altura - COLLISION_SKIN;

    // Testa colisões com caixas estáticas (cenário)
    for (const CollisionAABB& box : w.aabbs)
    {
        if (CylinderIntersectsAABB(pos_xz, w.player.raio, y_min, y_max, box))
            return true;
    }

    // Testa colisões com paredes diagonais
    for (const CollisionLine& line : w.lines) {
        if (CylinderIntersectsLine(pos_xz, w.player.raio, y_min, y_max, line))
            return true;
    }

    // Testa colisão dinâmica com a PORTA FECHADA
    if (!s.buttonPressed) {
        if (CylinderIntersectsAABB(pos_xz, w.player.raio, y_min, y_max, w.closedDoor))
            return true;
    }

    // Testa colisão dinâmica com a CAIXA
    // O jogador só esbarra na caixa se NÃO estiver segurando ela
    if (!s.holdingBox)
    {
        // Calcula a AABB da caixa naquele exato frame (model matrix vem pronta)
        CollisionAABB boxAABB = ComputeWorldAABB(w.boxLocalMin, w.boxLocalMax, s.boxModel);

        // Testa se o cilindro do jogador bateu nessa AABB gerada
        if (CylinderIntersectsAABB(pos_xz, w.player.raio, y_min, y_max, boxAABB))
            return true;
    }

    // Testa colisão dinâmica com o RÁDIO
    if (!s.holdingRadio)
    {
        // Calcula a AABB usando as bounding boxes locais carregadas pelo tinyobj
        // (usando a malha "Shell" como referência), cacheadas em w.
        CollisionAABB radioAABB = ComputeWorldAABB(w.radioLocalMin, w.radioLocalMax, s.radioModel);

        if (CylinderIntersectsAABB(pos_xz, w.player.raio, y_min, y_max, radioAABB))
            return true;
    }

    return false;
}

// Verifica se a caixa está atravessando o cenário físico
bool BoxCollidesAt(const CollisionWorld& w, const SceneState& s, glm::vec3 pos) {
    // Escala aproximada do cubo no mundo (metade do tamanho total dele)
    float halfBox = 0.3f;
    glm::vec3 min_box = pos + glm::vec3(-halfBox, -halfBox, -halfBox);
    glm::vec3 max_box = pos + glm::vec3(halfBox, halfBox, halfBox);

    // Testa colisões com o cenário retilíneo (chão, teto, paredes AABB)
    for (const CollisionAABB& aabb : w.aabbs) {
        if (max_box.x > aabb.min.x && min_box.x < aabb.max.x &&
            max_box.y > aabb.min.y && min_box.y < aabb.max.y &&
            max_box.z > aabb.min.z && min_box.z < aabb.max.z)
        {
            return true;
        }
    }

    // Testa colisões com paredes diagonais (aproveitando o teste do cilindro)
    glm::vec2 pos_xz(pos.x, pos.z);
    for (const CollisionLine& line : w.lines) {
        // Usamos a meia-largura (halfBox) como raio para o teste contra a linha
        if (CylinderIntersectsLine(pos_xz, halfBox, min_box.y, max_box.y, line)) {
            return true;
        }
    }

    // Testa colisão da caixa contra a PORTA FECHADA
    if (!s.buttonPressed) {
        if (max_box.x > w.closedDoor.min.x && min_box.x < w.closedDoor.max.x &&
            max_box.y > w.closedDoor.min.y && min_box.y < w.closedDoor.max.y &&
            max_box.z > w.closedDoor.min.z && min_box.z < w.closedDoor.max.z)
        {
            return true;
        }
    }

    // Testa colisão dinâmica contra o RÁDIO
    // Usa os mesmos tamanhos configurados em RadioCollidesAt
    float r_xz = 0.15f;
    float r_y  = 0.22f;
    glm::vec3 min_radio = s.radioPos + glm::vec3(-r_xz, -r_y, -r_xz);
    glm::vec3 max_radio = s.radioPos + glm::vec3(r_xz, r_y, r_xz);

    // Se a AABB da caixa sobrepor a AABB do rádio, é colisão!
    if (max_box.x > min_radio.x && min_box.x < max_radio.x &&
        max_box.y > min_radio.y && min_box.y < max_radio.y &&
        max_box.z > min_radio.z && min_box.z < max_radio.z)
    {
        return true;
    }

    return false;
}

bool RadioCollidesAt(const CollisionWorld& w, const SceneState& s, glm::vec3 pos) {
    float s_xz = 0.15f; // Largura do rádio

    // É esta variável 's_y' que você deve diminuir se ele ainda estiver flutuando,
    // ou aumentar se ele estiver afundando no chão!
    float s_y = 0.03f;

    glm::vec3 min_radio = pos + glm::vec3(-s_xz, -s_y, -s_xz);
    glm::vec3 max_radio = pos + glm::vec3(s_xz, s_y, s_xz);

    for (const CollisionAABB& aabb : w.aabbs) {
        if (max_radio.x > aabb.min.x && min_radio.x < aabb.max.x &&
            max_radio.y > aabb.min.y && min_radio.y < aabb.max.y &&
            max_radio.z > aabb.min.z && min_radio.z < aabb.max.z)
        {
            return true;
        }
    }
    glm::vec2 pos_xz(pos.x, pos.z);
    for (const CollisionLine& line : w.lines) {
        if (CylinderIntersectsLine(pos_xz, s_xz, min_radio.y, max_radio.y, line)) {
            return true;
        }
    }
    if (!s.buttonPressed) {
        if (max_radio.x > w.closedDoor.min.x && min_radio.x < w.closedDoor.max.x &&
            max_radio.y > w.closedDoor.min.y && min_radio.y < w.closedDoor.max.y &&
            max_radio.z > w.closedDoor.min.z && min_radio.z < w.closedDoor.max.z)
        {
            return true;
        }
    }

    // Testa colisão dinâmica contra a CAIXA
    // Usa o tamanho simplificado (0.25f) configurado em BoxCollidesAt
    float b_s = 0.25f;
    glm::vec3 min_box = s.boxPos + glm::vec3(-b_s, -b_s, -b_s);
    glm::vec3 max_box = s.boxPos + glm::vec3(b_s, b_s, b_s);

    // Se a AABB do rádio sobrepor a AABB da caixa, é colisão!
    if (max_radio.x > min_box.x && min_radio.x < max_box.x &&
        max_radio.y > min_box.y && min_radio.y < max_box.y &&
        max_radio.z > min_box.z && min_radio.z < max_box.z)
    {
        return true;
    }

    return false;
}

bool IsButtonTriggered(const SceneState& s, const glm::vec3& playerPos) {
    // Definir a área do botão (o gatilho)
    glm::vec3 b_min = glm::vec3(8.5f, -1.0f, -1.5f);
    glm::vec3 b_max = glm::vec3(9.5f, -0.5f, -0.5f);

    // Testa Jogador
    // O cilindro do jogador está sobre o botão?
    if (playerPos.x >= b_min.x && playerPos.x <= b_max.x &&
        playerPos.z >= b_min.z && playerPos.z <= b_max.z &&
        (playerPos.y + PLAYER_FEET_Y_OFFSET) <= b_max.y) {
        return true;
    }

    // Testa Caixa
    // A caixa está sobre o botão?
    if (s.boxPos.x >= b_min.x && s.boxPos.x <= b_max.x &&
        s.boxPos.z >= b_min.z && s.boxPos.z <= b_max.z &&
        s.boxPos.y <= b_max.y) {
        return true;
    }

    return false;
}

// Verifica se há piso imediatamente abaixo dos pés do jogador. Usado para só
// permitir o pulo quando ele está apoiado em uma superfície (sem pulo duplo).
bool IsPlayerOnGround(const CollisionWorld& w, const SceneState& s, const glm::vec3& playerPos)
{
    const float probe_dist = 0.05f;
    return PlayerCollidesAt(w, s, glm::vec3(playerPos.x, playerPos.y - probe_dist, playerPos.z));
}
