
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

motores_c3 x(2);                         //0 indica motor(es) DC em CN1 
                                         //1,2,3 ou 4=indica motor de passo em CN1

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
    x.runDC(0, 2500, 100, 1);             //ativa motor DC 0 por 2,5s, vel 100%
    x.runDC(1, 5000, 75, 0);              //ativa motor DC 1 por 5s, vel 50%

  }  

  if (x.xtipostep!=0){                    //se há motor de passo conectado
    if (x.where()==0){                    //e se o motor de passo já chegou ao seu último destino (está parado)
      x.beep(2, 200, 2000, 100);          //emite 2 beep de 200ms cada, 2000Hz, intervalo entre eles de 100ms 
      x.led(20, 100, 50);                 //pisca o LED 20 vezes com 100ms aceso e 50ms apagado
      sent=!sent;                         //inverte o sentido, voltas entre 1 e 10
      voltas=random(1, 11)*2048;          //velocidade entre 2 e 12 RPM, e começa a girar
      x.runStep(voltas, random(2,13), sent);   
    }  
  }
   
  x.setms(6000);
  while(x.getms()>0){}                    //apenas exemplo de espera não blocante (6 segundos)
}
