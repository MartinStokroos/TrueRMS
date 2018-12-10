# True RMS Library for Arduino
This repository contains the *TrueRMS* C++ library for Arduino. With this library it is possible to determine the average value and the *rms* (root mean square) or *effective* value of a signal. With this library it is also possible to calculate the (vector)power from voltage and current signals. The voltage and the voltage representation of a current, can be measured with the ADC by using additional input circuitry for scaling the measured quantity down to within the 0-5V range, appropriate for the Arduino ADC. This library uses a simple method for unit scaling, only by setting the full scale peak-to-peak value of the ac input signal. The library is easy portable to other platforms.

## Function
The following library classes are implemented:

* `Average`
* `Rms`
* `Power`

*Average* calculates the average value over a number of samples, for example from the ADC, *Rms* calculates the root-mean-square value of a signal and  *Power* calculates the power from two input signals, usually voltage and current.

The following methods exists:

`.begin()`

`.start()`

`.stop()`

`.update()`

`.publish()`

Initialize the class instance with the member function `begin()`. This method must be called at first to set the scaling, the size of the sample window, the number of bits and the acquisition mode.

For the class *Average*, use:

`void begin(float range, unsigned char window, unsigned char nob, bool mode);`

With:

- `range` is the maximum expected full swing of the ac-signal (peak-to-peak value).

- `window` the length of the sample window, expressed in number of samples.

- `nob` is the bit resolution of the input signal, usually this equals the ADC bit depth. Use the predefined constants: `ADC_8BIT`, `ADC_10BIT`, `ADC_12BIT`.

- `mode` sets the mode in continuous scan or single scan. Use the predefined constants: `CNT_SCAN` or `SGL_SCAN`.


The public defined variables are:

`int instVal`, the value of the current acquired sample,

`float average`, the average value result,

`bool acquire` status bit, TRUE if scan is pending.


For the class *Rms*, use:

`void begin(float range, unsigned char window, unsigned char nob, bool blr, bool mode);`

With:

- `range` is the maximum expected full swing of the ac-signal (peak-to-peak value).

- `window` the length of the sample window expressed in number of samples.

- `nob` is the bit resolution of the input signal, usually this is the ADC bit depth. Use the predefined constants: `ADC_8BIT`, `ADC_10BIT` or 
`ADC_12BIT`.

- `blr` sets the automatically baseline restoration function on or off. Use the predifined constants: `BLR_ON` or `BLR_OFF`.

- `mode` sets the mode in continuous scan / single scan. Use predifined constants: `CNT_SCAN` or `SGL_SCAN`.


The public defined variables are:

`int instVal`, the value of the current acquired sample,

`float rmsVal`, the rms value result,

`int dcBias`, the dcBias value in ADC-units, only relevant when BLR_ON,

`bool acquire` status bit, TRUE if scan is pending.


For the class *Power*, use:

`void begin(float range1, float range2, unsigned char window, unsigned char nob, bool blr, bool mode);`

With:

- `range1, range2` is the maximum expected full swing of the ac-signals (peak-to-peak value) of the voltage and current.

- `window` the length of the sample window expressed in number of samples.

- `nob` is the bit resolution of the input signal, usually this is the ADC bit depth. Use the predifined constants: `ADC_8BIT`, `ADC_10BIT` or `ADC_12BIT`.

- `blr` sets the automatically baseline restoration function on or off. Use the predifined constants: `BLR_ON` or `BLR_OFF`.

- `mode` sets the mode in continuous scan / single scan. `CNT_SCAN` or `SGL_SCAN`.


The public defined variables are:

`int instVal1`, the value of the current acquired sample of the voltage,

`int instVal2`, the value of the current acquired sample of the current,

`float rmsVal1`, RMS value1 (voltage)

`float rmsVal2`, RMS value2 (current)

`int dcBias1`, the dcBias1 value in ADC-units, only relevant when BLR_ON,

`int dcBias2`, the dcBias2 value in ADC-units, only relevant when BLR_ON,

`float apparentPwr`, the apparent power,

