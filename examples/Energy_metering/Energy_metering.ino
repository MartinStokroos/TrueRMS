/*
 *
 * File: Energy.ino
 * Purpose: TrueRMS library example project
 * Version: 1.0.1
 * Modified: 18-05-2020
 * Date: 10-10-2019
 * URL: https://github.com/MartinStokroos/TrueRMS
 * License: MIT License
 *
 *
 * This is an example of a complete AC-power meter application, measuring energy, apparent power, real power, power factor,
 * rms voltage and current. Use external electronic circuitry to scale down the AC-voltage or current to be 
 * compliant with the ADC input voltage range of 0-5V. To measure line voltages, use a differential amplifier+level 
 * shifter circuit with resistive voltage dividers at the input. Use for example a current sensor with a Hall sensor
 * based on the zero flux method. ALWAYS USE AN ISOLATION TRANSFORMER FOR SAFETY!
 * 
 * The RMS_WINDOW defines the number of samples used to calculate the RMS-value. The length of the RMS_WINDOW must 
 * be specified as a whole number and must fit at least one cycle of the base frequency of the input signal.
 * If RMS_WINDOW + sample-rate does not match with the fundamental frequency of the input signal(s), slow fluctuations 
 * in the rms values and power readings will occure.
 *
*/

#include <TrueRMS.h>

#define LPERIOD 2000    // loop period time in us. In this case 2.0ms
#define ADC_VIN 0       // define the used ADC input channel for voltage
#define ADC_IIN 1       // define the used ADC input channel for current
//#define RMS_WINDOW 20   // rms window of 20 samples, means 2 periods @50Hz
#define RMS_WINDOW 40   // rms window of 40 samples, means 4 periods @50Hz
//#define RMS_WINDOW 25   // rms window of 25 samples, means 3 periods @60Hz
//#define RMS_WINDOW 50   // rms window of 50 samples, means 6 periods @60Hz

unsigned long nextLoop;
int acVolt;
int acCurr;
int cnt=0;
float acVoltRange = 700; // peak-to-peak voltage scaled down to 0-5V is 700V (=700/2*sqrt(2) = 247.5Vrms max).
float acCurrRange = 5; // peak-to-peak current scaled down to 0-5V is 5A (=5/2*sqrt(2) = 1.77Arms max).

Power acPower;  // create an instance of Power


void setup() {  // run once:
	Serial.begin(115200);
  // configure for automatic base-line restoration and continuous scan mode:
	acPower.begin(acVoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  
  acPower.start(); //start measuring
  
	nextLoop = micros() + LPERIOD; // Set the loop timer variable for the next loop interval.
	}



void loop() {
	// run repeatedly:
	acVolt = analogRead(ADC_VIN); // read the ADC, channel for Vin
	acCurr = analogRead(ADC_IIN); // read the ADC, channel for Iin
	acPower.update(acVolt, acCurr);
	//RmsReading.update(adcVal-512);  // without automatic baseline restoration (BLR_OFF), 
	                                  // substract a fixed DC offset in ADC-units here.
	cnt++;
	if(cnt >= 500) { // publish every sec
		acPower.publish();
		Serial.print(acPower.rmsVal1, 1); // [V]
		Serial.print(", ");
		Serial.print(acPower.rmsVal2, 1); // [A]
		Serial.print(", ");
		Serial.print(acPower.realPwr, 1); // [P]
		Serial.print(", ");
		Serial.println(acPower.energy/3600, 2); // [Wh]
		cnt=0;
	}

	while(nextLoop > micros());  // wait until the end of the time interval
	nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
}
// end of Energy.ino
