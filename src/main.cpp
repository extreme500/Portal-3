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
#include "portal.h"
#include "collisions.h"
#include "bezier.h"


// Headers da SoLoud (sfx e músicas)
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "soloud_queue.h"

// object_id e constantes geométricas (movidos para escopo de arquivo para
// que DrawScene(), definida acima de main(), também os enxergue).
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

        // Constantes
        #ifndef M_PI
        #define M_PI   3.14159265358979323846
        #endif

        #ifndef M_PI_2
        #define M_PI_2 1.57079632679489661923
        #endif


// Variável global do motor de áudio
SoLoud::Soloud g_Soloud;

// Canais de Mixagem (Buses)
SoLoud::Bus g_BusMusic;
SoLoud::Bus g_BusSFX;

// Arquivos de voz da GLaDOS
SoLoud::Wav g_Glados001;
SoLoud::Wav g_Glados002;

// A Fila que vai organizar a ordem
SoLoud::Queue g_GladosDialogueQueue;

// Sons da Caixa
SoLoud::Wav g_SfxBeepIn;
SoLoud::Wav g_SfxBeepOut;

// Efeitos Sonoros 3D do Cenário
SoLoud::Wav g_SfxButton;
SoLoud::Wav g_SfxButtonUp;
SoLoud::Wav g_SfxDoor;
SoLoud::Wav g_SfxDoorClose;

// Controle do Rádio estilo Portal
glm::vec3 g_RadioPosition = glm::vec3(-0.5f, -1.0f, 0.5f); // Posição inicial do modelo
SoLoud::WavStream g_RadioMusic;                            // Stream para músicas longas
int g_RadioMusicHandle = 0;                                // ID da instância tocando
float g_RadioVelocityY = 0.0f; // Controla a gravidade do rádio
glm::vec3 g_RadioVel = glm::vec3(0.0f); // momento horizontal (xz) carregado por portais
float g_RadioAngleY = (float)M_PI*1.4/2.0;    // Controla a rotação para encarar o jogador
bool g_IsHoldingRadio = false; // Estado de carregar o rádio

// Rastreadores de Estado (Edge Detection)
bool g_EstavaBotaoPressionado = false;
bool g_EstavaPortaAberta = false;

// Variável para lembrar se estávamos segurando a caixa/rádio no frame passado
bool g_EstavaSegurandoCaixa = false;
bool g_EstavaSegurandoRadio = false;

// Handles (IDs) das instâncias vivas dos canais
int g_BusMusicHandle = 0;
int g_BusSFXHandle = 0;
int g_BGMHandle = 0;

// Variáveis Globais de Volume (Para a futura UI)
float g_VolumeGlobal = 1.0f; // Volume Master (0.0 a 1.0)
float g_VolumeMusic  = 0.1f; // Volume das Músicas
float g_VolumeSFX    = 0.5f; // Volume dos Efeitos Sonoros

// Controle de Mute
bool g_IsMusicMuted = false;

// Array com os 4 sons de passos
SoLoud::Wav g_SfxWalk[4];

// Controles do Menu de Pause e Estado do Jogo
bool g_GameStarted = false; // Começa como falso (Menu Principal)
bool g_IsPaused = true;     // O jogo já começa pausado!

enum MenuState { MENU_MAIN, MENU_SETTINGS, MENU_CREDITS };
MenuState g_MenuState = MENU_MAIN;

// Variáveis para o Hover do Menu
float g_MenuMouseX = 0.0f;
float g_MenuMouseY = 0.0f;

// Música dos Créditos e Voice Lines
SoLoud::WavStream g_MusicWantYouGone;
int g_CreditsMusicHandle = 0;
int g_GladosHandle = 0; // Para tocarmos a voz só quando o jogo começar!

// Estado de Vitória e Música Final
bool g_GameWon = false;
SoLoud::WavStream g_MusicStillAlive;
int g_WinMusicHandle = 0;

// Cronômetro para controlar o intervalo entre os passos
float g_FootstepTimer = 100.0f; // Começa alto para o 1º passo tocar instantaneamente

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
void SetObjectId(GLint object_id); // Define o object_id no shader e vincula as texturas do objeto
void BindTexturesForObject(GLint object_id); // Vincula às unidades 0/1 as texturas usadas pelo object_id
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

// Detecção e resposta a colisões: os tipos e as queries estão no módulo
// collisions (collisions.h). As funções abaixo permanecem no main por
// dependerem do estado global do jogo (Pos_Player, g_CollisionWorld, etc.).
SceneState CurrentSceneState(); // snapshot do estado dinâmico do frame (def. abaixo)
void TryMovePlayer(float dx, float dz);
void TryMovePlayerVertical(float dy);
void SetupCollisionAABBs();
void ResetScene();


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
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void TextRendering_ShowPauseMenu(GLFWwindow* window);
void TextRendering_ShowWinScreen(GLFWwindow* window);

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
float g_CameraTheta = (float)M_PI; // Ângulo no plano ZX em relação ao eixo Z
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
GLint g_portal_pass_uniform; // 0 = stencil/cor, 1 = moldura
GLint g_clipplane_uniform; // plano de recorte oblíquo (portais)
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;

// Lanterna
GLint g_flashlight_pos_uniform;
GLint g_flashlight_dir_uniform;
GLint g_flashlight_on_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// IDs das texturas e samplers carregados, indexados na mesma ordem em que são
// chamados em LoadTextureImage() (TextureImage0, TextureImage1, ...). Como o
// macOS limita o fragment shader a 16 samplers, não mantemos todas as texturas
// vinculadas simultaneamente: guardamos os IDs aqui e vinculamos, por objeto,
// apenas as necessárias às unidades 0 (difusa) e 1 (auxiliar). Veja
// BindTexturesForObject().
#define MAX_TEXTURES 64
GLuint g_TextureID[MAX_TEXTURES];
GLuint g_SamplerID[MAX_TEXTURES];

// Variável para a posição global do personagem (e, consequentemente, da câmera)
glm::vec4 Pos_Player = glm::vec4(.0f,.0f,.0f,1.0f);
float velocidade = 3; // Velocidade do personagem para andar

// Controle da Caixa estilo Portal
glm::vec3 g_BoxPosition = glm::vec3(11.0f, .0f, 5.0f);
float g_BoxVelocityY = 0.0f; // Controla a gravidade da caixa
glm::vec3 g_BoxVel = glm::vec3(0.0f); // momento horizontal (xz) carregado por portais
float g_BoxAngleY = 0.0f;    // Controla a rotação para ela "encarar" o jogador
bool g_IsHoldingBox = false;
bool g_EWasPressed = false;

// Controle botão (objeto) pressionado
bool g_IsButtonPressed = false;

// As dimensões físicas do jogador (cilindro de colisão) ficam em
// g_CollisionWorld.player; ver struct PlayerCollider em collisions.h.

// O modelo do jogador é desenhado com um deslocamento vertical de -1.0f em
// relação a Pos_Player (ver DrawVirtualObject de player_model_*: a matriz de
// modelagem usa "Pos_Player.y - 1.0f"), de forma que Pos_Player.y == 0.0f
// corresponde aos pés apoiados no chão das salas (y = -1.0f). O cilindro de
// colisão usa essa mesma referência para os seus pés. As constantes
// PLAYER_FEET_Y_OFFSET e COLLISION_SKIN estão definidas em collisions.h.

// Física vertical: gravidade e pulo
const float GRAVIDADE  = -9.8f; // aceleração da gravidade, em unidades/s²
const float FORCA_PULO =  4.5f; // velocidade vertical inicial ao pular, em unidades/s
float g_PlayerVelocityY = 0.0f; // velocidade vertical atual do jogador
glm::vec3 g_PlayerPortalVel = glm::vec3(0.0f); // momento horizontal (xz) ao sair de um portal
bool  g_SpaceWasPressed = false; // estado da tecla espaço no frame anterior (detecção de borda de subida, evita pulo contínuo ao segurar a tecla)

// Estado estático de colisão do cenário: AABBs (paredes, chão, teto, botão,
// pilar, bolo), paredes diagonais, hitbox da porta 2, dimensões do jogador e
// bboxes locais da caixa/rádio. Preenchido uma única vez em SetupCollisionAABBs().
// Ver struct CollisionWorld em collisions.h.
CollisionWorld g_CollisionWorld;

// Variáveis para controle de tempo
float ultimoFrame = 0.0f;
float deltaTime = 0.0f;

// Variável que controla a lanterna do jogador
bool g_FlashlightEnabled = false;

// Modo de câmera ativa
enum CameraMode { CAMERA_FPS, CAMERA_SECURITY };
CameraMode g_CameraMode = CAMERA_SECURITY;

// Controle de múltiplas câmeras de segurança ---
int g_ActiveSecurityCamera = 0; // 0 = Sala 1, 1 = Sala 2
int g_TotalSecurityCameras = 2; //

// Variável para movimentação do modelo da Câmera de Segurança
int mov_sec_camera = 2;

// Altura dos olhos do jogador na câmera FPS
const float FPS_EYE_HEIGHT = 0.3f;

// Duração de uma passagem completa da câmera de segurança (em segundos)
const float SECURITY_CAM_SWEEP_PERIOD = 14.0f;

// Última posição conhecida do cursor (usada para calcular deltas de movimento)
double g_LastCursorPosX, g_LastCursorPosY;

// Câmera atualmente sendo renderizada (real ou virtual de um portal).
// Usada por billboards como o modelo da câmera de segurança.
glm::vec4 g_RenderCameraViewVector = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

