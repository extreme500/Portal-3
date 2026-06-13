//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Computação Gráfica e Visualização I
//               Prof. Eduardo Gastal
//
//     CÓDIGO BASE PARA O TRABALHO FINAL
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <set>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "bezier.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};


// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename, bool deveRepetir); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

// Declaração de funções utilizadas para detecção e resposta a colisões
struct CollisionAABB; // forward declaration (struct definida acima de main())
bool CylinderIntersectsAABB(glm::vec2 pos_xz, float raio, float y_min, float y_max, const CollisionAABB& box);
bool PlayerCollidesAt(const glm::vec3& pos);
bool BoxCollidesAt(glm::vec3 pos);
bool IsButtonTriggered();
bool IsPlayerOnGround();
void TryMovePlayer(float dx, float dz);
void TryMovePlayerVertical(float dy);
CollisionAABB ComputeWorldAABB(glm::vec3 local_min, glm::vec3 local_max, const glm::mat4& model);
void SetupCollisionAABBs();


// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;

// Lanterna
GLint g_flashlight_pos_uniform;
GLint g_flashlight_dir_uniform;
GLint g_flashlight_on_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// Variável para a posição global do personagem (e, consequentemente, da câmera)
glm::vec4 Pos_Player = glm::vec4(.0f,.0f,-2.0f,1.0f);
float velocidade = 3; // Velocidade do personagem para andar

// Controle da Caixa estilo Portal
glm::vec3 g_BoxPosition = glm::vec3(11.0f, .0f, 5.0f);
float g_BoxVelocityY = 0.0f; // Controla a gravidade da caixa
float g_BoxAngleY = 0.0f;    // Controla a rotação para ela "encarar" o jogador
bool g_IsHoldingBox = false;
bool g_EWasPressed = false;

// Controle botão (objeto) pressionado
bool g_IsButtonPressed = false;

// Representação física do jogador para fins de colisão: um cilindro vertical
// cujos pés ficam em "Pos_Player.y + PLAYER_FEET_Y_OFFSET" (ver abaixo) e cujo
// topo fica altura unidades acima disso.
struct PlayerCollider {
    float raio   = 0.25f;
    float altura = 1.4f;
};
PlayerCollider g_PlayerCollider;

// O modelo do jogador é desenhado com um deslocamento vertical de -1.0f em
// relação a Pos_Player (ver DrawVirtualObject de player_model_*: a matriz de
// modelagem usa "Pos_Player.y - 1.0f"), de forma que Pos_Player.y == 0.0f
// corresponde aos pés apoiados no chão das salas (y = -1.0f). O cilindro de
// colisão usa essa mesma referência para os seus pés.
const float PLAYER_FEET_Y_OFFSET = -1.0f;

// Margem de tolerância ("skin") usada para encolher o intervalo vertical
// testado em PlayerCollidesAt — evita que o jogador fique "grudado" ao tocar
// exatamente o chão/teto, o que travaria também o movimento horizontal.
const float COLLISION_SKIN = 0.02f;

// Física vertical: gravidade e pulo
const float GRAVIDADE  = -9.8f; // aceleração da gravidade, em unidades/s²
const float FORCA_PULO =  4.5f; // velocidade vertical inicial ao pular, em unidades/s
float g_PlayerVelocityY = 0.0f; // velocidade vertical atual do jogador
bool  g_SpaceWasPressed = false; // estado da tecla espaço no frame anterior (detecção de borda de subida, evita pulo contínuo ao segurar a tecla)

// Caixa delimitadora axis-aligned em coordenadas de mundo, usada nos testes
// de colisão do cenário (paredes, chão, teto, cubo, botão, porta)
struct CollisionAABB {
    glm::vec3 min;
    glm::vec3 max;
};
std::vector<CollisionAABB> g_CollisionAABBs;

// Representa uma parede diagonal no plano XZ
struct CollisionLine {
    glm::vec2 p1;    // Ponto inicial da parede (X, Z)
    glm::vec2 p2;    // Ponto final da parede (X, Z)
    float y_min;     // Base da parede (chão)
    float y_max;     // Topo da parede (teto)
};

// Vetor global para armazenar as paredes diagonais do cenário
std::vector<CollisionLine> g_CollisionLines;

// Variáveis para controle de tempo
float ultimoFrame = 0.0f;
float deltaTime = 0.0f;

// Variável que controla a lanterna do jogador
bool g_FlashlightEnabled = false;

// Modo de câmera ativa
enum CameraMode { CAMERA_FPS, CAMERA_SECURITY };
CameraMode g_CameraMode = CAMERA_FPS;

// Controle de múltiplas câmeras de segurança ---
int g_ActiveSecurityCamera = 0; // 0 = Sala 1, 1 = Sala 2
int g_TotalSecurityCameras = 2; //

// Variável para movimentação do modelo da Câmera de Segurança
int mov_sec_camera = 1;

// Altura dos olhos do jogador na câmera FPS
const float FPS_EYE_HEIGHT = 0.3f;

// Duração de uma passagem completa da câmera de segurança (em segundos)
const float SECURITY_CAM_SWEEP_PERIOD = 14.0f;

