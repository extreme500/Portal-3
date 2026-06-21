#ifndef _PORTAL_H
#define _PORTAL_H

// ===========================================================================
//  Portais (Portal-3) — módulo dedicado.
//
//  Um portal é preso a uma superfície (parede/chão/teto) e orientado por uma
//  base ortonormal derivada da NORMAL da superfície. Como a base vem apenas da
//  normal informada, paredes diagonais (não paralelas aos eixos) funcionam sem
//  nenhum caso especial.
//
//  Portais andam SEMPRE em pares (azul + laranja). Atravessar um teleporta a
//  entidade para o par, transformando linearmente seus vetores físicos.
//
//  Este header não inclui <glad/glad.h> de propósito (ordem de include): os IDs
//  de GL são guardados como int/unsigned int, equivalentes a GLint/GLuint.
// ===========================================================================

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

// object_id usados no fragment shader para a superfície/moldura dos portais.
// Valores altos para não colidirem com os object_id já existentes na cena.
#define PORTAL_BLUE    90
#define PORTAL_ORANGE  91

// ---------------------------------------------------------------------------
//  Portal: um portal individual.
// ---------------------------------------------------------------------------
struct Portal
{
    glm::vec3 center  = glm::vec3(0.0f);                 // centro (coords de mundo)
    glm::vec3 normal  = glm::vec3(0.0f, 0.0f, 1.0f);     // unitária, aponta p/ fora da parede
    glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);     // unitária, tangente (define o "roll")
    float     width   = 1.2f;
    float     height  = 2.0f;
    int       colorId = PORTAL_BLUE;

    // Vetor "right" da base local (= up x normal).
    glm::vec3 right() const;

    // Base ortonormal rígida [ right | up | normal | center ]. É o sistema de
    // coordenadas local do portal E a model matrix usada para desenhar/estampar
    // o quad (the_plane, cuja face tem normal +z, alinha com 'normal').
    glm::mat4 frame() const;
    glm::mat4 frameInverse() const;

    // Distância com sinal de um ponto ao plano do portal (>0 = lado da normal).
    float signedDistance(const glm::vec3& p) const;

    // Fábrica: cria UM portal sobre uma parede a partir de um ponto e da normal
    // da superfície. 'upHint' é só uma dica e é reprojetada para ficar
    // perpendicular à normal; se a normal for ~vertical (chão/teto), há fallback
    // automático de eixo.
    static Portal onWall(glm::vec3 center, glm::vec3 normal,
                         glm::vec3 upHint = glm::vec3(0.0f, 1.0f, 0.0f));
};

// ---------------------------------------------------------------------------
//  Estado de travessia por entidade (de que lado de cada portal ela estava no
//  frame anterior). Mantenha UMA instância por entidade que pode atravessar.
// ---------------------------------------------------------------------------
struct PortalCrossingState
{
    float sideBlue    = 0.0f;
    float sideOrange  = 0.0f;
    bool  initialized = false;
    float bodyRadius  = 0.0f; // raio de colisão da entidade (offset frontal p/ detecção)
};

// ---------------------------------------------------------------------------
//  Contexto de GL de que o módulo precisa para renderizar. Configure UMA vez no
//  início (depois de carregar shaders e a geometria do quad).
// ---------------------------------------------------------------------------
struct PortalGLContext
{
    int    modelUniform      = -1;
    int    viewUniform       = -1;
    int    projectionUniform = -1;
    int    clipPlaneUniform  = -1;   // uniform vec4 clipPlane (vertex shader)
    int    objectIdUniform   = -1;
    int    portalPassUniform = -1;   // uniform int portalPass (fragment shader)
    void (*drawScene)()      = nullptr; // desenha toda a geometria do mundo
    void (*drawPlane)()      = nullptr; // desenha o quad "the_plane" (DrawVirtualObject)
};

void Portal_SetGLContext(const PortalGLContext& ctx);

