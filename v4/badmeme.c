#pragma config(Sensor, in1,    catpot,         sensorPotentiometer)
#pragma config(Sensor, in2,    flippot,        sensorPotentiometer)
#pragma config(Sensor, in3,    liftpot,        sensorPotentiometer)
#pragma config(Sensor, dgtl1,  LED,            sensorDigitalOut)
#pragma config(Motor,  port1,           Catapult,      tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           LeftBase,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           LeftBase2,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           LeftBase3,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           Lintake,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           Lintake2,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           RightBase,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           RightBase2,    tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           RightBase3,    tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          Catapult2,      tmotorVex393_HBridge, openLoop)
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

/*
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
		SensorValue[LED] = true;
		wait1Msec(100);
		SensorValue[LED] = false;
	}
}
*/
//int count = 0;
//int idk = 0;
task main ()
{
	//initialize
	isPIDRunning = false;
	kP = 0.1;
	kI = 0.0;
	kD = 0.0;
	SensorValue[LED] = false;
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
		motor[RightBase] = (FB - T + LR);
		motor[LeftBase] = (FB + T - LR);
		motor[RightBase2] = (FB - T - LR);
		motor[LeftBase2]= (FB + T + LR);
		motor[RightBase3] = (FB - T - LR);
		motor[LeftBase3]= (FB + T + LR);
		// LIFT CTRL


	if(vexRT[Btn6U] == 1)
		{
			motor[Catapult] = 127;
			motor[Catapult2] = 127;
		}
		else if(vexRT[Btn8U] == 1)
		{
			motor[Catapult] = -127;
			motor[Catapult2] = -127;
		}
		else
		{
			motor[Catapult] = 0;
			motor[Catapult2] = 0;
		}

		if(vexRT[Btn6D] == 1)
		{
			motor[Lintake] = -127;
			motor[Lintake2] = 127;
		}
		else if(vexRT[Btn8D] == 1)
		{
			motor[Lintake] = 127;
			motor[Lintake2] = -127;
		}
		else if(vexRT[Btn5D] == 1)
		{
			motor[Lintake] = -127;
			motor[Lintake2] = -127;
		}
		else if(vexRT[Btn5U] == 1)
		{
			motor[Lintake] = 127;
			motor[Lintake2] = 127;
		}
		else
		{
			motor[Lintake] = 0;
			motor[Lintake2] = 0;
		}





	}

	if (mode == false) { // BALL MODE
	// X DRIVE REVERSED
	motor[RightBase] = (FB - T + LR);
		motor[LeftBase] = (FB + T - LR);
		motor[RightBase2] = (FB - T - LR);
		motor[LeftBase2]= (FB + T + LR);
		motor[RightBase3] = (FB - T - LR);
		motor[LeftBase3]= (FB + T + LR);

  	if(vexRT[Btn6U] == 1) {
			//startTask(LauncherControl);
		}

  //if flippot > 1680 lower flipper
	//if lift high lower it

}

	}
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
