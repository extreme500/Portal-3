#include "portal.h"
#include "matrices.h"

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
    float x = dotproduct(d, portal.right());
    float y = dotproduct(d, portal.up);
    return std::fabs(x) <= portal.width * 0.5f
        && std::fabs(y) <= portal.height * 0.5f;
}

// ---------------------------------------------------------------------------
//  Portal — núcleo geométrico
// ---------------------------------------------------------------------------
glm::vec3 Portal::right() const
{
    return normalized(crossproduct(up, normal));
}

glm::mat4 Portal::frame() const
{
    // Base ortonormal destra: r x u = n. Reortonormalizamos por segurança.
    glm::vec3 n = normalized(normal);
    glm::vec3 r = normalized(crossproduct(up, n));
    glm::vec3 u = crossproduct(n, r);

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
    // Matrix_Inverse é exato e barato para 4x4.
    return Matrix_Inverse(frame());
}

float Portal::signedDistance(const glm::vec3& p) const
{
    return dotproduct(p - center, normal);
}

Portal Portal::onWall(glm::vec3 center, glm::vec3 normal, glm::vec3 upHint)
{
    Portal p;
    p.center = center;

    glm::vec3 n = normalized(normal);

    // Se a dica de "up" for ~paralela à normal (parede ~ chão/teto), trocamos de
    // eixo para evitar base degenerada.
    glm::vec3 h = normalized(upHint);
    if (std::fabs(dotproduct(h, n)) > 0.99f)
        h = (std::fabs(n.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f)
                                     : glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 r = normalized(crossproduct(h, n)); // right
    glm::vec3 u = normalized(crossproduct(n, r)); // up reortonormalizado

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
    glm::mat4 flip = Matrix_Rotate_Y(PORTAL_PI);
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
    return position - normalized(p.normal) * bodyRadius;
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

bool PortalPair::teleportCore(PortalCrossingState& state, glm::vec3& position,
                              glm::vec3* velocity, glm::mat4& dirMatrix) const
{
    const Portal* from = detectCrossing(state, position);
    if (!from) return false;

    const Portal& to = other(*from);
    position  = glm::vec3(positionMatrix(*from, to) * glm::vec4(position, 1.0f)); // sem flip
    dirMatrix = transitionMatrix(*from, to);                                      // com flip
    if (velocity)
        *velocity = glm::vec3(dirMatrix * glm::vec4(*velocity, 0.0f));
    resetSides(state, position); // evita re-trigger imediato no portal de destino
    return true;
}

bool PortalPair::teleportIfCrossed(PortalCrossingState& state,
                                   glm::vec3& position,
                                   glm::vec3* velocity,
                                   float* yaw) const
{
    glm::mat4 dirMatrix;
    if (!teleportCore(state, position, velocity, dirMatrix)) return false;

    if (yaw)
    {
        // Direção horizontal na convenção do main: (-sin(yaw), 0, -cos(yaw)).
        glm::vec3 dir(-std::sin(*yaw), 0.0f, -std::cos(*yaw));
        glm::vec3 d = glm::vec3(dirMatrix * glm::vec4(dir, 0.0f));
        *yaw = std::atan2(-d.x, -d.z);
    }
    return true;
}

bool PortalPair::teleportPlayer(PortalCrossingState& state,
                                glm::vec3& position,
                                glm::vec3& velocity,
                                float& theta, float& phi) const
{
    glm::mat4 dirMatrix;
    if (!teleportCore(state, position, &velocity, dirMatrix)) return false;

    // Direção de olhar (convenção esférica do main.cpp):
    //   view = (-cos(phi)sin(theta), sin(phi), -cos(phi)cos(theta))
    glm::vec3 viewDir(-std::cos(phi) * std::sin(theta),
                       std::sin(phi),
                      -std::cos(phi) * std::cos(theta));
    glm::vec3 d = normalized(glm::vec3(dirMatrix * glm::vec4(viewDir, 0.0f)));

    phi = std::asin(clampf(d.y, -1.0f, 1.0f));
    float cphi = std::cos(phi);
    if (std::fabs(cphi) > 1e-4f)
        theta = std::atan2(-d.x / cphi, -d.z / cphi);
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
    glm::vec3 n = normalized(p.normal);
    glm::vec3 u = normalized(p.up);
    glm::vec3 c = p.center + n * pushOut;

    glm::mat4 basis(
        glm::vec4(r, 0.0f),   // local X -> right
        glm::vec4(n, 0.0f),   // local Y -> normal (the_plane aponta em +Y)
        glm::vec4(u, 0.0f),   // local Z -> up
        glm::vec4(c, 1.0f)
    );
    return basis * Matrix_Scale(p.width * 0.5f, 1.0f, p.height * 0.5f);
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

// Câmera virtual: aplica a transição 'M' (origem->destino, já com o giro de 180°
// no eixo Y) à câmera de 'view', produzindo a vista "do outro lado" do destino.
// Repetir M a cada nível dá a cadeia M^k do hall of mirrors.
static glm::mat4 virtualViewThrough(const glm::mat4& M, const glm::mat4& view)
{
    glm::vec3 eye = glm::vec3(Matrix_Inverse(view) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 fwd = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    glm::vec3 up  =  glm::vec3(view[0][1], view[1][1], view[2][1]);
    glm::vec3 vEye = glm::vec3(M * glm::vec4(eye, 1.0f));
    glm::vec3 vFwd = glm::vec3(M * glm::vec4(fwd, 0.0f));
    glm::vec3 vUp  = glm::vec3(M * glm::vec4(up,  0.0f));
    return Matrix_Camera_View(glm::vec4(vEye, 1.0f), glm::vec4(vFwd, 0.0f), glm::vec4(vUp, 0.0f));
}

void PortalPair::stampStencil(const Portal& portal, const glm::mat4& view,
                              int compareRef, unsigned int stencilOp,
                              unsigned int depthFunc) const
{
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // só o stencil; sem cor
    glDepthMask(GL_FALSE);                               // nem profundidade
    glDepthFunc(depthFunc);
    glStencilFunc(GL_EQUAL, compareRef, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, stencilOp);
    glUniform1i(g_PortalGL.portalPassUniform, 0); // stencil: mantém dentro da elipse
    glUniformMatrix4fv(g_PortalGL.viewUniform, 1, GL_FALSE, glm::value_ptr(view));
    drawPortalQuad(portal);
}

void PortalPair::drawFramePair() const
{
    // portalPass=1 descarta o interior da elipse, deixando só o anel (a borda).
    // GL_LEQUAL respeita a oclusão pela cena já desenhada no depth buffer.
    glUniform1i(g_PortalGL.portalPassUniform, 1);
    glDepthFunc(GL_LEQUAL);
    drawPortalQuad(blue);
    drawPortalQuad(orange);
    glUniform1i(g_PortalGL.portalPassUniform, 0);
    glDepthFunc(GL_LESS);
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

    // ---- 1) Marca a janela aninhada (INCR onde o pai já está marcado) ----
    // O nível 1 parte do fundo (stencil 0); os profundos, da janela do nível
    // anterior. LEQUAL: a parede do portal (DrawScene) está à mesma profundidade
    // do quad — LESS falharia — e a oclusão por geometria mais próxima é
    // preservada. O quad usa a 'view' DESTE nível (onde o portal reaparece).
    stampStencil(portal, view, level - 1, GL_INCR, GL_LEQUAL);

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

    // ---- 3) Câmera virtual deste nível ----
    // M (com giro de 180° no eixo Y local) coloca a câmera no lado oposto do
    // destino, olhando através dele, e espelha o movimento da câmera real (a
    // parte linear de dest_R * flipY * source_R⁻¹ é a identidade).
    glm::mat4 M = transitionMatrix(portal, dest);
    glm::mat4 virtualView = virtualViewThrough(M, view);

    // ---- 4) Renderiza a CENA deste nível, recortada na janela ----
    // Plano de recorte (mundo): mantém só o lado da sala do portal de destino,
    // descartando o que está atrás dele. É o MESMO plano em todos os níveis,
    // pois sempre renderizamos o mundo real a partir de uma câmera deslocada.
    glm::vec3 n = normalized(dest.normal);
    glm::vec4 clipPlane(n, -dotproduct(dest.center, n) - 0.001f);

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
    // O DrawScene não desenha as superfícies dos portais, então sem isto os
    // níveis aninhados ficariam sem borda. Ficam confinadas à janela
    // (stencil == level, ainda ativo) e usam a virtualView do passo 4; o anel
    // fica FORA da elipse, sobrevivendo ao recorte que o passo 5 fará nela.
    drawFramePair();

    // ---- 5) Recursão: desce um nível pela câmera virtual ----
    // O portal reaparece DENTRO da cena recém-desenhada; a chamada filha esculpe
    // a próxima janela aninhada e sobrescreve só essa região. O nível mais raso
    // continua íntegro fora dela.
    if (depth > 1)
        renderOneView(portal, virtualView, projection, depth - 1, level + 1);

    // ---- 5b) Objetos transparentes (vidro) DESTE nível, por ÚLTIMO ----
    // Depois do opaco, das molduras E da recursão, para que o "glass effect"
    // tinja tudo que estiver atrás do vidro nesta vista — inclusive o portal de
    // destino reaparecido: tanto a sua moldura (passo 4b) quanto a vista
    // see-through gerada pela recursão (passo 5). Se desenhado antes da recursão,
    // esta o sobrescreveria. A recursão alterou câmera/recorte/stencil, então os
    // restauramos para os DESTE nível. O vidro testa profundidade mas não a
    // escreve (ver DrawTransparentObjects): só tinge o que está atrás dele e fica
    // confinado à janela (stencil == level).
    if (g_PortalGL.drawTransparent)
    {
        glUniformMatrix4fv(g_PortalGL.viewUniform, 1, GL_FALSE, glm::value_ptr(virtualView));
        glUniform4fv(g_PortalGL.clipPlaneUniform, 1, glm::value_ptr(clipPlane));
        glEnable(GL_CLIP_DISTANCE0);
        glStencilFunc(GL_EQUAL, level, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDepthFunc(GL_LESS);
        g_PortalGL.drawTransparent();
        glDisable(GL_CLIP_DISTANCE0);
        glUniform4fv(g_PortalGL.clipPlaneUniform, 1, glm::value_ptr(noClip));
    }

    // ---- 6) Desfaz a marca deste nível (DECR: level -> level-1) ----
    // GL_ALWAYS: mexe SÓ no stencil, na mesma região do passo 1, sem tocar a cor
    // (vista já desenhada) nem a profundidade.
    stampStencil(portal, view, level, GL_DECR, GL_ALWAYS);

    // Restaura o estado padrão de cor/profundidade para o resto do laço.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void PortalPair::renderViews(const glm::mat4& view, const glm::mat4& projection,
                             int depth) const
{
    if (!g_PortalGL.drawScene || !g_PortalGL.drawPlane) return;

    // A marcação de stencil é auto-limpante (INCR ao descer, DECR ao subir):
    // cada portal começa do fundo (stencil 0) e o devolve a 0 ao terminar, então
    // os dois portais — e os três pares — não interferem entre si.
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
    // Molduras emissivas dos portais na vista real (sem stencil): o depth buffer
    // tem as profundidades da cena (DrawScene), então o GL_LEQUAL de drawFramePair
    // faz objetos em frente ao portal ocultarem a moldura.
    drawFramePair();
}
