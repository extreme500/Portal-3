// ===========================================================================
//  Portais (Portal-3) — implementação.
//
//  Estado por milestone:
//    M2 (FEITO) -> núcleo matemático (frame/transition/transform*) + fábricas.
//    M4         -> renderViews/renderSurfaces (stencil + câmera virtual + clip).
//    M5         -> teleportIfCrossed/teleportPlayer (travessia SNAP).
//
//  Usamos glm diretamente (cross/normalize/inverse/rotate) em vez de matrices.h,
//  pois aquele header define funções não-inline e incluí-lo aqui (2º translation
//  unit) causaria erro de "multiple definition" no link.
// ===========================================================================

#include "portal.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>

#include <cmath>

static const float PORTAL_PI = 3.14159265358979323846f;

// Contexto de GL (configurado por Portal_SetGLContext no início do programa).
static PortalGLContext g_PortalGL;

void Portal_SetGLContext(const PortalGLContext& ctx)
{
    g_PortalGL = ctx;
}

// Projeta 'p' no plano do portal e testa se cai dentro do retângulo do portal.
static bool withinRect(const Portal& portal, const glm::vec3& p)
{
    glm::vec3 d = p - portal.center;
    float x = glm::dot(d, portal.right());
    float y = glm::dot(d, portal.up);
    return std::fabs(x) <= portal.width * 0.5f
        && std::fabs(y) <= portal.height * 0.5f;
}

// ---------------------------------------------------------------------------
//  Portal — núcleo geométrico
// ---------------------------------------------------------------------------
glm::vec3 Portal::right() const
{
    return glm::normalize(glm::cross(up, normal));
}

glm::mat4 Portal::frame() const
{
    // Base ortonormal destra: r x u = n. Reortonormalizamos por segurança.
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 r = glm::normalize(glm::cross(up, n));
    glm::vec3 u = glm::cross(n, r);

    // glm::mat4 recebe COLUNAS: [ right | up | normal | center ].
    return glm::mat4(
        glm::vec4(r, 0.0f),
        glm::vec4(u, 0.0f),
        glm::vec4(n, 0.0f),
        glm::vec4(center, 1.0f)
    );
}

glm::mat4 Portal::frameInverse() const
{
    // frame() é uma transformação rígida (rotação ortonormal + translação);
    // glm::inverse é exato e barato para 4x4.
    return glm::inverse(frame());
}

float Portal::signedDistance(const glm::vec3& p) const
{
    return glm::dot(p - center, normal);
}