void DrawTransparentObjects()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glm::mat4 model = Matrix_Identity();

    // Desenhamos o plano das paredes de vidro (Frente 1/2)
    model = Matrix_Translate(+.0f,.0f,+1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    SetObjectId(GLASS);
    DrawVirtualObject("the_plane");

    // Desenhamos o plano das paredes de vidro (Frente 2/2)
    model = Matrix_Translate(+.0f,2.0f,+1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    SetObjectId(GLASS);
    DrawVirtualObject("the_plane");

    // Desenhamos o plano das paredes de vidro (Trás 1/2)
    model = Matrix_Translate(+.0f,.0f,-1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    SetObjectId(GLASS);
    DrawVirtualObject("the_plane");

    // Desenhamos o plano das paredes de vidro (Trás 2/2)
    model = Matrix_Translate(+.0f,2.0f,-1.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    SetObjectId(GLASS);
    DrawVirtualObject("the_plane");

    // Desenhamos o plano das paredes de vidro (Direita 1/2)
    model = Matrix_Translate(-1.0f,.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    SetObjectId(GLASS);
    DrawVirtualObject("the_plane");

    // Desenhamos o plano das paredes de vidro (Direita 2/2)
    model = Matrix_Translate(-1.0f,2.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    SetObjectId(GLASS);
    DrawVirtualObject("the_plane");

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

// Desenha TODA a geometria do mundo (jogador, salas, objetos). Foi extraída
// do loop principal para poder ser re-renderizada pela câmera virtual dos
// portais (efeito see-through). Apenas emite draw calls: não altera estado de
// jogo/física. As matrizes view/projection já devem estar nos uniforms.
void DrawScene()
{
    glm::mat4 model = Matrix_Identity();

        //Renderização do Personagem do Jogador
        model = Matrix_Translate(Pos_Player.x, Pos_Player.y-1.0f, Pos_Player.z) * Matrix_Rotate_Y(g_CameraTheta + M_PI) * Matrix_Scale(1/55.00,1/55.00,1/55.00);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(PLAYER_HEAD);
        DrawVirtualObject("player_model_head");
        SetObjectId(PLAYER_EYE);
        DrawVirtualObject("player_model_left_eye");
        DrawVirtualObject("player_model_right_eye");
        SetObjectId(PLAYER_TORSO);
        DrawVirtualObject("player_model_torso");
        SetObjectId(PLAYER_LEGS);
        DrawVirtualObject("player_model_legs");
        SetObjectId(PLAYER_HAIR);
        DrawVirtualObject("player_model_hair");

        
        // Cena Da 1R

        // Cubículo de vidro
        glDisable(GL_CULL_FACE);
        // Desenhamos o plano da parede (Esquerda 1/2) (repetida fatorRepeticao vezes) 
        model = Matrix_Translate(+1.0f,.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 2/2) (repetida fatorRepeticao vezes) 
        model = Matrix_Translate(+1.0f, 2.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(1,1,1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL);
        DrawVirtualObject("the_plane");
        glEnable(GL_CULL_FACE);

        
        // Desenhamos o plano do chão (1R)
        float fatorRepeticao = 4.0f;
        model = Matrix_Translate(0.0f,-1.0f,0.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(FLOOR);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano do teto (1R)
        model = Matrix_Translate(0.0f,3.0f,0.0f) * Matrix_Rotate_X(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(CEILING);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 1R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(.0f, -1.0f + fatorRepeticao,-4.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 1R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(.0f, -1.0f + fatorRepeticao, +4.0f)*Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 1R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(-4.0f, -1.0f + fatorRepeticao, .0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");
        
        // Desenhamos o plano da parede (Esquerda 1R 1/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.0f, -1.0f + fatorRepeticao, +3.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 1R 2/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.0f, -1.0f + fatorRepeticao, -3.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 1R 3/3) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.0f, 2.0f,0.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.0f, -1.0f, .0f) * Matrix_Rotate_Y(3 * M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(DOOR_WALL); 
        DrawVirtualObject("door_wall");

        // Desenhamos o modelo da porta (Aberta 1R)
        model = Matrix_Translate(+4.1f, -1.0f, .0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Scale(1.5f,1.5f,1.5f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(DOOR);
        DrawVirtualObject("portal_door_combined_model_2");

        // Desenhamos o rádio 
        model = Matrix_Translate(g_RadioPosition.x, g_RadioPosition.y, g_RadioPosition.z) * Matrix_Rotate_Y(g_RadioAngleY) * Matrix_Scale(1.0/9.0f, 1.0f/9.0, 1.0f/9.0); // Coordenada do radio
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        SetObjectId(RADIO_SHELL);
        DrawVirtualObject("Shell"); // Nome do objeto no .obj
        SetObjectId(RADIO_MAIN);
        DrawVirtualObject("Radio"); // Nome após separar no Blender
        SetObjectId(RADIO_GRID);
        DrawVirtualObject("Radio.001"); // Nome após separar no Blender
        SetObjectId(RADIO_SCREEN);
        DrawVirtualObject("Radio.002"); // Nome após separar no Blender
        SetObjectId(RADIO_LIGHT);
        DrawVirtualObject("Light_Indicator");
        DrawVirtualObject("Light.001"); // Desenha ambas as luzes com a mesma textura
        SetObjectId(RADIO_ANTENNA);
        DrawVirtualObject("Antenna");
        SetObjectId(RADIO_BUTTON);
        DrawVirtualObject("Button"); // Nome após separar no Blender
        SetObjectId(RADIO_BUTTON_RIFLED);
        DrawVirtualObject("Button.001"); // Nome após separar no Blender
        SetObjectId(RADIO_BASE_PART);
        DrawVirtualObject("Base");

        // Desenhamos a câmera que segue o jogador/curva de Bézier (1R)
        // Primeiro, desenhamos o suporte que é estático na parede (para ficar mais visualmente agradável, vamos "espelhar" o modelo, sendo necessário mudar a renderização da geometria)
        glFrontFace(GL_CW);
        model = Matrix_Translate(-3.8f, 2.8f, -3.8f) * Matrix_Scale(-1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        SetObjectId(SEC_CAM);
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
            sec_cam_front = glm::normalize(glm::vec3(g_RenderCameraViewVector));
        }
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rotation_matrix = glm::inverse(glm::lookAt(glm::vec3(0.0f), sec_cam_front, up));
        model = Matrix_Translate(-3.8f, 2.8f, -3.8f) * rotation_matrix * Matrix_Rotate_Y(M_PI) * Matrix_Scale(1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        SetObjectId(SEC_CAM);
        DrawVirtualObject("security_camera_2.001");

        // Cena Da 2R

        // Desenhamos o plano do chão 1/2 (2R)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(+8.0f,-1.0f,.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(FLOOR);
        DrawVirtualObject("the_plane");

         // Desenhamos o plano do chão 2/2 (2R)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(+8.0f,-1.0f,+8.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(FLOOR);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano do teto 1/2 (2R)
        model = Matrix_Translate(+8.0f,3.0f,0.0f) * Matrix_Rotate_X(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(CEILING);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano do teto 2/2 (2R)
        model = Matrix_Translate(+8.0f,3.0f,8.0f) * Matrix_Rotate_X(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(CEILING);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 1/4) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.2f, -1.0f + fatorRepeticao, +3.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 2/4) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+4.2f, -1.0f + fatorRepeticao, -3.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 3/4) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.2f, 2.0f,0.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 2R 4/4) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(+4.2f, -1.0f + fatorRepeticao, 9.0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 2R-1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(+4.2f, -1.0f, .0f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(DOOR_WALL); 
        DrawVirtualObject("door_wall");
        
        // Desenhamos o plano da parede (Diagonal 2R) (repetida fatorRepeticao vezes) 
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+11.0f, -1.0f+ fatorRepeticao,-2.7f) * Matrix_Rotate_Y(-M_PI_2/2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 2R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(8.0f, -1.0f + fatorRepeticao,-4.0f)*Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 2R 1/2) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(12.0f, -1.0f + fatorRepeticao, 0.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Esquerda 2R 2/2) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(12.0f, -1.0f + fatorRepeticao, +8.0f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 2R 1/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+11.0f, -1.0f + fatorRepeticao, +6.0f) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 2R 2/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+5.0f, -1.0f + fatorRepeticao, 6.0f) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Frente 2R 3/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(8.0f, 2.0f,+6.0f)*Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(8.0f, -1.0f,+6.0f) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(DOOR_WALL); 
        DrawVirtualObject("door_wall");

        // Desenhamos o modelo do cubo
        model = Matrix_Translate(g_BoxPosition.x, g_BoxPosition.y, g_BoxPosition.z) * Matrix_Rotate_Y(g_BoxAngleY) * Matrix_Scale(1.0f/8.0f, 1.0f/8.0f, 1.0f/8.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(CUBE_003);
        DrawVirtualObject("Cube.003");
        DrawVirtualObject("Cube.002");
        SetObjectId(CUBE_CIRCLE1);
        DrawVirtualObject("Circle");
        SetObjectId(CUBE_CIRCLE2);
        DrawVirtualObject("Circle.002");
        SetObjectId(CUBE_CIRCLE3);
        DrawVirtualObject("Circle.003");
        SetObjectId(CUBE);
        DrawVirtualObject("Cube");

        // Verifica se tem algum objeto em cima do botão
        g_IsButtonPressed = IsButtonTriggered(CurrentSceneState(), glm::vec3(Pos_Player));

        // Desenhamos o modelo da porta (Fechada ou Aberta 2R-3R)
        model = Matrix_Translate(8.0f, -1.0f,+6.1f) * Matrix_Rotate_Y(M_PI) * Matrix_Scale(1.5f,1.5f,1.5f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(DOOR);
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
        SetObjectId(BUTTON);
        DrawVirtualObject("portal_button_reduced_2");
        model = Matrix_Translate(9.0f, -1.0f + y_offset,-1.0f)*Matrix_Scale(2.0,2.0,2.0);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(BUTTON_001);
        DrawVirtualObject("portal_button_reduced_2.001");


        // Desenhamos a câmera que segue o jogador/curva de Bézier (2R)
        // Primeiro, desenhamos o suporte que é estático na parede (para ficar mais visualmente agradável, vamos "espelhar" o modelo, sendo necessário mudar a renderização da geometria)
        glFrontFace(GL_CW);
        model = Matrix_Translate(8.0f, 2.8f, 5.8f) * Matrix_Rotate_Y(M_PI) * Matrix_Scale(-1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        SetObjectId(SEC_CAM);
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
            sec_cam_front = glm::normalize(glm::vec3(g_RenderCameraViewVector));
        }
        up = glm::vec3(0.0f, 1.0f, 0.0f);
        rotation_matrix = glm::inverse(glm::lookAt(glm::vec3(0.0f), sec_cam_front, up));
        model = Matrix_Translate(8.0f, 2.8f, 5.8f) * rotation_matrix * Matrix_Rotate_Y(M_PI) * Matrix_Scale(1.0f / 90.0f, 1.0f / 90.0f, 1.0f / 90.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        SetObjectId(SEC_CAM);
        DrawVirtualObject("security_camera_2.001");


        // Agora, por fim, vamos para a Sala 3

        // Desenhamos o plano da parede (Frente 3R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(8.1f, -1.0f,+10.1f)*Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 3R 1/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+11.0f, -1.0f + fatorRepeticao, +6.2f)* Matrix_Rotate_Y(M_PI) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 3R 2/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 2.0f;
        model = Matrix_Translate(+5.0f, -1.0f + fatorRepeticao, 6.2f)* Matrix_Rotate_Y(M_PI) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_2);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Trás 3R 3/3) (repetida fatorRepeticao vezes)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(8.0f, 2.0f,+6.2f) * Matrix_Rotate_Y(M_PI) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL);
        DrawVirtualObject("the_plane");

        // Desenhamos o modelo da parede que vai na porta (Aberta 1R)
        fatorRepeticao = 1.0f;
        model = Matrix_Translate(8.0f, -1.0f,+6.2f)* Matrix_Rotate_Y(M_PI) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(DOOR_WALL); 
        DrawVirtualObject("door_wall");

        // Desenhamos o plano da parede (Esquerda 3R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(10.0f, -1.0f,+10.1f) * Matrix_Rotate_Y(M_PI_2) *Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o plano da parede (Direita 3R) (repetida fatorRepeticao vezes)
        fatorRepeticao = 4.0f;
        model = Matrix_Translate(6.0f, -1.0f,+10.1f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(fatorRepeticao,fatorRepeticao,fatorRepeticao);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(WALL_4);
        DrawVirtualObject("the_plane");

        // Desenhamos o pilar que segura o bolo (Direita 3R)
        model = Matrix_Translate(7.8f, -1.0f,8.15f) * Matrix_Rotate_Y(M_PI_2) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(0.2,0.2,0.8);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(RADIO_BUTTON_RIFLED);
        DrawVirtualObject("the_plane");

        // Desenhamos o pilar que segura o bolo (Esquerda 3R)
        model = Matrix_Translate(8.2f, -1.0f,8.15f) * Matrix_Rotate_Y(3*M_PI_2) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(0.2,0.2,0.8);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(RADIO_BUTTON_RIFLED);
        DrawVirtualObject("the_plane");

        // Desenhamos o pilar que segura o bolo (Frente 3R)
        model = Matrix_Translate(8.0f, -1.0f,8.35f) * Matrix_Rotate_Y(M_PI) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(0.2,0.2,0.8);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(RADIO_BUTTON_RIFLED);
        DrawVirtualObject("the_plane");

        // Desenhamos o pilar que segura o bolo (Trás 3R)
        model = Matrix_Translate(8.0f, -1.0f,7.95f) * Matrix_Rotate_X(3*M_PI_2) * Matrix_Scale(0.2,0.2,0.8);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(RADIO_BUTTON_RIFLED);
        DrawVirtualObject("the_plane");

        // Desenhamos o pilar que segura o bolo (Cima 3R)
        model = Matrix_Translate(8.0f, -0.2f, 8.15f) * Matrix_Scale(0.2,0.2,0.2);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(RADIO_BASE_PART);
        DrawVirtualObject("the_plane");

        // Desenhamos o bolo
        model = Matrix_Translate(8.0f, -0.1999f, 8.15f) * Matrix_Scale(0.1,0.1,0.1);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        SetObjectId(CAKE);
        DrawVirtualObject("Sphere.004");
        SetObjectId(CAKE_CANDLE);
        DrawVirtualObject("Sphere.002");
        SetObjectId(CAKE_CHERRY);
        DrawVirtualObject("Sphere.001");
        SetObjectId(CAKE_CHERRY_CREAM);
        DrawVirtualObject("Sphere.003");
        SetObjectId(CAKE_BOTTOM);
        DrawVirtualObject("Sphere.005");
}

// --- Portais ---------------------------------------------------------------
// Um par de portais e os estados de travessia das entidades que o atravessam
// (jogador, caixa, rádio). Mantém juntos os dados que sempre andam juntos.
struct PortalSet {
    PortalPair          pair;
    PortalCrossingState player, box, radio;
};
static const int NUM_PORTAL_PAIRS = 1;
PortalSet g_PortalSets[NUM_PORTAL_PAIRS]; // [0] Sala 1 · [1] teto/parede · [2] paredes adjacentes

// Raio de colisão de cada entidade (usado no offset frontal p/ detecção de travessia).
// Deve refletir o raio do cilindro (jogador) ou metade da largura da AABB (caixa/rádio).
static const float PLAYER_BODY_RADIUS = 0.25f;
static const float BOX_BODY_RADIUS    = 0.3f;
static const float RADIO_BODY_RADIUS  = 0.2f;

// Thunks: permitem ao módulo de portais chamar a renderização definida aqui.
static void Portal_DrawScene() { DrawScene(); DrawTransparentObjects();}
static void Portal_DrawPlane() { DrawVirtualObject("the_plane"); }

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

    // Stencil buffer (8 bits) necessário para o efeito see-through dos portais.
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    /* // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Portal 3", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    } */

    // Pega o monitor principal do usuário
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    
    // Pega as especificações de vídeo (Resolução e Taxa de Atualização) desse monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Damos algumas dicas para o GLFW respeitar as cores e os Hertz do monitor (Opcional, mas recomendado)
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // Criamos a janela em Tela Cheia passando a resolução nativa e o ponteiro do monitor
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Portal 3", primaryMonitor, NULL);
    
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
    //FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.
    // Pegamos o tamanho REAL do Framebuffer em pixels (Ignorando a escala do Windows/Mac)
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    
    // Forçamos a chamada inicial com os pixels exatos do seu monitor!
    FramebufferSizeCallback(window, fb_width, fb_height);

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

    // Carregamos imagens para serem utilizadas como textura
    LoadTextureImage("../../data/radio/textures/Radio_Metallic.png", false); // TextureImage0
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
    LoadTextureImage("../../data/cake/cake.png",false);      // TextureImage13
    LoadTextureImage("../../data/button/textures/portal_button_blue.jpeg",false);      // TextureImage14
    LoadTextureImage("../../data/cake/candle.png",false);      // TextureImage15
    LoadTextureImage("../../data/door/textures/portal_door_02_E_upscayl_2x_ultramix-balan.png",false);      // TextureImage16
    LoadTextureImage("../../data/door/textures/portal_door_02_upscayl_8x_ultramix-balance.png",false);      // TextureImage17
    LoadTextureImage("../../data/floor/metallic_floor.jpg",true);      // TextureImage18
    LoadTextureImage("../../data/ceiling/portal_ceiling.png",true);      // TextureImage19
    LoadTextureImage("../../data/sec_camera/textures/security_camera.png",true);      // TextureImage20
    LoadTextureImage("../../data/sec_camera/textures/internal_ground_ao_texture.jpeg",true);      // TextureImage21
    // Texturas do Rádio
    LoadTextureImage("../../data/radio/textures/Shell_Base_Color.png", false);        // TextureImage22
    LoadTextureImage("../../data/radio/textures/Radio_Base_Color.png", false);        // TextureImage23
    LoadTextureImage("../../data/radio/textures/Radio_Grid_Base_Color.png", false);   // TextureImage24
    LoadTextureImage("../../data/radio/textures/Radio_Screen_Base_Color.png", false); // TextureImage25
    LoadTextureImage("../../data/radio/textures/Light_Base_Color.png", false);        // TextureImage26
    LoadTextureImage("../../data/radio/textures/Antenna_Base_Color.png", false);      // TextureImage27
    LoadTextureImage("../../data/radio/textures/Button_Base_Color.png", false);       // TextureImage28
    LoadTextureImage("../../data/radio/textures/Button_Rifled_Base_Color.png", false);// TextureImage29
    LoadTextureImage("../../data/radio/textures/Base_Base_Color.png", false);         // TextureImage30
    LoadTextureImage("../../data/radio/textures/Radio_Screen_Emissive.png", false); // TextureImage31



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

    ObjModel radio("../../data/radio/source/radio.obj");
    ComputeNormals(&radio);
    BuildTrianglesAndAddToVirtualScene(&radio);

    ObjModel cake("../../data/cake/cake.obj");
    ComputeNormals(&cake);
    BuildTrianglesAndAddToVirtualScene(&cake);


    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Construímos as AABBs de colisão do cenário (paredes, chão, teto, cubo,
    // botão), uma única vez, já que a geometria estática não muda em runtime.
    SetupCollisionAABBs();

    // --- Instanciação dos portais (API: ponto + normal da parede via onWall) ---
    // onWall deriva a base ortonormal só da normal, então qualquer parede/teto
    // funciona igual. Cada par é vinculado (azul + laranja).
    const float kPortalW = 1.0f, kPortalH = 2.0f;

    // Sala 1: portal dentro do cubículo e fora.
    g_PortalSets[0].pair = PortalPair::create(
        Portal::onWall(glm::vec3(+0.98f,0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f)), // azul    — parede frontal
        Portal::onWall(glm::vec3(-0.5f, 0.0f, 3.98f), glm::vec3(0.0f, 0.0f, -1.0f)), // laranja — parede traseira
        kPortalW, kPortalH);


    // Raio de colisão por entidade (offset frontal na detecção de travessia).
    for (int i = 0; i < NUM_PORTAL_PAIRS; ++i)
    {
        g_PortalSets[i].player.bodyRadius = PLAYER_BODY_RADIUS;
        g_PortalSets[i].box.bodyRadius    = BOX_BODY_RADIUS;
        g_PortalSets[i].radio.bodyRadius  = RADIO_BODY_RADIUS;
    }

    PortalGLContext pctx;
    pctx.modelUniform      = g_model_uniform;
    pctx.viewUniform       = g_view_uniform;
    pctx.projectionUniform = g_projection_uniform;
    pctx.clipPlaneUniform  = g_clipplane_uniform;
    pctx.objectIdUniform   = g_object_id_uniform;
    pctx.portalPassUniform = g_portal_pass_uniform;
    pctx.drawScene         = Portal_DrawScene;
    pctx.drawPlane         = Portal_DrawPlane;
    Portal_SetGLContext(pctx);

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Câmera começa com cursor visível (menu)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Inicializa a SoLoud
    g_Soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF | SoLoud::Soloud::ENABLE_VISUALIZATION);

    // Liga os canais no motor principal E GUARDA O ID DELES!
    g_BusMusicHandle = g_Soloud.play(g_BusMusic);
    g_BusSFXHandle   = g_Soloud.play(g_BusSFX);
    g_Soloud.setPause(g_BusSFXHandle, 1);

    // Aplica os volumes iniciais NA INSTÂNCIA VIVA usando o ID
    g_Soloud.setGlobalVolume(g_VolumeGlobal);
    g_Soloud.setVolume(g_BusMusicHandle, g_VolumeMusic);
    g_Soloud.setVolume(g_BusSFXHandle, g_VolumeSFX);

    // Carrega os 4 sons de passo (ajuste o caminho se necessário)
    g_SfxWalk[0].load("../../data/audio/sfx/Walk1.wav");
    g_SfxWalk[1].load("../../data/audio/sfx/Walk2.wav");
    g_SfxWalk[2].load("../../data/audio/sfx/Walk3.wav");
    g_SfxWalk[3].load("../../data/audio/sfx/Walk4.wav");

    // Carrega os beeps da caixa
    g_SfxBeepIn.load("../../data/audio/sfx/Beep-in.mp3");
    g_SfxBeepOut.load("../../data/audio/sfx/Beep-out.mp3");

    // Carrega os sons de pressionar o botão e abrir a porta
    g_SfxButton.load("../../data/audio/sfx/Ground_Button.mp3");
    g_SfxButtonUp.load("../../data/audio/sfx/Ground_ButtonUp.mp3");
    g_SfxDoor.load("../../data/audio/sfx/Door_Open.mp3");
    g_SfxDoorClose.load("../../data/audio/sfx/Door_Close.mp3");

    // Carrega as músicas 
    SoLoud::WavStream musicaDeFundo;
    musicaDeFundo.load("../../data/audio/music/Ambiente.mp3");
    musicaDeFundo.setLooping(1); // Faz repetir para sempre

    g_MusicStillAlive.load("../../data/audio/music/Still_Alive.mp3");

    g_MusicWantYouGone.load("../../data/audio/music/Want_You_Gone.mp3");

    // Carrega a música do rádio (ajuste o nome do arquivo se necessário)
    g_RadioMusic.load("../../data/audio/music/Radio.mp3"); 
    g_RadioMusic.setLooping(true); // Faz a música do rádio rodar em loop infinito

    // Configura a atenuação linear para o som sumir se o jogador se afastar
    g_RadioMusic.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 0.75f);
    g_RadioMusic.set3dMinMaxDistance(1.0f, 4.0f); // O som zera completamente a 10 unidades de distância

    // Inicia o áudio 3D dentro do canal de músicas (g_BusMusic) na posição inicial do rádio
    g_RadioMusicHandle = g_BusSFX.play3d(g_RadioMusic, g_RadioPosition.x, g_RadioPosition.y, g_RadioPosition.z);
    
    // Define o volume inicial dele bem baixinho para ficar de fundo ambiente
    g_Soloud.setVolume(g_RadioMusicHandle, 0.2f);

    // Carrega os arquivos
    g_Glados001.load("../../data/audio/sfx/Glados_001.mp3");
    g_Glados002.load("../../data/audio/sfx/Glados_002.mp3");



    // Toca a música de fundo
    g_BGMHandle = g_BusMusic.play(musicaDeFundo);

    // CONFIGURAÇÃO DE ATENUAÇÃO 3D (VOLUME A DISTÂNCIA)
    // O parâmetro LINEAR_DISTANCE força o volume a cair até 0% no MaxDistance.
    // O segundo parâmetro (1.0f) é o fator de rolloff (caída padrão).
    
    g_SfxButton.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 0.75f);
    g_SfxButton.set3dMinMaxDistance(1.0f, 10.0f);
    g_SfxButtonUp.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 0.75f);
    g_SfxButtonUp.set3dMinMaxDistance(1.0f, 10.0f);
    g_SfxDoor.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 0.75f);
    g_SfxDoor.set3dMinMaxDistance(1.0f, 10.0f);
    g_SfxDoorClose.set3dAttenuation(SoLoud::AudioSource::LINEAR_DISTANCE, 0.75f);
    g_SfxDoorClose.set3dMinMaxDistance(1.0f, 10.0f);

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

        // Calcula a velocidade do frame: se segurar SHIFT (esquerdo ou direito), corre mais rápido!
        float velocidade_atual = velocidade;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        {
            velocidade_atual = velocidade * 1.6f; // 60% mais rápido ao correr
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && g_GameStarted)
        {
            desired_dx += fwd_x * velocidade_atual * deltaTime;
            desired_dz += fwd_z * velocidade_atual * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && g_GameStarted)
        {
            desired_dx -= fwd_x * velocidade_atual * deltaTime;
            desired_dz -= fwd_z * velocidade_atual * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && g_GameStarted)
        {
            desired_dx -= right_x * velocidade_atual * deltaTime;
            desired_dz -= right_z * velocidade_atual * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && g_GameStarted)
        {
            desired_dx += right_x * velocidade_atual * deltaTime;
            desired_dz += right_z * velocidade_atual * deltaTime;
        }

        // Momento horizontal carregado ao sair de um portal (decai com o tempo).
        desired_dx += g_PlayerPortalVel.x * deltaTime;
        desired_dz += g_PlayerPortalVel.z * deltaTime;
        g_PlayerPortalVel *= std::exp(-deltaTime * 3.0f);

        TryMovePlayer(desired_dx, desired_dz);

        // Pulo: detectamos a borda de subida da tecla espaço (o instante em que
        // ela é pressionada, não enquanto fica segurada) e só permitimos pular
        // quando o jogador está apoiado no chão (sem pulo duplo no ar)
        bool spacePressedNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spacePressedNow && !g_SpaceWasPressed && IsPlayerOnGround(g_CollisionWorld, CurrentSceneState(), glm::vec3(Pos_Player)) && g_GameStarted)
        {
            // Aplica a força física para cima
            g_PlayerVelocityY = FORCA_PULO;

            if(g_CameraMode == CAMERA_FPS)
            {
                // Sorteia qual dos 4 passos vai tocar
                int randomStep = rand() % 4;
                
                // Toca o som no canal de Efeitos Sonoros e "captura" a ID
                int jumpHandle = g_BusSFX.play(g_SfxWalk[randomStep]);
                
                // Aumenta o volume dessa instância em 50% (1.5f) ou até dobrar (2.0f)
                // Como usamos o Handle, isso afeta SÓ ESSE PULO, não os passos normais!
                g_Soloud.setVolume(jumpHandle, 0.7f); 
                
                // [BÔNUS DE GAME FEEL] 
                // Diminuir a velocidade do áudio em 15% deixa o som mais "grave/pesado",
                // vendendo a ilusão de que o jogador fez muito mais força nas pernas.
                g_Soloud.setRelativePlaySpeed(jumpHandle, 0.85f);
            }
        }
        g_SpaceWasPressed = spacePressedNow;

        // Gravidade: aceleramos a velocidade vertical e tentamos aplicar o
        // deslocamento resultante; colisões com chão/teto zeram a velocidade
        g_PlayerVelocityY += GRAVIDADE * deltaTime;
        TryMovePlayerVertical(g_PlayerVelocityY * deltaTime);

        // --- Portais: travessia do jogador (SNAP) ---
        // Monta a velocidade 3D atual (horizontal pretendida + vertical), deixa o
        // portal transformá-la e devolve posição, olhar e velocidade do outro lado.
        if (deltaTime > 1e-5f)
        {
            glm::vec3 ppos = glm::vec3(Pos_Player);
            glm::vec3 pvel(desired_dx / deltaTime, g_PlayerVelocityY, desired_dz / deltaTime);
            if (g_PortalSets[0].pair.teleportPlayer(g_PortalSets[0].player, ppos, pvel,
                                         g_CameraTheta, g_CameraPhi))
            {
                Pos_Player = glm::vec4(ppos, 1.0f);
                g_PlayerVelocityY = pvel.y;
                g_PlayerPortalVel = glm::vec3(pvel.x, 0.0f, pvel.z);
            }
        }


        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
            bool isMoving = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            if (isMoving)
            {
                float t = (float)glfwGetTime();
                
                // Se estiver correndo, os passos são mais rápidos
                float freq_multiplier = 1.0f;
                if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
                    freq_multiplier = 1.4f;
                }

                // Y sobe e desce (Reduzimos a amplitude de 0.05 para 0.02 para suavizar)
                camera_position_c.y += 0.02f * sin(t * 10.0f * freq_multiplier);
                
                // Z avança e recua (Reduzimos a amplitude de 0.025 para 0.01)
                float bob_z = 0.01f * sin(t * 20.0f * freq_multiplier);
                
                camera_position_c.x += bob_z * (-sin(g_CameraTheta));
                camera_position_c.z += bob_z * (-cos(g_CameraTheta));
            }

            // Direção: vetor em coordenadas esféricas controlado pelo mouse
            float vx = -cos(g_CameraPhi) * sin(g_CameraTheta);
            float vy =  sin(g_CameraPhi);
            float vz = -cos(g_CameraPhi) * cos(g_CameraTheta);
            camera_view_vector = glm::vec4(vx, vy, vz, 0.0f);

            // Sistema de Som de Passos (Footsteps)
            if (isMoving && IsPlayerOnGround(g_CollisionWorld, CurrentSceneState(), glm::vec3(Pos_Player))) 
            {
                // Acumula o tempo que passou
                g_FootstepTimer += deltaTime;
                
                // Define o intervalo dependendo se está correndo ou andando
                // (Um passo a cada 0.45s andando, ou 0.30s correndo)
                bool isSprinting = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
                                    glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
                
                float stepInterval = isSprinting ? 0.30f : 0.45f;

                // Se o tempo acumulado passou do intervalo, toca o som!
                if (g_FootstepTimer >= stepInterval) 
                {
                    // Sorteia um número de 0 a 3
                    int randomStep = rand() % 4;
                    
                    // Toca o passo sorteado no canal de Efeitos Sonoros
                    int stepHandle = g_BusSFX.play(g_SfxWalk[randomStep]);
                    g_Soloud.setVolume(stepHandle, 0.4f);
                    
                    // Reseta o cronômetro para começar a contar o próximo passo
                    g_FootstepTimer = 0.0f; 
                }
            } 
            else if (!isMoving) 
            {
                // Se o jogador parar completamente, deixamos o timer engatilhado.
                // Assim, quando ele voltar a andar, o primeiro passo soa na mesma hora!
                g_FootstepTimer = 100.0f; 
            }
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


        // Atualiza a posição do ouvinte (Jogador) para a máquina de som 3D
        g_Soloud.set3dListenerParameters(
            camera_position_c.x, camera_position_c.y, camera_position_c.z, // Onde eu estou
            camera_view_vector.x, camera_view_vector.y, camera_view_vector.z, // Para onde olho
            camera_up_vector.x, camera_up_vector.y, camera_up_vector.z // Qual lado é "cima"
        );

        // Manda a SoLoud recalcular o volume das orelhas baseando-se na nova posição!
        g_Soloud.update3dAudio();


        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        glUniform4fv(g_flashlight_pos_uniform, 1, glm::value_ptr(camera_position_c));
        glUniform4fv(g_flashlight_dir_uniform, 1, glm::value_ptr(camera_view_vector));
        glUniform1i(g_flashlight_on_uniform, (int)g_FlashlightEnabled);

        
        // Lógica de Interação com a Caixa (Pegar/Soltar - Física, Gravidade e Raycast)
        // ----------------------------------------------------------------
        // Lógica de Interação com Objetos (Caixa e Rádio - Tecla E)
        // ----------------------------------------------------------------
        bool ePressedNow = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        
        if (ePressedNow && !g_EWasPressed && g_CameraMode == CAMERA_FPS)
        {
            // Se já estiver segurando a caixa, solta ela
            if (g_IsHoldingBox) {
                g_IsHoldingBox = false;
            } 
            // Se já estiver segurando o rádio, solta ele
            else if (g_IsHoldingRadio) {
                g_IsHoldingRadio = false;
            } 
            // Se a mão estiver livre, decide qual pegar com base na proximidade e mira (Garante exclusão mútua)
            else {
                // Posição aproximada do bolo que configuramos no cenário (X: 8.0, Y: -0.2, Z: 8.15)
                glm::vec3 cakePos = glm::vec3(8.0f, -0.2f, 8.15f);
                float distToCake = glm::distance(glm::vec3(Pos_Player), cakePos);
                glm::vec3 dirToCake = glm::normalize(cakePos - glm::vec3(Pos_Player));
                float dotCake = glm::dot(dirToCake, glm::vec3(camera_view_vector));

                // CHECA A VITÓRIA (O BOLO) PRIMEIRO
                if (distToCake < 2.5f && dotCake > 0.85f && !g_GameWon) {
                    g_GameWon = true; // Venceu o jogo!
                    
                    // Pausa todo o som do jogo e toca a música tema final!
                    g_Soloud.setPause(g_BGMHandle, 1);
                    g_Soloud.setPause(g_BusSFXHandle, 1);
                    g_Soloud.stop(g_GladosHandle); // Caso a GLaDOS ainda esteja falando
                    
                    g_WinMusicHandle = g_BusMusic.play(g_MusicStillAlive);
                    g_Soloud.setVolume(g_WinMusicHandle, 1.0f);
                }
                // CHECA OS OBJETOS FÍSICOS SE NÃO GANHOU
                else {
                    float distToBox = glm::distance(glm::vec3(Pos_Player), g_BoxPosition);
                    float distToRadio = glm::distance(glm::vec3(Pos_Player), g_RadioPosition);
                    
                    glm::vec3 dirToBox = glm::normalize(g_BoxPosition - glm::vec3(Pos_Player));
                    glm::vec3 dirToRadio = glm::normalize(g_RadioPosition - glm::vec3(Pos_Player));
                    
                    float dotBox = glm::dot(dirToBox, glm::vec3(camera_view_vector));
                    float dotRadio = glm::dot(dirToRadio, glm::vec3(camera_view_vector));
                    
                    if (distToBox < 2.0f && dotBox > 0.85f && dotBox > dotRadio) {
                        g_IsHoldingBox = true;
                        g_BoxVelocityY = 0.0f;
                    } 
                    else if (distToRadio < 2.0f && dotRadio > 0.85f && dotRadio > dotBox) {
                        g_IsHoldingRadio = true;
                        g_RadioVelocityY = 0.0f;
                    }
                }
            }
        }
        g_EWasPressed = ePressedNow;

        // ================================================================
        // PROCESSAMENTO DE MOVIMENTO E GRAVIDADE DA CAIXA
        // ================================================================
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
                if (BoxCollidesAt(g_CollisionWorld, CurrentSceneState(), testPos)) {
                    break; // Bateu na parede, usamos a última posição válida (finalPos)
                }
                finalPos = testPos;
            }
            g_BoxPosition = finalPos;

            g_PortalSets[0].box.initialized = false;

            // Magia do Face-Tracking: Copia a rotação da câmera (Eixo Y) para o cubo
            g_BoxAngleY = g_CameraTheta;

            // Depois de bastante teste, percebi que, por mais que fizesse sentido no contexto do jogo, 
            // Esse sistema está mais ativamente atrapalhando do que ajudando, então decidi removê-lo
            /* // SISTEMA ANTI-ESMAGAMENTO (A Lógica do Portal)
            // Se você anda contra a parede, a parede empurra a caixa na direção da câmera.
            // Se a distância entre a origem do raio (jogador) e a caixa ficar menor que 1.0 unidades,
            // o jogador solta a caixa automaticamente para evitar que a câmera entre nela.
            float distToPlayer = glm::distance(start, finalPos);
            if (distToPlayer < 1.0f) {
                g_IsHoldingBox = false;
            } */
        }
        else
        {
            // O jogador soltou a caixa: Gravidade!
            g_BoxVelocityY += GRAVIDADE * deltaTime;
            
            // Simula onde a caixa vai estar no próximo frame
            glm::vec3 candidatePos = g_BoxPosition;
            candidatePos.y += g_BoxVelocityY * deltaTime;

            // SISTEMA ANTI-PRISÃO (Caixa bate no jogador/cenário e não atravessa)
            // Se essa queda não fizer ela bater no chão ou num objeto, ela cai
            if (!BoxCollidesAt(g_CollisionWorld, CurrentSceneState(), candidatePos)) {
                g_BoxPosition.y = candidatePos.y;
            } else {
                g_BoxVelocityY = 0.0f; // Bateu, zera a velocidade de queda
            }

            // Momento horizontal vindo de um portal: integra xz com colisão e atrito.
            if (std::fabs(g_BoxVel.x) + std::fabs(g_BoxVel.z) > 1e-4f)
            {
                glm::vec3 ch = g_BoxPosition;
                ch.x += g_BoxVel.x * deltaTime;
                ch.z += g_BoxVel.z * deltaTime;
                if (!BoxCollidesAt(g_CollisionWorld, CurrentSceneState(), ch)) g_BoxPosition = ch;
                else { g_BoxVel.x = 0.0f; g_BoxVel.z = 0.0f; }
                g_BoxVel *= std::exp(-deltaTime * 1.5f);
            }

            // Travessia de portal (SNAP) para a caixa: transforma posição,
            // velocidade (vertical + horizontal) e rotação.
        
            glm::vec3 bvel(g_BoxVel.x, g_BoxVelocityY, g_BoxVel.z);
            float byaw = g_BoxAngleY;
            if (g_PortalSets[0].pair.teleportIfCrossed(g_PortalSets[0].box, g_BoxPosition, &bvel, &byaw))
            {
                g_BoxVelocityY = bvel.y;
                g_BoxVel = glm::vec3(bvel.x, 0.0f, bvel.z);
                g_BoxAngleY = byaw;
            }
        }

        // ================================================================
        // PROCESSAMENTO DE MOVIMENTO E GRAVIDADE DO RÁDIO
        // ================================================================
        if (g_IsHoldingRadio)
        {
            // O jogador está segurando o rádio: o Raycast
            glm::vec3 start = glm::vec3(camera_position_c);
            start.y -= 0.1f; // Abaixa da linha dos olhos para não cegar o jogador
            glm::vec3 dir = glm::vec3(camera_view_vector);
            
            float max_dist = 1.5f;
            float step = 0.1f;
            glm::vec3 finalPos = start;

            // Empurra o rádio aos poucos na direção da visão. 
            // Se ele bater na parede no caminho, ele para antes de atravessar.
            for (float d = 0.2f; d <= max_dist; d += step) {
                glm::vec3 testPos = start + dir * d;
                if (RadioCollidesAt(g_CollisionWorld, CurrentSceneState(), testPos)) {
                    break; // Bateu na parede, usamos a última posição válida (finalPos)
                }
                finalPos = testPos;
            }
            g_RadioPosition = finalPos;

            // Enquanto segurado, mantém o estado de travessia "zerado" para não
            // disparar um teleporte espúrio ao soltar perto de um portal.
            g_PortalSets[0].radio.initialized = false;

            // Magia do Face-Tracking: Copia a rotação da câmera (Eixo Y) para o rádio
            g_RadioAngleY = g_CameraTheta;

            // Depois de bastante teste, percebi que, por mais que fizesse sentido no contexto do jogo, 
            // Esse sistema está mais ativamente atrapalhando do que ajudando, então decidi removê-lo
            // SISTEMA ANTI-ESMAGAMENTO (A Lógica do Portal)
            // Se você anda contra a parede, a parede empurra o rádio na direção da câmera.
            // Se a distância entre a origem do raio (jogador) e o rádio ficar menor que 1.0 unidades,
            // o jogador solta o rádio automaticamente para evitar que a câmera entre nele.
            /* float distToPlayer = glm::distance(start, finalPos);
            if (distToPlayer < 1.0f) {
                g_IsHoldingRadio = false;
            } */
        }
        else
        {
            // O jogador soltou o rádio: Gravidade!
            g_RadioVelocityY += GRAVIDADE * deltaTime;
            
            // Simula onde o rádio vai estar no próximo frame
            glm::vec3 candidatePos = g_RadioPosition;
            candidatePos.y += g_RadioVelocityY * deltaTime;

            // SISTEMA ANTI-PRISÃO (Rádio bate no jogador/cenário e não atravessa)
            // Se essa queda não fizer ele bater no chão ou num objeto, ele cai
            if (!RadioCollidesAt(g_CollisionWorld, CurrentSceneState(), candidatePos)) {
                g_RadioPosition.y = candidatePos.y;
            } else {
                g_RadioVelocityY = 0.0f; // Bateu, zera a velocidade de queda
            }

            // Momento horizontal vindo de portais (rádio)
            if (std::fabs(g_RadioVel.x) + std::fabs(g_RadioVel.z) > 1e-4f)
            {
                glm::vec3 ch = g_RadioPosition;
                ch.x += g_RadioVel.x * deltaTime;
                ch.z += g_RadioVel.z * deltaTime;
                if (!RadioCollidesAt(g_CollisionWorld, CurrentSceneState(), ch)) g_RadioPosition = ch;
                else { g_RadioVel.x = 0.0f; g_RadioVel.z = 0.0f; }
                g_RadioVel *= std::exp(-deltaTime * 1.5f);
            }

            // Travessia de portal (SNAP) para o rádio
            glm::vec3 rvel(g_RadioVel.x, g_RadioVelocityY, g_RadioVel.z);
            float ryaw = g_RadioAngleY;
            if (g_PortalSets[0].pair.teleportIfCrossed(g_PortalSets[0].radio, g_RadioPosition, &rvel, &ryaw))
            {
                g_RadioVelocityY = rvel.y;
                g_RadioVel = glm::vec3(rvel.x, 0.0f, rvel.z);
                g_RadioAngleY = ryaw;
            }
        }

        // Feedback Sonoro da Caixa (Edge Detection)
        bool segurandoAgora = g_IsHoldingBox; 

        // DETECÇÃO DE PEGAR (Transição de Falso para Verdadeiro)
        if (segurandoAgora && !g_EstavaSegurandoCaixa) 
        {
            // O jogador ACABOU de pegar a caixa neste exato frame
            // Toca no Bus, pega a ID e abaixa pra 50%
            int beepInHandle = g_BusSFX.play(g_SfxBeepIn);
            g_Soloud.setVolume(beepInHandle, 0.5f);
        }
        // DETECÇÃO DE SOLTAR (Transição de Verdadeiro para Falso)
        else if (!segurandoAgora && g_EstavaSegurandoCaixa) 
        {
            // O jogador (ou a física do jogo) ACABOU de soltar a caixa
            // Toca no Bus, pega a ID e abaixa pra 50%
            int beepOutHandle = g_BusSFX.play(g_SfxBeepOut);
            g_Soloud.setVolume(beepOutHandle, 0.5f);
        }

        // Atualiza a memória para o próximo frame
        g_EstavaSegurandoCaixa = segurandoAgora;

        // Feedback Sonoro do Rádio (Edge Detection)
        bool segurandoRadioAgora = g_IsHoldingRadio; 

        // DETECÇÃO DE PEGAR (Transição de Falso para Verdadeiro)
        if (segurandoRadioAgora && !g_EstavaSegurandoRadio) 
        {
            // O jogador ACABOU de pegar o rádio!
            int beepInHandle = g_BusSFX.play3d(g_SfxBeepIn, g_RadioPosition.x, g_RadioPosition.y, g_RadioPosition.z);
            g_Soloud.setVolume(beepInHandle, 0.5f);
        }
        // DETECÇÃO DE SOLTAR (Transição de Verdadeiro para Falso)
        else if (!segurandoRadioAgora && g_EstavaSegurandoRadio) 
        {
            // O jogador (ou a gravidade) ACABOU de soltar o rádio!
            int beepOutHandle = g_BusSFX.play3d(g_SfxBeepOut, g_RadioPosition.x, g_RadioPosition.y, g_RadioPosition.z);
            g_Soloud.setVolume(beepOutHandle, 0.5f);
        }

        // Atualiza a memória para o próximo frame
        g_EstavaSegurandoRadio = segurandoRadioAgora;

        // Feedback Sonoro do Botão (Áudio 3D)
        // Variável que diz se tem algo em cima do botão neste exato frame
        bool botaoPressionadoAgora = g_IsButtonPressed; 
        
        if (botaoPressionadoAgora && !g_EstavaBotaoPressionado) 
        {
            // O botão acabou de ser afundado! Toca o som na posição XYZ dele.
            g_BusSFX.play3d(g_SfxButton, 9.0f, -1.0f, -1.0f);
        } 
        else if (!botaoPressionadoAgora && g_EstavaBotaoPressionado) 
        {
            // O botão acabou de ser solto e subiu!
            g_BusSFX.play3d(g_SfxButtonUp, 9.0f, -1.0f, -1.0f);
        }
        // Atualiza a memória
        g_EstavaBotaoPressionado = botaoPressionadoAgora;


        // Feedback Sonoro da Porta (Áudio 3D)
        bool portaAbertaAgora = g_IsButtonPressed; 
        
        if (portaAbertaAgora && !g_EstavaPortaAberta) 
        {
            // A porta acabou de ser destrancada! Toca o som na posição XYZ dela.
            g_BusSFX.play3d(g_SfxDoor, 8.0f, -1.0f, 6.1f);
        }
        else if (!portaAbertaAgora && g_EstavaPortaAberta) 
        {
            // A porta acabou de ser destrancada! Toca o som na posição XYZ dela.
            g_BusSFX.play3d(g_SfxDoorClose, 8.0f, -1.0f, 6.1f);
        }
        // Atualiza a memória
        g_EstavaPortaAberta = portaAbertaAgora;

        // Atualiza a posição tridimensional do áudio do rádio com base na variável global
        g_Soloud.set3dSourcePosition(g_RadioMusicHandle, g_RadioPosition.x, g_RadioPosition.y, g_RadioPosition.z);

        // Informações de base sobre a CENA
        // O chão dela acontece em -1.0f
        // O teto dela acontece em +3.0f
        // As paredes da primeira sala vão de -4.0 até +4.0 em x e em z
        // As paredes (naturalmente/1.0f de fatorRepeticao) tem 2.0f de altura
        // 1R corresponde à Sala 1, e 2R corresponde à Sala 2

        // Câmera atual (usada por billboards). Os portais sobrescrevem isto com
        // a câmera virtual ao renderizar suas vistas.
        g_RenderCameraViewVector = camera_view_vector;

        // Desenha o mundo. (renderViews() re-renderiza DrawScene via câmera virtual.)
        DrawScene();

        // Portais: re-renderiza a cena pela câmera virtual recortada na janela de
        // cada portal (efeito see-through) e desenha suas superfícies/molduras.
        // O 3º argumento é a profundidade de recursão ("hall of mirrors"): o
        // efeito só aparece em pares que se encaram. Custo ~linear no nível.
        const int kPortalRecursionDepth = 3;
        for (int i = 0; i < NUM_PORTAL_PAIRS; ++i)
        {
            g_PortalSets[i].pair.renderViews(view, projection, kPortalRecursionDepth);
            g_PortalSets[i].pair.renderSurfaces();
        }

        // AGORA desenhamos os objetos transparentes por cima de tudo
        DrawTransparentObjects();

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // Imprimimos na tela o Menu de Pause
        TextRendering_ShowPauseMenu(window);

        // Imprimimos na tela o Menu de Vitória
        TextRendering_ShowWinScreen(window);

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

    g_Soloud.deinit();

    // Fim do programa
    return 0;
}

// ----------------------------------------------------------------------------
// Funções de detecção e resposta a colisões (jogador vs. cenário)
// ----------------------------------------------------------------------------

// As funções geométricas puras CylinderIntersectsAABB, CylinderIntersectsLine
// e ComputeWorldAABB foram movidas para o módulo collisions (collisions.h/.cpp).

// As queries de colisão (PlayerCollidesAt, BoxCollidesAt, RadioCollidesAt,
// IsButtonTriggered, IsPlayerOnGround) foram movidas para o módulo collisions
// (collisions.h/.cpp); recebem o estado por parâmetro (CollisionWorld/SceneState).

// Monta o snapshot do estado dinâmico do frame (posições/matrizes das entidades
// móveis e flags) a ser passado às queries do módulo collisions. As model
// matrices da caixa e do rádio são computadas aqui (com Matrix_*), já que o
// módulo de colisão é desacoplado de matrices.h.
SceneState CurrentSceneState()
{
    SceneState s;
    s.boxPos        = g_BoxPosition;
    s.radioPos      = g_RadioPosition;
    s.boxModel      = Matrix_Translate(g_BoxPosition.x, g_BoxPosition.y, g_BoxPosition.z)
                    * Matrix_Rotate_Y(g_BoxAngleY)
                    * Matrix_Scale(1.0f/8.0f, 1.0f/8.0f, 1.0f/8.0f);
    s.radioModel    = Matrix_Translate(g_RadioPosition.x, g_RadioPosition.y, g_RadioPosition.z)
                    * Matrix_Rotate_Y(g_RadioAngleY)
                    * Matrix_Scale(1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f);
    s.holdingBox    = g_IsHoldingBox;
    s.holdingRadio  = g_IsHoldingRadio;
    s.buttonPressed = g_IsButtonPressed;
    return s;
}

// Aplica um deslocamento vertical 'dy' ao jogador (gravidade/pulo), verificando
// colisão contra o cenário. Se a posição candidata colide (chão ao cair, teto
// ao subir), a velocidade vertical é zerada e o jogador permanece onde está.
void TryMovePlayerVertical(float dy)
{
    SceneState s = CurrentSceneState();
    glm::vec3 candidate = glm::vec3(Pos_Player.x, Pos_Player.y + dy, Pos_Player.z);
    if (!PlayerCollidesAt(g_CollisionWorld, s, candidate))
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

    const SceneState s = CurrentSceneState();

    // Tenta mover no eixo X
    glm::vec3 candidate_x = glm::vec3(Pos_Player.x + dx, Pos_Player.y, Pos_Player.z);
    if (!PlayerCollidesAt(g_CollisionWorld, s, candidate_x))
    {
        Pos_Player.x = candidate_x.x;
    }
    else if (IsPlayerOnGround(g_CollisionWorld, s, glm::vec3(Pos_Player)))
    {
        // Bateu em X! O jogador está no chão, então testamos a posição mais alta (o degrau)
        glm::vec3 candidate_step_x = glm::vec3(Pos_Player.x + dx, Pos_Player.y + MAX_STEP_HEIGHT, Pos_Player.z);
        if (!PlayerCollidesAt(g_CollisionWorld, s, candidate_step_x))
        {
            Pos_Player.x = candidate_step_x.x;
            Pos_Player.y = candidate_step_x.y; // Levanta o jogador para cima do obstáculo
        }
    }

    // Tenta mover no eixo Z
    glm::vec3 candidate_z = glm::vec3(Pos_Player.x, Pos_Player.y, Pos_Player.z + dz);
    if (!PlayerCollidesAt(g_CollisionWorld, s, candidate_z))
    {
        Pos_Player.z = candidate_z.z;
    }
    else if (IsPlayerOnGround(g_CollisionWorld, s, glm::vec3(Pos_Player)))
    {
        // Bateu em Z! Testamos a subida no degrau também.
        glm::vec3 candidate_step_z = glm::vec3(Pos_Player.x, Pos_Player.y + MAX_STEP_HEIGHT, Pos_Player.z + dz);
        if (!PlayerCollidesAt(g_CollisionWorld, s, candidate_step_z))
        {
            Pos_Player.z = candidate_step_z.z;
            Pos_Player.y = candidate_step_z.y;
        }
    }
}

// (ComputeWorldAABB foi movida para o módulo collisions — collisions.h/.cpp.)

// Preenche g_CollisionWorld.aabbs com as AABBs de mundo do cenário estático contra
// o qual o jogador colide: paredes/chão/teto das duas salas (definidas
// manualmente, já que a geometria é simples e alinhada aos eixos) mais o
// cubo e o botão (cuja AABB local é transformada pela matriz "model" usada ao
// desenhá-los). Chamada uma única vez, no setup, pois o cenário é estático.
void SetupCollisionAABBs()
{
    g_CollisionWorld.aabbs.clear();
    g_CollisionWorld.lines.clear();

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
            g_CollisionWorld.aabbs.push_back({ glm::vec3(x_lo, y_lo, z_lo), glm::vec3(x_hi, y_hi, z_hi) });
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
    g_CollisionWorld.aabbs.push_back({ glm::vec3(1.0f, y_min, -1.0f), glm::vec3(1.0f + wall_t, y_max, 1.0f) });
    // Parede de vidro em x=-1
    g_CollisionWorld.aabbs.push_back({ glm::vec3(-1.0f - wall_t, y_min, -1.0f), glm::vec3(-1.0f + wall_t, y_max, 1.0f) });
    // Parede de vidro em z=+1
    g_CollisionWorld.aabbs.push_back({ glm::vec3(-1.0f, y_min, 1.0f - wall_t), glm::vec3(1.0f, y_max, 1.0f + wall_t) });
    // Parede de vidro em z=-1
    g_CollisionWorld.aabbs.push_back({ glm::vec3(-1.0f, y_min, -1.0f - wall_t), glm::vec3(1.0f, y_max, -1.0f + wall_t) });

    // Botão: mesma matriz "model" usada para desenhá-lo (Matrix_Translate(9,-1,-1) * Matrix_Scale(1.65,1.65,1.65))
    glm::mat4 model_button = Matrix_Translate(9.0f, -1.0f, -1.0f) * Matrix_Scale(1.65f, 1.65f, 1.65f);
    g_CollisionWorld.aabbs.push_back(ComputeWorldAABB(g_VirtualScene["portal_button_reduced_2"].bbox_min, g_VirtualScene["portal_button_reduced_2"].bbox_max, model_button));

    // Paredes Diagonais:
    // Parede diagonal da Sala 2 (Centro: 11.0, -2.7 | Rotação: -45 graus | Escala: 2.0)
    // Calculamos as extremidades aplicando o seno/cosseno de 45 graus (0.707) * escala (2.0) = 1.414
    float offsetX = 1.414f;
    float offsetZ = 1.414f;
    
    g_CollisionWorld.lines.push_back({
        glm::vec2(11.0f - offsetX, -2.7f - offsetZ), // Ponto 1 (~ 9.586, -4.114)
        glm::vec2(11.0f + offsetX, -2.7f + offsetZ), // Ponto 2 (~ 12.414, -1.286)
        y_min,                                       // Chão (-1.0f)
        y_max                                        // Teto (3.0f)
    });

    // Hitbox dinâmica da porta 2 (Fechada)
    // Usamos 3.14159265f no lugar de M_PI pois a macro foi definida dentro do main
    glm::mat4 model_door_closed = Matrix_Translate(8.0f, -1.0f, 6.1f) 
                                * Matrix_Rotate_Y(3.14159265f) 
                                * Matrix_Scale(1.5f, 1.5f, 1.5f);
                                
    g_CollisionWorld.closedDoor = ComputeWorldAABB(
        g_VirtualScene["portal_door_combined_model_1"].bbox_min, 
        g_VirtualScene["portal_door_combined_model_1"].bbox_max, 
        model_door_closed
    );

    // Hitboxes Físicas do Pilar e do Bolo (Sala 3)
    // O pilar está centralizado em X=8.0 e Z=8.15.
    // Com a escala de 0.2, os limites horizontais são exatamente:
    // X: 8.0 - 0.2 até 8.0 + 0.2 (7.8f a 8.2f)
    // Z: 8.15 - 0.2 até 8.15 + 0.2 (7.95f a 8.35f)
    // E a altura vai do chão (-1.0f) até o topo do pilar (-0.2f)
    g_CollisionWorld.aabbs.push_back({ 
        glm::vec3(7.8f, -1.0f, 7.95f),  // Mínimo (X, Y, Z)
        glm::vec3(8.2f, -0.2f, 8.35f)   // Máximo (X, Y, Z)
    });

    // O bolo está em cima do pilar (Y = -0.2f) com escala 0.1.
    // X: 8.0 - 0.1 até 8.0 + 0.1 (7.9f a 8.1f)
    // Z: 8.15 - 0.1 até 8.15 + 0.1 (8.05f a 8.25f)
    // A altura vai do topo do pilar (-0.2f) até o topo das velas (cerca de 0.0f)
    g_CollisionWorld.aabbs.push_back({
        glm::vec3(7.9f, -0.2f, 8.05f),  // Mínimo (X, Y, Z)
        glm::vec3(8.1f,  0.0f, 8.25f)   // Máximo (X, Y, Z)
    });

    // Dimensões do cilindro do jogador (usa os defaults de PlayerCollider).
    g_CollisionWorld.player = PlayerCollider{};

    // Cacheia as AABBs locais (carregadas pelo tinyobj) da caixa ("Cube") e do
    // rádio ("Shell"). PlayerCollidesAt as transforma a cada frame pela model
    // matrix dinâmica (vinda do SceneState) para obter a AABB de mundo precisa.
    g_CollisionWorld.boxLocalMin   = g_VirtualScene["Cube"].bbox_min;
    g_CollisionWorld.boxLocalMax   = g_VirtualScene["Cube"].bbox_max;
    g_CollisionWorld.radioLocalMin = g_VirtualScene["Shell"].bbox_min;
    g_CollisionWorld.radioLocalMax = g_VirtualScene["Shell"].bbox_max;
}

// Restaura a posição e o estado de todos os elementos dinâmicos do jogo
void ResetScene()
{
    // 1. Reset do Jogador e Câmera
    Pos_Player = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    g_CameraTheta = (float)M_PI;
    g_CameraPhi = 0.0f;
    g_PlayerVelocityY = 0.0f;
    g_SpaceWasPressed = false;
    
    // 2. Reset da Caixa
    g_BoxPosition = glm::vec3(11.0f, 0.0f, 5.0f);
    g_BoxVelocityY = 0.0f;
    g_BoxAngleY = 0.0f;
    g_IsHoldingBox = false;
    g_EWasPressed = false;
    g_EstavaSegurandoCaixa = false;

    // 2.5 Reset do Rádio
    g_RadioPosition = glm::vec3(-0.5f, -1.0f, 0.5f); // Lembrar de mudar quando for colocar ele na posição inicial real/correta (dentro do vidro)
    g_RadioVelocityY = 0.0f;
    g_RadioAngleY = (float)M_PI*1.4/2.0;
    g_IsHoldingRadio = false;
    g_EstavaSegurandoRadio = false;

    // 3. Reset do Cenário
    g_IsButtonPressed = false;
    g_FlashlightEnabled = false;

    // 4. Reset dos Modos de Visualização
    g_CameraMode = CAMERA_FPS;
    mov_sec_camera = 1;

    // 5. Reset dos estados de travessia dos portais
    for (int i = 0; i < NUM_PORTAL_PAIRS; ++i)
    {
        g_PortalSets[i].player.initialized = false;
        g_PortalSets[i].box.initialized    = false;
        g_PortalSets[i].radio.initialized  = false;
    }
    g_PlayerPortalVel = glm::vec3(0.0f);

    // 6. Reset das variáveis de debug/poses (Opcional, mas recomendado)
    g_AngleX = 0.0f; 
    g_AngleY = 0.0f; 
    g_AngleZ = 0.0f;
    g_ForearmAngleX = 0.0f; 
    g_ForearmAngleZ = 0.0f;
    g_TorsoPositionX = 0.0f; 
    g_TorsoPositionY = 0.0f;

    g_GameWon = false;
    g_Soloud.stop(g_WinMusicHandle);
}

// Função que carrega uma imagem para ser utilizada como textura
// Vincula às unidades de textura 0 (difusa) e 1 (auxiliar) as texturas usadas
// pelo objeto identificado por object_id, conforme o mapeamento do
// shader_fragment.glsl. Objetos de cor sólida (sem textura) deixam as unidades
// sem textura vinculada. Necessário porque o macOS limita o fragment shader a
// 16 samplers (veja comentário em LoadTextureImage()).
void BindTexturesForObject(GLint object_id)
{
    int diffuse = -1; // índice em g_TextureID da textura difusa
    int aux     = -1; // índice da textura auxiliar (especular/metalness/emissivo)

    switch (object_id)
    {
        case WALL: case WALL_2: case WALL_4: case DOOR_WALL: diffuse = 1;  break;
        case FLOOR:                                          diffuse = 18; break;
        case CEILING:                                        diffuse = 19; break;
        case PLAYER_HEAD:                                    diffuse = 2;  break;
        case PLAYER_EYE:                                     diffuse = 3;  break;
        case PLAYER_TORSO:                                   diffuse = 4;  break;
        case PLAYER_LEGS:                                    diffuse = 5;  break;
        case PLAYER_HAIR:                                    diffuse = 6;  break;
        case CUBE_003:                          diffuse = 8;  aux = 11; break;
        case CUBE:                              diffuse = 9;  aux = 12; break;
        case CUBE_CIRCLE3:                                   diffuse = 7;  break;
        case BUTTON: case BUTTON_001:                        diffuse = 14; break;
        case DOOR:                                           diffuse = 17; break;
        case SEC_CAM:                           diffuse = 20; aux = 21; break;
        case RADIO_SHELL:                                    diffuse = 22; break;
        case RADIO_MAIN:                        diffuse = 23; aux = 0;  break;
        case RADIO_GRID:                                     diffuse = 24; break;
        case RADIO_SCREEN:                      diffuse = 25; aux = 31; break;
        case RADIO_LIGHT:                                    diffuse = 26; break;
        case RADIO_ANTENNA:                                  diffuse = 27; break;
        case RADIO_BUTTON:                                   diffuse = 28; break;
        case RADIO_BUTTON_RIFLED:                            diffuse = 29; break;
        case RADIO_BASE_PART:                                diffuse = 30; break;
        case CAKE:                                           diffuse = 13; break;
        case CAKE_CANDLE:                                    diffuse = 15; break;
        default: break; // objetos de cor sólida não amostram textura
    }

    // Para objetos sem textura (cor sólida) ou sem mapa auxiliar, vinculamos
    // mesmo assim uma textura válida de fallback (a primeira carregada). Essas
    // unidades não são amostradas nessas branches do shader, mas isso evita o
    // aviso do driver do macOS "using zero texture because texture unloadable".
    GLuint fallbackTex = (g_NumLoadedTextures > 0) ? g_TextureID[0] : 0;
    GLuint fallbackSmp = (g_NumLoadedTextures > 0) ? g_SamplerID[0] : 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse >= 0 ? g_TextureID[diffuse] : fallbackTex);
    glBindSampler(0, diffuse >= 0 ? g_SamplerID[diffuse] : fallbackSmp);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, aux >= 0 ? g_TextureID[aux] : fallbackTex);
    glBindSampler(1, aux >= 0 ? g_SamplerID[aux] : fallbackSmp);
}

// Define o object_id no fragment shader e já vincula as texturas necessárias
// para aquele objeto. Substitui o antigo glUniform1i(g_object_id_uniform, ...).
void SetObjectId(GLint object_id)
{
    glUniform1i(g_object_id_uniform, object_id);
    BindTexturesForObject(object_id);
}

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

    // Carregamos a textura na unidade 0 apenas para enviá-la à GPU. A partir
    // daqui ela NÃO fica permanentemente vinculada a uma unidade de textura:
    // guardamos seu ID (e o do sampler) e a vinculamos sob demanda, por objeto,
    // em BindTexturesForObject() — necessário para respeitar o limite de 16
    // samplers do macOS.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (g_NumLoadedTextures < MAX_TEXTURES)
    {
        g_TextureID[g_NumLoadedTextures] = texture_id;
        g_SamplerID[g_NumLoadedTextures] = sampler_id;
    }

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
    g_portal_pass_uniform = glGetUniformLocation(g_GpuProgramID, "portalPass");
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");
    g_flashlight_pos_uniform = glGetUniformLocation(g_GpuProgramID, "flashlight_pos"); // Lanterna
    g_flashlight_dir_uniform = glGetUniformLocation(g_GpuProgramID, "flashlight_dir");
    g_flashlight_on_uniform  = glGetUniformLocation(g_GpuProgramID, "flashlight_on");
    g_clipplane_uniform      = glGetUniformLocation(g_GpuProgramID, "clipPlane"); // recorte dos portais

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura.
    // Agora o shader usa apenas duas samplers (limite de 16 do macOS): a textura
    // difusa fica na unidade 0 e a auxiliar (especular/metalness/emissivo) na
    // unidade 1. As texturas corretas são vinculadas por objeto em
    // BindTexturesForObject().
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureDiffuse"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureAux"), 1);
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

    // Interação com o Menu de Pause
    if (g_IsPaused && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float mx = (float)(2.0 * xpos / width - 1.0);
        float my = (float)(1.0 - 2.0 * ypos / height);

        auto IsClicked = [&](float x_min, float x_max, float y_min, float y_max) {
            return (mx >= x_min && mx <= x_max && my >= y_min && my <= y_max);
        };

        float left_x = -0.85f;

        if (g_MenuState == MENU_MAIN) 
        {
            if (IsClicked(left_x, -0.4f, 0.05f, 0.15f)) { // Jogar
                // É a primeira vez clicando em jogar?
                if (!g_GameStarted) {
                    g_GameStarted = true;
                    mov_sec_camera = 1; // A câmera para de rodar e fixa no jogador!
                    // Libera os efeitos sonoros do mundo!
                    g_Soloud.setPause(g_BusSFXHandle, 0);
                    // Toca a fila da GLaDOS no canal de SFX!
                    g_GladosHandle = g_BusSFX.play(g_GladosDialogueQueue);
                    g_Soloud.setVolume(g_GladosHandle, 0.7f);
                    // Coloca os arquivos na fila
                    g_GladosDialogueQueue.play(g_Glados001);
                    g_GladosDialogueQueue.play(g_Glados002);
                }

                g_IsPaused = false;
                g_CameraMode = CAMERA_FPS;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            if (IsClicked(left_x, -0.4f, -0.10f, 0.0f)) { // Reiniciar
                // Só funciona se o jogo já tiver começado!
                if (g_GameStarted) {
                    ResetScene(); 
                    g_IsPaused = false;
                    g_CameraMode = CAMERA_FPS;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
                }
            }
            if (IsClicked(left_x, -0.2f, -0.25f, -0.15f)) { // Configurações
                g_MenuState = MENU_SETTINGS;
            }
            if (IsClicked(left_x, -0.4f, -0.40f, -0.30f)) { // Créditos
                g_MenuState = MENU_CREDITS;
                // PAUSA a música ambiente (1 = pausado)
                g_Soloud.setPause(g_BGMHandle, 1); 
                g_Soloud.setPause(g_BusSFXHandle, 1);
                g_CreditsMusicHandle = g_BusMusic.play(g_MusicWantYouGone);
            }
            if (IsClicked(left_x, -0.5f, -0.55f, -0.45f)) { // Sair
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
        }
        else if (g_MenuState == MENU_SETTINGS)
        {
            if (IsClicked(left_x, -0.4f, -0.65f, -0.55f)) { // Voltar
                g_MenuState = MENU_MAIN;
                g_IsMusicMuted = false;
            }
            
            float bar_x = -0.2f; // X base dos botões < e >

            // Global
            if (IsClicked(bar_x, bar_x+0.1f, 0.25f, 0.35f)) g_VolumeGlobal -= 0.1f;
            if (IsClicked(bar_x+0.6f, bar_x+0.7f, 0.25f, 0.35f)) g_VolumeGlobal += 0.1f;
            if (g_VolumeGlobal < 0.0f) g_VolumeGlobal = 0.0f;
            if (g_VolumeGlobal > 1.0f) g_VolumeGlobal = 1.0f;
            
            // Música
            if (IsClicked(bar_x, bar_x+0.1f, 0.05f, 0.15f)) g_VolumeMusic -= 0.1f;
            if (IsClicked(bar_x+0.6f, bar_x+0.7f, 0.05f, 0.15f)) g_VolumeMusic += 0.1f;
            if (g_VolumeMusic < 0.0f) g_VolumeMusic = 0.0f;
            if (g_VolumeMusic > 1.0f) g_VolumeMusic = 1.0f;
            
            // Sfx
            if (IsClicked(bar_x, bar_x+0.1f, -0.15f, -0.05f)) g_VolumeSFX -= 0.1f;
            if (IsClicked(bar_x+0.6f, bar_x+0.7f, -0.15f, -0.05f)) g_VolumeSFX += 0.1f;
            if (g_VolumeSFX < 0.0f) g_VolumeSFX = 0.0f;
            if (g_VolumeSFX > 1.0f) g_VolumeSFX = 1.0f;

            // Aplica instantaneamente no motor SoLoud
            g_Soloud.setGlobalVolume(g_VolumeGlobal);
            g_Soloud.setVolume(g_BusMusicHandle, g_VolumeMusic);
            g_Soloud.setVolume(g_BusSFXHandle, g_VolumeSFX);
        }
        else if (g_MenuState == MENU_CREDITS)
        {
            if (IsClicked(left_x, -0.4f, -0.65f, -0.55f)) { // Voltar
                g_MenuState = MENU_MAIN;
                g_Soloud.stop(g_CreditsMusicHandle); // Para a música dos créditos
                // DESPAUSA a música ambiente (0 = tocando)
                g_Soloud.setPause(g_BGMHandle, 0); 
                g_Soloud.setPause(g_BusSFXHandle, 0);
            }
        }
    }

    // Interação com a Tela de Vitória
    if (g_GameWon && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float mx = (float)(2.0 * xpos / width - 1.0);
        float my = (float)(1.0 - 2.0 * ypos / height);

        // Checa se clicou no botão "[ JOGAR NOVAMENTE ]" no centro inferior
        if (mx >= -0.35f && mx <= 0.35f && my >= -0.65f && my <= -0.45f) {
            ResetScene(); 
            // ResetScene lida com g_GameWon = false, mas precisamos garantir que o modo volte pro FPS ou Menu Principal.
            g_GameStarted = false; // Manda de volta pro Menu Principal
            g_IsPaused = true;
            g_CameraMode = CAMERA_SECURITY;
            
            g_Soloud.stop(g_WinMusicHandle); // Para a música Still Alive
            g_Soloud.setPause(g_BGMHandle, 0); // Devolve a música ambiente
            // O cursor se manterá normal pois estamos mandando pro Menu Principal
        }
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Pega a posição do mouse e converte pra NDC pro Menu usar!
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    g_MenuMouseX = (float)(2.0 * xpos / width - 1.0);
    g_MenuMouseY = (float)(1.0 - 2.0 * ypos / height);
    
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

    if (g_LeftMouseButtonPressed && g_GameStarted)
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

    if (g_RightMouseButtonPressed && g_GameStarted)
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

    if (g_MiddleMouseButtonPressed && g_GameStarted)
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
    
    // Se o usuário apertar a tecla R, reinicia a cena inteira.
    if (key == GLFW_KEY_R && action == GLFW_PRESS && g_CameraMode == CAMERA_FPS)
    {
        // Se o cara apertou R durante a tela de vitória, volta pro Menu Principal!
        if (g_GameWon) {

            ResetScene();
            g_GameWon = false;
            g_GameStarted = false; // Manda pro Menu Principal
            g_IsPaused = true;
            g_CameraMode = CAMERA_SECURITY;
            mov_sec_camera = 2; // Volta pro Bézier
            
            // Arruma os sons de volta pro estado de Título
            g_Soloud.stop(g_WinMusicHandle); // Para a música de vitória
            g_Soloud.setPause(g_BGMHandle, 0); // Despausa o ambiente
            g_Soloud.setPause(g_BusSFXHandle, 1); // Pausa os SFX de novo até ele jogar
            
            // Traz o mouse de volta para o menu
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
        }
        else
        {
            ResetScene();
        }

        
        
        fflush(stdout);
    }

    // Se o usuário apertar a tecla T, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fflush(stdout);
    }

    // Tecla Esc abre o Menu de Pause
    // Menu de Pause Híbrido (Câmera de Segurança)
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        
        // Se o jogo ainda não começou, o ESC não faz NADA!
        if (!g_GameStarted) return;
        if (g_GameWon) return;

        g_IsPaused = !g_IsPaused;
        
        if (g_IsPaused) {
            // Entra no pause: Muda pra câmera de segurança e mostra o mouse
            g_MenuState = MENU_MAIN;
            g_IsHoldingBox = false;
            g_IsHoldingRadio = false;
            g_CameraMode = CAMERA_SECURITY;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            // Sai do pause: Volta pro jogador, esconde mouse, e reseta os créditos
            g_CameraMode = CAMERA_FPS;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY); 
            // Para os créditos e garante que o ambiente volte a tocar
            g_Soloud.stop(g_CreditsMusicHandle); 
            g_Soloud.setPause(g_BGMHandle, 0);
            g_Soloud.setPause(g_BusSFXHandle, 0);
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
    
    // Se o usuário apertar a tecla M, muta/desmuta a música
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        g_IsMusicMuted = !g_IsMusicMuted;
        
        if (g_IsMusicMuted) {
            // Muta a instância que está tocando!
            g_Soloud.setVolume(g_BusMusicHandle, 0.0f); 
        } else {
            // Restaura o volume para o valor salvo
            g_Soloud.setVolume(g_BusMusicHandle, g_VolumeMusic); 
        }
        
        fprintf(stdout, "Musica %s\n", g_IsMusicMuted ? "Mutada" : "Desmutada");
        fflush(stdout);
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

void TextRendering_ShowPauseMenu(GLFWwindow* window)
{
    if (!g_IsPaused) return;

    // Função para o hover: Se o mouse passar em cima, o texto cresce um pouco e "acende"
    auto GetHoverScale = [&](float x_min, float x_max, float y_min, float y_max, float base_scale) {
        if (g_MenuMouseX >= x_min && g_MenuMouseX <= x_max && g_MenuMouseY >= y_min && g_MenuMouseY <= y_max)
            return base_scale * 1.2f; 
        return base_scale;
    };

    // Função para gerar a barra de volume visual
    auto GetVolumeBar = [](float vol) -> std::string {
        int bars = (int)(vol * 10.0f + 0.1f); // Arredonda seguro (0 a 10)
        std::string s = "[";
        for (int i = 0; i < 10; i++) {
            s += (i < bars ? '|' : ' ');
        }
        s += "] " + std::to_string(bars * 10) + "%";
        return s;
    };

    // Usamos um X fixo para alinhar tudo à esquerda, igual Portal!
    float left_x = -0.85f;

    if (g_MenuState == MENU_MAIN) 
    {
        // Título gigante simulando a logo
        TextRendering_PrintString(window, "PORTAL 3", left_x, 0.4f, 3.0f);
        
        // Botões alinhados à esquerda
        TextRendering_PrintString(window, "Jogar", left_x, 0.1f, GetHoverScale(left_x, -0.4f, 0.05f, 0.15f, 1.5f));

        if (g_GameStarted) {
            TextRendering_PrintString(window, "Reiniciar", left_x, -0.05f, GetHoverScale(left_x, -0.4f, -0.10f, 0.0f, 1.5f));
        }

        TextRendering_PrintString(window, "Configuracoes", left_x, -0.2f, GetHoverScale(left_x, -0.2f, -0.25f, -0.15f, 1.5f));
        TextRendering_PrintString(window, "Creditos", left_x, -0.35f, GetHoverScale(left_x, -0.4f, -0.40f, -0.30f, 1.5f));
        TextRendering_PrintString(window, "Sair", left_x, -0.5f, GetHoverScale(left_x, -0.5f, -0.55f, -0.45f, 1.5f));
    } 
    else if (g_MenuState == MENU_SETTINGS) 
    {
        TextRendering_PrintString(window, "CONFIGURACOES", left_x, 0.6f, 2.0f);
        
        // TEXTOS FIXOS
        TextRendering_PrintString(window, "Volume Global", left_x, 0.3f, 1.2f);
        TextRendering_PrintString(window, "Volume Musica", left_x, 0.1f, 1.2f);
        TextRendering_PrintString(window, "Volume Efeitos", left_x, -0.1f, 1.2f);

        // BOTÕES INTERATIVOS (< e >) E AS BARRAS
        // Para alinhar, colocamos as barras num X fixo mais à direita (-0.2f)
        float bar_x = -0.2f;

        TextRendering_PrintString(window, "<", bar_x, 0.3f, GetHoverScale(bar_x, bar_x+0.1f, 0.25f, 0.35f, 1.2f));
        TextRendering_PrintString(window, GetVolumeBar(g_VolumeGlobal), bar_x + 0.1f, 0.3f, 1.2f);
        TextRendering_PrintString(window, ">", bar_x + 0.6f, 0.3f, GetHoverScale(bar_x+0.6f, bar_x+0.7f, 0.25f, 0.35f, 1.2f));

        TextRendering_PrintString(window, "<", bar_x, 0.1f, GetHoverScale(bar_x, bar_x+0.1f, 0.05f, 0.15f, 1.2f));
        TextRendering_PrintString(window, GetVolumeBar(g_VolumeMusic), bar_x + 0.1f, 0.1f, 1.2f);
        TextRendering_PrintString(window, ">", bar_x + 0.6f, 0.1f, GetHoverScale(bar_x+0.6f, bar_x+0.7f, 0.05f, 0.15f, 1.2f));

        TextRendering_PrintString(window, "<", bar_x, -0.1f, GetHoverScale(bar_x, bar_x+0.1f, -0.15f, -0.05f, 1.2f));
        TextRendering_PrintString(window, GetVolumeBar(g_VolumeSFX), bar_x + 0.1f, -0.1f, 1.2f);
        TextRendering_PrintString(window, ">", bar_x + 0.6f, -0.1f, GetHoverScale(bar_x+0.6f, bar_x+0.7f, -0.15f, -0.05f, 1.2f));

        // MAPEAMENTO MOSTRADO COMO TEXTO CORRIDO EMBAIXO
        TextRendering_PrintString(window, "Mover: WASD | Pular: Espaco | Correr: Shift | Pegar: E | Lanterna: F | Mutar Musicas: M", left_x, -0.32f, 1.0f);
        TextRendering_PrintString(window, "Trocar Camera de Seguranca: <-/-> | Trocar Modo das Cameras de Seguranca: 1/2", left_x, -0.4f, 1.0f);

        TextRendering_PrintString(window, "Voltar", left_x, -0.6f, GetHoverScale(left_x, -0.4f, -0.65f, -0.55f, 1.5f));
    }
    else if (g_MenuState == MENU_CREDITS) 
    {
        TextRendering_PrintString(window, "CREDITOS", left_x, 0.5f, 2.0f);
        TextRendering_PrintString(window, "Desenvolvedores: Gabriel Pieruccini Knopp e Tobias Cadona Marion", left_x, 0.2f, 1.5f);
        TextRendering_PrintString(window, "Todos modelos usados listados no README.md", left_x, 0.0f, 1.5f);
        TextRendering_PrintString(window, "Trabalho inspirado nas obras Portal e Portal 2 da Valve", left_x, -0.2f, 1.5f);
        TextRendering_PrintString(window, "Obrigado por jogar!", left_x, -0.4f, 1.5f);
        
        TextRendering_PrintString(window, "Voltar", left_x, -0.6f, GetHoverScale(left_x, -0.4f, -0.65f, -0.55f, 1.5f));
    }
}

void TextRendering_ShowWinScreen(GLFWwindow* window)
{
    if (!g_GameWon) return;

    float left_x = -0.85f;

    // Textos alinhados à esquerda com a temática da Aperture
    TextRendering_PrintString(window, "PARABENS, OBRIGADO POR JOGAR!", left_x, 0.6f, 2.0f);
    
    TextRendering_PrintString(window, "> AVALIACAO DA COBAIA: SUCESSO EXCEPCIONAL", left_x, 0.3f, 1.2f);
    TextRendering_PrintString(window, "> PROTOCOLO DE FESTA: INICIADO", left_x, 0.15f, 1.2f);
    TextRendering_PrintString(window, "> AVISO: O BOLO E REAL.", left_x, 0.0f, 1.2f);

    // Efeito visual pulsante suave na instrução de reiniciar (usando o tempo do GLFW)
    float pulse = (sin(glfwGetTime() * 4.0f) + 1.0f) / 2.0f; // Varia de 0.0 a 1.0
    TextRendering_PrintString(window, "Aperte 'R' para reiniciar o sistema...", left_x, -0.4f, 1.2f + 0.1f * pulse);
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

