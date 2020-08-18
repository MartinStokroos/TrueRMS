/*
 * File: AC_powermeter_advanced.ino
 * Purpose: High resolution power measurement example running on interrupt basis.
 * Version: 1.1.0
 * Modified: 18-08-2020
 * Release date: 18-05-2020
 * 
 * URL: https://github.com/MartinStokroos/TrueRMS
 * 
 * License: MIT License
 *
 * Copyright (c) M.Stokroos 2020
 *
 *
 * This example measures the rms-voltage, rms-current, apparent power, real power, power factor and net energy. 
 * 
 * Use external electronic circuitry to scale down the AC-voltage or current to be compliant with the ADC input 
 * voltage range of 0-5V. To measure the line voltage use for example a differential amplifier+levelshifter 
 * circuit with resistive voltage dividers at the input and a current sensor with Hall-sensor 
 * based on the zero flux method.
 * 
 * ALWAYS USE AN ISOLATION TRANSFORMER FOR SAFETY!
 * 
 * The RMS_WINDOW defines the number of samples used to calculate the RMS-value. The length of the RMS_WINDOW must 
 * be specified as a whole number and must fit at least one cycle of the base frequency of the input signal.
 * If RMS_WINDOW + sample-rate does not match with the fundamental frequency of the input signal, slow fluctuations 
 * in the rms values and power readings will occure.
 *
 * Arduino analog input pin mapping:
 * AN0 = scaled ac mains voltage, biased on 2.5V DC.
 * AN1 = scaled ac current, biased on 2.5V DC
 * 
 * Added to version 1.1.0:
 * ADC clip detection and input bias voltage check. The LED will flash if the ADC clips and the LED will turn on 
 * when the AN0 and/or AN1 DC offset is out of range.
 * 
 */


#include <TrueRMS.h>  // https://github.com/MartinStokroos/TrueRMS

#define PIN_LED 13 // Arduino LED
#define LPERIOD 100000    // main loop period in us. In this case 100ms.
#define ADCMAX 500 // ADC peak amplitude
#define BIAS_UPPER_LIM 522 // upper limit for DC bias
#define BIAS_LOWER_LIM 500 // lower limit for DC bias

//#define RMS_WINDOW 60   // rms window of 60 samples, means 1 periods @50Hz
#define RMS_WINDOW 120   // rms window of 120 samples, means 2 periods @50Hz

//scaling of measured quantities
const int gridVoltRange = 750.0; // Vpeak-to-peak full scale.
const float currRange = 111.5; // Apeak-to-peak full scale. Calibration value for the LEM LTS-15-NP current transducer.

volatile int adcVal;
volatile boolean adcClip = false;
volatile unsigned int clipCount;
unsigned long nextLoop;
unsigned char printMuxIdx = 0;
bool publish;
int pubIdx = 0; //counter publishing rate

Power2 powerMon; //create instance of Power2






// ******************************************************************
// Setup
// ******************************************************************
void setup() {
  cli(); // disable interrupts

  // initialize serial communications:
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);

  // initialize ADC for continuous sampling mode
  DIDR0 = 0x3F; // digital inputs disabled for ADC0D to ADC5D
  bitSet(ADMUX, REFS0); // Select Vcc=5V as the ADC reference voltage
  bitClear(ADMUX, REFS1);
  bitClear(ADMUX, MUX0); // selecting ADC CH# 0
  bitClear(ADMUX, MUX1);
  bitClear(ADMUX, MUX2);
  bitClear(ADMUX, MUX3);
  bitSet(ADCSRA, ADEN); // AD-converter enabled
  bitSet(ADCSRA, ADATE); // auto-trigger enabled
  bitSet(ADCSRA, ADIE); // ADC interrupt enabled

  bitClear(ADCSRA, ADPS0); // ADC clock prescaler set to 64 (250kHz). We'll lose some accuracy at this speed...
  bitSet(ADCSRA, ADPS1);
  bitSet(ADCSRA, ADPS2);
  
  bitClear(ADCSRB, ACME); // Analog Comparator (ADC)Multiplexer enable OFF
  bitClear(ADCSRB, ADTS0); // select trigger from timer 1
  bitSet(ADCSRB, ADTS1);
  bitSet(ADCSRB, ADTS2);
  bitSet(ADCSRA, ADSC); // start conversion

  /* TIMER1 configured for phase and frequency correct PWM (mode 8), top=ICR1 */
  bitSet(TCCR1B, CS10);   // prescaler=1
  bitClear(TCCR1B, CS11);
  bitClear(TCCR1B, CS12);
  bitClear(TCCR1A, WGM10);   // mode 8
  bitClear(TCCR1A, WGM11);
  bitClear(TCCR1B, WGM12);
  bitSet(TCCR1B, WGM13);
  // top value. f=fclk/(2*N*TOP)
  ICR1 = 0x0535; //f=6kHz, min 0x03FF;
  #define ICR1_OFFSET 0x09B;

  // init measurements
  powerMon.begin(gridVoltRange, currRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  powerMon.start();

  // enable timer compare interrupt
  bitSet(TIMSK1, TOIE1); // enable Timer1 Interrupt
  
  sei(); // enable interrupts
  nextLoop = micros() + LPERIOD; // Set the loop timer variable.
}




