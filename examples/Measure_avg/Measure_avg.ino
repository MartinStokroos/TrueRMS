/*
 *
 * File: Measure_avg.ino
 * Purpose: TrueRMS library example project
 * Version: 1.0.2
 * Date: 25-05-2019
 * URL: https://github.com/MartinStokroos/TrueRMS
 * License: MIT License
 *
 *
 * This example measures the average value of the input voltage. 
 * AVG_WINDOW defines the number of samples from which the average is calculated.
 *
*/

#include <TrueRMS.h>

#define LPERIOD 1000    // loop period time in us. In this case 1.0ms
#define ADC_INPUT 0     // define the used ADC input channel
#define AVG_WINDOW 20   // window of 20 samples.

unsigned long nextLoop;
int adcVal;
int cnt=0;
float VoltRange = 5.00; // The full scale value is set to 5.00 Volts but can be changed when using an
                        // input voltage divider in front of the ADC.

Average MeasAvg; // Create an instance of Average.


void setup() {
  // run once:
	Serial.begin(115200);
  // configure for continuous scan mode:
	MeasAvg.begin(VoltRange, AVG_WINDOW, ADC_10BIT, CNT_SCAN);
  
  MeasAvg.start(); //start measuring
  
	nextLoop = micros() + LPERIOD; // Set the loop timer variable for the next loop interval.
	}



void loop() {
  // run repeatedly:
	adcVal = analogRead(ADC_INPUT); // read the ADC
	MeasAvg.update(adcVal);
	cnt++;

	if(cnt >= 500) { // publish every 0.5s
		MeasAvg.publish();
		Serial.println(MeasAvg.average,2);
		cnt=0;
}

	while(nextLoop > micros());  // wait until the end of the time interval
	nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
}

// end of Measure_avg.ino

