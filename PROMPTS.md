## Prompts para IA
- Ajuda com texturas: https://g.co/gemini/share/cca94185943a 
- Ajuda com deltatime, movimentação do personagem e textura GL_REPEAT: perdemos a conversa exata por termos feito durante o laboratório, em aula, e não estarmos logado em uma conta. Mas, em resumo, a IA ajudou com os seguintes trechos: 

*   "float atualFrame = (float)glfwGetTime(); 
    deltaTime = atualFrame - ultimoFrame;
    ultimoFrame = atualFrame;"

*   "if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        Pos_Player.z += 1.0f * deltaTime;}"

*   "if (deveRepetir){
        glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);}
    else{
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);}"

*   "float fatorRepeticao = 4.0f; 
    U = texcoords.x * fatorRepeticao;
    V = texcoords.y * fatorRepeticao;"

- Ajuda com arrumar a câmera_fps com a cena e com o modelo do personagem, os pontos da curva de Bézier, a lanterna do jogador e iluminação da cena: https://gemini.google.com/share/bdbf5fb92d12 
- Ajuda com a parede que vai junto com a porta, câmera look-at para o jogador, renderização de outros modelos (como as câmeras de seguranças, o rádio e o bolo) e menu de "pause" e vitória/interface: https://share.gemini.google/2yAry1D42Mnx   
- Ajuda com colisões, lógica de carregar a caixa, lógica de pressionar o botão, reinicialização da cena, corrida, sons e músicas: https://share.gemini.google/I0lcyr5ZhIsq 