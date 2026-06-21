# Computação Gráfica e Visualização I (INF01047) - INF/UFRGS
Este repositório contém o código base para o trabalho final. O enunciado completo do trabalho final está no Moodle: https://moodle.ufrgs.br/mod/assign/view.php?id=6018620

## Integrantes da dupla
- **Aluno 1 - Nome**: Gabriel Pieruccini Knopp
- **Aluno 1 - Cartão UFRGS**: 00594975

- **Aluno 2 - Nome**: Tobias Cadoná Marion
- **Aluno 2 - Cartão UFRGS**: 00590278

# Descrição da aplicação
- A aplicação "Portal 3" desenvolvida consiste na primeira fase do jogo Portal, contendo nela os portais da primeira sala - junto com o pequeno rádio, a interação da caixa com o botão para a abrir a porta da segunda sala, as câmeras de segurança espalhadas pelo ambiente e uma pequena sala de "chegada", que não estava no jogo original, mas que serve como substituta para o elevador que ficava no final. O programa utiliza OpenGL para criar um ambiente 3D interativo onde o usuário pode explorar a cena, manipular objetos e atravessar portais para resolver o puzzle simples da fase, integrando os conceitos vistos na cadeira referente a transformações, câmeras, iluminação, colisões em tempo real e outros.

# Contribuições de cada membro
- Gabriel Pieruccini Knopp
    * Importação, edição e renderização de todos modelos 3D (Chell, cubo, câmera de segurança, botão, porta, parede, rádio, chão, teto, bolo e etc.), juntamente do mapeamento de suas texturas;
    * Montagem do cenário;
    * Revisão e correção das câmeras, adição do modo look-at para o jogador e sincronização do movimento com o modelo da câmera de segurança;
    * Implementação da iluminação da cena, juntamente com a função de usar uma lanterna;
    * Revisão e correção das colisões e implementação das colisões na sala 3;
    * Implementação da função de carregar o cubo e o rádio;
    * Implementação da colisão dinâmica com o cubo e o rádio;
    * Implementação da função de apertar o botão da sala e abrir a porta, quando o jogador ou a caixa estiverem em cima dele;
    * Implementação de efeitos sonoros e músicas;
    * Implementação de reinicialização da cena, botão de corrida e botão de mutar a música;
    * Implementação da interface, do menu de início, do menu de pause e do menu de vitória;
    * Revisão e pequenos ajustes nos portais da cena.

- Tobias Cadoná Marion
    * Implementação da visão da câmera em primeira pessoa e em movimento de curva de Bézier;
    * Implementação da física do jogador, da função de pular e de colisões do jogador com o cenário nas salas 1 e 2;
    * Implementação do sistema de portais e refatoração do fragment shader.

# Uso de IA
- A dupla fez uso das ferramentas Gemini (Pro) e Claude para o desenvolvimento da aplicação, ajudando com a maior parte das implementações, sendo todo o auxílio descrito com totalidade no arquivo PROMPTS.md. Sobre o Gemini Pro, ele foi extraordinariamente útil durante todo o projeto, sendo capaz de realizar cálculos automaticamente para montagem das colisões da cena e sempre conseguindo chegar nos resultados desejados, contanto que fosse guiado até lá com cuidado. O único ponto que o Gemini devidamente sofreu para conseguir auxiliar (ou não conseguiu) foi para com algumas correções dos portais/interação deles com a cena, que teve de ser feita manualmente.

# Imagens mostrando o funcionamento da aplicação

![1](screenshots\1.png)

![2](screenshots\2.png)

![3](screenshots\3.png)

![4](screenshots\4.png)

![5](screenshots\5.png)

# Manual descrevendo a utilização da aplicação
- A utilização da aplicação revela-se altamente intuitiva, pois a navegação é feita com o mouse e, ainda, todos os controles estão listados ao abrir o menu de "Configurações". De todo modo, são eles: 
    * Mover: WASD
    * Pular: Espaco
    * Correr: Shift
    * Pegar: E
    * Lanterna: F
    * Mutar Musicas: M
    * Trocar Camera de Seguranca: <-/-> 
    * Trocar Modo das Cameras de Seguranca: 1/2
- O objetivo da aplicação é chegar no final.

# Passos necessários para compilação e execução da aplicação
- Independentemente do sistema operacional, a execução do binário compilado foi automatizada para funcionar ao rodar o comando make run, fazendo com que o script de build acesse o respectivo diretório de saída (./bin/Linux/, ./bin/Debug/ ou ./bin/macOS/) e inicie a aplicação por meio do arquivo executável (como ./main ou main.exe).

