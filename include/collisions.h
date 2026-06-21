#ifndef _COLLISIONS_H
#define _COLLISIONS_H

// ===========================================================================
//  Colisões (Portal-3) — módulo dedicado.
//
//  Detecção e resposta a colisões do jogador, da caixa e do rádio contra o
//  cenário (AABBs alinhadas aos eixos e paredes diagonais). O módulo é
//  DESACOPLADO do restante do jogo, à semelhança do módulo de portais: todo o
//  estado é passado explicitamente por parâmetro/struct. Em particular, este
//  módulo NÃO conhece SceneObject nem g_VirtualScene — as bounding boxes locais
//  dos objetos são injetadas pelo chamador.
// ===========================================================================

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// Caixa delimitadora axis-aligned em coordenadas de mundo, usada nos testes de
// colisão do cenário (paredes, chão, teto, cubo, botão, porta).
struct CollisionAABB {
    glm::vec3 min;
    glm::vec3 max;
};

// Representa uma parede diagonal no plano XZ (segmento com faixa de altura).
struct CollisionLine {
    glm::vec2 p1;    // Ponto inicial da parede (X, Z)
    glm::vec2 p2;    // Ponto final da parede (X, Z)
    float y_min;     // Base da parede (chão)
    float y_max;     // Topo da parede (teto)
};

// Representação física do jogador para fins de colisão: um cilindro vertical.
struct PlayerCollider {
    float raio   = 0.25f;
    float altura = 1.4f;
};

// ---------------------------------------------------------------------------
//  Funções geométricas puras (sem dependência de estado global).
// ---------------------------------------------------------------------------

// Testa se um cilindro vertical (centrado em pos_xz, com raio fixo e faixa de
// altura [y_min, y_max]) sobrepõe a CollisionAABB 'box'. O teste é decomposto
// em (1) overlap do intervalo vertical e (2) distância do centro do círculo
// (projeção do cilindro no plano XZ) ao retângulo formado pela AABB em XZ.
bool CylinderIntersectsAABB(glm::vec2 pos_xz, float raio, float y_min, float y_max, const CollisionAABB& box);

// Testa o cilindro vertical contra uma parede diagonal (CollisionLine).
bool CylinderIntersectsLine(glm::vec2 pos_xz, float raio, float y_min_jogador, float y_max_jogador, const CollisionLine& line);

// Calcula a AABB de mundo de um objeto a partir da sua AABB local e da matriz
// "model" usada para desenhá-lo: transforma os 8 cantos da AABB local pela
// matriz e recomputa o min/max resultante. Válido mesmo quando a matriz inclui
// rotação e/ou escala.
CollisionAABB ComputeWorldAABB(glm::vec3 local_min, glm::vec3 local_max, const glm::mat4& model);

#endif // _COLLISIONS_H