// Última posição conhecida do cursor (usada para calcular deltas de movimento)
double g_LastCursorPosX, g_LastCursorPosY;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Portal 3", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Carregamos duas imagens para serem utilizadas como textura
    LoadTextureImage("../../data/red_brick_diff_1k.jpg",true);      // TextureImage0
    LoadTextureImage("../../data/wall/Portal-concrete_modular_wall002.png",true); // TextureImage1
    LoadTextureImage("../../data/chell/textures/chell_head_diffuse.png",false);      // TextureImage2
    LoadTextureImage("../../data/chell/textures/eyeball_l.png",false);      // TextureImage3
    LoadTextureImage("../../data/chell/textures/chell_torso_diffuse.png",false);      // TextureImage4
    LoadTextureImage("../../data/chell/textures/chell_legs_diffuse.png",false);      // TextureImage5
    LoadTextureImage("../../data/chell/textures/chell_hair.png",false);      // TextureImage6
    LoadTextureImage("../../data/cube/textures/apertureTexture.jpg",false);      // TextureImage7
    LoadTextureImage("../../data/cube/textures/dirt.jpg",false);      // TextureImage8
    LoadTextureImage("../../data/cube/textures/dirtMain2.jpg",false);      // TextureImage9
    LoadTextureImage("../../data/cube/textures/normal.jpg",false);      // TextureImage10
    LoadTextureImage("../../data/cube/textures/specular.jpg",false);      // TextureImage11
    LoadTextureImage("../../data/cube/textures/SpecularMap.png",false);      // TextureImage12
    LoadTextureImage("../../data/button/textures/portal_button_blue emission.png",false);      // TextureImage13
    LoadTextureImage("../../data/button/textures/portal_button_blue.jpeg",false);      // TextureImage14
    LoadTextureImage("../../data/door/textures/internal_ground_ao_texture.jpeg",false);      // TextureImage15
    LoadTextureImage("../../data/door/textures/portal_door_02_E_upscayl_2x_ultramix-balan.png",false);      // TextureImage16
    LoadTextureImage("../../data/door/textures/portal_door_02_upscayl_8x_ultramix-balance.png",false);      // TextureImage17
    LoadTextureImage("../../data/floor/metallic_floor.jpg",true);      // TextureImage18
    LoadTextureImage("../../data/ceiling/portal_ceiling.png",true);      // TextureImage19
    LoadTextureImage("../../data/sec_camera/textures/security_camera.png",true);      // TextureImage20
    LoadTextureImage("../../data/sec_camera/textures/internal_ground_ao_texture.jpeg",true);      // TextureImage21


    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel bunnymodel("../../data/bunny.obj");
    ComputeNormals(&bunnymodel);
    BuildTrianglesAndAddToVirtualScene(&bunnymodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel playermodel("../../data/chell/source/Chell.obj");
    ComputeNormals(&playermodel);
    BuildTrianglesAndAddToVirtualScene(&playermodel);
     
    ObjModel portalcube("../../data/cube/source/cube.obj");
    ComputeNormals(&portalcube);
    BuildTrianglesAndAddToVirtualScene(&portalcube);

    ObjModel portalbutton("../../data/button/source/Button.obj");
    ComputeNormals(&portalbutton);
    BuildTrianglesAndAddToVirtualScene(&portalbutton);

    ObjModel portaldoor("../../data/door/source/Door.obj");
    ComputeNormals(&portaldoor);
    BuildTrianglesAndAddToVirtualScene(&portaldoor);
    
    ObjModel portalopendoor("../../data/door/source/OpenDoor.obj");
    ComputeNormals(&portalopendoor);
    BuildTrianglesAndAddToVirtualScene(&portalopendoor);

    ObjModel doorwall("../../data/door_wall/door_wall.obj");
    ComputeNormals(&doorwall);
    BuildTrianglesAndAddToVirtualScene(&doorwall);

    ObjModel securitycamera("../../data/sec_camera/source/Sec_camera.obj");
    ComputeNormals(&securitycamera);
    BuildTrianglesAndAddToVirtualScene(&securitycamera);


    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Construímos as AABBs de colisão do cenário (paredes, chão, teto, cubo,
    // botão), uma única vez, já que a geometria estática não muda em runtime.
    SetupCollisionAABBs();

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Câmera FPS começa com cursor capturado e invisível
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Inicializamos ultimoFrame aqui para evitar um deltaTime gigante no primeiro
    // frame (o carregamento dos assets leva vários segundos, e ultimoFrame = 0
    // causaria gravidade acumulada suficiente para teleportar o jogador para -∞)
    ultimoFrame = (float)glfwGetTime();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização

        // Cálculos de tempo
        float atualFrame = (float)glfwGetTime();
        deltaTime = atualFrame - ultimoFrame;
        ultimoFrame = atualFrame;

        // Limitador de Delta Time (Anti-Tunneling) ---
        // Se o frame demorar mais de 0.05 segundos (menos de 20 FPS) devido a 
        // arrastar a janela ou lag, nós travamos o tempo para a física não explodir.
        if (deltaTime > 0.05f) 
        {
            deltaTime = 0.05f;
        }

        // Movimento do jogador relativo à direção da câmera FPS
        float fwd_x = -sin(g_CameraTheta);
        float fwd_z = -cos(g_CameraTheta);
        float right_x =  cos(g_CameraTheta);
        float right_z = -sin(g_CameraTheta);

        // Acumulamos o deslocamento desejado neste frame e só então tentamos
        // aplicá-lo, verificando colisão eixo a eixo (TryMovePlayer faz o
        // jogador "deslizar" ao longo de paredes em vez de travar nelas).
        float desired_dx = 0.0f;
        float desired_dz = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            desired_dx += fwd_x * velocidade * deltaTime;
            desired_dz += fwd_z * velocidade * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            desired_dx -= fwd_x * velocidade * deltaTime;
            desired_dz -= fwd_z * velocidade * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            desired_dx -= right_x * velocidade * deltaTime;
            desired_dz -= right_z * velocidade * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            desired_dx += right_x * velocidade * deltaTime;
            desired_dz += right_z * velocidade * deltaTime;
        }

        TryMovePlayer(desired_dx, desired_dz);

        // Pulo: detectamos a borda de subida da tecla espaço (o instante em que
        // ela é pressionada, não enquanto fica segurada) e só permitimos pular
        // quando o jogador está apoiado no chão (sem pulo duplo no ar)
        bool spacePressedNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spacePressedNow && !g_SpaceWasPressed && IsPlayerOnGround())
            g_PlayerVelocityY = FORCA_PULO;
        g_SpaceWasPressed = spacePressedNow;

        // Gravidade: aceleramos a velocidade vertical e tentamos aplicar o
        // deslocamento resultante; colisões com chão/teto zeram a velocidade
        g_PlayerVelocityY += GRAVIDADE * deltaTime;
        TryMovePlayerVertical(g_PlayerVelocityY * deltaTime);


        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        // Câmera FPS: posição na cabeça do jogador, direção controlada pelo mouse
        // Câmera de segurança: posição fixa no canto superior, lookat animado por Bézier cúbica
        glm::vec4 camera_position_c;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        if (g_CameraMode == CAMERA_FPS)
        {
            // Distância que a câmera deve ficar à frente do centro do modelo
            float camera_offset_forward = 0.1f;

            // Vetor de direção "frente" no plano horizontal (ignorando inclinação da cabeça)
            float dir_x = -sin(g_CameraTheta);
            float dir_z = -cos(g_CameraTheta);

            // Cabeça do jogador + deslocamento (offset) na direção que ele está olhando
            camera_position_c = Pos_Player + glm::vec4(
                camera_offset_forward * dir_x, 
                FPS_EYE_HEIGHT, 
                camera_offset_forward * dir_z, 
                0.0f
            );
            
            // Bob de câmera ao caminhar: oscilação em Y e Z (eixo de profundidade da câmera)
            bool isMoving = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
            if (isMoving)
            {
                float t = (float)glfwGetTime();
                // Y sobe e desce uma vez por passada
                camera_position_c.y += 0.05f * sin(t * 10.0f);
                // Z avança e recua duas vezes por passada (stride)
                float bob_z = 0.025f * sin(t * 20.0f);
                camera_position_c.x += bob_z * (-sin(g_CameraTheta));
                camera_position_c.z += bob_z * (-cos(g_CameraTheta));
            }

            // Direção: vetor em coordenadas esféricas controlado pelo mouse
            float vx = -cos(g_CameraPhi) * sin(g_CameraTheta);
            float vy =  sin(g_CameraPhi);
            float vz = -cos(g_CameraPhi) * cos(g_CameraTheta);
            camera_view_vector = glm::vec4(vx, vy, vz, 0.0f);
        }
        else // CAMERA_SECURITY
        {
            // Define a posição base dependendo de qual câmera está ativa
            if (g_ActiveSecurityCamera == 0) {
                camera_position_c = glm::vec4(-3.8f, 2.8f, -3.8f, 1.0f); // Sala 1
            } else {
                camera_position_c = glm::vec4(8.0f, 2.8f, 5.8f, 1.0f);   // Sala 2
            }

            // Define a direção da visão com base no modo (mov_sec_camera)
            if (mov_sec_camera == 1) 
            {
                // --- MODO 1: Seguir o Jogador (LookAt) ---
                glm::vec3 target_player = glm::vec3(Pos_Player.x, Pos_Player.y + 0.5f, Pos_Player.z);
                
                // Aplica o sistema anti-parede apenas para a câmera 0 (para não atravessar a parede)
                if (g_ActiveSecurityCamera == 0) {
                    float dist_x = std::abs(target_player.x - camera_position_c.x);
                    float dist_z = std::abs(target_player.z - camera_position_c.z);
                    float dist = std::max(dist_x, dist_z);
                    float t = glm::clamp((dist - 1.0f) / (1.5f - 1.0f), 0.0f, 1.0f);
                    float limite_parede = glm::mix(-1.3f, -2.8f, t);
                    
                    if (target_player.x < limite_parede) target_player.x = limite_parede;
                    if (target_player.z < limite_parede) target_player.z = limite_parede;
                }

                // O vetor de visão global é do alvo até a câmera
                glm::vec3 dir = target_player - glm::vec3(camera_position_c);
                camera_view_vector = glm::vec4(dir.x, dir.y, dir.z, 0.0f);
            }
            else
            {
                // --- MODO 2: Varredura Automática (Bézier) ---
                // O cálculo de tempo é o mesmo para todas as câmeras
                float raw  = fmod((float)glfwGetTime(), 2.0f * SECURITY_CAM_SWEEP_PERIOD) / SECURITY_CAM_SWEEP_PERIOD;
                float ping = raw < 1.0f ? raw : 2.0f - raw;
                int   seg  = (int)(ping * 3.0f);
                if (seg > 2) seg = 2;
                float local_t = ping * 3.0f - (float)seg;
                if (local_t > 1.0f) local_t = 1.0f;
                float smooth_t = local_t * local_t * (3.0f - 2.0f * local_t);
                
                if (g_ActiveSecurityCamera == 0) 
                {
                    // --- CÂMERA 0: Sala 1 ---

                    const glm::vec4 wp[4] = {
                        glm::vec4( 3.8f,  1.0f, -3.8f, 1.0f), 
                        glm::vec4(-3.8f,  1.0f,  3.8f, 1.0f), 
                        glm::vec4( 3.8f, -0.8f, -3.8f, 1.0f), 
                        glm::vec4(-3.8f, -0.8f,  3.8f, 1.0f), 
                    };

                    glm::vec4 from = wp[seg];
                    glm::vec4 to   = wp[seg + 1];
                    glm::vec4 lookat_target = BezierCubic(
                        from, from + (to - from) * (1.0f / 3.0f), from + (to - from) * (2.0f / 3.0f), to, smooth_t
                    );
                    camera_view_vector = lookat_target - camera_position_c;
                }
                else if (g_ActiveSecurityCamera == 1)
                {
                    // --- CÂMERA 1: Sala 2 (Em cima da porta olhando pra frente) ---

                    // Sala 2 vai de X: 4.2 a 12, Z: -4 a 6. A porta aberta está em (8, y, 6).
                const glm::vec4 wp[4] = {
                        glm::vec4( 4.5f,  1.0f,  2.0f, 1.0f), // 0: Perto Esquerda
                        glm::vec4(11.5f,  1.0f, -3.5f, 1.0f), // 1: Longe Direita
                        glm::vec4( 4.5f, -0.8f, -3.5f, 1.0f), // 2: Longe Esquerda
                        glm::vec4(11.5f, -0.8f,  2.0f, 1.0f), // 3: Perto Direita
                    };

                    glm::vec4 from = wp[seg];
                    glm::vec4 to   = wp[seg + 1];
                    glm::vec4 lookat_target = BezierCubic(
                        from, from + (to - from) * (1.0f / 3.0f), from + (to - from) * (2.0f / 3.0f), to, smooth_t
                    );
                    camera_view_vector = lookat_target - camera_position_c;
                }
            }
        }

        // Garantir que position_c é ponto (w=1) e view_vector é vetor (w=0),
        // pois dotproduct() em matrices.h rejeita pontos com exit()
        camera_position_c.w = 1.0f;
        camera_view_vector.w = 0.0f;

        // Computamos a matriz "View"
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Agora computamos a matriz de Projeção.
        // Computamos a matriz de Projeção (Sempre Perspectiva)
        float nearplane = -0.1f;  
        float farplane  = -20.0f; 
        float field_of_view = 3.141592f / 3.0f; // 60 graus
        glm::mat4 projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // Constantes
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

        // Constantes
        #define M_PI   3.14159265358979323846
        #define M_PI_2 1.57079632679489661923

        // Defina uniformes no shader (supondo que você buscou os IDs com glGetUniformLocation)
        // Dentro do loop, perto de onde você envia view e projection:
        glUniform4fv(g_flashlight_pos_uniform, 1, glm::value_ptr(camera_position_c));
        glUniform4fv(g_flashlight_dir_uniform, 1, glm::value_ptr(camera_view_vector));
        glUniform1i(g_flashlight_on_uniform, (int)g_FlashlightEnabled);

        
        // Lógica de Interação com a Caixa (Pegar/Soltar - Física, Gravidade e Raycast)
        bool ePressedNow = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        
        if (ePressedNow && !g_EWasPressed && g_CameraMode == CAMERA_FPS)
        {
            if (g_IsHoldingBox) {
                g_IsHoldingBox = false; // Solta a caixa
            } else {
                // Tenta pegar a caixa
                float dist = glm::distance(glm::vec3(Pos_Player), g_BoxPosition);
                glm::vec3 dirToBox = glm::normalize(g_BoxPosition - glm::vec3(Pos_Player));
                float dotProduct = glm::dot(dirToBox, glm::vec3(camera_view_vector));
                
                if (dist < 2.0f && dotProduct > 0.85f) {
                    g_IsHoldingBox = true;
                    g_BoxVelocityY = 0.0f; // Corta a gravidade enquanto segura
                }
            }
        }
        g_EWasPressed = ePressedNow;

        if (g_IsHoldingBox)
        {
            // O jogador está segurando a caixa: o Raycast
            glm::vec3 start = glm::vec3(camera_position_c);
            start.y -= 0.1f; // Abaixa da linha dos olhos para não cegar o jogador
            glm::vec3 dir = glm::vec3(camera_view_vector);
            
            float max_dist = 1.5f;
            float step = 0.1f;
            glm::vec3 finalPos = start;

            // Empurra a caixa aos poucos na direção da visão. 
            // Se ela bater na parede no caminho, ela para antes de atravessar.
            for (float d = 0.2f; d <= max_dist; d += step) {
                glm::vec3 testPos = start + dir * d;
                if (BoxCollidesAt(testPos)) {
                    break; // Bateu na parede, usamos a última posição válida (finalPos)
                }
                finalPos = testPos;
            }
            g_BoxPosition = finalPos;
            
            // Magia do Face-Tracking: Copia a rotação da câmera (Eixo Y) para o cubo
            g_BoxAngleY = g_CameraTheta;

            // SISTEMA ANTI-ESMAGAMENTO (A Lógica do Portal)
            // Se você anda contra a parede, a parede empurra a caixa na direção da câmera.
            // Se a distância entre a origem do raio (jogador) e a caixa ficar menor que 0.6 unidades,
            // o jogador solta a caixa automaticamente para evitar que a câmera entre nela.
            float distToPlayer = glm::distance(start, finalPos);
            if (distToPlayer < 1.0f) {
                g_IsHoldingBox = false;
            }
        }
        else
        {
            // O jogador soltou a caixa: Gravidade!
            g_BoxVelocityY += GRAVIDADE * deltaTime;
            
            // Simula onde a caixa vai estar no próximo frame
            glm::vec3 candidatePos = g_BoxPosition;
            candidatePos.y += g_BoxVelocityY * deltaTime;

            // SISTEMA ANTI-PRISÃO (Caixa bate no jogador e não atravessa)
            // Criamos a hitbox temporária da caixa no frame exato da queda
            glm::mat4 candidate_model = Matrix_Translate(candidatePos.x, candidatePos.y, candidatePos.z) 
                                      * Matrix_Rotate_Y(g_BoxAngleY) 
                                      * Matrix_Scale(1.0f/8.0f, 1.0f/8.0f, 1.0f/8.0f);
            
            CollisionAABB candidateAABB = ComputeWorldAABB(
                g_VirtualScene["Cube"].bbox_min, 
                g_VirtualScene["Cube"].bbox_max, 
                candidate_model
            );
            
            // Variáveis do corpo do jogador
            glm::vec2 pos_xz(Pos_Player.x, Pos_Player.z);
            float feet_y = Pos_Player.y + PLAYER_FEET_Y_OFFSET;
            
            // Em vez de usar a altura do cilindro físico (-0.4), 
            // usamos a altura da câmera (+0.3) para a caixa bater na sua "cabeça" real!
            float y_max_player_real = Pos_Player.y + FPS_EYE_HEIGHT; 

            // Testa se a caixa vai encostar no jogador (do pé até a cabeça)
            bool bateuNoJogador = CylinderIntersectsAABB(pos_xz, g_PlayerCollider.raio, feet_y, y_max_player_real, candidateAABB);

            // Se essa queda não fizer ela bater no chão ou num objeto, ela cai
            if (!BoxCollidesAt(candidatePos)) {
                g_BoxPosition.y = candidatePos.y;
            } else {
                g_BoxVelocityY = 0.0f; // Bateu, zera a velocidade de queda
            }
        }

        // Informações de base sobre a CENA
        // O chão dela acontece em -1.0f
        // O teto dela acontece em +3.0f
        // As paredes da primeira sala vão de -4.0 até +4.0 em x e em z
        // As paredes (naturalmente/1.0f de fatorRepeticao) tem 2.0f de altura
        // 1R corresponde à Sala 1, e 2R corresponde à Sala 2

        model = Matrix_Translate(Pos_Player.x, Pos_Player.y-1.0f, Pos_Player.z) * Matrix_Rotate_Y(g_CameraTheta + M_PI) * Matrix_Scale(1/55.00,1/55.00,1/55.00);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PLAYER_HEAD);
        DrawVirtualObject("player_model_head");
        glUniform1i(g_object_id_uniform, PLAYER_EYE);
        DrawVirtualObject("player_model_left_eye");
        DrawVirtualObject("player_model_right_eye");
        glUniform1i(g_object_id_uniform, PLAYER_TORSO);
        DrawVirtualObject("player_model_torso");
        glUniform1i(g_object_id_uniform, PLAYER_LEGS);
        DrawVirtualObject("player_model_legs");
        glUniform1i(g_object_id_uniform, PLAYER_HAIR);
        DrawVirtualObject("player_model_hair");

        
        // Cena Da 1R
        
        // Desenhamos o plano do chão (1R)
        float fatorRepeticao = 4.0f;
        model = Matrix_Translate(0.0f,-1.0f,0.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, FLOOR);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano do teto (1R)
        model = Matrix_Translate(0.0f,3.0f,0.0f) * Matrix_Rotate_X(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, CEILING);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 1R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(.0f, -1.0f + fatorRepeticao,-4.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 1R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(.0f, -1.0f + fatorRepeticao, +4.0f)*Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 1R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(-4.0f, -1.0f + fatorRepeticao, .0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");
        
        // Desenhamos o plano da parede (Esquerda 1R 1/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.0f, -1.0f + fatorRepeticao, +3.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 1R 2/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.0f, -1.0f + fatorRepeticao, -3.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 1R 3/3) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.0f, 2.0f,0.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.0f, -1.0f, .0f) * Matrix_Rotate_Y(3 * M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, DOOR_WALL); 
        DrawVirtualObject("door_wall");

        // Desenhamos o modelo da porta (Aberta 1R)
        model = Matrix_Translate(+4.1f, -1.0f, .0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Scale(1.5f,1.5f,1.5f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, DOOR);
        DrawVirtualObject("portal_door_combined_model_2");

        // Desenhamos a câmera que segue o jogador/curva de Bézier (1R)
        // Primeiro, desenhamos o suporte que é estático na parede (para ficar mais visualmente agradável, vamos "espelhar" o modelo, sendo necessário mudar a renderização da geometria)
        glFrontFace(GL_CW);
        model = Matrix_Translate(-3.8f, 2.8f, -3.8f) * Matrix_Scale(-1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SEC_CAM);
        DrawVirtualObject("security_camera_2");
        glFrontFace(GL_CCW);
        // Depois, desenhamos a "câmera", que se movimenta
        glm::vec3 sec_cam_pos = glm::vec3(-3.8f, 2.8f, -3.8f);
        glm::vec3 sec_cam_front;
        if (g_CameraMode == CAMERA_FPS)
        {
            if (mov_sec_camera == 1) 
            {
                // MODO 1: Lente mira no jogador
                glm::vec3 target_player = glm::vec3(Pos_Player.x, Pos_Player.y + 0.5f, Pos_Player.z);
                
                // Se o jogador chegar muito perto da parede da câmera (que está em -3.8),
                // nós "travamos" o alvo da câmera em uma distância segura para não atravessar a parede.
                // Calculamos a distância do jogador para a câmera, ignorando o Y
                float dist_x = std::abs(target_player.x - sec_cam_pos.x);
                float dist_z = std::abs(target_player.z - sec_cam_pos.z);
                float dist = std::max(dist_x, dist_z);

                // Mapeamos a distância para um fator de 0.0 (perto) a 1.0 (longe)
                // Se estiver a 1.0 de distância ou menos, fator = 0.0. Se estiver a 1.5 ou mais, fator = 1.0.
                float t = glm::clamp((dist - 1.0f) / (1.5f - 1.0f), 0.0f, 1.0f);

                // Interpolação linear (Lerp): o limite varia suavemente entre -1.38 e -2.8
                float limite_parede = glm::mix(-1.3f, -2.8f, t);

                // Aplicação do limite calculado
                if (target_player.x < limite_parede) target_player.x = limite_parede;
                if (target_player.z < limite_parede) target_player.z = limite_parede;

                sec_cam_front = glm::normalize(target_player - sec_cam_pos);
            }
            else if (mov_sec_camera == 2)
            {
                // MODO 2: Lente faz o Bézier
                float raw  = fmod((float)glfwGetTime(), 2.0f * SECURITY_CAM_SWEEP_PERIOD) / SECURITY_CAM_SWEEP_PERIOD;
                float ping = raw < 1.0f ? raw : 2.0f - raw;

                const glm::vec4 wp[4] = {
                    glm::vec4( 3.8f,  1.0f, -3.8f, 1.0f), 
                    glm::vec4(-3.8f,  1.0f,  3.8f, 1.0f), 
                    glm::vec4( 3.8f, -0.8f, -3.8f, 1.0f), 
                    glm::vec4(-3.8f, -0.8f,  3.8f, 1.0f), 
                };

                int seg = (int)(ping * 3.0f);
                if (seg > 2) seg = 2;
                float local_t = ping * 3.0f - (float)seg;
                if (local_t > 1.0f) local_t = 1.0f;

                float smooth_t = local_t * local_t * (3.0f - 2.0f * local_t);

                glm::vec4 from = wp[seg];
                glm::vec4 to   = wp[seg + 1];
                glm::vec4 lookat_target = BezierCubic(
                    from,
                    from + (to - from) * (1.0f / 3.0f),
                    from + (to - from) * (2.0f / 3.0f),
                    to,
                    smooth_t
                );
                
                // Vetor de direção = (Alvo do Bézier) - (Posição da Câmera Física)
                glm::vec3 bezier_dir = glm::vec3(lookat_target) - sec_cam_pos;
                sec_cam_front = glm::normalize(bezier_dir);
            }
        }
        else // CAMERA_SECURITY
        {
            // Se estamos enxergando PELA câmera de segurança, o modelo físico 
            // e a câmera global olham exatamente para a mesma direção.
            sec_cam_front = glm::normalize(glm::vec3(camera_view_vector));
        }
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rotation_matrix = glm::inverse(glm::lookAt(glm::vec3(0.0f), sec_cam_front, up));
        model = Matrix_Translate(-3.8f, 2.8f, -3.8f) * rotation_matrix * Matrix_Rotate_Y(M_PI) * Matrix_Scale(1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SEC_CAM);
        DrawVirtualObject("security_camera_2.001");

        // Cena Da 2R

        // Desenhamos o plano do chão 1/2 (2R)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(+8.0f,-1.0f,.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, FLOOR);
        DrawVirtualObject("the_plane");

         // Desenhamos o plano do chão 2/2 (2R)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(+8.0f,-1.0f,+8.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, FLOOR);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano do teto 1/2 (2R)
        model = Matrix_Translate(+8.0f,3.0f,0.0f) * Matrix_Rotate_X(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, CEILING);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano do teto 2/2 (2R)
        model = Matrix_Translate(+8.0f,3.0f,8.0f) * Matrix_Rotate_X(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, CEILING);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 1/4) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.2f, -1.0f + fatorRepeticao, +3.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 2/4) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.2f, -1.0f + fatorRepeticao, -3.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 3/4) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.2f, 2.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 4/4) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(+4.2f, -1.0f + fatorRepeticao, 9.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 2R-1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.2f, -1.0f, .0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, DOOR_WALL); 
        DrawVirtualObject("door_wall");
        
        // Desenhamos o plano da parede (Diagonal 2R) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+11.0f, -1.0f+ fatorRepeticao,-2.7f) * Matrix_Rotate_Y(-M_PI_2/2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 2R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(8.0f, -1.0f + fatorRepeticao,-4.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 2R 1/2) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(12.0f, -1.0f + fatorRepeticao, 0.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 2R 2/2) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(12.0f, -1.0f + fatorRepeticao, +8.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 2R 1/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+11.0f, -1.0f + fatorRepeticao, +6.0f) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 2R 2/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+5.0f, -1.0f + fatorRepeticao, 6.0f) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 2R 3/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(8.0f, 2.0f,+6.0f)*Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(8.0f, -1.0f,+6.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, DOOR_WALL); 
        DrawVirtualObject("door_wall");

        // Desenhamos o modelo do cubo
        model = Matrix_Translate(g_BoxPosition.x, g_BoxPosition.y, g_BoxPosition.z) * Matrix_Rotate_Y(g_BoxAngleY) * Matrix_Scale(1.0f/8.0f, 1.0f/8.0f, 1.0f/8.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, CUBE_003);
        DrawVirtualObject("Cube.003");
        DrawVirtualObject("Cube.002");
        glUniform1i(g_object_id_uniform, CUBE_CIRCLE1);
        DrawVirtualObject("Circle");
        glUniform1i(g_object_id_uniform, CUBE_CIRCLE2);
        DrawVirtualObject("Circle.002");
        glUniform1i(g_object_id_uniform, CUBE_CIRCLE3);
        DrawVirtualObject("Circle.003");
        glUniform1i(g_object_id_uniform, CUBE);
        DrawVirtualObject("Cube");

        // Verifica se tem algum objeto em cima do botão
        g_IsButtonPressed = IsButtonTriggered();

        // Desenhamos o modelo da porta (Fechada ou Aberta 2R-3R)
        model = Matrix_Translate(8.0f, -1.0f,+6.1f) * Matrix_Rotate_Y(M_PI) * Matrix_Scale(1.5f,1.5f,1.5f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, DOOR);
        if (g_IsButtonPressed) {
        // Desenha o modelo da porta ABERTA
            DrawVirtualObject("portal_door_combined_model_2"); 
        } else {
            // Desenha o modelo da porta FECHADA
            DrawVirtualObject("portal_door_combined_model_1");
        }

        // Desenhamos o modelo do botão
        float y_offset = g_IsButtonPressed ? -0.05f : 0.0f;
        model = Matrix_Translate(9.0f, -1.0f,-1.0f)*Matrix_Scale(2.0,2.0,2.0);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, BUTTON);
        DrawVirtualObject("portal_button_reduced_2");
        model = Matrix_Translate(9.0f, -1.0f + y_offset,-1.0f)*Matrix_Scale(2.0,2.0,2.0);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, BUTTON_001);
        DrawVirtualObject("portal_button_reduced_2.001");


        // Desenhamos a câmera que segue o jogador/curva de Bézier (2R)
        // Primeiro, desenhamos o suporte que é estático na parede (para ficar mais visualmente agradável, vamos "espelhar" o modelo, sendo necessário mudar a renderização da geometria)
        glFrontFace(GL_CW);
        model = Matrix_Translate(8.0f, 2.8f, 5.8f) * Matrix_Rotate_Y(M_PI) * Matrix_Scale(-1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SEC_CAM);
        DrawVirtualObject("security_camera_2");
        glFrontFace(GL_CCW);
        // Depois, desenhamos a "câmera", que se movimenta
        sec_cam_pos = glm::vec3(8.0f, 2.8f, 5.8f);
        if (g_CameraMode == CAMERA_FPS)
        {
            if (mov_sec_camera == 1) 
            {
                // --- MODO 1: Lente mira no jogador (Câmera 2) ---
                glm::vec3 target_player = glm::vec3(Pos_Player.x, Pos_Player.y + 0.5f, Pos_Player.z);
                
                // Calculamos o quão de lado o jogador está em relação à câmera (eixo X)
                // Posição da câmera 2 no eixo X é 8.0f
                float dist_x = std::abs(target_player.x - 8.0f);

                // Mapeamos essa distância lateral para o nosso fator 't' (0.0 a 1.0)
                // Se ele estiver até 1.0 unidade pro lado, t = 0 (Perto)
                // Se ele estiver a 3.0 unidades ou mais pro lado, t = 1 (Longe nos cantos)
                float t = glm::clamp((dist_x - 1.0f) / (3.0f - 1.0f), 0.0f, 1.0f);

                // Interpolação linear (Lerp): 
                // Se t=0 (jogador na frente), o limite de Z é 5.6 (quase debaixo da câmera).
                // Se t=1 (jogador nos cantos), forçamos o limite de Z para 4.5 (empurra a visão pro meio da sala).
                float limite_parede_z = glm::mix(1.5f, 7.5f, t);

                // Aplica o limite! Como a câmera olha para valores negativos de Z (fundo da sala),
                // nós não deixamos o alvo da câmera ultrapassar o limite seguro.
                if (target_player.z > limite_parede_z) {
                    target_player.z = limite_parede_z;
                }

                sec_cam_front = glm::normalize(target_player - sec_cam_pos);
            }
            else if (mov_sec_camera == 2)
            {
                // MODO 2: Lente faz o Bézier
                float raw  = fmod((float)glfwGetTime(), 2.0f * SECURITY_CAM_SWEEP_PERIOD) / SECURITY_CAM_SWEEP_PERIOD;
                float ping = raw < 1.0f ? raw : 2.0f - raw;

                const glm::vec4 wp[4] = {
                    glm::vec4( 4.5f,  1.0f,  2.0f, 1.0f), // 0: Perto Esquerda
                    glm::vec4(11.5f,  1.0f, -3.5f, 1.0f), // 1: Longe Direita
                    glm::vec4( 4.5f, -0.8f, -3.5f, 1.0f), // 2: Longe Esquerda
                    glm::vec4(11.5f, -0.8f,  2.0f, 1.0f), // 3: Perto Direita
                };

                int seg = (int)(ping * 3.0f);
                if (seg > 2) seg = 2;
                float local_t = ping * 3.0f - (float)seg;
                if (local_t > 1.0f) local_t = 1.0f;

                float smooth_t = local_t * local_t * (3.0f - 2.0f * local_t);

                glm::vec4 from = wp[seg];
                glm::vec4 to   = wp[seg + 1];
                glm::vec4 lookat_target = BezierCubic(
                    from,
                    from + (to - from) * (1.0f / 3.0f),
                    from + (to - from) * (2.0f / 3.0f),
                    to,
                    smooth_t
                );
                
                // Vetor de direção = (Alvo do Bézier) - (Posição da Câmera Física)
                glm::vec3 bezier_dir = glm::vec3(lookat_target) - sec_cam_pos;
                sec_cam_front = glm::normalize(bezier_dir);
            }
        }
        else // CAMERA_SECURITY
        {
            // Se estamos enxergando PELA câmera de segurança, o modelo físico 
            // e a câmera global olham exatamente para a mesma direção.
            sec_cam_front = glm::normalize(glm::vec3(camera_view_vector));
        }
        up = glm::vec3(0.0f, 1.0f, 0.0f);
        rotation_matrix = glm::inverse(glm::lookAt(glm::vec3(0.0f), sec_cam_front, up));
        model = Matrix_Translate(8.0f, 2.8f, 5.8f) * rotation_matrix * Matrix_Rotate_Y(M_PI) * Matrix_Scale(1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SEC_CAM);
        DrawVirtualObject("security_camera_2.001");


        // Voltamos para a Sala 1 (a parte transparente precisa ser renderizada por último)

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        // Desenhamos o plano da parede (Esquerda 1/2) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+1.0f,.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 2/2) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+1.0f, 2.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano das paredes de vidro (Frente 1/2)
        model = Matrix_Translate(+.0f,.0f,+1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GLASS);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano das paredes de vidro (Frente 2/2)
        model = Matrix_Translate(+.0f,2.0f,+1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GLASS);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano das paredes de vidro (Trás 1/2)
        model = Matrix_Translate(+.0f,.0f,-1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GLASS);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano das paredes de vidro (Trás 2/2)
        model = Matrix_Translate(+.0f,2.0f,-1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GLASS);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano das paredes de vidro (Direita 1/2)
        model = Matrix_Translate(-1.0f,.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GLASS);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano das paredes de vidro (Direita 2/2)
        model = Matrix_Translate(-1.0f,2.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GLASS);
        DrawVirtualObject("the_plane");


        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// ----------------------------------------------------------------------------
