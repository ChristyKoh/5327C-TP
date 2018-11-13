#define LEFT -1
#define RIGHT 1

int goalTicks, avgTicks, error, power;
int kP = .035;
int thresh = 10;
bool isLoading = false;
bool isFiring = false;

void drive(int s){
	motor[LB1] = s;
	motor[LB2] = s;
	motor[LB3] = s;
	motor[RB1] = s;
	motor[RB2] = s;
	motor[RB3] = s;
}

void driveTime(int s, int t) {
	drive(s);
	wait1Msec(t);
	drive(0);
}

void driveDiff(int l, int r) {
	motor[LB1] = l;
	motor[LB2] = l;
	motor[LB3] = l;
	motor[RB1] = r;
	motor[RB2] = r;
	motor[RB3] = r;
}

void driveFor(int dist) { //base drive distance function yay
	while(SensorValue[TrackerL] - goalTicks > thresh) {
		error = SensorValue[TrackerL] - goalTicks;
		// let's say I want to determine kP such that the base starts slowing down at 5"
		// solve the equation for kP:
		// maxPower = tickErrorAt5Inches * kP
		//      127 = 5"*(360*5)/(2*pi*radiusOfGear)
		//			 kP = .035
		drive(error*kP);
	}
	drive(0);
}

int driveDir;
void driveTicks(int goal, int threshold) {
	SensorValue[TrackerL] = 0;
	driveDir = goal<0?-1:1;
	while(abs(goal - SensorValue[TrackerL])> threshold) {
		drive(driveDir*127);
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

void intake() {
	motor[IntakeL] = -127;
	motor[IntakeR] = 127;
}

void outtake(){
	motor[IntakeL] = 127;
	motor[IntakeR] = -127;
}

void stoptake(){
	motor[IntakeL] = 0;
	motor[IntakeR] = 0;
}

task load() {
	if(isFiring || isLoading) return;
	isLoading = true;
	while(SensorValue[CatBtn] == 0){
			motor[Catapult1] = 127;
			motor[Catapult2] = 127;
	}
	motor[Catapult1] = 0;
	motor[Catapult2] = 0;
	isLoading = false;
}

void launch() {
	if(isLoading || isFiring)	return;
	isFiring = true;
	motor[Catapult1] = 127;
	motor[Catapult2] = 127;
	wait1Msec(500);
	while(SensorValue[CatBtn] == 1){
		motor[Catapult1] = 127;
		motor[Catapult2] = 127;
	}
	motor[Catapult1] = 0;
	motor[Catapult2] = 0;
	isFiring = false;
}

//AUTONS
void pew(int side) {
	motor[Catapult1] = 127;
	motor[Catapult2] = 127;
	wait1Msec(1000);
	motor[Catapult1] = 0;
	motor[Catapult2] = 0;
	rot(side * 127);
	delay(26);
	drive(0);
	intake();
	driveDiff(110, 127);
	delay(1700);
	drive(0);
	stoptake();
}
void pew2(int side) {
	motor[Catapult1] = 127;
	motor[Catapult2] = 127;
	wait1Msec(1000);
	motor[Catapult1] = 0;
	motor[Catapult2] = 0; //launch first ball
	intake(); //intake on
	driveTime(127, 1500); //push flag & intake
	drive(-127); // shift back a teensy bit
	delay(70);
	drive(0);
	delay(3000); //wait for intake
	driveTime(-127, 350); //drive back to shoot 2nd flag
	rot(side*-60); //turn a teensy bit
	delay(25);
	drive(0);
	startTask(load); //load
	delay(2000);
	launch(); //launch
	stoptake();
}
void pewpark(int side) {
	pew(side);
	driveDiff(-100, -127);
	delay(1500);
	drive(0);
	/*rot(side * 127);
	delay(150);
	drive(0);
	driveTime(-127,50);*/
}
void park(int side) {
	driveTime(127, 400);
	delay(300);
	rot(side * -127);
	delay(450);
	rot(side* 60);
	delay(70);
	drive(0);
	drive(-127);
	delay(500);
	drive(0);
	delay(100);
	driveTime(127, 1500);
}
void pewplus(int side) {
	pew(side);
	driveTicks(-5000, 50);
	rot(127);
	delay(300);
	rot(0);
	intake();
	driveTime(127,500);
	delay(200);
	driveTime(-127, 500);
}
void parktest(int side) {
	driveTime(127, 1500);
	driveTicks(-8000, 50);
	rot(-127);
	delay(300);
	rot(0);
	driveTime(-127, 300);
	//driveTime(127, 1200);
}
void pewpew(int side) {
	pew(side);
	intake();
	drive(-100); // shift back a teensy bit
	delay(50);
	drive(0); // intake!!
	delay(4000);
	driveDiff(-115, -127); //drive back
	delay(500);
	drive(0);
	delay(600);
	startTask(load); //load
	delay(1000);
	stoptake();
	delay(500);
	launch();
}
