# Computação Gráfica e Visualização I (INF01047) - INF/UFRGS

Este repositório contém o código base para o trabalho final. O enunciado completo do trabalho final está no Moodle:

https://moodle.ufrgs.br/mod/assign/view.php?id=6018620

# Agrupando coisas que iremos usar no trabalho:
Modelo do personagem
* https://sketchfab.com/3d-models/chell-3b239db8827e49d5aabdb9aba7e18952
Modelo do cubo
* https://sketchfab.com/3d-models/portal-cube-b67becc862054d9fab781b3e4030a1af
Modelo da portal gun
* https://sketchfab.com/3d-models/portal-gun-old-eaff0d242c3542fd8f7b63a5ec49d2da
Video de gameplay
* https://www.youtube.com/watch?v=KZDJNJeTYQI

# Direcionando implementações:
A sua aplicação deve possibilitar interação em tempo real.
* Andar pela cena, pegar cubos e atirar portais.

A sua aplicação deve possuir algum objetivo e lógica de controle não-trivial.
* O objetivo é completar puzzle da sala para chegar no final, colocando o cubo em botões e usando portais para se locomover.

A sua aplicação deve utilizar as matrizes que vimos em aula para transformações geométricas (Model matrix), projeções (Projection matrix), e especificação do sistema de coordenadas da câmera (View matrix).
* Transormações geométricas para mover as caixas, jogador e portais. Projeções para a renderização dos portais (literalmente um exemplo usado na aula de projeção). Como devemos ter dois tipos de câmera, uma delas pode ser a visão do personagem (câmera livre, como no jogo) e a outra pode ser uma câmera look-at travada no personagem de algum lugar da sala (ou terceira pessoa, mas acho mais complicado), lançar um portal e pegar e mover um objeto dependem da câmera do personagem. 

A sua aplicação deve possibilitar interação com o usuário através do mouse e do teclado.
* Funcionalidade básica de um jogo, como será o nosso caso. Novamente, andar pela cena, pegar cubos e atirar portais.

A sua aplicação deve incluir implementação dos seguintes conceitos de Computação Gráfica:
* Objetos virtuais representados através de malhas poligonais complexas (malhas de triângulos).
    Modelos das caixas, arma do portal e personagem podem ser malhas poligonais complexas.
* Transformações geométricas de objetos virtuais.
    Mover as caixas e andar são transformações.
* Controle de câmeras virtuais.
    Como mencionado, a câmera do personagem é controlada pelo mouse, e pode existir uma outra câmera look-at travada no personagem (como se fosse pela qual a GLADOS enxerga, que costuma ficar no canto do teto).
* No mínimo um objeto virtual deve ser copiado com duas ou mais instâncias, isto é, utilizando duas ou mais Model matrix aplicadas ao mesmo conjunto de vértices.
    Podemos fazer mais de um botão, sendo necessária mais de uma caixa.
* Testes de intersecção entre objetos virtuais.
    Serão necessários testes de colisão entre personagem-parede, personagem-caixa, caixa-caixa, caixa-botão e caixa-parede ("parede" inclui chão e teto).
* Modelos de iluminação de objetos geométricos.
    A cena deverá conter uma ou mais fontes de iluminação (lâmpadas pelas salas) e, portanto, podemos iluminar os objetos de acordo.
* Mapeamento de texturas.
    Caixas, personagem, paredes (especialemente para diferenciar qual pode possuir um portal nela), chão, teto e outros objetos devem possuir texturas (as quais devem estar públicas na internet, ou nos próprios arquivos do jogo.
* Curvas de Bézier.
    A decidir que tipo de coisa exatamente iremos fazer uma movimentação por curva de bezier (pode ser uma rotação de uma câmera da sala - que vai da esquerda para direita, ou até uma torreta? ou outro objeto que fique mexendo em função sem necessariamente precisar interferir no jogo).
* Animação de Movimento baseada no tempo.
    Basicamente todas as movimentações dos objetos (além da câmera) irão depender do tempo (já que possuem gravidade e caem no chão).