// Funções de detecção e resposta a colisões (jogador vs. cenário)
// ----------------------------------------------------------------------------

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

// Testa se o cilindro do jogador, na posição candidata 'pos' (mesma referência
// de Pos_Player — não os pés diretamente, ver PLAYER_FEET_Y_OFFSET), colide
// com alguma AABB do cenário registrada em g_CollisionAABBs.
//
// O intervalo vertical testado é "encolhido" por COLLISION_SKIN em cada ponta:
// sem isso, ao ficar exatamente apoiado no chão (pés tocando o topo da AABB do
// piso) o teste consideraria o jogador permanentemente colidindo também na
// horizontal, travando todo o movimento. Esse encolhimento permite uma
// penetração tolerável de poucos centímetros, imperceptível no jogo.
bool PlayerCollidesAt(const glm::vec3& pos)
{
    glm::vec2 pos_xz(pos.x, pos.z);
    float feet_y = pos.y + PLAYER_FEET_Y_OFFSET;
    float y_min = feet_y + COLLISION_SKIN;
    float y_max = feet_y + g_PlayerCollider.altura - COLLISION_SKIN;

    // Testa colisões com caixas estáticas (cenário)
    for (const CollisionAABB& box : g_CollisionAABBs)
    {
        if (CylinderIntersectsAABB(pos_xz, g_PlayerCollider.raio, y_min, y_max, box))
            return true;
    }

    // Testa colisões com paredes diagonais
    for (const CollisionLine& line : g_CollisionLines) {
        if (CylinderIntersectsLine(pos_xz, g_PlayerCollider.raio, y_min, y_max, line))
            return true;
    }

    // Testa colisão dinâmica com a CAIXA
    // O jogador só esbarra na caixa se NÃO estiver segurando ela
    if (!g_IsHoldingBox) 
    {
        // Cria a matriz do cubo exatamente onde ele está no mundo agora
        glm::mat4 model_cube = Matrix_Translate(g_BoxPosition.x, g_BoxPosition.y, g_BoxPosition.z) 
                            * Matrix_Rotate_Y(g_BoxAngleY) 
                            * Matrix_Scale(1.0f/8.0f, 1.0f/8.0f, 1.0f/8.0f);
    
        // Calcula a AABB da caixa naquele exato frame
        CollisionAABB boxAABB = ComputeWorldAABB(
            g_VirtualScene["Cube"].bbox_min, 
            g_VirtualScene["Cube"].bbox_max, 
            model_cube
        );
        
        // Testa se o cilindro do jogador bateu nessa AABB gerada
        if (CylinderIntersectsAABB(pos_xz, g_PlayerCollider.raio, y_min, y_max, boxAABB))
            return true;    
    }

    return false;
}

