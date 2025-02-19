#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "ws2818b.pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

#define LED_QTD 25
#define LED_PIN 7
#define BOTAO_B 6  
#define BUZZER_PIN 21

// Configurações do Display
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};

uint8_t ssd[ssd1306_buffer_length];  // Buffer global

// Procedimentos relacionados ao buzzer
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(pwm_gpio_to_slice_num(pin), &config, true);
    pwm_set_gpio_level(pin, 0);
}
void play_tone(uint pin, uint freq, uint duration) {
    if (freq == 0) {
        sleep_ms(duration);
        return;
    }
    uint slice = pwm_gpio_to_slice_num(pin);
    uint32_t top = clock_get_hz(clk_sys) / freq - 1;
    pwm_set_wrap(slice, top);
    pwm_set_gpio_level(pin, top / 2);  // Reduzindo o ciclo de trabalho para 1/4
    sleep_ms(duration);
    pwm_set_gpio_level(pin, 0);
    sleep_ms(30);
}

// Estrutura que representa um LED RGB
struct pixel_t {
    uint8_t G, R, B;   // Estrutura que representa um LED RGB
};

// Array que armazena as cores dos 25 LEDs
typedef struct pixel_t npLED_t;
npLED_t leds[LED_QTD]; // Array que armazena as cores dos  25 LEDs

PIO np_pio; // PIO para controlar os LEDs WS2812B
uint sm;   // Máquina de estado do PIO

int tabuleiro[3][3] = {0}; // Tabuleiro 3x3 (0 = vazio (Apagado), 1 = X (Vermelho), 2 = O (Azul))
int jogador_atual = 1;    // Define qual jogador está jogando, sempre começa com o 1 = X (Vermelho)

