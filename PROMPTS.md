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

-