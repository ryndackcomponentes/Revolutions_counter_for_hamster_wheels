/*******************************************************************************************
/  Projeto: Contador de voltas para Hamster
/
/  Descrição: Implementa um velocimetro e um contador de pulsos para calculo da velocidade
/  de um hamster em uma rodinha.
/  
/  Autor: Willian Mateus Ferreira dos Santos    Data:14/11/2023 
/
*******************************************************************************************/
#define F_CPU 16000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCD.h"

//Variaveis globais
int voltas=0;
int voltas_total=0;
int32_t i=0;
int voltas_cont=0;
float distancia=0;
float tempo=0;
float raio=0.2;
float velocidade=0;
bool set;

//protótipos das funções.
void calcula_velocidade(float, float);
void Liga_LCD(void);
void configura_contador(void);
void loop(void);
void configura_timer(void);

//Interrupção por estouro do timer0, nela se encontra a base de tempo usada para o calculo 
//da velocidade da rodinha do hamster.
ISR(TIMER0_OVF_vect){
    i++;
    if (i>624999){

        i=0;

        set = 0;
        voltas = 0;
        voltas_cont=0;
        TCNT0 = 0;
        TCCR0B &= ~(1<<CS00);
    }

}

//Interrupção por mudança de estado do pino PB0, aqui se realiza a leitura do sensor TCRT5000
//para se determinar se o hamster completou uma volta completa, caso 
ISR(PCINT0_vect){
    if(PINB & (1<<PB0)){
        _delay_ms(50);
    }
    else{
        voltas +=1;
        voltas_total +=1;
        set = 1;
        tempo = i;
        i = 0;

        _delay_ms(50);
    }
}


//Inicia um display LCD 16x2, configurando-o para operação com 4 bits, em seguida exibe a
//mensagem "contador de voltas" por 1 segundo, antes de limpar o display e exibir "Voltas: " e 
//"Vel: " na primeira e segunda linha respectivamente.
void Liga_LCD(void){
    LCD_init();
    LCD_clear();
    LCD_move_cursor(0,3);
    LCD_write("Contador de");
    LCD_move_cursor(1,5);
    LCD_write("Voltas");
    _delay_ms(2000);
    LCD_clear();
}

//Configura o pino PB0 como entrada, com o pull-up ligado, além disso habilita a interrupção por mu-
//dança de estado do pino PB0.
void configura_contador(void){

    DDRB   &= ~(1<<PB0);
    PORTB  |=  (1<<PB0);

    PCICR  |=  (1<<PCIE0);
    PCMSK0 |=  (1<<PCINT0);
}

//Loop em constante execução. Verifica se houve incremento no numero de voltas e caso o numero de voltas
//tenha aumentado atualiza o display com o novo valor de velocidade em km/h, e a quantidade de voltas atual.
void loop(void){
    char str_distancia[20];
    char str_velocidade[20];
    int vel_int;
    int vel_dec;
    int distancia_int;
    int distancia_dec;
    while(1){

        if(voltas>voltas_cont){

            calcula_velocidade(tempo, raio);
            if(set){
                configura_timer();
                distancia_int = floor(distancia);
                distancia_dec = 100*(distancia-distancia_int);
                sprintf(str_distancia, "Distancia: %d,%d[m]           ", distancia_int, distancia_dec);
                vel_int = floor(velocidade);
                vel_dec = 100*velocidade - 100*vel_int;
                sprintf(str_velocidade, "Vel: %d,%d[km/h] ", vel_int, vel_dec);
                LCD_move_cursor(0,0);
                LCD_write(str_distancia);
                LCD_move_cursor(1,0);
                LCD_write(str_velocidade);
                voltas_cont=voltas;
            }
            else {
                voltas = 0;
                LCD_clear();
                LCD_move_cursor(0,0);
                LCD_write("Voltas:             ");
                LCD_move_cursor(1,0);
                LCD_write("Vel:                ");                
            }
        }
        else {
            sprintf(str_distancia, "%d", distancia);
        }
    }   
}

//Inicializa o timer0 no modo de operação normal desconectado. Essa função é ativada quando a variavel set
//possui valor lógico verdadeiro, caso ela seja falso o timer se manterá desligado.
void configura_timer(void){
    TCCR0A = 0;
    TCCR0B = 0;

    TCCR0B |= (1<<CS00);
    

    TCNT0 = 0;

    TIMSK0 |= (1<<TOIE0);

}

//Recebe o valor de tempo gerado pela interrupção pelo estouro de timer e o valor do raio, transforma o tempo
//para segundos e calcula a velocidade em km/h.
void calcula_velocidade(float tempo, float raio){
    if (voltas > 1){
        tempo = 10*tempo/(625000);
        distancia = (voltas_total)*2*3.1416*raio;
        velocidade = 3.6*2*3.1416*raio/tempo;
    }
    else {
        velocidade = 0;
    }
}

//Função main.
void main(void){   
    SREG   |=  (1<<7);
    voltas  = 1;
    Liga_LCD();
    configura_contador();
    configura_timer();

    loop();
}

