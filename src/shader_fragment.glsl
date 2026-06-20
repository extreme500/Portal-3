#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Lanterna do jogador
uniform vec4 flashlight_pos;
uniform vec4 flashlight_dir;
uniform int flashlight_on;

// Identificador que define qual objeto está sendo desenhado no momento // Constantes
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define PLAYER_HEAD 3
#define PLAYER_EYE 4
#define PLAYER_TORSO 5
#define PLAYER_LEGS 6
#define PLAYER_HAIR 7
#define CUBE_003 8
#define CUBE_CIRCLE1 9
#define CUBE_002 10
#define CUBE 11
#define CUBE_CIRCLE2 12
#define CUBE_CIRCLE3 13
#define BUTTON 14
#define BUTTON_001 15
#define DOOR 16
#define WALL 17
#define FLOOR 18
#define CEILING 19
#define GLASS 20
#define WALL_2 21
#define WALL_4 22
#define SEC_CAM 23
#define DOOR_WALL 24
#define RADIO_SHELL 25
#define RADIO_MAIN 26
#define RADIO_GRID 27
#define RADIO_SCREEN 28
#define RADIO_LIGHT 29
#define RADIO_ANTENNA 30
#define RADIO_BUTTON 31
#define RADIO_BUTTON_RIFLED 32
#define RADIO_BASE_PART 33
#define CAKE 34
#define CAKE_CANDLE 35
#define CAKE_CHERRY 36
#define CAKE_CHERRY_CREAM 37
#define CAKE_BOTTOM 38

uniform int object_id;

// Passo de renderização do portal: 0 = stencil + cor (descarta fora da elipse),
// 1 = moldura (descarta dentro da elipse, mantém a borda).
uniform int portalPass;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura.
//
// IMPORTANTE (compatibilidade com macOS): o macOS limita o fragment shader a
// no máximo 16 samplers ativos, mas a cena usa ~30 texturas distintas. Para
// respeitar esse limite, em vez de declarar uma sampler por textura, usamos
// apenas duas: a textura difusa do objeto atual (TextureDiffuse) e uma textura
// auxiliar (TextureAux), usada para mapa especular / metalness / emissivo.
// O código C++ (BindTexturesForObject) vincula, antes de cada objeto, a(s)
// textura(s) correta(s) às unidades 0 e 1 de acordo com o object_id.
uniform sampler2D TextureDiffuse; // unidade de textura 0
uniform sampler2D TextureAux;     // unidade de textura 1

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

vec3 baseColor		 = 1.3*vec3(.82, .67, .16);
float metallic		 = 1.0;
float subsurface	 = 0.0;
float specular		 = 1.0;
float roughness		 = 0.6;
float specularTint	 = 0.0;
float anisotropic	 = 0.0;
float sheen			 = 0.0;
float sheenTint		 = 0.0;
float clearcoat		 = 0.0;
float clearcoatGloss = 1.0;

const float PI = 3.14159265358979323846;

// Luzes direcionais configuradas diretamente no shader.
// --- LUZES DA CENA (Point Lights) ---
const int LIGHT_COUNT = 11; // 4 da Sala 1 + 6 da Sala 2 + 1 da Sala 3

const vec3 LIGHT_POSITIONS[LIGHT_COUNT] = vec3[](
    // ================= SALA 1 =================
    vec3( 3.8, 2.9,  3.8), // Canto Superior Direito (Frente)
    vec3(-3.8, 2.9,  3.8), // Canto Superior Esquerdo (Frente)
    vec3( 3.8, 2.9, -3.8), // Canto Superior Direito (Trás)
    vec3(-3.8, 2.9, -3.8), // Canto Superior Esquerdo (Trás)

    // ================= SALA 2 =================
    // A Sala 2 vai de X=4 a 12, e Z=-4 a 12.
    
    // Fundo da Sala 2 (Z = 11.0)
    vec3( 4.8, 2.9, 11.0), // Esquerda Fundo
    vec3(11.2, 2.9, 11.0), // Direita Fundo
    
    // Meio da Sala 2 (Z = 4.0)
    vec3( 4.4, 2.9,  5.8), // Esquerda Meio
    vec3(11.8, 2.9,  5.8), // Direita Meio
    
    // Frente da Sala 2 (Z perto de -3.0)
    vec3( 4.4, 2.9, -3.8), // Esquerda Frente (Perto da porta da Sala 1)
    
    // A PAREDE DIAGONAL: Fica em X=11.0, Z=-2.7. 
    // Vamos puxar essa luz mais para o meio (X=9.5, Z=-1.5) para ela não ficar dentro do concreto!
    vec3( 9.5, 2.9, -3.8),

    // Cima da Sala 3
    vec3(8.0, 2.9, 8.0) // Esquerda Frente (Perto da porta da Sala 1)
);

