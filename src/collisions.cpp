// ===========================================================================
//  Colisões (Portal-3) — implementação.
//
//  Ver include/collisions.h para a visão geral do módulo.
// ===========================================================================

#include "collisions.h"

#include <limits>
#include "matrices.h" // operações próprias: dotproduct, clampf, Vector_Min/Max

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
    float closest_x = clampf(pos_xz.x, box.min.x, box.max.x);
    float closest_z = clampf(pos_xz.y, box.min.z, box.max.z);

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
    float proj = dotproduct(ap, ab);
    float ab_len_sq = dotproduct(ab, ab);     // Comprimento da parede ao quadrado

    // Calcula 't', que é a porcentagem da projeção ao longo da parede
    float t = proj / ab_len_sq;

    // Grampeia 't' entre 0 e 1 para garantir que não estamos checando um ponto além da parede
    t = clampf(t, 0.0f, 1.0f);

    // Encontra a coordenada exata do ponto mais próximo na parede
    glm::vec2 closest_point = line.p1 + t * ab;

    // Calcula a distância do jogador até esse ponto
    glm::vec2 distance_vec = pos_xz - closest_point;

    // Se a distância ao quadrado for menor que o raio ao quadrado, é colisão!
    return dotproduct(distance_vec, distance_vec) < (raio * raio);
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
        world_min = Vector_Min(world_min, world_corner);
        world_max = Vector_Max(world_max, world_corner);
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

// ----------------------------------------------------------------------------
// Resposta a colisão: movimento do jogador.
// ----------------------------------------------------------------------------

// Aplica um deslocamento vertical 'dy' ao jogador (gravidade/pulo), verificando
// colisão contra o cenário. Se a posição candidata colide (chão ao cair, teto
// ao subir), a velocidade vertical é zerada e o jogador permanece onde está.
void TryMovePlayerVertical(const CollisionWorld& w, const SceneState& s,
                           glm::vec4& playerPos, float& velY, float dy)
{
    glm::vec3 candidate = glm::vec3(playerPos.x, playerPos.y + dy, playerPos.z);
    if (!PlayerCollidesAt(w, s, candidate))
        playerPos.y = candidate.y;
    else
        velY = 0.0f;
}

// Move o jogador pelo deslocamento desejado (dx, dz), testando e aplicando
// cada eixo separadamente. Isso produz o efeito de "deslizar" ao longo de
// paredes: se o movimento em um eixo causa colisão, ele é descartado, mas o
// movimento no outro eixo continua sendo aplicado normalmente.
void TryMovePlayer(const CollisionWorld& w, const SceneState& s,
                   glm::vec4& playerPos, float dx, float dz)
{
    // Define a altura máxima de um degrau que o jogador consegue subir apenas andando.
    const float MAX_STEP_HEIGHT = 0.3f;

    // Tenta mover no eixo X
    glm::vec3 candidate_x = glm::vec3(playerPos.x + dx, playerPos.y, playerPos.z);
    if (!PlayerCollidesAt(w, s, candidate_x))
    {
        playerPos.x = candidate_x.x;
    }
    else if (IsPlayerOnGround(w, s, glm::vec3(playerPos)))
    {
        // Bateu em X! O jogador está no chão, então testamos a posição mais alta (o degrau)
        glm::vec3 candidate_step_x = glm::vec3(playerPos.x + dx, playerPos.y + MAX_STEP_HEIGHT, playerPos.z);
        if (!PlayerCollidesAt(w, s, candidate_step_x))
        {
            playerPos.x = candidate_step_x.x;
            playerPos.y = candidate_step_x.y; // Levanta o jogador para cima do obstáculo
        }
    }

    // Tenta mover no eixo Z
    glm::vec3 candidate_z = glm::vec3(playerPos.x, playerPos.y, playerPos.z + dz);
    if (!PlayerCollidesAt(w, s, candidate_z))
    {
        playerPos.z = candidate_z.z;
    }
    else if (IsPlayerOnGround(w, s, glm::vec3(playerPos)))
    {
        // Bateu em Z! Testamos a subida no degrau também.
        glm::vec3 candidate_step_z = glm::vec3(playerPos.x, playerPos.y + MAX_STEP_HEIGHT, playerPos.z + dz);
        if (!PlayerCollidesAt(w, s, candidate_step_z))
        {
            playerPos.z = candidate_step_z.z;
            playerPos.y = candidate_step_z.y;
        }
    }
}

