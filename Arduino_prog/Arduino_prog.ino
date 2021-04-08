  #include <math.h>      //Operacoes matematicas
#include <PID_v1.h>    //Controle PID
#include <TimerOne.h>  //Interrupcao
#include <HX711.h>     //Leitura emplificadores HX711

//HX711 e MUX

int porta_DOUT_MUX_inf = 8; //Pino de leitura dos HX da parte inferior
int porta_DOUT_MUX_sup = 9; //Pino de leitura dos HX da parte superior
int porta_sel_MUX[4] = {4, 5, 6, 7}; //Portas de selecao do mux

int SSCK = 3;             //SKC comum para todos amplificadores
float temp[24];           //Armazena as temperaturas
float tensao[24];         //Armazena as temperaturas
float tensao_a[24];
float erro_max = 50000;   //Erro maximo de leitura

HX711 HX_sup[12]; //Amplificadores superiores
HX711 HX_inf[12]; //Amplificadores inferiores

//PWM
float resolucao_PWM = 255;
int porta_rele[12] = {26, 28, 29, 30, 31, 32, 33, 34, 35, 38, 39, 40};
float cont[12];
float valor_PWM[12];
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
                   PID(&Input[11], &Output[11], &Setpoint, aggKp, aggKi, aggKd, DIRECT)};

//Configuracao do print
char input;
int channel = 0;
char *secoes[] = {"Peito", "Barriga", "Costa Superior","Costa Inferior", "Braço Direito", "Braço Esquerdo","Antebraço Direito", "Antebraço Esquerdo",
                  "Cintura", "Nadegas", "Coxa Direita", "Coxa Esquerda", "Canela Direita", "Canela Esquerda", "Tornozelo Direito", "Tornozelo Esquerdo",
                  "Pe Direito", "Pe Esquerdo", "Sola Direita", "Sola Esquerda"};
int tensao_conf[] = {23, 20, 17, 14, 22, 19, 16, 13, 11, 8, 5, 2, 10, 7, 4, 1, 9, 6, 3, 0};


//====================================================================================================

void setup() 
{
  Serial.begin(9600);

  //PWM

  float periodo_interrupcao = (1 / (freq * resolucao_PWM)) * 1000000;  //É o período em que a interrupção será executada em us  
  
  //Define pinos de selecao do mux
  for (int i = 0; i < 4; i++){
    pinMode(porta_sel_MUX[i], OUTPUT);
  }
  
  //Zera a matriz de contadores, define os pinos e configura o PID e os HX711
 
  for (int i = 0; i < 12; i++){   
      cont[i] = 0;
      valor_PWM[i] = 0;
      
      //Seleciona a porta do mux comecando da 4   
        for (int j = 0; j <= 3; j++){
          digitalWrite(porta_sel_MUX[j], ((i + 4) >> j) & 1);     
        }

      //Inicializa HX711 superiores
      if (i != 0 && i != 3 && i != 6 && i != 9){
        HX_sup[i].begin(porta_DOUT_MUX_sup, SSCK);   
        HX_sup[i].power_up();     
        }

      //Inicializa HX711 inferiores 
      HX_inf[i].begin(porta_DOUT_MUX_inf, SSCK);   
      HX_inf[i].power_up();
      
      pinMode(porta_rele[i], OUTPUT);
      digitalWrite(porta_rele[i], LOW);

      myPID[i].SetMode(AUTOMATIC);// ou MANUAL, isso é, desligado
      myPID[i].SetOutputLimits(0, 255); // é o PWM
      myPID[i].SetSampleTime(Sample_Time); //Variable that determines how fast our PID loop runs
  }
  
  for (int i = 0; i < 12; i++){
     digitalWrite(porta_rele[i], HIGH);
  }

 // Serial.println("SolaE TornozeloE CoxaE SolaD TornozeloD CoxaD PeE CanelaE Nadega PeD CanelaD Frente");
  
  //Configura a interrupção
  Timer1.initialize(periodo_interrupcao);          //Define o periodo em que a interrupção será executada 
  Timer1.attachInterrupt(gera_PWM);                //É a função que será executada quando a interrupção for requisitada
  
  //Diz qual canal esta sendo usado
  Serial.println("Mostrando temperatura " + String(secoes[channel]));    


}

//====================================================================================================

void loop() 
{
  for (int i = 0; i < 12; i++){
      le_tensao(i);
      calcula_temp(i);
      calcula_PID(i);
      
     /* if(Serial.available()){
        input = Serial.read();
        
        if(input == 'd' && channel != 23){
          channel++;  
        }
        else if(input == 'a' && channel != 0){
          channel--; 
        }
        Serial.println("Mostrando temperatura " + String(secoes[channel]));  
      }*/
      
    }
    //Teste de calibracao do peito (funcione, por favooooooooooooor)
    float x = tensao[tensao_conf[0]];
    tensao[tensao_conf[0]] = 3.2251*pow(10,-14)*x*x + 1.442*pow(10,-6)*x + 29.84;

  for (int i = 0; i < 20; i++){
    Serial.print(tensao[tensao_conf[i]]);  
    Serial.print(";  ");  
  }
  Serial.println(' ');

}

//====================================================================================================

void gera_PWM(){
  for (int i = 0; i < 12; i++){
     valor_PWM[i] = 120; //Debugzinho

    if (cont[i] == valor_PWM[i]){
      digitalWrite(porta_rele[i], HIGH);
    }

    if(cont[i] == resolucao_PWM){
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

void le_tensao(int canal){

  //Seleciona canal mux, começando do 4
  for (int j = 0; j <= 3; j++){
        digitalWrite(porta_sel_MUX[j], ((canal + 4) >> j) & 1);     
    }
  
  HX_inf[canal].power_up();
  tensao[canal] = HX_inf[canal].get_units();
  HX_inf[canal].power_down();
  
  delay(1); // Sem esse delay por algum motivo a leitura sai errada
  
  if (canal != 0 && canal != 3 && canal != 6 && canal != 9){
    HX_sup[canal].power_up();
    tensao[canal + 12] = HX_sup[canal].get_units();
    HX_sup[canal].power_down();
  }

}

//====================================================================================================

void calcula_temp(int i){
  temp[i] = 1 * tensao[i]; 
}

//====================================================================================================

void calcula_PID(int i){
  Input[i] = temp[i];
  myPID[i].Compute();
  valor_PWM[i] = Output[i];
}

//====================================================================================================

/*
  //Faz a média de 10 valores desconsiderando os picos
  double comp = 0;
  double leitura = 0;
  double buff = 0;
  
  comp = section[canal - 4].get_units();
  leitura = section[canal - 4].get_units();

  for (int j = 0; j < 10; j++){
    while (abs(leitura - comp) > erro_max){
      leitura = section[canal - 4].get_units();
    }
  
    buff += leitura;
  
  }
    //tensao[canal] = buff / 10;
  */
