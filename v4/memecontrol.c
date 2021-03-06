#pragma config(Sensor, in1,    catpot,         sensorPotentiometer)
#pragma config(Sensor, in2,    flippot,        sensorPotentiometer)
#pragma config(Sensor, in3,    liftpot,        sensorPotentiometer)
#pragma config(Sensor, dgtl1,  LED,            sensorDigitalOut)
#pragma config(Motor,  port1,           Catapult,      tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           RightLift,     tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           LeftFrontBase, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           LeftBackBase,  tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           Flipper,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           Intake,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           RightBackBase, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           RightFrontBase, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           LeftLift,      tmotorVex393_MC29, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

bool mode = true;	//false;

//PID variables
bool isPIDRunning = false;
int PID_DELAY = 20;
int liftDir = -1;
int holdVal = SensorValue[liftpot];
float kP, kI, kD;
int err, errPrev, errDiff, errSum;
int power;

int thresh = 10;
int FB;
int LR;
int T;

task liftPIDTask() {
	for(;;){
		if (!isPIDRunning) {											//if PID is not running, get outta here
			continue; //do nothing
		}
		else {
			if (liftDir != 0){ 											//if just released button
				liftDir = 0; 													//stop subsequent calls from changing hold pot val
				delay(100); 													//wait for lift to settle
				holdVal = SensorValue[liftpot];
			}

			errPrev = err;

			err = holdVal - SensorValue[liftpot];		// calc error
			errSum += err; 													// calc integral
			errDiff = (errPrev - err) / PID_DELAY; 	// calc differential

			power = kP * err + kI * errSum + kD * errDiff;

			motor[RightLift] = power;
			motor[LeftLift] = power;
		}
		delay(PID_DELAY); //supposedly controlled sample time, can also use timers for better accuracy?
	}
}

task LauncherControl () { //Triggered by button 6U in ball mode (mode == false)

	//Needs to automatically bring the lift all the way down and cap flipper out a bit

	if(SensorValue[catpot] < 2000) {
		while(SensorValue[catpot] < 2700) {
			motor[Catapult] = -127;
		}
		motor[Catapult] = -20;
	}
	else {
		while(SensorValue[catpot] >= 2700) {
			motor[Catapult] = -127;
		}
		motor[Catapult] = 0;
	}
}

//int count = 0;
//int idk = 0;
task main ()
{
	//initialize
	isPIDRunning = false;
	kP = 0.1;
	kI = 0.0;
	kD = 0.0;
	//startTask(liftPIDTask);

while (true) {
	/*
	motor[LeftFrontBase]  = vexRT[Ch3] + vexRT[Ch1];
	motor[LeftBackBase]   = vexRT[Ch3] - vexRT[Ch1];
	motor[RightFrontBase] = vexRT[Ch2] - vexRT[Ch1];
	motor[RightBackBase]  = vexRT[Ch2] + vexRT[Ch1];
	*/

	//Joystick thresh
		FB = 0;
		LR = 0;
		T = 0;
	if (abs(vexRT[Ch3]) > thresh){
		FB = vexRT[Ch3];}
	if (abs(vexRT[Ch4]) > thresh){
		LR = vexRT[Ch4];}
	if (abs(vexRT[Ch1]) > thresh){
		T = vexRT[Ch1];}

	if(vexRT[Btn7U] == 1) {
		if(mode == false) {
			mode = true;
		}
		else {
			mode = false;
		}
		wait1Msec(200);
		}

	if(mode == true) { //CAP MODE
		// X DRIVE
		motor[RightBackBase] = -(FB + T + LR);
		motor[LeftBackBase] = -(FB - T - LR);
		motor[RightFrontBase] = -(FB + T - LR);
		motor[LeftFrontBase]= -(FB - T + LR);

		// LIFT CTRL
		if(vexRT[Btn6U] == 1)
		{
			//isPIDRunning = false;
			liftDir = 1;
			motor[RightLift] = 127;
			motor[LeftLift] = 127;
		}
		else if(vexRT[Btn6D] == 1)
		{
			//isPIDRunning = false;
			liftDir = -1;
			motor[RightLift] = -127;
			motor[LeftLift] = -127;
		}
		else {
			//PID
			/*if(liftDir >= 0) isPIDRunning = true; //PID whoahhh
			else {
				isPIDRunning = false;
				motor[RightLift] = 0;    		// don't hold if dir is down
				motor[LeftLift] = 0;
			}*/

			//NOPID
			if(liftDir > 0) {
				motor[RightLift] = 20;    		// hold pwr at 20 after lifting up
				motor[LeftLift] = 20;
			}
			else {
			  motor[RightLift] = 0;    		// don't hold if dir is down
				motor[LeftLift] = 0;
			}

		}

	if(vexRT[Btn5U] == 1)
		{
			motor[Flipper] = 127;
		}
		else if(vexRT[Btn5D] == 1)
		{
			motor[Flipper] = -127;
		}
		else
		{
			motor[Flipper] = 0;
		}



	}

	if (mode == false) { // BALL MODE
	// X DRIVE REVERSED
	motor[LeftFrontBase] = FB + T + LR;
	motor[RightFrontBase] = FB - T - LR;
	motor[LeftBackBase] = FB + T - LR;
	motor[RightBackBase]= FB - T + LR;

  	if(vexRT[Btn6U] == 1) {
			startTask(LauncherControl);
		}

  //if flippot > 1680 lower flipper
	//if lift high lower it

if(vexRT[Btn5U] == 1)
			{
				motor[Flipper] = 127;
			}
		else if(vexRT[Btn5D] == 1)
			{
				motor[Flipper] = -127;
			}
		else
			{
				motor[Flipper] = 0;
			}

		if(vexRT[Btn6D] == 1)
			{
				motor[Intake] = -127;
			}
		else if(vexRT[Btn8D] == 1)
			{
				motor[Intake] = 127;
			}
		else
			{
				motor[Intake] = 0;
			}

	}



	}
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