// ----------------------------------------------------------------------------
// Construção do mundo estático de colisão.
// ----------------------------------------------------------------------------

// Monta o CollisionWorld com as AABBs de mundo do cenário estático contra o
// qual as entidades colidem: paredes/chão/teto das três salas (definidos
// manualmente, já que a geometria é simples e alinhada aos eixos), botão, porta
// fechada, pilar e bolo. As bboxes locais e model matrices de objetos vêm em
// 'geo' (preenchido pelo chamador), mantendo este módulo livre de g_VirtualScene
// e de matrices.h.
CollisionWorld BuildCollisionWorld(const CollisionSceneGeometry& geo)
{
    CollisionWorld world;

    // Espessura "fake" usada para dar volume às paredes/chão/teto no teste de colisão
    const float wall_t = 0.1f;

    // Limites das salas (x,z) e altura (y), conforme mapeamento da cena
    const float sala1_min = -4.0f, sala1_max = 4.0f;
    const float sala2_min = 4.2f,  sala2_max = 12.0f;
    float s3_xmin = 6.0f, s3_xmax = 10.0f;
    float s3_zmin = 6.2f, s3_zmax = 10.1f;
    const float y_min = -1.0f, y_max = 3.0f;

    // A porta entre a Sala 1 e a Sala 2 (centrada em Z = 0)
    const float porta_z = 0.0f;
    // A nova porta no final da Sala 2 (centrada em X = 8.0)
    const float porta2_x = 8.0f;

    const float porta_meia_largura = 0.6f;
    const float porta_topo_y = 0.5f;

    // Cria uma laje retangular de parede (AABB)
    auto add_wall_slab = [&](float x_lo, float x_hi, float z_lo, float z_hi, float y_lo, float y_hi)
    {
        if (z_lo < z_hi && x_lo < x_hi)
            world.aabbs.push_back({ glm::vec3(x_lo, y_lo, z_lo), glm::vec3(x_hi, y_hi, z_hi) });
    };

    // Corta um buraco em uma parede que corre paralela ao eixo Z
    auto add_wall_with_doorway = [&](float x_lo, float x_hi, float z_lo, float z_hi)
    {
        add_wall_slab(x_lo, x_hi, z_lo, porta_z - porta_meia_largura, y_min, y_max);
        add_wall_slab(x_lo, x_hi, porta_z + porta_meia_largura, z_hi, y_min, y_max);
        add_wall_slab(x_lo, x_hi, porta_z - porta_meia_largura, porta_z + porta_meia_largura, porta_topo_y, y_max);
    };

    // Corta um buraco em uma parede que corre paralela ao eixo X (NOVO)
    auto add_wall_with_doorway_z = [&](float x_lo, float x_hi, float z_lo, float z_hi)
    {
        add_wall_slab(x_lo, porta2_x - porta_meia_largura, z_lo, z_hi, y_min, y_max);
        add_wall_slab(porta2_x + porta_meia_largura, x_hi, z_lo, z_hi, y_min, y_max);
        add_wall_slab(porta2_x - porta_meia_largura, porta2_x + porta_meia_largura, z_lo, z_hi, porta_topo_y, y_max);
    };

    // Constrói a sala inteira. Adicionamos o parâmetro 'porta_em_z_max' com valor padrão 'false'.
    auto add_room_aabbs = [&](float x_min, float x_max, float z_min, float z_max, bool porta_em_x_min, bool porta_em_x_max, bool porta_em_z_max = false)
    {
        // Chão e teto
        add_wall_slab(x_min - wall_t, x_max + wall_t, z_min - wall_t, z_max + wall_t, y_min - wall_t, y_min);
        add_wall_slab(x_min - wall_t, x_max + wall_t, z_min - wall_t, z_max + wall_t, y_max, y_max + wall_t);

        // Parede em x = x_min
        if (porta_em_x_min)
            add_wall_with_doorway(x_min - wall_t, x_min, z_min - wall_t, z_max + wall_t);
        else
            add_wall_slab(x_min - wall_t, x_min, z_min - wall_t, z_max + wall_t, y_min, y_max);

        // Parede em x = x_max
        if (porta_em_x_max)
            add_wall_with_doorway(x_max, x_max + wall_t, z_min - wall_t, z_max + wall_t);
        else
            add_wall_slab(x_max, x_max + wall_t, z_min - wall_t, z_max + wall_t, y_min, y_max);

        // Parede em z = z_min (Fundos)
        add_wall_slab(x_min - wall_t, x_max + wall_t, z_min - wall_t, z_min, y_min, y_max);

        // Parede em z = z_max (Frente)
        if (porta_em_z_max)
            add_wall_with_doorway_z(x_min - wall_t, x_max + wall_t, z_max, z_max + wall_t);
        else
            add_wall_slab(x_min - wall_t, x_max + wall_t, z_max, z_max + wall_t, y_min, y_max);
    };

    // Chão e teto físicos da Sala 3
    add_wall_slab(s3_xmin - wall_t, s3_xmax + wall_t, s3_zmin - wall_t, s3_zmax + wall_t, y_min - wall_t, y_min);
    add_wall_slab(s3_xmin - wall_t, s3_xmax + wall_t, s3_zmin - wall_t, s3_zmax + wall_t, y_max, y_max + wall_t);

    // Parede Lateral Direita da Sala 3 (X = 6.0)
    add_wall_slab(s3_xmin - wall_t, s3_xmin, s3_zmin - wall_t, s3_zmax + wall_t, y_min, y_max);

    // Parede Lateral Esquerda da Sala 3 (X = 10.0)
    add_wall_slab(s3_xmax, s3_xmax + wall_t, s3_zmin - wall_t, s3_zmax + wall_t, y_min, y_max);

    // Parede da Frente da Sala 3 (Z = 10.1)
    add_wall_slab(s3_xmin - wall_t, s3_xmax + wall_t, s3_zmax, s3_zmax + wall_t, y_min, y_max);

    // Parede de Trás com Conexão/Porta da Sala 3 (Z = 6.2)
    // Deixa o vão livre entre X = 7.4 e X = 8.6 para você passar vindo da Sala 2!
    add_wall_slab(s3_xmin - wall_t, porta2_x - porta_meia_largura, s3_zmin - wall_t, s3_zmin, y_min, y_max);
    add_wall_slab(porta2_x + porta_meia_largura, s3_xmax + wall_t, s3_zmin - wall_t, s3_zmin, y_min, y_max);
    add_wall_slab(porta2_x - porta_meia_largura, porta2_x + porta_meia_largura, s3_zmin - wall_t, s3_zmin, porta_topo_y, y_max);

    // Sala 1: x,z em [-4,+4] — porta na parede x=+4
    add_room_aabbs(sala1_min, sala1_max, sala1_min, sala1_max, false, true);

    // Sala 2: x em [+4.2,+12], z em [-4,+6] — porta na parede x=+4.2 E porta na parede z=+6.0
    // O último 'true' ativa o novo buraco que criamos!
    add_room_aabbs(sala2_min, sala2_max, -4.0f, 6.0f, true, false, true);

    // Paredes internas da Sala 1 ("prisão" central de vidro + parede sólida)
    // Cada parede é um the_plane ([-1,1] em local) com as rotações aplicadas na renderização,
    // resultando em lajes verticais de espessura wall_t centradas nas posições abaixo.
    // Parede sólida em x=+1 (Agora a colisão começa exatamentente onde o visual começa!)
    world.aabbs.push_back({ glm::vec3(1.0f, y_min, -1.0f), glm::vec3(1.0f + wall_t, y_max, 1.0f) });
    // Parede de vidro em x=-1
    world.aabbs.push_back({ glm::vec3(-1.0f - wall_t, y_min, -1.0f), glm::vec3(-1.0f + wall_t, y_max, 1.0f) });
    // Parede de vidro em z=+1
    world.aabbs.push_back({ glm::vec3(-1.0f, y_min, 1.0f - wall_t), glm::vec3(1.0f, y_max, 1.0f + wall_t) });
    // Parede de vidro em z=-1
    world.aabbs.push_back({ glm::vec3(-1.0f, y_min, -1.0f - wall_t), glm::vec3(1.0f, y_max, -1.0f + wall_t) });

    // Botão: AABB de mundo (model matrix e bbox local vêm em geo).
    world.aabbs.push_back(ComputeWorldAABB(geo.buttonMin, geo.buttonMax, geo.buttonModel));

    // Paredes Diagonais:
    // Parede diagonal da Sala 2 (Centro: 11.0, -2.7 | Rotação: -45 graus | Escala: 2.0)
    // Calculamos as extremidades aplicando o seno/cosseno de 45 graus (0.707) * escala (2.0) = 1.414
    float offsetX = 1.414f;
    float offsetZ = 1.414f;

    world.lines.push_back({
        glm::vec2(11.0f - offsetX, -2.7f - offsetZ), // Ponto 1 (~ 9.586, -4.114)
        glm::vec2(11.0f + offsetX, -2.7f + offsetZ), // Ponto 2 (~ 12.414, -1.286)
        y_min,                                       // Chão (-1.0f)
        y_max                                        // Teto (3.0f)
    });

    // Hitbox dinâmica da porta 2 (Fechada): AABB de mundo (model matrix em geo).
    world.closedDoor = ComputeWorldAABB(geo.doorMin, geo.doorMax, geo.doorModel);

    // Hitboxes Físicas do Pilar e do Bolo (Sala 3)
    // O pilar está centralizado em X=8.0 e Z=8.15.
    // Com a escala de 0.2, os limites horizontais são exatamente:
    // X: 8.0 - 0.2 até 8.0 + 0.2 (7.8f a 8.2f)
    // Z: 8.15 - 0.2 até 8.15 + 0.2 (7.95f a 8.35f)
    // E a altura vai do chão (-1.0f) até o topo do pilar (-0.2f)
    world.aabbs.push_back({
        glm::vec3(7.8f, -1.0f, 7.95f),  // Mínimo (X, Y, Z)
        glm::vec3(8.2f, -0.2f, 8.35f)   // Máximo (X, Y, Z)
    });

    // O bolo está em cima do pilar (Y = -0.2f) com escala 0.1.
    // X: 8.0 - 0.1 até 8.0 + 0.1 (7.9f a 8.1f)
    // Z: 8.15 - 0.1 até 8.15 + 0.1 (8.05f a 8.25f)
    // A altura vai do topo do pilar (-0.2f) até o topo das velas (cerca de 0.0f)
    world.aabbs.push_back({
        glm::vec3(7.9f, -0.2f, 8.05f),  // Mínimo (X, Y, Z)
        glm::vec3(8.1f,  0.0f, 8.25f)   // Máximo (X, Y, Z)
    });

    // Dimensões do cilindro do jogador (usa os defaults de PlayerCollider).
    world.player = PlayerCollider{};

    // Cacheia as AABBs locais (carregadas pelo tinyobj) da caixa ("Cube") e do
    // rádio ("Shell"). PlayerCollidesAt as transforma a cada frame pela model
    // matrix dinâmica (vinda do SceneState) para obter a AABB de mundo precisa.
    world.boxLocalMin   = geo.boxLocalMin;
    world.boxLocalMax   = geo.boxLocalMax;
    world.radioLocalMin = geo.radioLocalMin;
    world.radioLocalMax = geo.radioLocalMax;

    return world;
}
