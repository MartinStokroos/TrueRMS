/*
*
* File: TrueRMS.cpp
* Purpose: Average, RMS, Power and Energy measurement library.
* Version: 1.3.0
* File Date: 13-10-2019
* Release Date: 07-11-2018
* URL: https://github.com/MartinStokroos/TrueRMS
* License: MIT License
*
*
* Copyright (c) M.Stokroos 2018
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
* modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*/


/*
*
* Changes in version 1.3.0:
* - Added energy meetering functionality in publish function of classes Power and Power2.
*
* Changes in version 1.2.0:
* - Name of constants SGLS and CNTS renamed into SGL_SCAN and CNT_SCAN.
*
* Changes in version 1.1.0:
* - Introduced a software flag to indicate the completion of a single scan acquisition run. This flag is cleared after calling
* publish().
* - Introduced the Rms2 and Power2 classes. Rms2 and Power2 perform better when used in an interrupt service routine by 
* spreading out the processing burden over the different sample time slots within the RMS window. Rms2 occupies one 
* extra sample time slot (window length+1) for doing the baseline restoration calculation (when BLR is on).
* - The method update() of the Power2 class has been split up in: update1() and update2(). update1() is called first and 
* processes the first sample (for example, a sample of the input voltage) and update2() processes the second sample (a sample
* of the input current, or vice versa). Sampling voltage and current usually happens sequentially in a multiplexed ADC.
* Furthermore, an extra sample time slot (window length+1) is occupied by the baseline restoration calculation (when BLR is on).
* - When BLR is on, instVal now returns the baseline restored sample value.
* - The power factor variable is: pf (lowercase letters)
*
*/


#include "TrueRMS.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


//*********** Init functions ***********//

void Average::begin(float _range, unsigned char _avgWindow, unsigned char _adcNob, bool _mode) {
	mode = _mode;
	avgWindow = _avgWindow;
	scaling = _range / (float)(_avgWindow * pow(2, _adcNob));
	temp_sumInstVal = 0;
	sampleIdx=0;
}


void Rms::begin(float _range, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode) {
	mode = _mode;
	rmsWindow = _rmsWindow;
	blr = _blr;
	dcBias = pow(2, _adcNob) / 2;	// dc-bias starting value
	scalingSq = sq(_range) / (float)(_rmsWindow * pow(2, 2*_adcNob));
	temp_sumSqInstVal=0;
	sampleIdx=0;
}


void Rms2::begin(float _range, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode) {
	mode = _mode;
	blr = _blr;
	rmsWindow = _rmsWindow;
	dcBias = pow(2, _adcNob) / 2;	// dc-bias starting value
	scalingSq = sq(_range) / (float)(_rmsWindow * pow(2, 2*_adcNob));
	temp_sumSqInstVal=0;
	sampleIdx=0;
}


void Power::begin(float _range1, float _range2, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode) {
	mode = _mode;
	blr = _blr;
	rmsWindow = _rmsWindow;
	scaling1 = _range1 / (float)pow(2, _adcNob);
	scalingSq1 = sq(_range1) / (float)(_rmsWindow * pow(2, 2*_adcNob));
	scaling2 = _range2 / (float)pow(2, _adcNob);
	scalingSq2 = sq(_range2) / (float)(_rmsWindow * pow(2, 2*_adcNob));
	scaling3 = scaling1*scaling2 / _rmsWindow;
	temp_sumSqInstVal1=0;
	temp_sumSqInstVal2=0;
	temp_sumInstPwr=0;
	sampleIdx=0;
}


void Power2::begin(float _range1, float _range2, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode) {
	mode = _mode;
	blr = _blr;
	rmsWindow = _rmsWindow;
	scaling1 = _range1 / (float)pow(2, _adcNob);
	scalingSq1 = sq(_range1) / (float)(_rmsWindow * pow(2, 2*_adcNob));
	scaling2 = _range2 / (float)pow(2, _adcNob);
	scalingSq2 = sq(_range2) / (float)(_rmsWindow * pow(2, 2*_adcNob));
	scaling3 = scaling1*scaling2 / _rmsWindow;
	temp_sumSqInstVal1=0;
	temp_sumSqInstVal2=0;
	temp_sumInstPwr=0;
	sampleIdx=0;
}


void Average::start() {
	acquire=true;
	acqRdy=false;
}


void Rms::start() {
	acquire=true;
	acqRdy=false;
}


void Rms2::start() {
	acquire=true;
	acqRdy=false;
}


void Power::start() {
	acquire=true;
	acqRdy=false;
}


void Power2::start() {
	acquire=true;
	acqRdy=false;
}


void Average::stop() {
	acquire=false;
}


void Rms::stop() {
	acquire=false;
}


void Rms2::stop() {
	acquire=false;
}


