#ifndef PID_AutoTune_v0
#define PID_AutoTune_v0
//#define LIBRARY_VERSION	0.0.1


#include <stdbool.h>

  //commonly used functions **************************************************************************
    void pidat_Init(double*, double*);                       	// * Constructor.  links the Autotune to a given PID
    int pidat_Runtime();						   			   	// * Similar to the PID Compue function, returns non 0 when done
	void pidat_Cancel();									   	// * Stops the AutoTune	
	
	void pidat_SetOutputStep(double);						   	// * how far above and below the starting value will the output step?	
	double pidat_GetOutputStep();							   	// 
	
	void pidat_SetControlType(int); 						   	// * Determies if the tuning parameters returned will be PI (D=0)
	int pidat_GetControlType();							   	//   or PID.  (0=PI, 1=PID)			
	
	void pidat_SetLookbackSec(int);							// * how far back are we looking to identify peaks
	int pidat_GetLookbackSec();								//
	
	void pidat_SetNoiseBand(double);							// * the autotune will ignore signal chatter smaller than this value
	double pidat_GetNoiseBand();								//   this should be acurately set
	
	double pidat_GetKp();										// * once autotune is complete, these functions contain the
	double pidat_GetKi();										//   computed tuning parameters.  
	double pidat_GetKd();										//
	
 
    void pidat_FinishUp();
	bool isMax, isMin;
	double *input, *output;
	double setpoint;
	double noiseBand;
	int controlType;
	bool running;
	unsigned long peak1, peak2, lastTime;
	int sampleTime;
	int nLookBack;
	int peakType;
	double lastInputs[101];
    double peaks[10];
	int peakCount;
	bool justchanged;
	bool justevaled;
	double absMax, absMin;
	double oStep;
	double outputStart;
	double Ku, Pu;
	

#endif