// Cor das luzes (O tom azulado frio de laboratório)
const vec3 LIGHT_COLORS[LIGHT_COUNT] = vec3[](
    // Sala 1 (4 luzes)
    vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8),
    
    // Sala 2 (6 luzes)
    vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8), 
    vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8), vec3(0.4, 0.6, 0.8),

    vec3(0.4, 0.6, 0.8)
);

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// object_id para portais
#define PORTAL_BLUE   90
#define PORTAL_ORANGE 91

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

	// Coeficiente de refletância difusa
	vec3 Kd0;

    // Fator de refletância especular (o quanto o objeto brilha)
    float Ks_map = 0.0;

    // Variáveis locais para controlar os nossos "hacks" de PBR
    vec3 emissive_color = vec3(0.0);
    float metalness = 0.0; // 0.0 = dielétrico (plástico/tinta), 1.0 = metal puro

    // Máscara elíptica para portais: recorta o quad "the_plane" em forma oval.
    // position_model está em coordenadas locais do modelo (the_plane tem vértices
    // em [-1,1] no plano XZ). Fragmentos dentro da elipse (inclinação < 1) são
    // mantidos no passe de stencil (portalPass==0); no passe de moldura
    // (portalPass==1) invertemos, mantendo só a borda externa.
    // Limiares da máscara elíptica do portal:
    //   INNER — dentro = cena virtual; fora = moldura ou descarte (stencil).
    //   OUTER — só usado no passe de moldura: descarta fragmentos além da
    //           borda externa da elipse, para o portal não ser um retângulo.
    const float PORTAL_INNER = 0.85;
    const float PORTAL_OUTER = 1.0;

    if (object_id == PORTAL_BLUE || object_id == PORTAL_ORANGE)
    {
        vec2 p_local = position_model.xz; // the_plane é XZ (Y é a normal)
        float d = dot(p_local, p_local);
        if (portalPass == 0) {
            // Stencil / cor → descarta fora da elipse (mantém interior).
            if (d > PORTAL_INNER) discard;
        } else {
            // Moldura → mantém só o anel entre INNER e OUTER.
            if (d <= PORTAL_INNER || d > PORTAL_OUTER) discard;
        }
    }

    if (object_id == PORTAL_BLUE)
    {
        Kd0 = vec3(0.0, 0.4, 0.9);
        emissive_color = vec3(0.0, 0.6, 1.0) * 0.6;
    }
    else if (object_id == PORTAL_ORANGE)
    {
        Kd0 = vec3(1.0, 0.5, 0.0);
        emissive_color = vec3(1.0, 0.7, 0.0) * 0.6;
    }
    else if ( object_id == WALL)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        // Multiplicamos por um fator (ex: 1.0). Quanto maior o número, mais ela se repete.
        float fatorRepeticao = 1.0f; 
        U = texcoords.x * fatorRepeticao;
        V = texcoords.y * fatorRepeticao;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse
		Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == WALL_2)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        // Multiplicamos por um fator (ex: 2.0). Quanto maior o número, mais ela se repete.
        float fatorRepeticao = 2.0f; 
        U = texcoords.x * fatorRepeticao;
        V = texcoords.y * fatorRepeticao;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse
		Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == WALL_4)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        // Multiplicamos por um fator (ex: 4.0). Quanto maior o número, mais ela se repete.
        float fatorRepeticao = 4.0f; 
        U = texcoords.x * fatorRepeticao;
        V = texcoords.y * fatorRepeticao;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse
		Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == FLOOR)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        // Multiplicamos por um fator (ex: 5.0). Quanto maior o número, mais ela se repete.
        float fatorRepeticao = 4.0f; 
        U = texcoords.x * fatorRepeticao;
        V = texcoords.y * fatorRepeticao;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse
		Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if ( object_id == CEILING)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        // Multiplicamos por um fator (ex: 5.0). Quanto maior o número, mais ela se repete.
        float fatorRepeticao = 4.0f; 
        U = texcoords.x * fatorRepeticao;
        V = texcoords.y * fatorRepeticao;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse
		Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if (object_id == PLAYER_HEAD)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == PLAYER_EYE)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == PLAYER_TORSO)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == PLAYER_LEGS)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == PLAYER_HAIR)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == CUBE_003)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse e o especular da TextureDiffuse

        // cor base do cubo (apertureTexture)
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

        Ks_map = texture(TextureAux, vec2(U,V)).r;
        
    }
    else if (object_id == CUBE)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse e o especular da TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

        Ks_map = texture(TextureAux, vec2(U,V)).r;
        
    }
    else if (object_id == CUBE_CIRCLE3)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

        Ks_map = 0.5;
        
    }
    else if (object_id == CUBE_CIRCLE1)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir de uma cor azul

        Kd0 = vec3(54.0/255.0, 192.0/255.0, 241.0/255.0);
        
    }
    else if (object_id == CUBE_CIRCLE2)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir de uma cor escura

        Kd0 = vec3(16.0/255.0, 16.0/255.0, 16.0/255.0).rgb;

        Ks_map = 0.5;
        
    }
    else if (object_id == BUTTON)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

    }
    else if (object_id == BUTTON_001)
    {


        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == DOOR)
    {

        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
    }
    else if (object_id == GLASS)
    {
        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir de uma cor branca

        Kd0 = vec3(1.0f,1.0f,1.0f);

        Ks_map = 0.5;
    }
    else if (object_id == SEC_CAM)
    {
        U = texcoords.x;
        V = texcoords.y;

		// Obtemos a refletância difusa a partir da leitura da imagem TextureDiffuse e o especular da TextureDiffuse

        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

        Ks_map = texture(TextureAux, vec2(U,V)).r;
        
    }
    else if ( object_id == DOOR_WALL)
    {
        float fatorRepeticao = 1.0; 
        U = texcoords.x * fatorRepeticao;
        V = texcoords.y * fatorRepeticao;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

        // O TRUQUE DE FLAT SHADING: Ignora a normal do arquivo e calcula 
        // a perpendicular verdadeira da face na hora de desenhar!
        vec3 true_normal = normalize(cross(dFdx(position_world.xyz), dFdy(position_world.xyz)));
        
        // Substitui a normal 'n' que seria usada na iluminação
        n = vec4(true_normal, 0.0);
    }
    else if ( object_id == RADIO_SHELL) 
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == RADIO_MAIN)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        // LÊ A TEXTURA ZERO! Pegamos o canal vermelho (r) pois a imagem é em preto e branco.
        metalness = texture(TextureAux, vec2(U,V)).r;
    }
    else if ( object_id == RADIO_GRID)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        metalness = 1.0; // Como não temos textura pra grade, forçamos 100% metal
    }
    else if ( object_id == RADIO_SCREEN) 
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;

        // Pega a textura Emissiva
        // Multiplica por 2.0 para fazer a telinha brilhar no escuro!
        emissive_color = texture(TextureAux, vec2(U,V)).rgb * 2.0;
    }
    else if ( object_id == RADIO_LIGHT) 
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        
        // Em vez de gastar o canal 32 com a imagem "Light_Emissive", nós simplesmente
        // usamos a própria cor base (Kd0) como luz e multiplicamos por um fator!
        emissive_color = Kd0 * 3.0;
    }
    else if ( object_id == RADIO_ANTENNA) 
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        metalness = 1.0; // Força 100% metal
    }
    else if ( object_id == RADIO_BUTTON)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == RADIO_BUTTON_RIFLED)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
        metalness = 1.0; // Força 100% metal
    }
    else if ( object_id == RADIO_BASE_PART)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == CAKE)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == CAKE_CANDLE)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureDiffuse, vec2(U,V)).rgb;
    }
    else if ( object_id == CAKE_CHERRY)
    {
        Kd0 = vec3(0.65, 0.05, 0.15);
    }
    else if ( object_id == CAKE_CHERRY_CREAM)
    {
        Kd0 = vec3(0.92, 0.90, 0.90);
    }
    else if ( object_id == CAKE_BOTTOM)
    {
        Kd0 = vec3(0.85, 0.40, 0.05);
    }






  // --- CÁLCULO DAS LUZES DO TETO ---
    vec3 total_diffuse = vec3(0.0);
    vec3 total_specular = vec3(0.0);

    for(int i = 0; i < LIGHT_COUNT; i++) {
        // Vetor que aponta do fragmento para a luz atual
        vec3 light_dir = LIGHT_POSITIONS[i] - p.xyz;
        float dist = length(light_dir); // Distância até a luz
        vec3 l = normalize(light_dir);

        // Atenuação: a luz perde força conforme chega no chão ou no meio da sala
        // Fraca/original, caso queiramos comparar:
        // float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.04 * (dist * dist));

        // Forte/nova: A luz perde força rapidamente, criando cantos escuros.
        float attenuation = 1.0 / (1.0 + 0.14 * dist + 0.07 * (dist * dist));

        // Termo Difuso (Lambert)
        float lambert = max(0.0, dot(n.xyz, l));
        vec3 diffuse = Kd0 * LIGHT_COLORS[i] * lambert;

        // Termo Especular (Blinn-Phong)
        vec3 h = normalize(v.xyz + l);
        float n_dot_h = max(0.0, dot(n.xyz, h));
        float shininess = mix(32.0, 64.0, metalness); 
        float specular_intensity = pow(n_dot_h, shininess);

        // O metal pinta o reflexo com a cor da própria textura (Kd0)
        vec3 base_specular_color = mix(vec3(1.0), Kd0, metalness);
        
        // O metal brilha mais forte com as luzes
        float metal_boost = mix(1.0, 3.0, metalness);

        float current_ks = Ks_map;
        // Se for uma peça do rádio e não tiver Ks_map definido, damos um brilho base de 0.3
        if (object_id >= RADIO_SHELL && object_id <= RADIO_BASE_PART && Ks_map == 0.0) {
            current_ks = 0.3; 
        }

        vec3 specular = base_specular_color * LIGHT_COLORS[i] * specular_intensity * current_ks * metal_boost;

        // vec3 specular = vec3(1.0) * LIGHT_COLORS[i] * specular_intensity * Ks_map; // Antigo

        // Somamos a contribuição dessa luz com a atenuação aplicada
        total_diffuse += diffuse * attenuation;
        total_specular += specular * attenuation;
    }

    // Luz ambiente bem fraca (para as sombras não ficarem 100% pretas)
    // A original, para comparação também, era 0.05, mas foi diminuída para criar um ambiente levemente mais escuro/dependente das luzes
    vec3 ambient_light = Kd0 * 0.01;

    // Consideramos a lanterna do jogador, se ligada
    vec3 flashlight_color = vec3(0.85, 0.95, 1.0); // Cor levemente amarelada
    vec3 final_flashlight = vec3(0.0);

    if (flashlight_on == 1) {
        vec3 L_spot = normalize(flashlight_pos.xyz - p.xyz);
        vec3 D_spot = normalize(flashlight_dir.xyz);
        
        // Cálculo do ângulo do cone (o dot product compara a luz com o centro do cone)
        float cos_theta = dot(-L_spot, D_spot);
        float cos_cutoff = cos(radians(25.0)); // 25 graus de abertura
        
        if (cos_theta > cos_cutoff) {
            // Atenuação por distância (para não iluminar o mundo todo)
            float dist = length(flashlight_pos.xyz - p.xyz);
            float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.01 * (dist * dist));
            
            // Intensidade baseada no ângulo (suaviza as bordas do cone)
            float epsilon = 0.05;
            float spot_intensity = smoothstep(cos_cutoff, cos_cutoff + epsilon, cos_theta);
            
            final_flashlight = Kd0 * flashlight_color * attenuation * spot_intensity;
        }
    }

    //  Cor final do fragmento somada com a luz da lanterna no final
    color.rgb = ambient_light + total_diffuse + total_specular + final_flashlight + emissive_color;

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    if (object_id == GLASS)
    {
        color.a = 0.5;
    }

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 