// Verifica se a caixa está atravessando o cenário físico
bool BoxCollidesAt(glm::vec3 pos) {
    // Escala aproximada do cubo no mundo (metade do tamanho total dele)
    float s = 0.3f; 
    glm::vec3 min_box = pos + glm::vec3(-s, -s, -s);
    glm::vec3 max_box = pos + glm::vec3(s, s, s);

    // 1. Testa colisões com o cenário retilíneo (chão, teto, paredes AABB)
    for (const CollisionAABB& aabb : g_CollisionAABBs) {
        if (max_box.x > aabb.min.x && min_box.x < aabb.max.x &&
            max_box.y > aabb.min.y && min_box.y < aabb.max.y &&
            max_box.z > aabb.min.z && min_box.z < aabb.max.z) 
        {
            return true;
        }
    }
    
    // 2. Testa colisões com paredes diagonais (aproveitando o teste do cilindro)
    glm::vec2 pos_xz(pos.x, pos.z);
    for (const CollisionLine& line : g_CollisionLines) {
        // Usamos a meia-largura (s) como raio para o teste contra a linha
        if (CylinderIntersectsLine(pos_xz, s, min_box.y, max_box.y, line)) {
            return true;
        }
    }

    return false;
}

bool IsButtonTriggered() {
    // Definir a área do botão (o gatilho)
    glm::vec3 b_min = glm::vec3(8.5f, -1.0f, -1.5f);
    glm::vec3 b_max = glm::vec3(9.5f, -0.5f, -0.5f);

    // Testa Jogador
    // O cilindro do jogador está sobre o botão?
    if (Pos_Player.x >= b_min.x && Pos_Player.x <= b_max.x &&
        Pos_Player.z >= b_min.z && Pos_Player.z <= b_max.z &&
        (Pos_Player.y + PLAYER_FEET_Y_OFFSET) <= b_max.y) {
        return true;
    }

    // Testa Caixa
    // A caixa está sobre o botão?
    if (g_BoxPosition.x >= b_min.x && g_BoxPosition.x <= b_max.x &&
        g_BoxPosition.z >= b_min.z && g_BoxPosition.z <= b_max.z &&
        g_BoxPosition.y <= b_max.y) {
        return true;
    }

    return false;
}

