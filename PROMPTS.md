# Prompts para IA

## Gemini

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
- Ajuda com colisões, lógica de carregar a caixa, lógica de pressionar o botão, reinicialização da cena, corrida, sons, músicas e ajuste da colisão com objetos e portais: https://share.gemini.google/OXTf5agIjLCT  
- Ajuda com correção dos portais e ajustes finais da cena: https://share.gemini.google/xY0yCIULQ3Hj 

## Seções do Claude code: 
| # | Sessão | Última modificação | Mensagens | Arquivo |
|---|--------|-------------------|-----------|---------|
| 1 | `095b05d5-9b56-40d3-9747-53329fdcd290` | 2026-06-08 21:21:09 | 74 | [📄](./claude-code-sessions/transcricoes/sessao_001_095b05d5-9b56-40d3-9747-53329fdcd290.txt) |
| 2 | `36c484f7-57e8-43fb-8645-1db3cbdd74d0` | 2026-06-07 20:51:47 | 99 | [📄](./claude-code-sessions/transcricoes/sessao_002_36c484f7-57e8-43fb-8645-1db3cbdd74d0.txt) |
| 3 | `4cc98a7e-66d7-4230-9b99-aba4c418ca0e` | 2026-05-25 18:06:19 | 15 | [📄](./claude-code-sessions/transcricoes/sessao_003_4cc98a7e-66d7-4230-9b99-aba4c418ca0e.txt) |
| 4 | `7b1c6ae8-91b5-4df2-95f7-a55d02c5ecab` | 2026-06-02 21:24:18 | 260 | [📄](./claude-code-sessions/transcricoes/sessao_004_7b1c6ae8-91b5-4df2-95f7-a55d02c5ecab.txt) |
| 5 | `agent-a5531aa679f9251ff` | 2026-06-02 20:45:08 | 12 | [📄](./claude-code-sessions/transcricoes/sessao_005_agent-a5531aa679f9251ff.txt) |
| 6 | `871d016d-428d-43e5-b22b-0134ac636bcb` | 2026-06-02 21:40:53 | 42 | [📄](./claude-code-sessions/transcricoes/sessao_006_871d016d-428d-43e5-b22b-0134ac636bcb.txt) |
| 7 | `ba492d79-c1c7-4a7f-b663-aeedf909260d` | 2026-06-18 22:26:26 | 424 | [📄](./claude-code-sessions/transcricoes/sessao_007_ba492d79-c1c7-4a7f-b663-aeedf909260d.txt) |
| 8 | `bb3a86ea-ffc7-4822-b89f-542e57d03c9a` | 2026-06-20 18:36:18 | 339 | [📄](./claude-code-sessions/transcricoes/sessao_008_bb3a86ea-ffc7-4822-b89f-542e57d03c9a.txt) |
| 9 | `agent-a46b2a66ccdb6afb8` | 2026-06-20 17:41:21 | 45 | [📄](./claude-code-sessions/transcricoes/sessao_009_agent-a46b2a66ccdb6afb8.txt) |
| 10 | `agent-af5d3dfa04ec589bc` | 2026-06-20 17:42:02 | 83 | [📄](./claude-code-sessions/transcricoes/sessao_010_agent-af5d3dfa04ec589bc.txt) |
| 11 | `agent-affeb12c95d14a583` | 2026-06-20 17:41:56 | 91 | [📄](./claude-code-sessions/transcricoes/sessao_011_agent-affeb12c95d14a583.txt) |
| 12 | `bfff5f56-3db1-47d4-8539-9bd3325455ed` | 2026-06-20 23:11:26 | 281 | [📄](./claude-code-sessions/transcricoes/sessao_012_bfff5f56-3db1-47d4-8539-9bd3325455ed.txt) |
| 13 | `d3ad15e2-c56d-404f-aa46-17a689784926` | 2026-06-08 20:18:48 | 238 | [📄](./claude-code-sessions/transcricoes/sessao_013_d3ad15e2-c56d-404f-aa46-17a689784926.txt) |
| 14 | `agent-a0c6fa0ded466fdb2` | 2026-06-07 20:57:36 | 19 | [📄](./claude-code-sessions/transcricoes/sessao_014_agent-a0c6fa0ded466fdb2.txt) |
| 15 | `agent-a47c4080bbfa4685c` | 2026-06-07 20:53:23 | 47 | [📄](./claude-code-sessions/transcricoes/sessao_015_agent-a47c4080bbfa4685c.txt) |
| 16 | `d82f5c09-de13-40a2-b7e5-23bed4cc57de` | 2026-06-20 17:27:11 | 307 | [📄](./claude-code-sessions/transcricoes/sessao_016_d82f5c09-de13-40a2-b7e5-23bed4cc57de.txt) |
| 17 | `f4abe1a1-da3f-479f-9627-34625b311e06` | 2026-06-21 13:39:09 | 391 | [📄](./claude-code-sessions/transcricoes/sessao_017_f4abe1a1-da3f-479f-9627-34625b311e06.txt) |
