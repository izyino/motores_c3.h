a-17
//
// Exemplo n.1 de utilização da biblioteca motores_c3.h
// emite beeps e piscadas de led, e movimentos repetitivos
// -----------------------------------------------------------
// Pressupõe um motor de passo 28byj48 (poderia ser o Nema-17)
// ou até dois motores DC (n.0 nos pinos 1 e 2 de CN1
// e n.1 nos pinos 3 e 4 de CN1) 
// -----------------------------------------------------------
//


#include <motores_c3.h>
                                         //0 indica motor(es) DC em CN1
motores_c3 x(4);                         //1,2,3 indica motor de passo 28BYJ48 em CN1 
                                         //4 indica motor de passo Nema-17 em CN1 

bool sent=false;
uint32_t voltas;

void setup() {
  x.begin();
  Serial.begin(115200);
}

void loop() {

  if (x.xtipostep==0){                    //se há motor(es) DC conectado(s)
    x.beep(2, 200, 2000, 100);            //emite 2 beep de 200ms cada, 2000Hz, intervalo entre eles de 100ms 
    x.led(20, 100, 50);                   //pisca o LED 20 vezes com 100ms aceso e 50ms apagado
    if (x.timetogo(0)==0){                //se o motor DC n.0 estiver parado
      x.runDC(0, 2500, 100, 1);           //ativa motor DC 0 por 2,5s, vel 100%
    }                                     //
    if (x.timetogo(1)==0){                //se o motor DC n.1 estiver parado
      x.runDC(1, 5000, 75, 0);            //ativa motor DC 1 por 5s, vel 50%
    }                                     //
  }  

  if (x.xtipostep!=0){                    //se há motor de passo conectado
    if (x.stepstogo()==0){                //e se o motor de passo já chegou ao seu último destino (está parado)
      x.beep(2, 200, 2000, 100);          //emite 2 beep de 200ms cada, 2000Hz, intervalo entre eles de 100ms 
      x.led(20, 100, 50);                 //pisca o LED 20 vezes com 100ms aceso e 50ms apagado
      sent=!sent;                         //inverte o sentido, voltas entre 1 e 10
      voltas=random(1, 11)*200;           //velocidade entre 1 e 60 RPM, e começa a girar
      x.runStep(voltas, random(1,61), sent);   
    }  
  }
   
  x.setms(1000);
  while(x.getms()>0){}                    //apenas exemplo de espera não blocante (1 segundos)
}