// ---------------------------------------------------------------------------
//  PortalPair: gestor do par (azul + laranja). Núcleo matemático + travessia +
//  renderização see-through.
// ---------------------------------------------------------------------------
class PortalPair
{
public:
    Portal blue;
    Portal orange;

    PortalPair() {}

    // Cria o par já VINCULADO (link cruzado), com cores e tamanho atribuídos.
    // É a API de instanciação para o desenvolvedor — uma linha.
    static PortalPair create(Portal blue, Portal orange,
                             float width = 1.2f, float height = 2.0f);

    // --- Núcleo matemático (também usado pela física e, no futuro, pelo clone) ---
    // Matriz para TRANSFORMAÇÃO DE DIREÇÃO (velocidade, orientação): inclui o
    // giro de 180° em torno do up local — sai-se de frente para fora do destino.
    glm::mat4 transitionMatrix(const Portal& from, const Portal& to) const;
    // Matriz para TRANSFORMAÇÃO DE POSIÇÃO: sem o giro, mantém o ponto no mesmo
    // lado do plano (frente → frente) ao invés de virá-lo para trás.
    glm::mat4 positionMatrix(const Portal& from, const Portal& to) const;
    glm::vec3 transformPoint (const Portal& from, const glm::vec3& p) const; // ponto
    glm::vec3 transformVector(const Portal& from, const glm::vec3& v) const; // vetor/direção

    // --- Travessia (SNAP) ---
    // Se o centro da entidade cruzou um dos portais neste frame (dentro do
    // retângulo do portal), teleporta-a para o par, transformando posição e,
    // se fornecidos, velocity (vetor) e yaw (rotação Y / direção, em radianos).
    // Ideal para objetos (caixa, rádio). Devolve true se teleportou.
    bool teleportIfCrossed(PortalCrossingState& state,
                           glm::vec3& position,
                           glm::vec3* velocity = nullptr,
                           float* yaw = nullptr) const;

    // Variante para o JOGADOR: além de posição/velocidade, transforma a direção
    // de olhar (theta/phi, na convenção esférica do main.cpp) atravessando o
    // portal, para continuidade visual. Devolve true se teleportou.
    bool teleportPlayer(PortalCrossingState& state,
                        glm::vec3& position,
                        glm::vec3& velocity,
                        float& theta, float& phi) const;

    // --- Renderização see-through (stencil, 'depth' níveis de recursão) ---
    // stencilOffset: deslocamento nos stencil refs (pares consecutivos usam
    // 1/2, 3/4, 5/6, …).
    void renderViews(const glm::mat4& view, const glm::mat4& projection,
                     int depth = 1, int stencilOffset = 0) const;
    // Desenha as superfícies/molduras dos portais (após a cena principal).
    void renderSurfaces() const;

private:
    // O portal-par de 'p' dentro deste par.
    const Portal& other(const Portal& p) const;
    // Detecta se 'position' cruzou (frente->trás, dentro do retângulo) um dos
    // portais desde o frame anterior; atualiza o estado. Devolve o portal de
    // ENTRADA, ou nullptr se não cruzou.
    const Portal* detectCrossing(PortalCrossingState& state,
                                 const glm::vec3& position) const;
    // Reinicializa os lados guardados no estado a partir da posição atual.
    void resetSides(PortalCrossingState& state, const glm::vec3& position) const;

    // Renderiza a vista vista por UM portal, recursivamente (hall of mirrors).
    // 'depth' é o número de níveis restantes; 'level' é o valor de stencil deste
    // nível (1 no topo, crescendo a cada recursão). Chame com level=1.
    void renderOneView(const Portal& portal, const glm::mat4& view,
                       const glm::mat4& projection, int depth, int level) const;
    // Desenha o quad de um portal (para stencil ou superfície) com sua model matrix.
    void drawPortalQuad(const Portal& portal) const;
};

#endif // _PORTAL_H