// Verifica se há piso imediatamente abaixo dos pés do jogador. Usado para só
// permitir o pulo quando ele está apoiado em uma superfície (sem pulo duplo).
bool IsPlayerOnGround()
{
    const float probe_dist = 0.05f;
    return PlayerCollidesAt(glm::vec3(Pos_Player.x, Pos_Player.y - probe_dist, Pos_Player.z));
}

// Aplica um deslocamento vertical 'dy' ao jogador (gravidade/pulo), verificando
// colisão contra o cenário. Se a posição candidata colide (chão ao cair, teto
// ao subir), a velocidade vertical é zerada e o jogador permanece onde está.
void TryMovePlayerVertical(float dy)
{
    glm::vec3 candidate = glm::vec3(Pos_Player.x, Pos_Player.y + dy, Pos_Player.z);
    if (!PlayerCollidesAt(candidate))
        Pos_Player.y = candidate.y;
    else
        g_PlayerVelocityY = 0.0f;
}

// Move o jogador pelo deslocamento desejado (dx, dz), testando e aplicando
// cada eixo separadamente. Isso produz o efeito de "deslizar" ao longo de
// paredes: se o movimento em um eixo causa colisão, ele é descartado, mas o
// movimento no outro eixo continua sendo aplicado normalmente.
void TryMovePlayer(float dx, float dz)
{
    // Define a altura máxima de um degrau que o jogador consegue subir apenas andando.
    // O seu botão é relativamente baixo, então 0.3f deve cobrir perfeitamente.
    const float MAX_STEP_HEIGHT = 0.3f; 

    // Tenta mover no eixo X
    glm::vec3 candidate_x = glm::vec3(Pos_Player.x + dx, Pos_Player.y, Pos_Player.z);
    if (!PlayerCollidesAt(candidate_x))
    {
        Pos_Player.x = candidate_x.x;
    }
    else if (IsPlayerOnGround()) 
    {
        // Bateu em X! O jogador está no chão, então testamos a posição mais alta (o degrau)
        glm::vec3 candidate_step_x = glm::vec3(Pos_Player.x + dx, Pos_Player.y + MAX_STEP_HEIGHT, Pos_Player.z);
        if (!PlayerCollidesAt(candidate_step_x))
        {
            Pos_Player.x = candidate_step_x.x;
            Pos_Player.y = candidate_step_x.y; // Levanta o jogador para cima do obstáculo
        }
    }

    // Tenta mover no eixo Z
    glm::vec3 candidate_z = glm::vec3(Pos_Player.x, Pos_Player.y, Pos_Player.z + dz);
    if (!PlayerCollidesAt(candidate_z))
    {
        Pos_Player.z = candidate_z.z;
    }
    else if (IsPlayerOnGround())
    {
        // Bateu em Z! Testamos a subida no degrau também.
        glm::vec3 candidate_step_z = glm::vec3(Pos_Player.x, Pos_Player.y + MAX_STEP_HEIGHT, Pos_Player.z + dz);
        if (!PlayerCollidesAt(candidate_step_z))
        {
            Pos_Player.z = candidate_step_z.z;
            Pos_Player.y = candidate_step_z.y;
        }
    }
}