// ******************************************************************
// Main loop
// ******************************************************************
void loop() {
  powerMon.publish();

  // Check ADC signal integrity
  if ( (powerMon.dcBias1 <= BIAS_LOWER_LIM) || (powerMon.dcBias1 >= BIAS_UPPER_LIM) || \ 
  (powerMon.dcBias2 <= BIAS_LOWER_LIM) || (powerMon.dcBias2 >= BIAS_UPPER_LIM) ) { adcClip=true; }
  
  if (adcClip) { digitalWrite(PIN_LED, HIGH); }
  else { digitalWrite(PIN_LED, LOW); }

  clipCount++;
  clipCount &= 0x0F;
  // reset adcClip flag
  if (clipCount==0){
     adcClip=false ;
  }

  if(pubIdx <= 0) {
    publish=true;
    pubIdx=10; //print every sec.
  }

  if(publish) {
    switch (printMuxIdx){
      case 0:
        //Serial.print(powerMon.dcBias1); // for debugging: print bias1 [ADC steps]
    	  Serial.print( round(powerMon.rmsVal1) ); // grid voltage [V]
    	  Serial.print(", ");
    	break;
      case 1:
        //Serial.print(powerMon.dcBias2); // for debugging: print bias2 [ADC steps]
        Serial.print(powerMon.rmsVal2, 1); // current [A]
        Serial.print(", ");
    	break;
      case 2:
        Serial.print( round(powerMon.apparentPwr) ); // [VA]
        Serial.print(", ");
      break;
      case 3:
        Serial.print( round(powerMon.realPwr) ); // [W]
        Serial.print(", ");
      break;
      case 4:
        Serial.print(powerMon.pf, 2);
        Serial.print(", ");
      break;
    	case 5:
        Serial.print( round(powerMon.energy/3600) ); // [Wh]
    	  Serial.println();
      break;
    }
    
    if(printMuxIdx == 5) {
		  Serial.println();
			printMuxIdx=0;
			publish=false;
		}
		else {
			printMuxIdx++;
		}
  }
  pubIdx--;

  while(nextLoop > micros()) {;}  // wait until the end of the time interval
  nextLoop += LPERIOD;  // set next loop time at current time + LOOP_PERIOD
}





/* ******************************************************************
* ADC ISR. The ADC is triggered by Timer1.
* *******************************************************************/
ISR(ADC_vect)
{
  // read the current ADC input channel
  adcVal=ADCL; // store low byte
  adcVal+=ADCH<<8; // store high byte

  if (ADMUX & 1) { // sampling @ 3kHz
    powerMon.update2(adcVal);
    ADMUX = (ADMUX & B11110000) | 0x00;
    // clip detection
    if (powerMon.instVal2 > ADCMAX) {adcClip=true;}
    if (powerMon.instVal2 < -ADCMAX) {adcClip=true;}
  }
  else {
    powerMon.update1(adcVal);
    ADMUX = (ADMUX & B11110000) | 0x01;
    // clip detection
    if (powerMon.instVal1 > ADCMAX) {adcClip=true;}
    if (powerMon.instVal1 < -ADCMAX) {adcClip=true;}
  }
}





/* ******************************************************************
*  Timer1 ISR running at 6000Hz. 
*********************************************************************/
ISR(TIMER1_OVF_vect) {
  // Empty ISR. Required here to trigger the ADC.
  // Important: ADC_vect ISR must be finished before the next call.
}
