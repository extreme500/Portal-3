# Computação Gráfica e Visualização I (INF01047) - INF/UFRGS

Este repositório contém o código base para o trabalho final. O enunciado completo do trabalho final está no Moodle:

https://moodle.ufrgs.br/mod/assign/view.php?id=6018620

# Créditos dos Modelos Usados no Trabalho:
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

# Como cada implementação exigida no trabalho se aplica no nosso jogo:
A sua aplicação deve possibilitar interação em tempo real.
* Andar pela cena e pegar cubos.

A sua aplicação deve possuir algum objetivo e lógica de controle não-trivial.
* O objetivo é completar o puzzle da sala para chegar no final, colocando o cubo no botão, depois de atravessar um portal para sair do cubículo de vidro. Essencialmente a fase 1 do jogo Portal.

A sua aplicação deve utilizar as matrizes que vimos em aula para transformações geométricas (Model matrix), projeções (Projection matrix), e especificação do sistema de coordenadas da câmera (View matrix).
* Transormações geométricas para mover a caixa, rádio e jogador. Projeções para a renderização dos portais (exemplo usado na aula de projeção).

A sua aplicação deve possibilitar interação com o usuário através do mouse e do teclado.
* Funcionalidade básica de um jogo, como será o nosso caso. Novamente, andar pela cena, pegar cubo e rádio e interface de menu.

A sua aplicação deve incluir implementação dos seguintes conceitos de Computação Gráfica:
* Objetos virtuais representados através de malhas poligonais complexas (malhas de triângulos).
    Modelos da caixa, personagem, rádio, câmeras de segurança e bolo são malhas poligonais complexas.
* Transformações geométricas de objetos virtuais.
    Mover as caixas e andar são transformações.
* Controle de câmeras virtuais.
    Como mencionado, a câmera do personagem é controlada pelo mouse, e existe o modo de câmera segurança que contém dois tipos de visualização: uma câmera look-at travada no personagem e outra que segue um movimento de Curva de Bézier.
* No mínimo um objeto virtual deve ser copiado com duas ou mais instâncias, isto é, utilizando duas ou mais Model matrix aplicadas ao mesmo conjunto de vértices.
    Existe múltiplas instâncias de paredes, paredes de vidro e portas.
* Testes de intersecção entre objetos virtuais.
    Há testes de colisão entre personagem-cenário, personagem-caixa, personagem-rádio, personagem-botão, caixa-rádio, caixa-cenário, caixa-botão e rádio-cenário (no jogo original, o rádio não serve para apertar um botão).
* Modelos de iluminação de objetos geométricos.
    A cena contêm diversas fontes de iluminação pelas salas, utilizando o método de iluminação de Phong.
* Mapeamento de texturas.
    Caixa, personagem, rádio, paredes, chão, teto, bolo, câmeras de segurança, botão e portas possuem texturas que vieram junto com os modelos 3D baixados.
* Curvas de Bézier.
    As câmeras de segurança possuem um modo de observar a sala seguindo um movimento esquerda-direita gerado por uma Curva de Bézier.
* Animação de Movimento baseada no tempo.
    Todas as movimentações dos objetos dependem do tempo (até porque possuem gravidade/caem no chão).

Funcionalidades Extras
* Efeitos sonoros
* Interface Gráfica (botões, etc.);
