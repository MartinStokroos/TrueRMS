/*
*
* File: TrueRMS.h
* Purpose: Average, RMS, Power and Energy measurement library.
* Version: 1.3.0
* File date: 13-10-2018
* Release Date: 07-11-2018
*
*
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


#ifndef TrueRMS_H_
#define TrueRMS_H_

#include "Arduino.h"

#define ADC_8BIT 8 // ADC bit depth
#define ADC_10BIT 10
#define ADC_12BIT 12

#define BLR_ON 1 // baseline restore switch option
#define BLR_OFF 0

#define SGL_SCAN 1 // single scan mode
#define CNT_SCAN 0 // continuous scanning mode


class Average
{
public:
	void begin(float _range, unsigned char _avgWindow, unsigned char _adcNob, bool _mode);
	void start(void);
	void stop(void);
	void update(int _instVal);
	void publish(void);
	int instVal;
	float average;
	bool acquire;
	bool acqRdy;
private:
	bool mode;
	int avgWindow;
	float scaling;
	short sampleIdx;
	long temp_sumInstVal;
	long sumInstVal;
};



class Rms
{
public:
	void begin(float _range, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode);
	void start(void);
	void stop(void);
	void update(int _instVal);
	void publish(void);
	int instVal;
	float rmsVal;
	int dcBias;
	bool acquire;
	bool acqRdy;
private:
	bool blr; // baseline restoration switch
	bool mode;
	int rmsWindow;
	float const alpha=0.7; // baseline restoration filter constant
	int error;
	short sampleIdx;
	float scaling;
	float scalingSq;
	float msVal;
	float sumInstVal;
	float temp_sumInstVal;
	float temp_sumSqInstVal;
	float sumSqInstVal;
};



class Rms2
{
public:
	void begin(float _range, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode);
	void start(void);
	void stop(void);
	void update(int _instVal);
	void publish(void);
	int instVal;
	float rmsVal;
	int dcBias;
	bool acquire;
	bool acqRdy;
private:
	bool blr;
	bool mode;
	int rmsWindow;
	float const alpha=0.7;
	int error;
	short sampleIdx;
	float scaling;
	float scalingSq;
	float msVal;
	float sumInstVal;
	float temp_sumInstVal;
	float temp_sumSqInstVal;
	float sumSqInstVal;
};



class Power
{
public:
	void begin(float _range1, float _range2, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode);
	void start(void);
	void stop(void);
	void update(int _instVal1, int _instVal2);
	void publish(void);
	float instVal1;
	float instVal2;
	float rmsVal1;
	float rmsVal2;
	int dcBias1;
	int dcBias2;
	float apparentPwr;
	float realPwr;
	float pf;
	float energy;
	bool acquire;
	bool acqRdy;
private:
	bool blr;
	bool mode;
	int rmsWindow;
	float const alpha=0.7;
	int error1;
	int error2;
	short sampleIdx;
	float scaling1;
	float scalingSq1;
	float scaling2;
	float scalingSq2;
	float scaling3;
	float msVal1;
	float msVal2;
	float temp_sumInstVal1;
	float temp_sumSqInstVal1;
	float sumInstVal1;
	float sumSqInstVal1;
	float temp_sumInstVal2;
	float temp_sumSqInstVal2;
	float sumInstVal2;
	float sumSqInstVal2;
	float temp_sumInstPwr;
	float sumInstPwr;
	unsigned long newTime;
	unsigned long oldTime;
	unsigned int dT;
	float eAcc;
};



class Power2
{
public:
	void begin(float _range1, float _range2, unsigned char _rmsWindow, unsigned char _adcNob, bool _blr, bool _mode);
	void start(void);
	void stop(void);
	void update1(int _instVal);
	void update2(int _instVal);
	void publish(void);
	float instVal1;
	float instVal2;
	float rmsVal1;
	float rmsVal2;
	int dcBias1;
	int dcBias2;
	float apparentPwr;
	float realPwr;
	float pf;
	float energy;
	bool acquire;
	bool acqRdy;
private:
	bool blr;
	bool mode;
	int rmsWindow;
	float const alpha=0.7;
	int error1;
	int error2;
	short sampleIdx;
	float scaling1;
	float scalingSq1;
	float scaling2;
	float scalingSq2;
	float scaling3;
	float msVal1;
	float msVal2;
	float temp_sumInstVal1;
	float temp_sumSqInstVal1;
	float sumInstVal1;
	float sumSqInstVal1;
	float temp_sumInstVal2;
	float temp_sumSqInstVal2;
	float sumInstVal2;
	float sumSqInstVal2;
	float temp_sumInstPwr;
	float sumInstPwr;
	unsigned long newTime;
	unsigned long oldTime;
	unsigned int dT;
	float eAcc;
};

#endif /* TrueRMS_H_ */



