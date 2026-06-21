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

// Deslocamento vertical entre Pos_Player (referência) e os pés do cilindro do
// jogador: Pos_Player.y == 0 corresponde aos pés apoiados no chão (y = -1).
constexpr float PLAYER_FEET_Y_OFFSET = -1.0f;

// Margem ("skin") que encolhe o intervalo vertical testado em PlayerCollidesAt,
// evitando que o jogador "grude" ao tocar exatamente o chão/teto.
constexpr float COLLISION_SKIN = 0.02f;

// ---------------------------------------------------------------------------
//  Estado ESTÁTICO do cenário, construído uma única vez (ver M3:
//  BuildCollisionWorld). Agrega tudo contra o que as entidades colidem.
// ---------------------------------------------------------------------------
struct CollisionWorld {
    std::vector<CollisionAABB> aabbs;   // paredes, chão, teto, botão, pilar, bolo...
    std::vector<CollisionLine> lines;   // paredes diagonais
    CollisionAABB closedDoor;           // hitbox da porta 2 (ativa quando fechada)
    PlayerCollider player;              // dimensões do cilindro do jogador
    // AABBs locais (de g_VirtualScene), cacheadas p/ recomputar a AABB de mundo
    // precisa da caixa ("Cube") e do rádio ("Shell") a cada frame.
    glm::vec3 boxLocalMin,   boxLocalMax;
    glm::vec3 radioLocalMin, radioLocalMax;
};

// ---------------------------------------------------------------------------
//  Estado DINÂMICO de um frame: posições/matrizes das entidades móveis e flags
//  do jogo. Montado pelo chamador (ver CurrentSceneState no main) e passado às
//  funções de query. As model matrices vêm prontas para que o módulo não
//  precise de matrices.h (cujas funções não são inline).
// ---------------------------------------------------------------------------
struct SceneState {
    glm::vec3 boxPos;       // posição atual da caixa (centro)
    glm::vec3 radioPos;     // posição atual do rádio (centro)
    glm::mat4 boxModel;     // model matrix da caixa  (Translate*Rotate_Y*Scale 1/8)
    glm::mat4 radioModel;   // model matrix do rádio  (Translate*Rotate_Y*Scale 1/9)
    bool holdingBox;        // jogador está carregando a caixa?
    bool holdingRadio;      // jogador está carregando o rádio?
    bool buttonPressed;     // botão acionado? (libera a porta 2)
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

// ---------------------------------------------------------------------------
//  Queries de colisão por entidade. Recebem o mundo estático (w), o estado
//  dinâmico do frame (s) e a posição candidata a testar.
// ---------------------------------------------------------------------------

// Testa se o cilindro do jogador, na posição candidata 'pos', colide com o
// cenário, a porta fechada, a caixa ou o rádio.
bool PlayerCollidesAt(const CollisionWorld& w, const SceneState& s, const glm::vec3& pos);

// Testa se a caixa, na posição candidata 'pos', atravessa o cenário, a porta
// fechada ou o rádio.
bool BoxCollidesAt(const CollisionWorld& w, const SceneState& s, glm::vec3 pos);

// Testa se o rádio, na posição candidata 'pos', atravessa o cenário, a porta
// fechada ou a caixa.
bool RadioCollidesAt(const CollisionWorld& w, const SceneState& s, glm::vec3 pos);

// Verifica se o jogador (ou a caixa) está sobre o botão que libera a porta 2.
bool IsButtonTriggered(const SceneState& s, const glm::vec3& playerPos);

// Verifica se há piso imediatamente abaixo dos pés do jogador (permite pular).
bool IsPlayerOnGround(const CollisionWorld& w, const SceneState& s, const glm::vec3& playerPos);

#endif // _COLLISIONS_H