void Power::stop() {
	acquire=false;
}


void Power2::stop() {
	acquire=false;
}




//*********** Periodically called functions ***********//

void Average::update(int _instVal) {
	instVal = _instVal;
	if(acquire) {
		temp_sumInstVal += instVal;
		if(sampleIdx > avgWindow-1) {
			sumInstVal = temp_sumInstVal;
			temp_sumInstVal=0;
			sampleIdx=0;
			if(mode == SGL_SCAN) { // update status bits
				acquire=false;
				acqRdy=true;
			}
		}
		sampleIdx++;
	}
}


void Rms::update(int _instVal) {
	if(acquire) {
		if(blr) {
			instVal = _instVal - dcBias; // subtract DC-offset to restore the baseline.
			temp_sumInstVal += instVal;
		}
		else {
			instVal=_instVal;
		}
		temp_sumSqInstVal += sq((float)instVal);
		if(sampleIdx == rmsWindow) {
			sumSqInstVal=temp_sumSqInstVal;
			if(blr) {
				sumInstVal = alpha*temp_sumInstVal + (1-alpha)*sumInstVal; // calculate running average of sum instant values.
				error = (int)round(sumInstVal / rmsWindow); // calculate error of DC-bias
				dcBias = dcBias + error; // restore baseline
				temp_sumInstVal=0;
				if(mode == SGL_SCAN) { //update status bits
					acquire=false;
					acqRdy=true;
				}
			}
			temp_sumSqInstVal=0;
			sampleIdx=0;
		}
		sampleIdx++;
	}
}


void Rms2::update(int _instVal) {
	if(acquire) {
		if(blr) {
			instVal = _instVal - dcBias; // subtract DC-offset to restore the baseline.
			if(sampleIdx <= rmsWindow) {
			temp_sumInstVal += instVal;
			temp_sumSqInstVal += sq((float)instVal);
			}
			if(sampleIdx == rmsWindow) {
				sumSqInstVal=temp_sumSqInstVal;
				sumInstVal = alpha*temp_sumInstVal + (1-alpha)*sumInstVal; // calculate running average of sum instant values.
			}
			if(sampleIdx == rmsWindow+1) { // additional sample time slot for blr calculation.
				error = (int)round(sumInstVal / rmsWindow); // calculate error of DC-bias
				dcBias = dcBias + error; // restore baseline
				temp_sumInstVal=0;
				temp_sumSqInstVal=0;
				sampleIdx=0;
				if(mode == SGL_SCAN) { // update status bits
					acquire=false;
					acqRdy=true;
				}
			}
		}
		else {
			instVal = _instVal;
			temp_sumSqInstVal += sq((float)instVal);
			if(sampleIdx == rmsWindow) {
				sumSqInstVal=temp_sumSqInstVal;
				temp_sumSqInstVal=0;
				sampleIdx=0;
				if(mode == SGL_SCAN) { // update status bits
					acquire=false;
					acqRdy=true;
				}
			}
		}
	sampleIdx++;
	}
}


void Power::update(int _instVal1, int _instVal2) {
	if(acquire) {
		if(blr) {
			instVal1 = _instVal1 - dcBias1; // subtract DC-offset to restore the baseline.
			temp_sumInstVal1 += instVal1;
			instVal2 = _instVal2 - dcBias2;
			temp_sumInstVal2 += instVal2;
		} // endif(blr)
		else {
			instVal1 = _instVal1;
			instVal2 = _instVal2;
		}
		temp_sumSqInstVal1 += sq(instVal1);
		temp_sumSqInstVal2 += sq(instVal2);
		temp_sumInstPwr += instVal1 * instVal2;
		if(sampleIdx == rmsWindow){
			sumSqInstVal1 = temp_sumSqInstVal1;
			sumSqInstVal2 = temp_sumSqInstVal2;
			sumInstPwr = temp_sumInstPwr;
			if(blr) {
				sumInstVal1 = alpha * temp_sumInstVal1 + (1-alpha) * sumInstVal1; // calculate running average of sum instant values.
				error1 = (int)round(sumInstVal1 / rmsWindow); // calculate error of DC-bias
				dcBias1 = dcBias1 + error1; // restore baseline
				sumInstVal2 = alpha * temp_sumInstVal2 + (1-alpha) * sumInstVal2;
				error2 = (int)round(sumInstVal2 / rmsWindow);
				dcBias2 = dcBias2 + error2;
				temp_sumInstVal1=0;
				temp_sumInstVal2=0;
			} // endif(blr)
			temp_sumSqInstVal1=0;
			temp_sumSqInstVal2=0;
			temp_sumInstPwr=0;
			sampleIdx=0;
			if(mode == SGL_SCAN) { // update status bits
				acquire=false;
				acqRdy=true;
			}
		}
		sampleIdx++;
	} // endif(acquire)
}


