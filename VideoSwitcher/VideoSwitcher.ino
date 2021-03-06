


#include "Timer1.h"
#include "ServoIn.h"


#define CAM1 13
#define CAM2 12
#define CAM3 8
#define CAM4 7
#define PWMIN 9
#define STATUS_LED 2

#define FLASH_STATUS_LED
//#define DEBUG_MODE

#ifdef DEBUG_MODE // if in debug mode we must flash status led
#define FLASH_STATUS_LED
#endif

int currentCam= 0;
int translatedInput = 0;
int nLoop = 0;

#define SERVOS 3

uint16_t g_values[SERVOS];                    // output buffer for ServoIn
uint8_t  g_workIn[SERVOIN_WORK_SIZE(SERVOS)]; // we need to have a work buffer for the ServoIn class

rc::ServoIn g_ServoIn(g_values, g_workIn, SERVOS);

void setup() {
  #ifdef DEBUG_MODE
  // serial stuff, only used in debug process
  Serial.begin(9600);
  #endif

  rc::Timer1::init();

  pinMode(STATUS_LED, OUTPUT);
  pinMode(CAM1, OUTPUT);
  pinMode(CAM2, OUTPUT);
  pinMode(CAM3, OUTPUT);
  pinMode(CAM4, OUTPUT);  
  pinMode(PWMIN, INPUT);


  //Initilize outputs to low
  digitalWrite(CAM1, LOW);
  digitalWrite(CAM2, LOW);
  digitalWrite(CAM3, LOW);
  digitalWrite(CAM4, LOW);
  setCam(CAM1); // turn cam1 on to start
  currentCam = CAM1;

//pwm capture stuff down here
  // only allow pin change interrupts for PB0 (digital pins 9-11)
  PCMSK0 = (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);
  // enable pin change interrupt 0
  PCICR = (1 << PCIE0);
  // start listening
  g_ServoIn.start();

}


void loop() {
  g_ServoIn.update(); // update servo in
  // translate servo input to usable range, since this channel is being used for other functins in my setup, this may not be needed
  translatedInput = translateInputs(g_values[1]);
  
  if (translatedInput < 100){
    setCam(CAM3);
  }
  else if (translatedInput < 250){
    setCam(CAM2);    
  }
  else {
    setCam(CAM1);
  }

  #ifdef FLASH_STATUS_LED
  if(nLoop == 300){
    digitalWrite(STATUS_LED, LOW);
  }
  else if(nLoop > 10000){
    digitalWrite(STATUS_LED, HIGH);
    nLoop = 0;
    #ifdef DEBUG_MODE
    // debug code to see current servo values
    Serial.write("\nThe servo in value is: ");
    Serial.print(String(g_values[1], DEC));
    Serial.print("\n\tTranslated to: ");
    Serial.print(String(translatedInput, DEC));
    Serial.print("\nOutputting to cam: ");
    Serial.print(String(currentCam, DEC));
    #endif
  }
  nLoop++;
  #endif
  
}


int translateInputs(int pwmIn){
  if(pwmIn > 1690){
    return pwmIn - 1700;
  }
  else if (pwmIn > 1308){
    return pwmIn - 1320;
  }
  else{
    return pwmIn - 930;
  }
}


void setCam(int camNum){
  if (camNum != currentCam){
    switch (camNum){
      case CAM1:
      digitalWrite(currentCam, LOW);
      digitalWrite(CAM1, HIGH);
      currentCam = CAM1;
      break;
      case CAM2:
      digitalWrite(currentCam, LOW);
      digitalWrite(CAM2, HIGH);
      currentCam = CAM2;
      break;
      case CAM3:
      digitalWrite(currentCam, LOW);
      digitalWrite(CAM2, HIGH);
      currentCam = CAM3;
      break;
      case CAM4:
      digitalWrite(currentCam, LOW);
      digitalWrite(CAM2, HIGH);
      currentCam = CAM4;
      break;
    }
  }
  currentCam = camNum;
}


// Interrupt handling code below, used for PWM timings
static uint8_t lastB = 0;
// Pin change port 0 interrupt
ISR(PCINT0_vect)
{
  // we need to call the ServoIn ISR here, keep code in the ISR to a minimum!
  uint8_t newB = PINB;
  uint8_t chgB = newB ^ lastB; // bitwise XOR will set all bits that have changed
  lastB = newB;
  
  // has any of the pins changed?
  if (chgB)
  {
    // find out which pin has changed
    if (chgB & _BV(1))
    {
      g_ServoIn.pinChanged(1, newB & _BV(1));
    }
    if (chgB & _BV(2))
    {
      g_ServoIn.pinChanged(2, newB & _BV(2));
    }
    if (chgB & _BV(3))
    {
      g_ServoIn.pinChanged(3, newB & _BV(3));
    }
  }
}
