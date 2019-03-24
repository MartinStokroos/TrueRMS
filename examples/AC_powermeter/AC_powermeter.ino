/*
 *
 * File: AC_powermeter.ino
 * Purpose: TrueRMS library example project
 * Version: 1.0.1
 * Date: 22-03-2019
 * URL: https://github.com/MartinStokroos/TrueRMS
 * License: MIT License
 *
 *
 * This is an example of a complete AC-power meter application, measuring apparent power, real power, power factor plus
 * rms voltage and current. Use external electronic circuitry to scale down the AC voltage or current to a range 
 * between 0 and 5 Volts, suitable for the Arduino ADC input. Use a differential amplifier+level shifter circuit 
 * with resistive voltage dividers at the input to measure the grid voltage. USE AN ISOLATION TRANSFORMER FOR SAFETY!
 * Use for example a current sensor based on the zero flux method with a Hall sensor (DCCT).
 * 
 * The number of samples used to capture the input signal, must be a whole number. The sample window, expressed in 
 * number of samples, must have a length equal to at least one cycle of the input signal. If this is not the case, 
 * a slow fluctuation in the rms and power readings will occure.
 *
*/

#include <TrueRMS.h>

#define LPERIOD 2000    // loop period time in us. In this case 2.0ms
#define ADC_VIN 0       // define the used ADC input channel for voltage
#define ADC_IIN 1       // define the used ADC input channel for current
#define RMS_WINDOW 20   // rms window of 20 samples, means 2 periods @50Hz
//#define RMS_WINDOW 25   // rms window of 25 samples, means 3 periods @60Hz

unsigned long nextLoop;
int acVolt;
int acCurr;
int cnt=0;
float acVoltRange = 700; // 0-5V down scaled peak-to-peak voltage is 700V (=700/2*sqrt(2) = 247.5Vrms max.)
float acCurrRange = 5; // 0-5V downscaled peak-to-peak current is 5A (=5/2*sqrt(2) = 1.77Arms max.)

Power acPower;  // create an instance of Power


void setup() {
	// run once:
	Serial.begin(115200);
 
	acPower.begin(acVoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, SGLS);
  acPower.start();
  
	nextLoop = micros() + LPERIOD; // Set the loop timer variable for the next loop interval.
	}



void loop() {
	// run repeatedly:
	acVolt = analogRead(ADC_VIN); // read the ADC, channel for Vin
	acCurr = analogRead(ADC_IIN); // read the ADC, channel for Iin
	acPower.update(acVolt, acCurr);
	//RmsReading.update(adcVal-512); // for testing w/o baseline restoration. Substract a fixed DC-offset (ADC)value here.

	cnt++;
	if(cnt >= 250) { // publish every 0.5s
		acPower.publish();
		Serial.print(acPower.rmsVal1,1);
		Serial.print(", ");
		Serial.print(acPower.rmsVal2,1);
		Serial.print(", ");
		Serial.print(acPower.apparentPwr,1);
		Serial.print(", ");
		Serial.print(acPower.realPwr,1);
		Serial.print(", ");
		Serial.print(acPower.pf,2);
		Serial.print(", ");
		Serial.print(acPower.dcBias1);
		Serial.print(", ");
		Serial.println(acPower.dcBias2);
		cnt=0;
    acPower.start(); // For single scan. Restart the acquisition after publishing.
	}

	while(nextLoop > micros());  // wait until the end of the time interval
	nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
}

// end of AC_powermeter.ino

