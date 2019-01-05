#include "robot-config.h"
#define RIGHT 1
#define LEFT -1
#define RED -1
#define BLUE 1
#define BACK -1

//namespace vex;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*        Description: Competition template for VCS VEX V5                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//Creates a competition object that allows access to Competition methods.
vex::competition    Competition;


/////////////////////////////////////// VARIABLES /////////////////////////////////////////////////////////////

//mode
bool isCatapultReady = false;
bool isBraking = false;
bool amIBlue = true;
bool areLEDsOn = false;
int isBallMode = 1; //ball mode

//sensor
int catVelocity;
int motOriginal;
int motOriginal2;

//base
int thresh = 10;
int FB;
int LR;
int T;
double brakePosition[4] = {0.0, 0.0, 0.0, 0.0}; //BL, BR, CR, CL

double BLpos, BRpos, CRpos, CLpos;

int flipRot = 0;

/////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////

void sleep(int t) {
    vex::task::sleep(t);
}

void toggleMode() {
    if(Brain.timer(vex::timeUnits::msec) > 300){
        isBallMode = -isBallMode;
        if(isBallMode == 1) {
            Brain.Screen.printAt(10,40,"Mode: Ball Mode %b", isBallMode);
        } else {
            Brain.Screen.printAt(10,40,"Mode: Cap Mode %b", isBallMode);
        }
        Brain.resetTimer();
    }
}

