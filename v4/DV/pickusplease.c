#pragma config(Sensor, dgtl1,  TrackerL,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  TrackerR,       sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  CatBtn,         sensorTouch)
#pragma config(Motor,  port1,           Catapult1,     tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           LB1,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           LB2,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           LB3,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           IntakeL,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           IntakeR,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           RB1,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           RB2,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           RB3,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          Catapult2,     tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#define BALL = true
#define CAP = false

bool mode = true;

int leftBase = 0;
int rightBase = 0;

int TruSpeed(int value, int power) {
	return sgn(value)*abs(pow(value,power));
}

int goalTicks, avgTicks, error, power;
int kP = .5;
int thresh = 10;

void drive(int s){
	motor[LB1] = s;
	motor[LB2] = s;
	motor[LB3] = s;
	motor[RB1] = s;
	motor[RB2] = s;
	motor[RB3] = s;
}

void driveFor(int dist) { //base drive distance function yay
	avgTicks = (SensorValue[TrackerL] + SensorValue[TrackerR]) >> 1;
	//TODO calculate goalTicks
	while(avgTicks - goalTicks > thresh) {
		error = avgTicks - goalTicks;
		// let's say I want to determine kP such that the base starts slowing down at 5"
		// solve the equation for kP:
		// maxPower = tickErrorAt5Inches * kP
		//      127 = 5"*(360*4)/(2*pi*radiusOfGear)
		//
		drive(error*kP);
	}
	drive(0);
}

void rot(int s){
	motor[LB1] = s;
	motor[LB2] = s;
	motor[LB3] = s;
	motor[RB1] = -s;
	motor[RB2] = -s;
	motor[RB3] = -s;
}

void rotFor(int deg){

}

task sensor() {
	for(;;) {

	}
}

task catapult() {
	bool isReady = false;
	for(;;){

		if(vexRT[Btn6U] == 1) {
			if(isReady) {
				while(SensorValue[CatBtn] == 0){
					motor[Catapult1] = 127;
					motor[Catapult2] = 127;
				}
			} else {
				motor[Catapult1] = 127;
				motor[Catapult2] = 127;
				wait1Msec(500);
				while(SensorValue[CatBtn] == 1){
					motor[Catapult1] = 127;
					motor[Catapult2] = 127;
				}
			}
			motor[Catapult1] = 0;
			motor[Catapult2] = 0;
			isReady = !isReady;
		} else if(vexRT[Btn8U] == 1){
			motor[Catapult1] = 127;
			motor[Catapult2] = 127;
		} else {
			motor[Catapult1] = 0;
			motor[Catapult2] = 0;
		}
	}
}

task differential() {
	for(;;){
		if(vexRT[Btn6D] == 1) { //intake
			motor[IntakeL] = -127;
			motor[IntakeR] = 127;
		} else if(vexRT[Btn8D] == 1) { //outtake
			motor[IntakeL] = 127;
			motor[IntakeR] = -127;
		} else if(vexRT[Btn5U] == 1) { //lift up
			motor[IntakeL] = 127;
			motor[IntakeR] = 127;
		} else if(vexRT[Btn5D] == 1) { //lift down
			motor[IntakeL] = -127;
			motor[IntakeR] = -127;
		} else {
			motor[IntakeL] = 0;
			motor[IntakeR] = 0;
		}
	}
}

task main()
{
	startTask(catapult);
	startTask(differential);

	for(;;){

		//if(vexRT[Btn7U]){
		//	mode = !mode;
		//	wait1Msec(200);
		//}

		//Drive
		leftBase = TruSpeed(vexRT[Ch3] + vexRT[Ch1],3);
		rightBase = TruSpeed(vexRT[Ch3] - vexRT[Ch1],3);
		//leftBase = vexRT[Ch3] + vexRT[Ch1];
		//rightBase = vexRT[Ch3] - vexRT[Ch1];
		motor[LB1] = leftBase;
		motor[LB2] = leftBase;
		motor[LB3] = leftBase;
		motor[RB1] = rightBase;
		motor[RB2] = rightBase;
		motor[RB3] = rightBase;
	}
}