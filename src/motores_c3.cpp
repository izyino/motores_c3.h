#include "motores_c3.h"


//----------------------------------------------------------------------
 motores_c3:: motores_c3(uint8_t t0)
{
  xtipostep=t0;
}


//----------------------------------------------------------------------
void  motores_c3::begin() {
  pinMode(0, OUTPUT); pinMode(1, OUTPUT); pinMode(2, OUTPUT); pinMode(3, OUTPUT); //motores 
  pinMode(20, OUTPUT); pinMode(21, OUTPUT); //enable/PWM motores
  pinMode(10, OUTPUT); //beep
  pinMode(8, OUTPUT);  //led

  digitalWrite(0,0); digitalWrite(1,0); digitalWrite(2,0); digitalWrite(3,0);
  digitalWrite(20,1); digitalWrite(21,1);
  digitalWrite(10,0); digitalWrite(8,1);

  ledcAttachPin(10, 2);                              //define pino 10 channel 2 (beep)
  ledcSetup(2, 1000, 8);                             //PWM sempre a 1KHz
  ledcWrite(2, 0);                                   //grava 0 nele (silencia)

  const uint8_t timerNumber = 0;
  hw_timer_t *timer100us = NULL;
  timer100us = timerBegin(timerNumber, 80, true);
  isrTable[timerNumber] = this;
  auto isr = getIsr(timerNumber);
  timerAttachInterrupt(timer100us, isr, false);
  timerAlarmWrite(timer100us, 100, true);
  timerAlarmEnable(timer100us);
}


//----------------------------------------------------------------------
void  motores_c3::runStep(uint32_t steps, uint8_t velstep, boolean cwstep)
{
  xvelstep=600000L/passos[xtipostep]/velstep;
  xvelnow=xvelstep;
  xcwstep=!cwstep;
  if (xcwstep){xfase=-1;}
  if (!xcwstep){xfase=4; if (xtipostep==3){xfase=8;}}
  digitalWrite(0,0); digitalWrite(1,0); digitalWrite(2,0); digitalWrite(3,0);
  xsteps=steps;
}


//----------------------------------------------------------------------
void  motores_c3::runDC(uint8_t n, uint32_t time, uint8_t veldc, boolean cwdc)
{
  xveldc[n]=veldc;
  xcwdc[n]=cwdc;

  if (xtipostep==0){                                 //se motores DC em CN1,
    ledcAttachPin(20+n, n);                          //define channel 0 ou 1, pino 20 ou 21 (motor DC 0 ou 1)
    ledcSetup(n, 1000, 8);                           //PWM sempre a 1KHz
    ledcWrite(n, xveldc[n]);                         //inicia com a velocidade
  }

  ledcWrite(n, int(float(xveldc[n])/100.0*255.0));
  xtime[n]=time*10;
}


//----------------------------------------------------------------------
uint32_t  motores_c3::stepstogo()
{
  return xsteps;
}


//----------------------------------------------------------------------
uint32_t  motores_c3::timetogo(uint8_t n)
{
  return xtime[n]/10;
}


//----------------------------------------------------------------------
void  motores_c3::beep(int xbnum, int xbdur, int xbfreq, int xbinter)
{
  bdur=xbdur*10; bfreq=xbfreq; binter=xbinter*10; bnum=xbnum; 
}


//----------------------------------------------------------------------
void  motores_c3::led(int xlnum, int xldur, int xlinter)
{
  ldur=xldur*10; linter=xlinter*10; lnum=xlnum; 
}


//----------------------------------------------------------------------
void  motores_c3::setms(uint32_t yms)
{
  xms=yms*10;
}


//----------------------------------------------------------------------
uint32_t  motores_c3::getms()
{
  return xms/10;
}


//----------------------------------------------------------------------
void  motores_c3::stopStep()
{
  xsteps=0;
}


//----------------------------------------------------------------------
void  motores_c3::stopDC(uint8_t n)
{
  xtime[n]=0;
}


//----------------------------------------------------------------------
void  motores_c3::stopBeep()
{
  bnum=0;
}


//----------------------------------------------------------------------
void  motores_c3::stopLed()
{
  lnum=0;
}