# Créditos dos Modelos Usados no Trabalho
Modelo do personagem
* https://sketchfab.com/3d-models/chell-3b239db8827e49d5aabdb9aba7e18952 
  
Modelo do cubo
* https://sketchfab.com/3d-models/portal-cube-b67becc862054d9fab781b3e4030a1af 

Modelo do botão
* https://sketchfab.com/3d-models/button-from-portal-2-original-f1c4fcfc668a4577a30f4d1c8ed3ca9a

Modelo da porta de saída
* https://sketchfab.com/3d-models/door-from-portal-2-original-e0fa18a874dc4a5bbc391f9bf876d2f5 

Modelo da câmera de segurança
* https://sketchfab.com/3d-models/portal-security-camera-c144c9c55c28489b83b37eebfa558abc 

Modelo do rádio
* https://sketchfab.com/3d-models/portal-radio-9896a03c4dbf4cb78c31ffcf1d850ece 

Modelo do bolo
* https://plewr.itch.io/portal-cake-low-poly 

Texturas usadas não listadas nos modelos acima foram tiradas diretamente dos arquivos originais do jogo

Musicas tiradas da Soundtrack Oficial do jogo
* https://open.spotify.com/intl-pt/artist/7d58WZ8qQHy2Sm5p52V2NP?si=MdJqsk3rQdeLj3rMRWaJ9g 

Video de gameplay 
* https://www.youtube.com/watch?v=wA82CD9YFG0 

Página do jogo na Steam
* https://store.steampowered.com/app/400/Portal/ 

# Como cada implementação exigida no trabalho se aplica no nosso jogo
A sua aplicação deve possibilitar interação em tempo real.
* Andar pela cena e pegar o cubo ou o rádio.

A sua aplicação deve possuir algum objetivo e lógica de controle não-trivial.
* O objetivo é completar o puzzle da sala para chegar no final, colocando o cubo no botão, depois de atravessar um portal para sair do cubículo de vidro. Essencialmente a fase 1 do jogo Portal.

A sua aplicação deve utilizar as matrizes que vimos em aula para transformações geométricas (Model matrix), projeções (Projection matrix), e especificação do sistema de coordenadas da câmera (View matrix).
* Transormações geométricas para mover a caixa, rádio e jogador. Projeções para a renderização dos portais (exemplo usado na aula de projeção).

A sua aplicação deve possibilitar interação com o usuário através do mouse e do teclado.
* Funcionalidade básica de um jogo, como será o nosso caso. Novamente, andar pela cena, pegar cubo e rádio e interface de menu.

A sua aplicação deve incluir implementação dos seguintes conceitos de Computação Gráfica:
* Objetos virtuais representados através de malhas poligonais complexas (malhas de triângulos).
    - Modelos da caixa, personagem, rádio, câmeras de segurança e bolo são malhas poligonais complexas.
* Transformações geométricas de objetos virtuais.
    - Mover as caixas e andar são transformações.
* Controle de câmeras virtuais.
    - Como mencionado, a câmera do personagem é controlada pelo mouse, e existe o modo de câmera segurança que contém dois tipos de visualização: uma câmera look-at travada no personagem e outra que segue um movimento de Curva de Bézier.
* No mínimo um objeto virtual deve ser copiado com duas ou mais instâncias, isto é, utilizando duas ou mais Model matrix aplicadas ao mesmo conjunto de vértices.
    - Existe múltiplas instâncias de paredes, paredes de vidro e portas.
* Testes de intersecção entre objetos virtuais.
    - Há testes de colisão entre personagem-cenário, personagem-caixa, personagem-rádio, personagem-botão, caixa-rádio, caixa-cenário, caixa-botão e rádio-cenário (no jogo original, o rádio não serve para apertar um botão).
* Modelos de iluminação de objetos geométricos.
    - A cena contêm diversas fontes de iluminação pelas salas, utilizando o método de iluminação de Phong.
* Mapeamento de texturas.
    - Caixa, personagem, rádio, paredes, chão, teto, bolo, câmeras de segurança, botão e portas possuem texturas que vieram junto com os modelos 3D baixados.
* Curvas de Bézier.
    - As câmeras de segurança possuem um modo de observar a sala seguindo um movimento esquerda-direita gerado por uma Curva de Bézier.
* Animação de Movimento baseada no tempo.
    - Todas as movimentações dos objetos dependem do tempo (até porque possuem gravidade/caem no chão).

Funcionalidades Extras
* Efeitos sonoros;
* Interface Gráfica (botões, etc.).