void Power2::update1(int _instVal) {
	if(acquire) {
		if(blr) {
			instVal1 = _instVal - dcBias1; // subtract DC-offset to restore the baseline.
			if(sampleIdx <= rmsWindow) {
				temp_sumInstVal1 += instVal1;
				temp_sumSqInstVal1 += sq(instVal1);
			}
			if(sampleIdx == rmsWindow) {
				sumSqInstVal1 = temp_sumSqInstVal1;
				sumInstVal1 = alpha * temp_sumInstVal1 + (1-alpha) * sumInstVal1; // calculate running average of sum instant values.
			}
			if(sampleIdx == rmsWindow+1) { // additional sample time slot for blr calculation
				error1 = (int)round(sumInstVal1 / rmsWindow); // calculate error of DC-bias
				dcBias1 = dcBias1 + error1; // restore baseline
			}
		} // endif(blr)
		else {
			instVal1 = _instVal;
			temp_sumSqInstVal1 += sq(instVal1);

			if(sampleIdx == rmsWindow) {
				sumSqInstVal1 = temp_sumSqInstVal1;
			}
		}
	} // endif(acquire)
}


void Power2::update2(int _instVal) {
		if(acquire) {
			if(blr) {
				if(sampleIdx <= rmsWindow) {
					instVal2 = _instVal - dcBias2;
					temp_sumInstVal2 += instVal2;
					temp_sumSqInstVal2 += sq(instVal2);
					temp_sumInstPwr += instVal1 * instVal2;
				}
				if(sampleIdx == rmsWindow) {
					sumSqInstVal2 = temp_sumSqInstVal2;
					sumInstPwr = temp_sumInstPwr;
					sumInstVal2 = alpha * temp_sumInstVal2 + (1-alpha) * sumInstVal2;
				}
				if(sampleIdx == rmsWindow+1) {
					error2 = (int)round(sumInstVal2 / rmsWindow);
					dcBias2 = dcBias2 + error2;
					temp_sumInstVal1=0;
					temp_sumInstVal2=0;
					temp_sumSqInstVal1=0;
					temp_sumSqInstVal2=0;
					temp_sumInstPwr=0;
					sampleIdx=0;
					if(mode == SGL_SCAN) { // update status bits
						acquire=false;
						acqRdy=true;
					}
				}
			} //endif(blr)
			else {
				instVal2 = _instVal;
				temp_sumSqInstVal2 += sq(instVal2);
				temp_sumInstPwr += instVal1 * instVal2;
				if(sampleIdx == rmsWindow) {
					sumSqInstVal2 = temp_sumSqInstVal2;
					sumInstPwr = temp_sumInstPwr;
					temp_sumSqInstVal1=0;
					temp_sumSqInstVal2=0;
					temp_sumInstPwr=0;
					sampleIdx=0;
					if(mode == SGL_SCAN) { // update status bits
						acquire=false;
						acqRdy=true;
					}
				}
			}
		sampleIdx++;
		} // endif(acquire)
}



//*********** Publishing ***********//
void Average::publish() {
	average = sumInstVal * scaling;
	acqRdy=false;
}


void Rms::publish() {
	msVal = sumSqInstVal * scalingSq;
	rmsVal = sqrt(msVal);
	acqRdy=false;
}


void Rms2::publish() {
	msVal = sumSqInstVal * scalingSq;
	rmsVal = sqrt(msVal);
	acqRdy=false;
}


void Power::publish() {
	msVal1 = sumSqInstVal1 * scalingSq1;
	rmsVal1 = sqrt(msVal1);
	msVal2 = sumSqInstVal2 * scalingSq2;
	rmsVal2 = sqrt(msVal2);
	apparentPwr = rmsVal1 * rmsVal2;
	realPwr = sumInstPwr * scaling3;
	pf = realPwr / apparentPwr;
	newTime = millis();
	dT = newTime - oldTime;
	oldTime = newTime;
	eAcc += sumInstPwr * dT;	// energy accumulator
	energy = eAcc*scaling3 / 1000; // output in Ws when inputs are in Volts and Amps.
	acqRdy=false;
}


void Power2::publish() {
	msVal1 = sumSqInstVal1 * scalingSq1;
	rmsVal1 = sqrt(msVal1);
	msVal2 = sumSqInstVal2 * scalingSq2;
	rmsVal2 = sqrt(msVal2);
	apparentPwr = rmsVal1 * rmsVal2;
	realPwr = sumInstPwr * scaling3;
	pf = realPwr / apparentPwr;
	newTime = millis();
	dT = newTime - oldTime;
	oldTime = newTime;
	eAcc += sumInstPwr * dT;
	energy = eAcc*scaling3 / 1000;
	acqRdy=false;
}
//end of TrueRMS.cpp