//----------------------------------------------------------------------
void IRAM_ATTR  motores_c3::onTimer100us()
{
  if (xms>0){xms--;}

  //processa os steps---------------------------------------------------------------------------------
  if (xtipostep!=0){
    if (xsteps>0){
      xvelnow--;
      if (xvelnow==0){
        xvelnow=xvelstep;
        int nf=3;if (xtipostep==3){nf=7;}
        if (xcwstep){xfase++;if (xfase>nf){xfase=0;}}else{xfase--;if (xfase<0){xfase=nf;}}
        motores_c3::go();
        xsteps--;
        if (xsteps==0){
          digitalWrite(0, 0);digitalWrite(1, 0);digitalWrite(2, 0);digitalWrite(3, 0);
        }
      }
    }
  }

  
  //processa os DCs------------------------------------------------------------------------------------
  if (xtipostep==0){
    for (k=0; k<2; k++){
      if (xtime[k]>0){
        if ( xcwdc[k]){digitalWrite(pinosdc[k][0], 0);digitalWrite(pinosdc[k][1], 1);}
        if (!xcwdc[k]){digitalWrite(pinosdc[k][1], 0);digitalWrite(pinosdc[k][0], 1);}
        xtime[k]--;
        if (xtime[k]==0){digitalWrite(pinosdc[k][0], 0);digitalWrite(pinosdc[k][1], 0);digitalWrite(pinosdc[k][2], 0);}
      }
    }
  }


  //processa os BEEPS------------------------------------------------------------------------------------
  if (bnum>0){
    if (bxpri){                           //if is the beginning of cycle to beep,
      bxinter=binter+1; bxdur=bdur;       //init the time variables
      bxpausa=false; bxpri=false;         //with default values or user values
      ledcSetup(2, bfreq, 8);             //
    }                                     // 
    if (!bxpausa && (bxdur>0)) {          //if it is beeping 
      ledcWrite(2, 127);bxdur--;          //keep the beep beeping for bxdur ms
      if(bxdur==0){                       //at end,
        ledcWrite(2, 0);                  //stop the beep and advise 
        bxpausa=true;                     //that pause fase is to be initiated
      }
    }
    if (bxpausa && (bxinter>0)){          //if it is in pause
      ledcWrite(2, 0);bxinter--;          //keep the beep stoped for bxinter ms
      if(bxinter==0){                     //at end, exit from pause, subtract 1 from quantity of desired 
        bxpausa=false;bnum--;bxpri=true;  //beeps and advise to reload the variables for a new cycle
      }
    }
  }


  //processa o LED------------------------------------------------------------------------------------
  if (lnum>0){
    if (lxpri){                           //if is the beginning of cycle to blink led,
      lxinter=linter+1; lxdur=ldur;       //init the time variables
      lxpausa=false; lxpri=false;         //with default values or user valuess
    }                                     // 
    if (!lxpausa && (lxdur>0)) {          //if the led is on (out of pause fase)
      digitalWrite(8, 0);lxdur--;         //keep the led on for lxdur ms
      if(lxdur==0){                       //at end,
        digitalWrite(8, 1);               //turn off the led and advise
        lxpausa=true;                     //that pause fase is to be initiated
      }
    }
    if (lxpausa && (lxinter>0)){          //if the led is off (pause fase)
      digitalWrite(8, 1);lxinter--;       //keep the led off for lxinter ms
      if(lxinter==0){                     //at end, exit from pause, subtract 1 from quantity of desired
        lxpausa=false;lnum--;lxpri=true;  //blinks and advise to reload the variables for a new cycle
      }
    }
  }
}


motores_c3 * motores_c3::isrTable[SOC_TIMER_GROUP_TOTAL_TIMERS];


//----------------------------------------------------------------------
void  motores_c3::go()
{
  switch (xtipostep) {
    case 1:  motores_c3::move1(); break;   //28byj-48, 2048 steps, full step, low torque, low consumption
    case 2:  motores_c3::move2(); break;   //28byj-48, 2048 steps, full step, high torque, high consumption
    case 3:  motores_c3::move3(); break;   //28byj-48, 4096 steps, half step, high torque, high consumption
    case 4:  motores_c3::move4(); break;   //NEMA17,    200 steps, only one mode
  }
}


//----------------------------------------------------------------------
void  motores_c3::move1(){   //28byj-48, 2048 steps, full step, low torque, low consumption
  switch (xfase) {
    case 0:  motores_c3::writ(0,0,0,1); break;   //0x01
    case 1:  motores_c3::writ(0,0,1,0); break;   //0x02
    case 2:  motores_c3::writ(0,1,0,0); break;   //0x04
    case 3:  motores_c3::writ(1,0,0,0); break;   //0x08
  }
}

void  motores_c3::move2(){   //28byj-48, 2048 steps, full step, high torque, high consumption
  switch (xfase) {
    case 0:  motores_c3::writ(1,0,0,1); break;   //0x09
    case 1:  motores_c3::writ(0,0,1,1); break;   //0x03
    case 2:  motores_c3::writ(0,1,1,0); break;   //0x06
    case 3:  motores_c3::writ(1,1,0,0); break;   //0x0C    
  }
}

void  motores_c3::move3(){   //28byj-48, 4096 steps, half step, high torque, high consumption
  switch (xfase) {
    case 0:  motores_c3::writ(1,0,0,1); break;   //0x09
    case 1:  motores_c3::writ(0,0,0,1); break;   //0x01
    case 2:  motores_c3::writ(0,0,1,1); break;   //0x03
    case 3:  motores_c3::writ(0,0,1,0); break;   //0x02
    case 4:  motores_c3::writ(0,1,1,0); break;   //0x06
    case 5:  motores_c3::writ(0,1,0,0); break;   //0x04
    case 6:  motores_c3::writ(1,1,0,0); break;   //0x0C
    case 7:  motores_c3::writ(1,0,0,0); break;   //0x08
  } 
}

void  motores_c3::move4(){   //NEMA17, 200 steps, only one mode
  switch (xfase) {
    case 0:  motores_c3::writ(1,0,1,0); break;   //0x0A
    case 1:  motores_c3::writ(0,1,1,0); break;   //0x06
    case 2:  motores_c3::writ(0,1,0,1); break;   //0x05
    case 3:  motores_c3::writ(1,0,0,1); break;   //0x09    
  }
}


//----------------------------------------------------------------------
void  motores_c3::writ(uint8_t px1, uint8_t px2, uint8_t px3, uint8_t px4)
{
  digitalWrite(0, px1);digitalWrite(1, px2);digitalWrite(2, px3);digitalWrite(3, px4);
}
//----------------------------------------------------------------------




