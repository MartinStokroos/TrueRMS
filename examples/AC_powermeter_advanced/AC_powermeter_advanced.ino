/*
 *
 * File: AC_powermeter_advanced.ino
 * Purpose: High resolution power measurement example running on interrupt basis.
 * Version: 1.0.1
 * Modified: 18-05-2020
 * Date: 18-05-2020
 * 
 * URL: https://github.com/MartinStokroos/TrueRMS
 * 
 * License: MIT License
 *
 * Copyright (c) M.Stokroos 2020
 *
 *
 * This is an example of a complete AC-power meter application, measuring apparent power, real power, power factor,
 * rms voltage and current. Use external electronic circuitry to scale down the AC-voltage or current to be 
 * compliant with the ADC input voltage range of 0-5V. To measure ac-line voltages, use a differential amplifier+level 
 * shifter circuit with resistive voltage dividers at the input. Use for example a current sensor with a Hall sensor
 * based on the zero flux method. ALWAYS USE AN ISOLATION TRANSFORMER FOR SAFETY!
 * 
 * The RMS_WINDOW defines the number of samples used to calculate the RMS-value. The length of the RMS_WINDOW must 
 * be specified as a whole number and must fit at least one cycle of the base frequency of the input signal.
 * If RMS_WINDOW + sample-rate does not match with the fundamental frequency of the input signal(s), fluctuations 
 * in the rms and power readings will occure.
 * 
 * In this example the powerMon update function is called from the ADC ISR. Voltage and current are alternately sampled. 
 * The ADC samples at 6kHz (exact multiple of 50Hz), so the voltage and the current are both sampled at 3kHz.
 *
 */


#include <TrueRMS.h>  // https://github.com/MartinStokroos/TrueRMS

#define PIN_LED 13 // Arduino LED
#define LPERIOD 100000    // main loop period in us. In this case 100ms.

//#define RMS_WINDOW 60   // rms window of 60 samples, means 1 periods @50Hz
#define RMS_WINDOW 120   // rms window of 120 samples, means 2 periods @50Hz

//scaling of measured quantities
const int gridVoltRange = 750.0; // Vpeak-to-peak full scale.
const float currRange = 111.5; // Apeak-to-peak full scale. Calibration value found for the LEM LTS-15-NP current transducer.

volatile int adcVal;
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

  if(pubIdx <= 0) {
    publish=true;
    pubIdx=10; //print every sec.
  }

  if(publish) {
    digitalWrite(PIN_LED, HIGH);
    switch (printMuxIdx){
      case 0:
    	  Serial.print( round(powerMon.rmsVal1) ); // grid voltage [V]
    	  Serial.print(", ");
    	break;
      case 1:
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
    digitalWrite(PIN_LED, LOW);
    
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
  }
  else {
    powerMon.update1(adcVal);
    ADMUX = (ADMUX & B11110000) | 0x01;
  }
}





/* ******************************************************************
*  Timer1 ISR running at 6000Hz. 
*********************************************************************/
ISR(TIMER1_OVF_vect) {
  // Empty ISR. Required here to trigger the ADC.
  // Important: ADC_vect ISR must be finished before the next call.
}
