#include<stdio.h>
#include<stdlib.h>



typedef struct{

	float Kp ;  //Proportionality constant
	float Ki;  //Integral constant
	float Kd;  //Derivative constant

	float T;  // sampling time 
		   
	float prevError; // for differentiator
	
	float integral ;
	float derivative;
	
	float prevMeasurement; // for integrator 

	float out;

}PID_obj;

void PID_init(PID_obj *PID);

float PID_output(PID_obj *PID, float setpoint, float measurement);