`float realPwr`, real power,

`float PF`, power factor,

`bool acquire`, status bit, TRUE if scan is pending.


<u>Common Methods</u>

This method starts the acquisition for both, the continuous scan and for single scan:
`void start(void);`

This method stops the acquisition:
`void stop(void);`

Update must be called on basis of a regular time interval to obtain accurate readings. The loop iteration time defines the sample rate. `Value` is the (instantaneous) sample value:
`void update(int Value);`

Publish calculates the output value(s) from the last acquisition run:
`void publish(void);`

## Usage
* First create an instance of the library object, for example we define *gridVolt*:
```
Rms gridVolt;
```
* Initialize the *gridVolt* on some place in your setup function:
```
void setup() {
	...
	gridVolt.begin(700, 40, ADC_10BIT, BLR_ON, CNT_SCAN);`
	...
}
```
The arguments mean:

The full ADC range (0 to 5Volts) represents a signal peak-to-peak value of 700V. This equals a voltage amplitude of 350V or 247.5Vrms for a sine wave.

The rms window is 40 samples, which means that the window covers two 50Hz cycles, when sampling at a rate of 1000 samples/sec.

The ADC bit resolution is 10bit (Arduino UNO).

BLR_ON means that the baseline restoration is switched on. To capture an AC-signal with the ADC, the zero value of the signal must be shifted towards the mid point of the ADC range by adding a DC-offset voltage with the ADC input circuitry. This offset must be corrected afterwards in software by subtracting a constant value from the ADC value. This correction can be done automatically with BLR_ON and calibration is not needed.
In figure 1, the blue line indicates the maximum scaled input signal with a voltage swing of 5V and biased on 2.5V. The green line shows an input signal with an amplitude of 1V and this will measure 1V/sqrt(2) = 0.71Vrms. 

With the option CNT_SCAN, the acquisition is set to the continous mode. The acquisition will restart automatically after the last sampling scan.

![Figure 1](figures/figure1.png)

 * Call `gridVolt.update(int adcVal);` from the main loop or from an Interrupt Service Routine (ISR). Make sure that the loop iterates with 1000Hz.
 
 * Calculate the results with `gridVolt.publish()` and obtain the rms value with: `float Voltage = gridVolt.rmsVal;`
 
``` 
void loop() { // loop must run at 1kHz
	...
	gridVolt.update(adcValue);
	
	counter++;
	if(counter >= 500) { // publish every 0.5s
	Voltage = gridVolt.rmsVal;
	counter = 0;
	}
	...
	while(loop_timer_not_expired) {1}
}
```

## Examples
`Measure_avg.ino` - This example shows how measure the average value of a signal from the ADC input. It can be used for example for measuring noisy dc voltages.

`Measure_rms.ino` -  With this example, the rms-value of the ADC input voltage is determined. 

`AC_powermeter.ino` - This is a complete AC-power measurement application. It needs voltage and a voltage representation of the current as input. It determines the apparent power, real power, power factor and the rms-values of the voltage and current.

## AC measurements with the Arduino
The easiest way to interface ac high voltages with the Arduino ADC is by using a voltage transducer, for example the *LV 25-P* voltage transducer from *LEM USA Inc.* This transducer provides galvanic isolation, scaling and level shifting in a single device. For current sensing, *LEM* also manufactures transducers like the *LEM_LA55-P*, with the same advantages as for the voltage transducer.

If one prefers to build input scaling circuits from discrete components, a detailed design description is given in the application note [tiduay6c.pdf](http://www.ti.com/lit/ug/tiduay6c/tiduay6c.pdf) of the Voltage Source Inverter Reference Design from Texas Instruments Incorporated. Take notice of the warnings! The proposed circuits can be adapted easily to the 0-5V range for the Arduino.

At all times, USE AN ISOLATION TRANSFORMER FOR SAFETY!

## Acknowledgement
A lot of time was saved in developing this library by using the alternative Arduino-IDE *Sloeber*. Sloeber is a wonderful Arduino plugin for Eclipse. Thanks to Jantje and his contributors! 
