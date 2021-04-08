#include <HX711.h>

//7 8 9 11 13 15

//Pinos de leitura
int SSCK = 3;
int porta_DOUT_MUX = 9;
int porta_sel_MUX[4] = {4, 5, 6, 7};

//Variaveis de leitura
float erro_max = 50;
float temp;
float tensao;

//Define o HX711
HX711 secao;

//Variavel que define o canal do mux que sera lido
int canal_mux = 11;

//output = 0  => valor cru; output = 1  => valor normalizado
int output = 0;
//====================================================================================================

void setup(){
    //Inicial comunicacao serial
    Serial.begin(9600);

    //Define pinos do mux
    for (int i = 0; i < 4; i++){
        pinMode(porta_sel_MUX[i], OUTPUT);
    } 

    //Seleciona o canal escolhido do mux
    for (int j = 0; j <= 3; j++){
        digitalWrite(porta_sel_MUX[j], ((canal_mux) >> j) & 1);     
        Serial.print(((canal_mux) >> j) & 1);
    }

    //Configura o HX711
    secao.begin(porta_DOUT_MUX, SSCK);
    

   /* if (!output){
        secao.set_scale(2280.f);
        secao.tare();
    }*/

    secao.power_up();
}

//====================================================================================================

void loop(){
   le_tensao();
   Serial.println(tensao);
  // delay(500);
}

//====================================================================================================

void le_tensao(){
    double comp = 0;
    double leitura = 0;
    double buff = 0;

    //Liga o HX711
    //secao.power_up();

    //Faz a média de 10 valores desconsiderando os picos
    comp = secao.get_units();   
    leitura = secao.get_units();
  
    //Se setup = 1 faz media e desconsidera picos; Se setup = 0 apenas le
    if(output){
       for (int j = 0; j < 10; j++){
            /*while (abs(comp - leitura) > erro_max){
                leitura = secao.get_units();
            }*/

            buff += secao.get_units();;
        } 

        tensao = buff / 10;
    }
    else{
      tensao = secao.get_units();
        
    }

    //Desliga o HX711
   // secao.power_down();
}



