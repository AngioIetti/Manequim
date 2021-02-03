#include <math.h>
#include <PID_v1.h> //Controle PID
#include "TimerOne.h"
#include "HX711.h"

//HX711

int porta_temp[12] = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53}; //Pinos de leitura DOUT
int SSCK = 41; //SKC comum para todos amplificadores

HX711 section[12] = {HX;

for (int i = 0; i < 12; i++){
  section[i].begin(porta_temp[i], SSKC);
}

//PWM
float RESOLUÇÃO_PWM = 255;
int porta_pwm[12] = {22, 23, 24, 25, 27, 28, 29, 30, 32, 33, 34, 35};
int cont[12];
int valor_PWM[12];
float freq = 1;

//Variaveis PID
double Setpoint = 32;
double Input[12], Output[12];
double consKp=10, consKi=20, consKd=0.0;  // parametros conservativos
double aggKp=53, aggKi=3.5, aggKd=0.1;  // paramentros agressivos
int Sample_Time=10; //Variable that determines how fast our PID loop runs (em ms)
int RESOLUCAO_PID = 255;

PID myPID[12] =   {PID( &Input[0],  &Output[0], &Setpoint, aggKp, aggKi, aggKd, DIRECT),// DIRECT = Aquecimento; REVERSE=resfriamento
                   PID( &Input[1],  &Output[1], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[2],  &Output[2], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[3],  &Output[3], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[4],  &Output[4], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[5],  &Output[5], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[6],  &Output[6], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[7],  &Output[7], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[8],  &Output[8], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID( &Input[9],  &Output[9], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID(&Input[10], &Output[10], &Setpoint, aggKp, aggKi, aggKd, DIRECT),
                   PID(&Input[11], &Output[11], &Setpoint, aggKp, aggKi, aggKd, DIRECT)}


//====================================================================================================

void setup() 
{
  Serial.begin(9600);

  //PWM

  float periodo_interrupcao = (1 / (freq * RESOLUCAO_PWM)) * 1000000;  //É o período em que a interrupção será executada em us
  
  //Configura a interrupção
  Timer1.initialize(periodo_interrupcao);          //Define o periodo em que a interrupção será executada  
  Timer1.attachInterrupt(gera_PWM);                //É a função que será executada quando a interrupção for requisitada

  //Zera a matriz de contadores, define os pinos e configura o PID
  for (int i = 0; i < 12; i++){   
      cont[i] = 0;
      VALOR_PWM[i] = 0;
      pinMode(porta_PWM[i], OUTPUT);
      pinMode(porta_temp[i], INPUT);
      myPID[i].SetMode(AUTOMATIC);// ou MANUAL, isso é, desligado
      myPID[i].SetOutputLimits(0, 255); // é o PWM
      myPID[i].SetSampleTime(Sample_Time); //Variable that determines how fast our PID loop runs
  }


}

//====================================================================================================

void loop() 
{
  for (int i = 0; i < 12; i++){
    le_tensao(i);
    calcula_temp(i);
    calcula_PID(i);
  }
  
}

//====================================================================================================

void gera_PWM(){
  for (int i = 0; i < 12; i++){
    // valor_PWM[i] = 50; //Debugzinho

    if (cont[i] == valor_PWM[i]){
      digitalWrite(porta_rele[i], HIGH);
    }

    if(cont[i] == RESOLUCAO_PWM){
      cont[i] = 0;
      digitalWrite(porta_rele[i], LOW);
    }

    if(!valor_PWM[i]){
      digitalWrite(porta_rele[i], HIGH);
    }

    cont[i]++;
  }
}

//====================================================================================================

void le_tensao(int i){
  double comp = 0;
  double leitura = 0;
  double buff = 0;

  section[i].power_up();

  //Faz a média de 10 valores desconsiderando os picos

  comp = section[i].get_units();
  leitura = section[i].get_units();

  for (int j = 0; j < 10; j++){
    while (abs(leitura - temp[i]) > erro_max){
      leitura = section[i].get_units();
    }
  
    buff += leitura;
  
  }

  tensao[i] = buff / 10;

}

//====================================================================================================

void calcula_temp(int i){
  temp[i] = 1 * tensão[i];
}

//====================================================================================================

void calcula_PID(int i){
  input[i] = temp[i];
  myPID[i].Compute();
  valor_PWM[i] = output[i];
}

//====================================================================================================
