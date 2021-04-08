/*
UNO - leitura dos 6 AD simples, com termistores e UR 
Controle tiristor usando o termistor 1
  
================================================================== */

#include <string.h>  
#include <Timer.h> // para o every
Timer tt_Timer; // para o every

//#define DEBUG //comenta se versao final (escreve serial para o Visual Studio entender) 
              //descomenta se quer debugar (escreve na serial print delhado)

int num_tot_canais = 6;

float SetPoint = 30.0;
float Histerese = 0.05;

double a[15], b[15], c[15], d[15], e[15], f[15], g[15];
double V[15], V_med[15], Valor[15], Soma_V[15];
double VV;

const float Resolucao = 1024.0;
const float V_ref = 5.0;

unsigned int contador_aquisicao=0;

int pino_rele = 8;

//==============================================================================================================================================
//==============================================================================================================================================

void setup() 

{
  pinMode(pino_rele, OUTPUT);
  Serial.begin(9600);

  tt_Timer.every(2000, Execucao_Diversa, (void*)2); // executa automaticamente a rotina a cada xxx ms

a[1]= 0.10900668;  b[1]= -1.10025500;  c[1]= 5.28794594;  d[1]= -14.75771141;  e[1]= 40.30831163;  f[1]= -33.78559142 + 0.12 ; 
a[2]= 0.10900668;  b[2]= -1.10025500;  c[2]= 5.28794594;  d[2]= -14.75771141;  e[2]= 40.30831163;  f[2]= -33.78559142 ; 
a[3]= 0.10900668;  b[3]= -1.10025500;  c[3]= 5.28794594;  d[3]= -14.75771141;  e[3]= 40.30831163;  f[3]= -33.78559142 ;
a[4]= 0.10900668;  b[4]= -1.10025500;  c[4]= 5.28794594;  d[4]= -14.75771141;  e[4]= 40.30831163;  f[4]= -33.78559142 ;

a[5] = 0.0; b[5] = 0.0; c[5] = 0.0; d[5] = 0.0; e[5]= 31.446541;  f[5] = -23.82075; // UR
//a[5]= 0.10900668;  b[5]= -1.10025500;  c[5]= 5.28794594;  d[5]= -14.75771141;  e[5]= 40.30831163;  f[5]= -33.78559142 ;

a[6]= 0.10900668;  b[6]= -1.10025500;  c[6]= 5.28794594;  d[6]= -14.75771141;  e[6]= 40.30831163;  f[6]= -33.78559142 ;

//a[9] = 0.0; b[9] = 0.0; c[9] = 0.0; d[9] = 0.0; e[9]= 31.446541;  f[9] = -23.82075; // UR


}

//================================================================================================================================================
//================================================================================================================================================

void loop() 
{
   tt_Timer.update(); // Checa se tem algum "every" para executar 

  // lê Voltagem canais analogicos e acumula numa soma. 
  // Só fará a média quando for mostrar display serial ou Visua Studio
  // está no loop para fazer o maior numero possivel de leituras e assim tira o ruido
 
  for (int i=1; i <= num_tot_canais; i++)
  {
    VV = analogRead(i-1)/Resolucao*V_ref; // retorna a voltagem
    Soma_V[i] = Soma_V[i] + VV;
  }
  contador_aquisicao = contador_aquisicao + 1;
  delay(100);     
}

//================================================================================================================================================
//================================================================================================================================================

 void Execucao_Diversa(void* context)
 { 
   Calcula_Medias_e_Aplica_Calibracao(); 
   Acionamento_Rele();
   
   for (int j=1; j <= num_tot_canais; j++)
   {  
       #ifdef DEBUG // escreve no Monitor Serial detalhado, bom para debug
            Serial.print("  Ch");  Serial.print(j);  Serial.print("= ");  
            Serial.print(Valor[j],1);
            Serial.print("; "); 

      #else        // escreve no Monitor Serial para o Visual Studio
           Serial.print(Valor[j]);
          //Serial.print(" "); // se quiser usar o plotar do arduino 
           Serial.print("; "); // se quiser saida para Visual Studio           
      #endif
            
   }
   Serial.println();   
 }


// ========================================================================================================================================

void Calcula_Medias_e_Aplica_Calibracao() 
{
  // Extrai a media das tensoes já lidas e aplica calibracao
     
  for (int i=1; i <= num_tot_canais; i++)
  {
    V_med[i] = Soma_V[i] / contador_aquisicao;  // extrai a media da soma lida no loop
   // V_med[k] = 3.3;
    Valor[i] = Calibrar_tensao(V_med[i], i); // retorna o valor da temperatura, etc
    Soma_V[i] = 0.0;
  }
  
  contador_aquisicao = 0;     
}

//================================================================================================================================================

void Acionamento_Rele() // chamado dentro Escreve no Monitor Serial
{
  digitalWrite(pino_rele,HIGH); 
  
  if ( Valor[1] <= (SetPoint - Histerese))
   {
     digitalWrite(pino_rele,HIGH);    
     Serial.println(Valor[1]);  // aciona rele 1
   }
   
   if ( Valor[1] >= (SetPoint + Histerese))
   {
     digitalWrite(pino_rele,LOW);      // desliga rele 1
   }

}
 
//================================================================================================================================================

double Calibrar_tensao(double tensao, int canal) 
{
  double Valor_calibrado;
  Valor_calibrado =    a[canal]*pow(tensao, 5) +
                       b[canal]*pow(tensao, 4) +
                       c[canal]*pow(tensao, 3) +
                       d[canal]*pow(tensao, 2) +
                       e[canal]*pow(tensao, 1) +
                       f[canal]*pow(tensao, 0);
                  
  return Valor_calibrado; 
}

//======================================================================================================