void intake() {
    INTAKE.spin(vex::directionType::fwd, 200, vex::velocityUnits::rpm);
    INTAKE2.spin(vex::directionType::fwd, 200, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Intake running at 200rpm");
}

void outtake() {
    INTAKE.spin(vex::directionType::rev, 200, vex::velocityUnits::rpm);
    INTAKE2.spin(vex::directionType::rev, 200, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Intake running at 200rpm");
}

void intakeStop() {
    INTAKE.stop();
    INTAKE2.stop();
    Brain.Screen.printAt(10,40,"Intake stopped.");
}

void catgo() {
    CATAPULT.spin(vex::directionType::fwd,200,vex::velocityUnits::rpm);
}

void catstop() {
    CATAPULT.stop();
}

void flipUp() {
    FLIPPER.spin(vex::directionType::fwd, 80, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
    //if(FLIPPER.rotation(vex::rotationUnits::deg) < 5) FLIPPER.startRotateTo(0,vex::rotationUnits::deg);
}

void flipDown() {
    FLIPPER.spin(vex::directionType::rev, 80, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
}

void flipStop() {
    FLIPPER.stop();
    flipRot = FLIPPER.rotation(vex::rotationUnits::deg);
}

void flipOne() {
    flipRot-=180;
    Brain.Screen.printAt(10,80,"FlipRot is ", flipRot);
    FLIPPER.startRotateTo(flipRot, vex::rotationUnits::deg, 75, vex::velocityUnits::pct);
}

void drive(int vel) { // + fwd - rev
    B_B_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_B_R.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
    B_M_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_M_R.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
    B_C_R.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_C_L.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
}

void driveRot(int deg, bool waitForCompletion=false, int vel=200) {
    if(waitForCompletion){
        B_B_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        B_B_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void driveTo(int deg, bool waitForCompletion=false, int vel=200) {
    if(waitForCompletion){
        B_B_L.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.rotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.rotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.rotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        B_B_L.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.startRotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.startRotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.startRotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void strafe(int vel) { //+ right - left
    B_B_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_B_R.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_M_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_M_R.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_C_R.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
    B_C_L.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
}

void strafeRot(int deg, bool waitForCompletion=false, int vel=200) {
    if(waitForCompletion){
        B_B_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        B_B_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

int stepVel;
int stepDelay;

void accel(int targetVel, int time=70, int step=10, bool isStrafe=false, bool isRot=false) { //speed up from zero to goal
    stepVel = 0;
    stepDelay = time/step;
    if(targetVel<0) step = -step;
    if(isStrafe){
        while (abs(stepVel) < abs(targetVel)) {
            strafe(stepVel);
            sleep(stepDelay); //wait stepDelay secs before 
            stepVel += step;
        }
    } else if(isRot){
        while (abs(stepVel) < abs(targetVel)) {
            driveRot(stepVel);
            sleep(stepDelay); //wait stepDelay secs before 
            stepVel += step;
        }
    } else {
        while (abs(stepVel) < abs(targetVel)) {
            drive(stepVel);
            sleep(stepDelay); //wait stepDelay secs before 
            stepVel += step;    
        }
    }
}

void driveAccel(int vel, int timeAcc, int timeDrive) {
    //accel to target speed in timeAcc ms
    //drive for timeDrive additional ms
    accel(vel, timeAcc, 10);
    sleep(timeDrive);
    drive(0);
}

void rotAccel(int vel, int timeAcc, int timeDrive) {
    //accel to target speed in timeAcc ms
    //drive for timeDrive additional ms
    accel(vel, timeAcc, 10, false, true);
    sleep(timeDrive);
    drive(0);
}

void strafeAccel(int vel, int timeAcc, int timeDrive) {
    accel(vel, timeAcc, 10, true, false);
    sleep(timeDrive);
    drive(0);
}

void rotFast(int vel) { //+ right - left
    B_B_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_B_R.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_M_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_M_R.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_C_R.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    B_C_L.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
}

void rot(int deg, bool waitForCompletion=false, int vel=200) { 
    if(waitForCompletion){
        B_B_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        B_B_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void rotTo(int deg, bool waitForCompletion=false, int vel=200) { 
    if(waitForCompletion){
        B_B_L.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        B_B_L.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_B_R.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_L.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_M_R.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_R.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        B_C_L.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void stopBase() {
    B_B_L.stop();
    B_B_R.stop();
    B_M_L.stop();
    B_M_R.stop();
    B_C_R.stop();
    B_C_L.stop();
}

void rotFor(int vel, int time){
    rotFast(vel);
    sleep(time);
    stopBase();
}

void driveFor(int vel, int time) {
    drive(vel);
    sleep(time);
    stopBase();
}

void strafeFor(int vel, int time) {
    strafe(vel);
    sleep(time);
    stopBase();
}

void align() {
    if(amIBlue) Vision.takeSnapshot(CAPBLUE);
    else Vision.takeSnapshot(CAPRED);
    if (Vision.largestObject.exists && Vision.largestObject.width>5) {   
    }
}

void trebuchet() {
    if(isCatapultReady) {
        //catapult down, fire
       // CATAPULT.RotateFor(1,vex::rotationUnits::rev,200,vex::velocityUnits::rpm);
        CATAPULT.spin(vex::directionType::fwd, 200,vex::velocityUnits::rpm);
        Brain.Screen.printAt(10,40, "Catapult Firing");
        sleep(500);
        CATAPULT.stop();
        isCatapultReady = false;
    } else {
        //catapult up, start moving arm
        CATAPULT.spin(vex::directionType::fwd, 200, vex::velocityUnits::rpm);
        Brain.Screen.printAt(10,40, "Catapult Running");
        while(Bumper.value() == 1){
            sleep(20);
        }
        CATAPULT.stop();
        isCatapultReady = true;
    }
}

/*
void brake() {
    if(isBraking) {
        //catapult down, fire
       // CATAPULT.RotateFor(1,vex::rotationUnits::rev,200,vex::velocityUnits::rpm);
        CATAPULT.spin(vex::directionType::rev, 200,vex::velocityUnits::rpm);
        Brain.Screen.printAt(10,40, "Releasing Brake");
        sleep(200);
        CATAPULT.stop();
        isBraking = false;
    } else {
        //catapult up, start moving arm
        CATAPULT.spin(vex::directionType::rev, 200, vex::velocityUnits::rpm);
        Brain.Screen.printAt(10,40, "Releasting Brake");
        while(Braker.value() == 1){
            sleep(320);
        }
        CATAPULT.stop();
        isBraking = true;
    }
}*/

void trebStop() {
    //called when bumper is pressed
    CATAPULT.stop();
    Brain.Screen.printAt(10,40, "Ready Position Reached");
    isCatapultReady = true;
}

void brakeStop() {
    //called when bumper is pressed
    CATAPULT.stop();
    Brain.Screen.printAt(10,40, "Brake Down");
    isBraking = true;
}
///////////////////////////////////////// TASKS /////////////////////////////////////////////////////////////////

int launchCallback() {
    for(;;) {
        Controller.ButtonR1.pressed(trebuchet);
        //Brain.Screen.printAt(10,150,"Launch Callback Running...");
        vex::task::sleep(100);
    }
    return 1;
}

/*
int brakeCallback() {
    for(;;) {
        Controller.ButtonL1.pressed(brake);
        //Brain.Screen.printAt(10,150,"Launch Callback Running...");
        vex::task::sleep(100);
    }
    return 1;
}*/

/*int brakeBase() {
    for(;;){
        if(isBraking){
            B_B_L.startRotateTo(BLpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            B_B_R.startRotateTo(BRpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            B_C_R.startRotateTo(CRpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            B_C_L.startRotateTo(CLpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            Brain.Screen.printAt(10, 120, "BL: %.2f", B_B_L.rotation(vex::rotationUnits::deg));
        }
        sleep(10);
    }
    return 1;
}*/

int mainCallback() {
    for(;;){
      //Controller.ButtonLeft.pressed(toggleLEDs);  
      
      Controller.ButtonRight.pressed(toggleMode);
      //Controller.ButtonRight.released(delayLEDsOff);

      Controller.ButtonR2.pressed(intake);
      Controller.ButtonL2.pressed(outtake);
      Controller.ButtonR2.released(intakeStop);
      Controller.ButtonL2.released(intakeStop);
      Controller.ButtonA.pressed(catgo);
      Controller.ButtonA.released(catstop);
      
      Controller.ButtonY.pressed(flipUp);
      Controller.ButtonY.released(flipStop);
      Controller.ButtonX.pressed(flipDown);
      Controller.ButtonX.released(flipStop);
      Controller.ButtonL1.pressed(flipOne);
        
      Brain.Screen.printAt(0,150,"Gyro: %.2f", Gyro.value(vex::rotationUnits::deg));
      Brain.Screen.printAt(0, 120, "TESTING");
        
      sleep(20);
    }
}

vex::task launchTask = vex::task(launchCallback);
//vex::task brakeTask = vex::task(brakeCallback);
vex::task mainTask = vex::task(mainCallback);
//vex::task baseTask = vex::task(brakeBase);

void toggleBrake() {
    if(Brain.timer(vex::timeUnits::msec) > 300){
        isBraking = !isBraking;
        if(isBraking) {
            BLpos = B_B_L.rotation(vex::rotationUnits::deg);
            BRpos = B_B_R.rotation(vex::rotationUnits::deg);
            CRpos = B_C_R.rotation(vex::rotationUnits::deg);
            CLpos = B_C_L.rotation(vex::rotationUnits::deg);
            //baseTask.resume();
            Brain.Screen.printAt(10,70,"Braking: %f", BLpos);
            Brain.Screen.printAt(10,100,"rotation: %f", B_B_L.rotation(vex::rotationUnits::deg));
        } else {
            //baseTask.suspend();
            Brain.Screen.printAt(10,70,"Braking: %b", isBraking);
        }
        Brain.resetTimer();
    }
}

///////////////////////////////////////// AUTONS /////////////////////////////////////////////////////////////////

void park() {
    // drives forward until parks
    while(Gyro.value(vex::rotationUnits::deg) > 10) {
        drive(100);
    }
    drive(0);
}

void newHopefullyReliable(int side = RIGHT) {
    intake();
    //driveAccel(200, 400, 600);
    driveRot(1160);
    sleep(1500);
    driveRot(-1140);
    sleep(1500);
    intakeStop();
    drive(0);
}

void oleReliable(int side = RIGHT) {
    // shoot preload, toggle bottom flag
    //isCatapultReady = true;
    intake();
    driveFor(200, 700);
    //driveAccel(150, 1000, 600);
    driveFor(-200, 1000);
    sleep(300);
    intakeStop();
    driveFor(50, 200);
    rot(side * 275, false, 50); 
    sleep(1000);
    //strafeFor(side * 200, 400); //slide to wall
    //strafeAccel(vel, timeacc, timeafteracc)
    //strafeAccel(side * -100, 700, 0);
    //driveAccel(100, 300, 700);
    driveFor(120, 700);
    rot(side * -10);
    sleep(200); 
    trebuchet(); // launch ball
    sleep(200); 
    trebuchet();
    sleep(200); 
    rot(side * 15);
    trebuchet();
    sleep(200);
    intake();
    driveAccel(150, 1000, 500);
    sleep(200);
    driveAccel(-150, 1000, 500);
    intakeStop();
}

void ezBackAuton(int side = RIGHT) {
    intake();
    driveRot(1160);
    sleep(1500);
    rot(side * 280);
    sleep(1100);
    intakeStop();
    driveFor(150, 1000); //park
}

void BackCapPark(int side = RIGHT) {
    intake();
    driveRot(1160);
    sleep(1500);
    rot(side * 280);
    sleep(1100);
    intakeStop();
    driveFor(150, 1000); //
}

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*                                                                           */
/*  You may want to perform some actions before the competition starts.      */
/*  Do them in the following function.  You must return from this function   */
/*  or the autonomous and usercontrol tasks will not be started.  This       */
/*  function is only called once after the cortex has been powered on and    */ 
/*  not every time that the robot is disabled.                               */
/*---------------------------------------------------------------------------*/

void pre_auton( void ) {
  // All activities that occur before the competition starts
  // Example: clearing encoders, setting servo positions, ...
    isBallMode = true;
    isCatapultReady = false;
    isBraking = false;
    amIBlue = true;
    areLEDsOn = false;
    
    Gyro.startCalibration();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void autonomous( void ) {
    Brain.Screen.print("Robot is in Autonomous mode");
    //oleReliable(RIGHT); //front auton
    //newHopefullyReliable(LEFT); //front auton
    //backCapPark(LEFT);   //back auton
    ezBackAuton(LEFT);
    //test();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/
float leftside;
float rightside;

void usercontrol( void ) {
  // User control code here, inside the loop
    
  Gyro.startCalibration();
  while(Gyro.isCalibrating()){
	  sleep(20);
  }
  sleep(1000);
    while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo 
    // values based on feedback from the joysticks.
      
      //Controller.ButtonL2.pressed(toggleBrake);
      
      
      //TANK DRIVE
      //leftside = isBallMode * Controller.Axis3.value()*1.6;
      //rightside = isBallMode * Controller.Axis2.value()*1.6;
      
      
      //ARCADE DRIVE
      FB = Controller.Axis3.value();
      //LR = Controller.Axis4.value();
      T = (int)(Controller.Axis1.value());
      
      //linear
      //leftside = isBallMode * (FB + T) * 1.6; //scale to 200rpm
      //rightside = isBallMode * (FB - T) * 1.6;
      
      //quadratic
      //leftside = copysign((isBallMode * 0.012 * pow((FB+T),2)), FB+T); 
      //rightside = copysign((isBallMode * 0.012 * pow((FB-T),2)), FB-T);
      
      //conditional
      leftside = isBallMode * (1.6 * FB + 0.012 * pow(T,2));
      rightside = isBallMode * (1.6 * FB - 0.012 * pow(T,2));
      
      B_B_L.spin(vex::directionType::fwd,leftside,vex::velocityUnits::rpm);
      B_B_R.spin(vex::directionType::rev,rightside,vex::velocityUnits::rpm);
      B_M_L.spin(vex::directionType::fwd,leftside,vex::velocityUnits::rpm);
      B_M_R.spin(vex::directionType::rev,rightside,vex::velocityUnits::rpm);
      B_C_R.spin(vex::directionType::fwd,leftside,vex::velocityUnits::rpm);
      B_C_L.spin(vex::directionType::rev,rightside,vex::velocityUnits::rpm);
/*
      B_B_L.spin(vex::directionType::fwd,isBallMode*2*(FB + isBallMode*T + LR),vex::velocityUnits::rpm);
      B_B_R.spin(vex::directionType::rev,isBallMode*2*(FB - isBallMode*T - LR),vex::velocityUnits::rpm);
      B_M_R.spin(vex::directionType::rev,isBallMode*2*(FB - isBallMode*T - LR),vex::velocityUnits::rpm);
      B_M_L.spin(vex::directionType::fwd,isBallMode*2*(FB + isBallMode*T + LR),vex::velocityUnits::rpm);
      B_C_R.spin(vex::directionType::fwd,isBallMode*2*(FB + isBallMode*T - LR),vex::velocityUnits::rpm);
      B_C_L.spin(vex::directionType::rev,isBallMode*2*(FB - isBallMode*T + LR),vex::velocityUnits::rpm);*/

    vex::task::sleep(20); //Sleep the task for a short amount of time to prevent wasted resources. 
  }
}

//
// Main will set up the competition functions and callbacks.
//
int main() {
    
    //Run the pre-autonomous function. 
    pre_auton();
    
    //Set up callbacks for autonomous and driver control periods.
    Competition.autonomous( autonomous );
    Competition.drivercontrol( usercontrol );

       
}