// Calcula a AABB de mundo de um objeto a partir da sua AABB local (de
// g_VirtualScene) e da matriz "model" usada para desenhá-lo: transforma os 8
// cantos da AABB local pela matriz e recomputa o min/max resultante. Válido
// mesmo quando a matriz inclui rotação e/ou escala.
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

// Preenche g_CollisionAABBs com as AABBs de mundo do cenário estático contra
// o qual o jogador colide: paredes/chão/teto das duas salas (definidas
// manualmente, já que a geometria é simples e alinhada aos eixos) mais o
// cubo e o botão (cuja AABB local é transformada pela matriz "model" usada ao
// desenhá-los). Chamada uma única vez, no setup, pois o cenário é estático.
void SetupCollisionAABBs()
{
    g_CollisionAABBs.clear();
    g_CollisionLines.clear();

    // Espessura "fake" usada para dar volume às paredes/chão/teto no teste de colisão
    const float wall_t = 0.1f;

    // Limites das salas (x,z) e altura (y), conforme mapeamento da cena
    const float sala1_min = -4.0f, sala1_max = 4.0f;
    const float sala2_min = 4.2f,  sala2_max = 12.0f;
    const float y_min = -1.0f, y_max = 3.0f;

    // A porta entre a Sala 1 (parede em x=+4) e a Sala 2 (parede em x=+4.2)
    // fica centrada em z=0. O vão é bem mais largo e alto que o cilindro do
    // jogador (raio 0.25, altura 0.6, pés ao nível do chão em y=-1), com
    // bastante folga, para garantir a passagem confortável entre as salas.
    const float porta_z = 0.0f;
    const float porta_meia_largura = 0.6f; // metade da largura do vão (vão total = 1.2)
    const float porta_topo_y = 0.5f;      // altura da verga (acima da cabeça do jogador, que vai até y ≈ 0.5)

    // Cria uma laje retangular de parede (AABB fina ao longo do eixo X)
    auto add_wall_slab = [&](float x_lo, float x_hi, float z_lo, float z_hi, float y_lo, float y_hi)
    {
        if (z_lo < z_hi)
            g_CollisionAABBs.push_back({ glm::vec3(x_lo, y_lo, z_lo), glm::vec3(x_hi, y_hi, z_hi) });
    };

    // Parede ao longo do eixo X com um vão de porta centrado em porta_z: gera
    // os segmentos à esquerda, à direita e a verga (acima da abertura), de modo
    // que o espaço [porta_z ± porta_meia_largura] x [chão, porta_topo_y] fique livre
    auto add_wall_with_doorway = [&](float x_lo, float x_hi, float z_lo, float z_hi)
    {
        add_wall_slab(x_lo, x_hi, z_lo, porta_z - porta_meia_largura, y_min, y_max);
        add_wall_slab(x_lo, x_hi, porta_z + porta_meia_largura, z_hi, y_min, y_max);
        add_wall_slab(x_lo, x_hi, porta_z - porta_meia_largura, porta_z + porta_meia_largura, porta_topo_y, y_max);
    };

    auto add_room_aabbs = [&](float x_min, float x_max, float z_min, float z_max, bool porta_em_x_min, bool porta_em_x_max)
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

        // Paredes em z = z_min e z = z_max
        add_wall_slab(x_min - wall_t, x_max + wall_t, z_min - wall_t, z_min, y_min, y_max);
        add_wall_slab(x_min - wall_t, x_max + wall_t, z_max, z_max + wall_t, y_min, y_max);
    };

    // Sala 1: x,z em [-4,+4] — porta na parede x=+4 (passagem para a Sala 2)
    add_room_aabbs(sala1_min, sala1_max, sala1_min, sala1_max, /*porta_em_x_min=*/false, /*porta_em_x_max=*/true);
    // Sala 2: x em [+4.2,+12], z em [-4,+6] — porta na parede x=+4.2 (passagem para a Sala 1)
    add_room_aabbs(sala2_min, sala2_max, -4.0f, 6.0f, /*porta_em_x_min=*/true, /*porta_em_x_max=*/false);

    // Paredes internas da Sala 1 ("prisão" central de vidro + parede sólida)
    // Cada parede é um the_plane ([-1,1] em local) com as rotações aplicadas na renderização,
    // resultando em lajes verticais de espessura wall_t centradas nas posições abaixo.
    // Parede sólida em x=+1
    g_CollisionAABBs.push_back({ glm::vec3(1.0f - wall_t, y_min, -1.0f), glm::vec3(1.0f + wall_t, y_max, 1.0f) });
    // Parede de vidro em x=-1
    g_CollisionAABBs.push_back({ glm::vec3(-1.0f - wall_t, y_min, -1.0f), glm::vec3(-1.0f + wall_t, y_max, 1.0f) });
    // Parede de vidro em z=+1
    g_CollisionAABBs.push_back({ glm::vec3(-1.0f, y_min, 1.0f - wall_t), glm::vec3(1.0f, y_max, 1.0f + wall_t) });
    // Parede de vidro em z=-1
    g_CollisionAABBs.push_back({ glm::vec3(-1.0f, y_min, -1.0f - wall_t), glm::vec3(1.0f, y_max, -1.0f + wall_t) });

    // Botão: mesma matriz "model" usada para desenhá-lo (Matrix_Translate(9,-1,-1) * Matrix_Scale(1.65,1.65,1.65))
    glm::mat4 model_button = Matrix_Translate(9.0f, -1.0f, -1.0f) * Matrix_Scale(1.65f, 1.65f, 1.65f);
    g_CollisionAABBs.push_back(ComputeWorldAABB(g_VirtualScene["portal_button_reduced_2"].bbox_min, g_VirtualScene["portal_button_reduced_2"].bbox_max, model_button));

    // Paredes Diagonais:
    // Parede diagonal da Sala 2 (Centro: 11.0, -2.7 | Rotação: -45 graus | Escala: 2.0)
    // Calculamos as extremidades aplicando o seno/cosseno de 45 graus (0.707) * escala (2.0) = 1.414
    float offsetX = 1.414f;
    float offsetZ = 1.414f;
    
    g_CollisionLines.push_back({
        glm::vec2(11.0f - offsetX, -2.7f - offsetZ), // Ponto 1 (~ 9.586, -4.114)
        glm::vec2(11.0f + offsetX, -2.7f + offsetZ), // Ponto 2 (~ 12.414, -1.286)
        y_min,                                       // Chão (-1.0f)
        y_max                                        // Teto (3.0f)
    });
}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename, bool deveRepetir)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    if (deveRepetir)
    {
        glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");
    g_flashlight_pos_uniform = glGetUniformLocation(g_GpuProgramID, "flashlight_pos"); // Lanterna
    g_flashlight_dir_uniform = glGetUniformLocation(g_GpuProgramID, "flashlight_dir");
    g_flashlight_on_uniform  = glGetUniformLocation(g_GpuProgramID, "flashlight_on");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage5"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage6"), 6);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage7"), 7);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage8"), 8);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage9"), 9);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage10"), 10);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage11"), 11);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage12"), 12);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage13"), 13);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage14"), 14);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage15"), 15);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage16"), 16);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage17"), 17);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage18"), 18);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage19"), 19);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage20"), 20);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage21"), 21);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice e que pertencem ao mesmo "smoothing group".

    // Obtemos a lista dos smoothing groups que existem no objeto
    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        assert(model->shapes[shape].mesh.smoothing_group_ids.size() == num_triangles);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            assert(sgroup >= 0);
            sgroup_ids.insert(sgroup);
        }
    }

    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve( 3*num_vertices );

    // Processamos um smoothing group por vez
    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

        // Acumulamos as normais dos vértices de todos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                glm::vec4  vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }

                const glm::vec4  a = vertices[0];
                const glm::vec4  b = vertices[1];
                const glm::vec4  c = vertices[2];

                const glm::vec4  n = crossproduct(b-a,c-a);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }

        // Computamos a média das normais acumuladas
        std::vector<size_t> normal_indices(num_vertices, 0);

        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;

            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n /= norm(n);

            model->attrib.normals.push_back( n.x );
            model->attrib.normals.push_back( n.y );
            model->attrib.normals.push_back( n.z );

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }

        // Escrevemos os índices das normais para os vértices dos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[ idx.vertex_index ];
                }
            }
        }

    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        // Em 2026-05-18, corrigido bug encontrado pelo aluno Arthur Prediger:
        // std::numeric_limits<float>::min() retorna o menor valor positivo
        // normalizado representável, não o menor valor possível (negativo). Para
        // inicializar o limite máximo da bounding box com um valor "muito
        // pequeno", deve ser usado std::numeric_limits<float>::lowest()
        const float minval = std::numeric_limits<float>::lowest();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
                
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // No modo FPS o cursor está capturado: qualquer movimento vira a câmera
    if (g_CameraMode == CAMERA_FPS)
    {
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        const float sensitivity = 0.002f;
        g_CameraTheta -= sensitivity * dx;
        g_CameraPhi   -= sensitivity * dy; // dy positivo = mouse para baixo = olhar para baixo

        float phimax = 3.141592f / 2.0f * 0.99f;
        if (g_CameraPhi >  phimax) g_CameraPhi =  phimax;
        if (g_CameraPhi < -phimax) g_CameraPhi = -phimax;

        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
        return;
    }

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;
    
        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;
    
        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;
    
        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f*dx;
        g_ForearmAngleX += 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f*dx;
        g_TorsoPositionY -= 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