Portal Portal::onWall(glm::vec3 center, glm::vec3 normal, glm::vec3 upHint)
{
    Portal p;
    p.center = center;

    glm::vec3 n = glm::normalize(normal);

    // Se a dica de "up" for ~paralela à normal (parede ~ chão/teto), trocamos de
    // eixo para evitar base degenerada.
    glm::vec3 h = glm::normalize(upHint);
    if (std::fabs(glm::dot(h, n)) > 0.99f)
        h = (std::fabs(n.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f)
                                     : glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 r = glm::normalize(glm::cross(h, n)); // right
    glm::vec3 u = glm::normalize(glm::cross(n, r)); // up reortonormalizado

    p.normal = n;
    p.up     = u;
    return p;
}

// ---------------------------------------------------------------------------
//  PortalPair — fábrica e núcleo de transformação
// ---------------------------------------------------------------------------
PortalPair PortalPair::create(Portal blue, Portal orange, float width, float height)
{
    blue.colorId   = PORTAL_BLUE;
    orange.colorId = PORTAL_ORANGE;
    blue.width   = orange.width  = width;
    blue.height  = orange.height = height;

    PortalPair pair;
    pair.blue   = blue;
    pair.orange = orange;
    return pair;
}

const Portal& PortalPair::other(const Portal& p) const
{
    return (&p == &blue) ? orange : blue;
}

glm::mat4 PortalPair::transitionMatrix(const Portal& from, const Portal& to) const
{
    // Leva o espaço local de 'from' ao de 'to', com giro de 180° em torno do
    // up local (eixo Y do frame): assim sai-se de FRENTE para fora do destino.
    // Usado para VELOCIDADE/ORIENTAÇÃO (w=0): a rotação inverte a direção de
    // entrada (apontando para o portal) para saída (apontando para fora dele).
    glm::mat4 flip = glm::rotate(glm::mat4(1.0f), PORTAL_PI, glm::vec3(0.0f, 1.0f, 0.0f));
    return to.frame() * flip * from.frameInverse();
}

glm::mat4 PortalPair::positionMatrix(const Portal& from, const Portal& to) const
{
    // Leva o espaço local de 'from' ao de 'to' SEM o giro: mantém o ponto no
    // mesmo lado do plano (frente → frente). Usado para POSIÇÃO (w=1):
    // atravessar um portal coloca a entidade na mesma posição relativa do
    // portal de destino, dentro da sala — e não atrás da parede.
    return to.frame() * from.frameInverse();
}

glm::vec3 PortalPair::transformPoint(const Portal& from, const glm::vec3& p) const
{
    glm::vec4 r = positionMatrix(from, other(from)) * glm::vec4(p, 1.0f);
    return glm::vec3(r);
}

glm::vec3 PortalPair::transformVector(const Portal& from, const glm::vec3& v) const
{
    // w = 0 -> aplica somente a parte linear (rotação), ignorando a translação.
    glm::vec4 r = transitionMatrix(from, other(from)) * glm::vec4(v, 0.0f);
    return glm::vec3(r);
}

// ---------------------------------------------------------------------------
//  Travessia (SNAP) — detecção compartilhada
// ---------------------------------------------------------------------------
// Retorna a posição da entidade deslocada para a sua "frente" (superfície frontal
// na direção da normal do portal), usada na detecção de travessia. Isso permite
// que o centro geométrico nunca cruze o plano (bloqueado pela parede), mas a
// superfície do corpo (pele/colisor) sim.
static glm::vec3 frontPosition(const Portal& p, const glm::vec3& position,
                                float bodyRadius)
{
    return position - glm::normalize(p.normal) * bodyRadius;
}

void PortalPair::resetSides(PortalCrossingState& state, const glm::vec3& position) const
{
    state.sideBlue    = blue.signedDistance(frontPosition(blue, position, state.bodyRadius));
    state.sideOrange  = orange.signedDistance(frontPosition(orange, position, state.bodyRadius));
    state.initialized = true;
}

const Portal* PortalPair::detectCrossing(PortalCrossingState& state,
                                         const glm::vec3& position) const
{
    float dB = blue.signedDistance(frontPosition(blue, position, state.bodyRadius));
    float dO = orange.signedDistance(frontPosition(orange, position, state.bodyRadius));

    const Portal* from = nullptr;
    if (state.initialized)
    {
        if (state.sideBlue > 0.0f && dB <= 0.0f && withinRect(blue, position))
            from = &blue;
        else if (state.sideOrange > 0.0f && dO <= 0.0f && withinRect(orange, position))
            from = &orange;
    }

    state.sideBlue    = dB;
    state.sideOrange  = dO;
    state.initialized = true;
    return from;
}

bool PortalPair::teleportIfCrossed(PortalCrossingState& state,
                                   glm::vec3& position,
                                   glm::vec3* velocity,
                                   float* yaw) const
{
    const Portal* from = detectCrossing(state, position);
    if (!from) return false;

    const Portal& to = other(*from);
    glm::mat4 Mp = positionMatrix(*from, to);   // posição: sem flip
    glm::mat4 Md = transitionMatrix(*from, to); // direção: com flip
    position = glm::vec3(Mp * glm::vec4(position, 1.0f));
    if (velocity)
        *velocity = glm::vec3(Md * glm::vec4(*velocity, 0.0f));
    if (yaw)
    {
        // Direção horizontal na convenção do main: (-sin(yaw), 0, -cos(yaw)).
        glm::vec3 dir(-std::sin(*yaw), 0.0f, -std::cos(*yaw));
        glm::vec3 d = glm::vec3(Md * glm::vec4(dir, 0.0f));
        *yaw = std::atan2(-d.x, -d.z);
    }
    resetSides(state, position); // evita re-trigger imediato no portal de destino
    return true;
}

bool PortalPair::teleportPlayer(PortalCrossingState& state,
                                glm::vec3& position,
                                glm::vec3& velocity,
                                float& theta, float& phi) const
{
    const Portal* from = detectCrossing(state, position);
    if (!from) return false;

    const Portal& to = other(*from);
    glm::mat4 Mp = positionMatrix(*from, to);   // posição: sem flip
    glm::mat4 Md = transitionMatrix(*from, to); // direção: com flip
    position = glm::vec3(Mp * glm::vec4(position, 1.0f));
    velocity = glm::vec3(Md * glm::vec4(velocity, 0.0f));

    // Direção de olhar (convenção esférica do main.cpp):
    //   view = (-cos(phi)sin(theta), sin(phi), -cos(phi)cos(theta))
    glm::vec3 viewDir(-std::cos(phi) * std::sin(theta),
                       std::sin(phi),
                      -std::cos(phi) * std::cos(theta));
    glm::vec3 d = glm::normalize(glm::vec3(Md * glm::vec4(viewDir, 0.0f)));

    phi = std::asin(glm::clamp(d.y, -1.0f, 1.0f));
    float cphi = std::cos(phi);
    if (std::fabs(cphi) > 1e-4f)
        theta = std::atan2(-d.x / cphi, -d.z / cphi);

    resetSides(state, position);
    return true;
}

// ---------------------------------------------------------------------------
//  Renderização see-through (stencil)
// ---------------------------------------------------------------------------

// Model matrix que posiciona o quad "the_plane" (plano XZ unitário, normal +Y)
// sobre o portal: mapeia o eixo local X->right, Y->normal, Z->up; escala para
// (width x height) e desloca 'pushOut' ao longo da normal (evita z-fighting com
// a parede atrás).
static glm::mat4 portalQuadModel(const Portal& p, float pushOut)
{
    glm::vec3 r = p.right();
    glm::vec3 n = glm::normalize(p.normal);
    glm::vec3 u = glm::normalize(p.up);
    glm::vec3 c = p.center + n * pushOut;

    glm::mat4 basis(
        glm::vec4(r, 0.0f),   // local X -> right
        glm::vec4(n, 0.0f),   // local Y -> normal (the_plane aponta em +Y)
        glm::vec4(u, 0.0f),   // local Z -> up
        glm::vec4(c, 1.0f)
    );
    return basis * glm::scale(glm::mat4(1.0f),
                              glm::vec3(p.width * 0.5f, 1.0f, p.height * 0.5f));
}

void PortalPair::drawPortalQuad(const Portal& portal) const
{
    if (!g_PortalGL.drawPlane) return;
    glm::mat4 model = portalQuadModel(portal, 0.01f);
    glUniformMatrix4fv(g_PortalGL.modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(g_PortalGL.objectIdUniform, portal.colorId);

    // A base de portalQuadModel tem determinante negativo (inverte a orientação
    // dos triângulos), fazendo o quad ser back-facing com glFrontFace(GL_CCW).
    // Desabilitamos culling para garantir que o quad seja renderizado de ambos
    // os lados — necessário no stencil, no depth-clear e na superfície.
    GLboolean cullWasOn = glIsEnabled(GL_CULL_FACE);
    if (cullWasOn) glDisable(GL_CULL_FACE);

    g_PortalGL.drawPlane();

    if (cullWasOn) glEnable(GL_CULL_FACE);
}

void PortalPair::renderOneView(const Portal& portal, const glm::mat4& view,
                               const glm::mat4& projection, int depth, int level) const
{
    // Renderização recursiva (hall of mirrors DENTRO do par): olhando por
    // 'portal' vê-se a sala do par dele e, nela, o próprio 'portal' de novo —
    // repetindo a vista 'depth' níveis. A recursão é uma CADEIA (segue sempre o
    // mesmo portal), então cada nível aplica de novo a MESMA transitionMatrix:
    // a câmera do nível k é M^k * câmera_real.
    //
    // Aninhamento via stencil auto-limpante (algoritmo canônico):
    //   • descendo, INCR marca a janela aninhada (stencil: level-1 -> level);
    //   • subindo, DECR a desfaz (level -> level-1).
    // Ao retornar do topo o stencil volta a 0 — sem interferência entre portais
    // nem entre os pares, dispensando deslocamentos de ref por par.
    (void)projection; // a projeção (FOV) não muda entre níveis; só a 'view'.
    if (depth <= 0) return;
    const Portal& dest = other(portal);

    // ---- 1) Marca a janela aninhada no stencil ----
    // INCR só onde o pai já está marcado (stencil == level-1): o nível 1 parte
    // do fundo (0); os mais profundos partem da janela do nível anterior. O quad
    // é posicionado pela 'view' DESTE nível (onde o portal reaparece na cena já
    // desenhada). LEQUAL: a parede do portal (desenhada por DrawScene) está na
    // mesma profundidade; LESS falharia. A oclusão por geometria mais próxima é
    // preservada.
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_EQUAL, level - 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glUniform1i(g_PortalGL.portalPassUniform, 0); // stencil: mantém dentro da elipse
    glDepthFunc(GL_LEQUAL);
    glUniformMatrix4fv(g_PortalGL.viewUniform, 1, GL_FALSE, glm::value_ptr(view));
    drawPortalQuad(portal);

    // ---- 2) Limpa a profundidade dentro da janela (stencil == level) ----
    // Empurra o depth para o "far" (1.0) na elipse, para a vista virtual poder
    // desenhar em qualquer profundidade.
    glStencilFunc(GL_EQUAL, level, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glDepthRange(1.0f, 1.0f);
    drawPortalQuad(portal);
    glDepthRange(0.0f, 1.0f);
    glDepthFunc(GL_LESS);

    // ---- 3) Câmera virtual deste nível: M aplicada à câmera de 'view' ----
    // O giro de 180° (eixo Y local) embutido em M coloca a câmera no lado oposto
    // do portal de destino, olhando através dele, e espelha exatamente o
    // movimento da câmera real (a parte linear de dest_R * flipY * source_R⁻¹ é
    // a identidade).
    glm::vec3 realEye = glm::vec3(glm::inverse(view) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 realFwd = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    glm::vec3 realUp  =  glm::vec3(view[0][1], view[1][1], view[2][1]);
    glm::mat4 M = transitionMatrix(portal, dest);
    glm::vec3 virtEye = glm::vec3(M * glm::vec4(realEye, 1.0f));
    glm::vec3 virtFwd = glm::vec3(M * glm::vec4(realFwd, 0.0f));
    glm::vec3 virtUp  = glm::vec3(M * glm::vec4(realUp,  0.0f));
    glm::mat4 virtualView = glm::lookAt(virtEye, virtEye + virtFwd, virtUp);

    // ---- 4) Renderiza a CENA deste nível, recortada na janela ----
    // Plano de recorte (mundo): mantém só o lado da sala do portal de destino,
    // descartando o que está atrás dele. É o MESMO plano em todos os níveis,
    // pois sempre renderizamos o mundo real a partir de uma câmera deslocada.
    glm::vec3 n = glm::normalize(dest.normal);
    glm::vec4 clipPlane(n, -glm::dot(dest.center, n) - 0.001f);

    glUniformMatrix4fv(g_PortalGL.viewUniform, 1, GL_FALSE, glm::value_ptr(virtualView));
    glUniform4fv(g_PortalGL.clipPlaneUniform, 1, glm::value_ptr(clipPlane));
    glEnable(GL_CLIP_DISTANCE0);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glStencilFunc(GL_EQUAL, level, 0xFF);   // só dentro da janela deste nível
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    if (g_PortalGL.drawScene) g_PortalGL.drawScene();

    glDisable(GL_CLIP_DISTANCE0);
    glm::vec4 noClip(0.0f);
    glUniform4fv(g_PortalGL.clipPlaneUniform, 1, glm::value_ptr(noClip));

    // ---- 4b) Molduras (bordas azul/laranja) DESTE nível ----
    // O DrawScene não desenha as superfícies dos portais (só a parede atrás);
    // sem isto os níveis aninhados mostrariam o see-through sem borda. Desenhamos
    // ambas as molduras como em renderSurfaces, mas pela câmera virtual deste
    // nível e confinadas à sua janela (stencil == level já está ativo). A borda é
    // o anel FORA da elipse (portalPass=1 descarta dentro), então sobrevive ao
    // recorte que o passo 5 fará na elipse. LEQUAL respeita a oclusão pela cena
    // recém-desenhada; viewUniform ainda é a virtualView do passo 4.
    glUniform1i(g_PortalGL.portalPassUniform, 1); // moldura: descarta dentro da elipse
    glDepthFunc(GL_LEQUAL);
    drawPortalQuad(blue);
    drawPortalQuad(orange);
    glUniform1i(g_PortalGL.portalPassUniform, 0);
    glDepthFunc(GL_LESS);

    // ---- 5) Recursão: desce um nível pela câmera virtual ----
    // O portal reaparece DENTRO da cena recém-desenhada (passo 4); a chamada
    // filha esculpe a próxima janela aninhada e sobrescreve só essa região com a
    // vista mais profunda. O nível mais raso continua íntegro fora dela.
    if (depth > 1)
        renderOneView(portal, virtualView, projection, depth - 1, level + 1);

    // ---- 6) Desfaz a marca deste nível (DECR: level -> level-1) ----
    // Posiciona o quad pela 'view' deste nível (mesma região do passo 1).
    // GL_ALWAYS + colorMask/depthMask desligados: mexe SÓ no stencil, sem tocar
    // a cor (a vista já desenhada) nem a profundidade.
    glUniformMatrix4fv(g_PortalGL.viewUniform, 1, GL_FALSE, glm::value_ptr(view));
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_EQUAL, level, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    drawPortalQuad(portal);

    // Restaura o estado padrão de cor/profundidade para o continuação do laço.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void PortalPair::renderViews(const glm::mat4& view, const glm::mat4& projection,
                             int depth, int stencilOffset) const
{
    if (!g_PortalGL.drawScene || !g_PortalGL.drawPlane) return;

    // A recursão é auto-limpante (INCR ao descer, DECR ao subir): cada portal
    // começa do fundo (stencil 0) e o devolve a 0 ao terminar. Por isso não há
    // mais interferência entre portais nem entre pares, e o antigo stencilOffset
    // tornou-se desnecessário (mantido na assinatura por compatibilidade).
    (void)stencilOffset;
    renderOneView(blue,   view, projection, depth, 1);
    renderOneView(orange, view, projection, depth, 1);

    // Estado limpo para a cena/HUD que vem depois.
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDepthRange(0.0f, 1.0f);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void PortalPair::renderSurfaces() const
{
    // Desenha os quads dos portais com cor emissiva azul/laranja. O fragment
    // shader aplica a máscara elíptica (discard) + emissive_color.
    //
    // Usamos GL_LEQUAL: o depth buffer tem as profundidades da cena real
    // (DrawScene). Assim, objetos reais em frente ao portal ocultam a
    // moldura — o portal só fica visível onde não há obstrução.
    glDepthFunc(GL_LEQUAL);
    glUniform1i(g_PortalGL.portalPassUniform, 1); // moldura: descarta dentro da elipse
    drawPortalQuad(blue);
    drawPortalQuad(orange);
    glUniform1i(g_PortalGL.portalPassUniform, 0); // restaura

    glDepthFunc(GL_LESS);
}
