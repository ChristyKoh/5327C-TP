#define BALL = 1
#define CAP = 0

int mode = 1;

int leftBase = 0;
int rightBase = 0;


task sensor() {
	for(;;) {

	}
}

task catapult() {
	bool isReady = SensorValue[CatBtn] == 0 ? false : true;
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