void Correcao_KeyCallback(int key, int action, int mod);

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // =======================
    // Não modifique esta chamada! Ela é utilizada para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    Correcao_KeyCallback(key, action, mod);
    // =======================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    // Tecla C alterna entre câmera FPS e câmera de segurança
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        if (g_CameraMode == CAMERA_FPS)
        {
            g_IsHoldingBox = false;
            g_CameraMode = CAMERA_SECURITY;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            g_CameraMode = CAMERA_FPS;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        }
    }

    // Tecla 1-2 alterna entre movimento da câmera de segurança FollowPlayer-Bézier
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        mov_sec_camera = 1;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        mov_sec_camera = 2;
    }

    // Alterna a lanterna
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        g_FlashlightEnabled = !g_FlashlightEnabled;
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS && g_CameraMode == CAMERA_SECURITY)
    {
        g_ActiveSecurityCamera = (g_ActiveSecurityCamera + 1) % g_TotalSecurityCameras;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS && g_CameraMode == CAMERA_SECURITY)
    {
        // Soma g_TotalSecurityCameras antes do módulo para evitar números negativos em C++
        g_ActiveSecurityCamera = (g_ActiveSecurityCamera - 1 + g_TotalSecurityCameras) % g_TotalSecurityCameras;
    }
    

}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

