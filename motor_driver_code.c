#include<stdio.h>
//#include "pid.h"
#include "pid.c"


//For Clockwise rotation 
int pwm1;

//For Anticlockwise rotation
int pwm2;

int interruptPin;

static int dirValue;             //dirValue = 1: Clockwise ; 0: Anticlockwise

//PID controller parameter:
kp = 1.0f;
kd = 1.0f;
ki = 1.0f;
T = 1 ;
setpoint = 100;
measurement = 0;

void motorRun(float speed){
	if(analogRead(pwm1)==HIGH){                             
		analogWrite(pwm1 ,speed); 
		analogWrite(pwm2, 0);
		dirValue = 1;
	}
	else if(analogRead(pwm2)==HIGH){
		analogWrite(pwm2 ,speed);
		analogWrite(pwm1, 0);
		dirValue = 0;
	}
	else{
		analogWrite(pwm2 ,0);
		analogWrite(pwm1, 0);
	}
}

void changeDir(){
	dirValue = ~dirValue;
}

void setDir(int newDir){         //newDir = 1: Clockwise ; 0: Anticlockwise
	dirValue = newDir;
}


void setup(){

	pinMode(pwm1 , OUTOUT);
	pinMode(pwm2 , OUTPUT);

	attachInterrupt(digitalPinToInterrupt(interruptPin), changeDir, RISING);

}

void loop(){

	PID_obj testPID = {kp ,ki ,kd , T};
	PID_init(&testPID);
	float speed = PID_output(&testPID , setpoint , measurement);

	motorRun(speed);
}

	