// Inicializa a matriz de LEDs e configura o PIO para controlar os LEDs
void npInit(uint pin) { 
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    
    for (uint i = 0; i < LED_QTD; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Atribui uma cor RGB a um LED da matriz.
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) { 
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Limpa o buffer de pixels.
void npClear() {
    for (uint i = 0; i < LED_QTD; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Escreve os dados do buffer nos LEDs da matriz.
void npWrite() {  
    for (uint i = 0; i < LED_QTD; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

void tocar_musicaFinal(uint pin) {
    const uint notas_final[] = {230, 294, 300, 300, 300}; 
    const uint duracao_notafinal[] = {100, 100, 100, 100, 100};

    for (int i = 0; i < sizeof(notas_final) / sizeof(notas_final[0]); i++) {
        play_tone(pin, notas_final[i], duracao_notafinal[i]);
        
    }
    
}

// Converte posição 3x3 para 5x5 (ajustando o espaçamento)
int obterIndice(int linha, int coluna) { 
    int x = coluna * 2;
    int y = linha * 2;
    if (y % 2 == 0) return 24 - (y * 5 + x);
    else return 24 - (y * 5 + (4 - x));
}

void tocar_musicaInicial(uint pin) {
    const uint notas_iniciais[] = {230, 294, 294, 300};
    const uint duracao_notas_iniciais[] = {100, 100, 100, 100};
    for (int i = 0; i < sizeof(notas_iniciais) / sizeof(notas_iniciais[0]); i++) {
        play_tone(pin, notas_iniciais[i], duracao_notas_iniciais[i]);
        
    }
    
}
// Piscar LEDs formando a "#".
void acenderLedsCerquilha() {
    const int leds_cerquilha1[] = {1, 3, 5, 6, 7, 8, 9, 11, 13, 15, 16, 17, 18, 19, 21, 23};

    // Cor Ciano
    for (int i = 0; i < sizeof(leds_cerquilha1) / sizeof(leds_cerquilha1[0]); i++) {
        npSetLED(leds_cerquilha1[i], 0, 40, 40); 
        
    }
    npWrite();
    tocar_musicaInicial(BUZZER_PIN);
    sleep_ms(400);
    npClear();

    // Cor Azul
    for (int i = 0; i < sizeof(leds_cerquilha1) / sizeof(leds_cerquilha1[0]); i++) {
        npSetLED(leds_cerquilha1[i], 0, 0, 40); 
        
    }
    npWrite();
    tocar_musicaInicial(BUZZER_PIN);
    sleep_ms(400);
    npClear();

    // Cor Roxo
    for (int i = 0; i < sizeof(leds_cerquilha1) / sizeof(leds_cerquilha1[0]); i++) {
        npSetLED(leds_cerquilha1[i], 40, 0, 40); 
       
    }
    npWrite();
    tocar_musicaInicial(BUZZER_PIN);
    sleep_ms(400);
    npClear();
}

void tocar_beepsimples(uint pin) {
    const uint beep_simples[] = {230};
    const uint duracao_beep[] = {100};
    for (int i = 0; i < sizeof(beep_simples) / sizeof(beep_simples[0]); i++) {
        play_tone(pin, beep_simples[i], duracao_beep[i]);
        
    }
}

// Desenha o tabuleiro na matriz
void desenhaTabuleiro(int cursor_linha, int cursor_coluna) {
    npClear();

    // LEDs do símbolo "#" na matriz 5x5
    const int leds_cerquilha[] = {1, 3, 5, 6, 7, 8, 9, 11, 13, 15, 16, 17, 18, 19, 21, 23};
    for (int i = 0; i < sizeof(leds_cerquilha)/sizeof(leds_cerquilha[0]); i++) {
        npSetLED(leds_cerquilha[i], 80, 80, 80); 
    }

    // Desenha o tabuleiro com "X" (Vermelho) e "O" (Azul)
    for (int linha = 0; linha < 3; linha++) {
        for (int coluna = 0; coluna < 3; coluna++) {
            int i = obterIndice(linha, coluna);
            if (tabuleiro[linha][coluna] == 1) {
                npSetLED(i, 80, 0, 0);  // "X" (vermelho)
            } else if (tabuleiro[linha][coluna] == 2) {
                npSetLED(i, 0, 0, 80);  // "O" (azul)
            }
        }
    }
    
    int cursor_index = obterIndice(cursor_linha, cursor_coluna);
    
    // Verifica se o cursor está sobre um LED já marcado (X ou O)
    if (tabuleiro[cursor_linha][cursor_coluna] == 0) {
        // O LED não está marcado, então coloca a cor verde normal.
        npSetLED(cursor_index, 0, 80, 0);
    } else if (tabuleiro[cursor_linha][cursor_coluna] == 1) {
        // Se o LED estiver marcado com X, mantém a cor vermelha em um tom mais claro.
        npSetLED(cursor_index, 40, 0, 0);
    } else if (tabuleiro[cursor_linha][cursor_coluna] == 2) {
        // Se o LED estiver marcado com O, mantém a cor azul em um tom mais claro.
        npSetLED(cursor_index, 0, 0, 40);
    }
    npWrite();
}

int posicao_vencedora[3][2]; // Armazena as coordenadas dos 3 LEDs vencedores

// Verifica se há um vencedor
int checarVencedor() {
    for (int i = 0; i < 3; i++) {
        if (tabuleiro[i][0] && tabuleiro[i][0] == tabuleiro[i][1] && tabuleiro[i][1] == tabuleiro[i][2]) {
            posicao_vencedora[0][0] = i; posicao_vencedora[0][1] = 0;
            posicao_vencedora[1][0] = i; posicao_vencedora[1][1] = 1;
            posicao_vencedora[2][0] = i; posicao_vencedora[2][1] = 2;
            return tabuleiro[i][0];
        }
        if (tabuleiro[0][i] && tabuleiro[0][i] == tabuleiro[1][i] && tabuleiro[1][i] == tabuleiro[2][i]) {
            posicao_vencedora[0][0] = 0; posicao_vencedora[0][1] = i;
            posicao_vencedora[1][0] = 1; posicao_vencedora[1][1] = i;
            posicao_vencedora[2][0] = 2; posicao_vencedora[2][1] = i;
            return tabuleiro[0][i];
        }
    }
    if (tabuleiro[0][0] && tabuleiro[0][0] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][2]) {
        posicao_vencedora[0][0] = 0; posicao_vencedora[0][1] = 0;
        posicao_vencedora[1][0] = 1; posicao_vencedora[1][1] = 1;
        posicao_vencedora[2][0] = 2; posicao_vencedora[2][1] = 2;
        return tabuleiro[0][0];
    }
    if (tabuleiro[0][2] && tabuleiro[0][2] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][0]) {
        posicao_vencedora[0][0] = 0; posicao_vencedora[0][1] = 2;
        posicao_vencedora[1][0] = 1; posicao_vencedora[1][1] = 1;
        posicao_vencedora[2][0] = 2; posicao_vencedora[2][1] = 0;
        return tabuleiro[0][2];
    }
    return 0;
}

int checarEmpate() {
    for (int linha = 0; linha < 3; linha++) {
        for (int coluna = 0; coluna < 3; coluna++) {
            if (tabuleiro[linha][coluna] == 0) {
                return 0; 
            }
        }
    }
    return 1;
}

// Desenha uma seta na Matriz, apontada para o Display.
void acenderleds_setaDisplay(){
    const int leds_setaDisplay[] = {0, 1, 2, 8, 9, 10, 12, 16, 24};
    for (int cont = 0; cont < 6; cont++){
        tocar_beepsimples(BUZZER_PIN);
        for (int i = 0; i < sizeof(leds_setaDisplay)/sizeof(leds_setaDisplay[0]); i++) {
            npSetLED(leds_setaDisplay[i], 0, 80, 0);  // Acende o LED
        }
        npWrite();
        sleep_ms(1000);
        npClear();
        npWrite();
        sleep_ms(100);
    }
    npClear();
    npWrite();
}

// Desenha uma seta na Matriz, apontada para o Joystick
void acenderleds_setaJoystick(){
    const int leds_setaJoystick[] = {2, 3, 4, 5, 6, 12, 14, 18, 20};
    for (int cont = 0; cont < 8; cont++){
        tocar_beepsimples(BUZZER_PIN);
        for (int i = 0; i < sizeof(leds_setaJoystick)/sizeof(leds_setaJoystick[0]); i++) {
            npSetLED(leds_setaJoystick[i], 0, 80, 0);  // Acende o LED
        }
        npWrite();
        sleep_ms(1000);
        npClear();
        npWrite();
        sleep_ms(100);
    }
    npClear();
    npWrite();
}

// Desenha uma seta na Matriz, apontada para o Botão_B.
void acenderleds_setaBotaoB(){
    const int leds_setaBotaoB[] = {8, 10, 11, 12, 13, 14, 18};
    
    for (int cont = 0; cont < 8; cont++){
        tocar_beepsimples(BUZZER_PIN);
        for (int i = 0; i < sizeof(leds_setaBotaoB)/sizeof(leds_setaBotaoB[0]); i++) {
            npSetLED(leds_setaBotaoB[i], 0, 80, 0);  // Acende o LED
        }
        npWrite();
        sleep_ms(1000);
        npClear();
        npWrite();
        sleep_ms(100);
    }
    npClear();
    npWrite();
}

// Escreve um texto no Display
void imprimir_texto(uint8_t *ssd, char *text[], uint8_t line_count, int x) {
    int y = 0;
    for (uint i = 0; i < line_count; i++) {
        ssd1306_draw_string(ssd, x, y, text[i]);
        y += 8;
    }

    calculate_render_area_buffer_length(&frame_area);
    render_on_display(ssd, &frame_area);
}

void reiniciarJogo() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tabuleiro[i][j] = 0;
        }
    }
    char *jogo_iniciado[] = {
        "                ",
        "                ",
        "Jogo Reiniciado  ",
    };
    imprimir_texto(ssd, jogo_iniciado, count_of(jogo_iniciado), 5);

    jogador_atual = 1;

    npClear();
    npWrite();
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    npInit(LED_PIN);
    npClear();
    stdio_init_all();

    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
    pwm_init_buzzer(BUZZER_PIN);

    while (true){
        char *text_bemvindo[] = {
            "                ",
            "  Bem-vindo ao  ",
            "                ",
            "  Jogo da Velha ",
            "                ",
            " do EmbarcaTech!"
        };
        for(int piscar = 0; piscar < 4; piscar++){
            imprimir_texto(ssd, text_bemvindo, count_of(text_bemvindo), 5);
            acenderLedsCerquilha();
            memset(ssd, 0, ssd1306_buffer_length);
            render_on_display(ssd, &frame_area);
            sleep_ms(50);
        }
        break;
    }

    while(true){
        char *text_instrucoes[] = {
            "              ",
            "              ",
            "              ",
            "   Instrucoes ",
            "              ",
            "     do Jogo  ",
            };
    
        imprimir_texto(ssd, text_instrucoes, count_of(text_instrucoes), 5);
        acenderleds_setaDisplay();
        memset(ssd, 0, ssd1306_buffer_length);
        render_on_display(ssd, &frame_area);

        char *text_instrucao1[] = {
            "              ",
            "              ",
            "Use o Joystick",
            "              ",
            " para se mover",
            "              ",
            "  pela matriz.",
            
        };
        imprimir_texto(ssd, text_instrucao1, count_of(text_instrucao1), 5);
        acenderleds_setaJoystick();
        memset(ssd, 0, ssd1306_buffer_length);
        render_on_display(ssd, &frame_area);

        char *text_instrucao3[] = {
            "              ",
            " Use o botao B",
            "              ",
            "  para marcar ",
            "              " ,
            "   um lugar  ",
            "              ",
            "   da matriz  ",
            
        };
        imprimir_texto(ssd, text_instrucao3, count_of(text_instrucao3), 5);
        acenderleds_setaBotaoB();
        memset(ssd, 0, ssd1306_buffer_length);
        render_on_display(ssd, &frame_area);

        char *jogo_iniciado[] = {
            "                ",
            "                ",
            " Jogo Iniciado  ",
        };
        imprimir_texto(ssd, jogo_iniciado, count_of(jogo_iniciado), 5);

        break;
    }

    int cursor_linha = 1, cursor_coluna = 1; // Cursor começa no meio
    desenhaTabuleiro(cursor_linha, cursor_coluna);

    while (true) {
        

        adc_select_input(0);
        uint adc_y_raw = adc_read();
        adc_select_input(1);
        uint adc_x_raw = adc_read();

        int nova_linha = adc_y_raw * 3 / 4096;
        int nova_coluna = adc_x_raw * 3 / 4096;
        
        desenhaTabuleiro(cursor_linha, cursor_coluna);
        if (nova_linha != cursor_linha || nova_coluna != cursor_coluna) {
            cursor_linha = nova_linha;
            cursor_coluna = nova_coluna;
            desenhaTabuleiro(cursor_linha, cursor_coluna);
        }
        

        if (!gpio_get(BOTAO_B)) {
            if (tabuleiro[cursor_linha][cursor_coluna] == 0) {
                tabuleiro[cursor_linha][cursor_coluna] = jogador_atual;
                jogador_atual = (jogador_atual == 1) ? 2 : 1;

                if (jogador_atual == 1){
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area);
                    char *vez_jogada[] = {
                        "                ",
                        "                ",
                        "Vez do Vermelho",
                    };
                    imprimir_texto(ssd, vez_jogada, count_of(vez_jogada), 5);

                } else if(jogador_atual == 2){
                    char *vez_jogada[] = {
                        "               ",
                        "               ",
                        "  Vez do Azul  ",
                    };
                    imprimir_texto(ssd, vez_jogada, count_of(vez_jogada), 5);
                }
                
                desenhaTabuleiro(cursor_linha, cursor_coluna);
                
                int vencedor = checarVencedor();
                if (vencedor) {
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area);
                    
                    // Pisca apenas os LEDs vencedores
                    if (vencedor == 1) {
                        char *text_vencedor[] = {
                            "                  ",
                            "                  ",
                            "Vermelho venceu   ",
                        };
                        imprimir_texto(ssd, text_vencedor, count_of(text_vencedor), 5);
                    } else if (vencedor == 2) {
                        char *text_vencedor[] = {
                            "               ",
                            "               ",
                            "  Azul venceu ",
                        };
                        imprimir_texto(ssd, text_vencedor, count_of(text_vencedor), 5);
                    }
                    
                    for (int i = 0; i < 5; i++) {
                        npClear();
                        const int leds_cerquilha[] = {1, 3, 5, 6, 7, 8, 9, 11, 13, 15, 16, 17, 18, 19, 21, 23};

                        for (int i = 0; i < sizeof(leds_cerquilha)/sizeof(leds_cerquilha[0]); i++) {
                            npSetLED(leds_cerquilha[i], 80, 80, 80);
                        }
                        npWrite();
                        sleep_ms(300);
                        
                        // Acende apenas os LEDs das posições vencedoras
                        for (int j = 0; j < 3; j++) {
                            int row = posicao_vencedora[j][0];
                            int col = posicao_vencedora[j][1];
                            npSetLED(obterIndice(row, col), vencedor == 1 ? 80 : 0, 0, vencedor == 2 ? 80 : 0);
                        }
                        npWrite();
                        sleep_ms(300);
                        tocar_musicaFinal(BUZZER_PIN);
                    }
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area);
                    reiniciarJogo();

                } else if (checarEmpate()) {
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area);
                    char *text_empate[] = {
                        "            ",
                        "            ",
                        "     Empate ",
                    };
                    imprimir_texto(ssd, text_empate, count_of(text_empate), 5);
                    tocar_musicaFinal(BUZZER_PIN);
                    sleep_ms(1000);
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area);
                    
                    reiniciarJogo();
                }
            }
            sleep_ms(300);
        }
        sleep_ms(100);
    }
}
