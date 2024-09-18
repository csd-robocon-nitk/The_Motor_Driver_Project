#include "pid.h"


void PID_init(PID_obj *PID){

	PID->integral = 0.0f;
	PID->prevError= 0.0f;

	PID->derivative = 0.0f;
	PID->prevMeasurement = 0.0f;

	PID->out = 0.0f;

}



float PID_output(PID_obj *PID ,float setpoint ,float measurement){

	float error = setpoint - measurement ;


	float proportional = PID->Kp*error;


	PID->integral = PID->integral+ 0.5f*PID->Ki*(error + PID->prevError)*PID->T;


	PID->derivative = PID->Kd*(error-PID->prevMeasurement);


	PID->out = proportional + PID->integral + PID->derivative;
	
	PID->prevError = error;
	PID->prevMeasurement = measurement;
	
	return PID->out;
}  
