# Jogo da Velha - Raspberry Pi Pico W

## Descrição
Este projeto implementa um jogo da velha (tic-tac-toe) utilizando a Raspberry Pi Pico W, uma matriz de LEDs 5x5, um display OLED, um joystick, botões e um buzzer. O jogo é jogado diretamente na matriz de LEDs, com feedback sonoro e visual para melhorar a interação do usuário.

## Componentes Utilizados
- **Raspberry Pi Pico W**
- **Matriz de LEDs WS2812B (5x5 - 25 LEDs RGB)**
- **Display OLED SSD1306 (I2C)**
- **Joystick** (para mover o cursor no tabuleiro)
- **Botão** (para confirmar jogadas)
- **Buzzer** (para efeitos sonoros)

## Funcionalidades
- Interface visual na matriz de LEDs, indicando posições e jogadas.
- Uso do joystick para mover o cursor pelo tabuleiro.
- Uso do botão para confirmar a jogada.
- Exibição de mensagens no display OLED.
- Indicação sonora para jogadas e eventos especiais (início do jogo, vitória, empate).
- Detecção automática de vitórias e empates.
- Reinicialização automática após o fim da partida.

## Estrutura do Código
O código é dividido em módulos principais:

1. **Configuração de hardware**:
   - Inicializa GPIOs, ADCs, PWM, I2C e periféricos.
   - Configura o display OLED, matriz de LEDs e buzzer.
   
2. **Controle do Jogo**:
   - Representação do tabuleiro (3x3) e conversão para a matriz 5x5.
   - Controle do cursor e interação com o jogador.
   - Verificação de vitórias e empates.
   
3. **Efeitos Visuais e Sonoros**:
   - Animações de LEDs.
   - Sons para diferentes eventos.
   
4. **Interface com o Usuário**:
   - Exibição de mensagens no display OLED.
   - Setas indicativas para uso dos controles.

## Como Jogar
1. **Inicialização**:
   - O jogo inicia com uma animação e som de abertura.
   - O tabuleiro é desenhado na matriz de LEDs.
   
2. **Movimentar o Cursor**:
   - Use o **joystick** para mover o cursor pela matriz.

3. **Fazer uma Jogada**:
   - Pressione o **botão** para confirmar a jogada.
   - O jogador 1 é representado pela cor **vermelha**.
   - O jogador 2 é representado pela cor **azul**.

4. **Finalização**:
   - O jogo verifica automaticamente vitórias e empates.
   - Em caso de vitória, os LEDs piscam destacando a linha vencedora.
   - Após o fim do jogo, a mensagem é exibida no display.
   - O jogo reinicia automaticamente após alguns segundos.

## Melhorias Futuras
- Implementação de mais efeitos, visuais e sonoros.
- Opção de jogar contra o algoritmo.

## Apresentação em Vídeo
https://youtube.com/shorts/L1O1Z8epDWc?si=lBBwd-6GBS6r742n

## Créditos
Neste projeto foi utilizado arquivos do repositório [BitDogLab-C](https://github.com/BitDogLab/BitDogLab-C).

