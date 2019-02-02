#include "robot-config.h"
#define RIGHT 1
#define LEFT -1
#define RED -1
#define BLUE 1
#define FRONT 1
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
bool isCollectorReady = false;
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
int FB,LR, T, Lift;
double brakePosition[4] = {0.0, 0.0, 0.0, 0.0}; //BL, BR, CR, CL

double BLpos, BRpos, CRpos, CLpos;

int flipRot = 0;

/////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////

void sleep(int t) {
    vex::task::sleep(t);
}

void toggleLEDs() {
    if(Brain.timer(vex::timeUnits::msec) > 300){
        areLEDsOn = !areLEDsOn;
        if(areLEDsOn){
            LED.state(100,vex::percentUnits::pct);
            LEDa.state(100,vex::percentUnits::pct);
            LEDb.state(100,vex::percentUnits::pct);
            LEDc.state(100,vex::percentUnits::pct);
            LEDd.state(100,vex::percentUnits::pct);
            LEDe.state(100,vex::percentUnits::pct);
            LEDf.state(100,vex::percentUnits::pct);
        } else {
            LED.state(0,vex::percentUnits::pct);
            LEDa.state(0,vex::percentUnits::pct);
            LEDb.state(0,vex::percentUnits::pct);
            LEDc.state(0,vex::percentUnits::pct);
            LEDd.state(0,vex::percentUnits::pct);
            LEDe.state(0,vex::percentUnits::pct);
            LEDf.state(0,vex::percentUnits::pct);
        }
        Brain.resetTimer();
    }
}

void toggleMode() {
    if(Brain.timer(vex::timeUnits::msec) > 350){
        isBallMode = -isBallMode;
        if(isBallMode == 1) {
            Brain.Screen.printAt(10,40,"Mode: Ball Mode %b", isBallMode);
        } else {
            Brain.Screen.printAt(10,40,"Mode: Cap Mode %b", isBallMode);
        }
        LED.state(100,vex::percentUnits::pct);
        areLEDsOn = true;
        Brain.resetTimer();
    }
}

void delayLEDsOff() {
    vex::task::sleep(500);
    LED.state(0,vex::percentUnits::pct);
    areLEDsOn = false;
}

void intake() {
    Intake.spin(vex::directionType::fwd, 600, vex::velocityUnits::rpm);
    Intake2.spin(vex::directionType::rev, 600, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Intake running at 600rpm");
}

void outtake() {
    Intake.spin(vex::directionType::rev, 600, vex::velocityUnits::rpm);
    Intake2.spin(vex::directionType::fwd, 600, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Intake running at 600rpm");
}

void intakeStop() {
    Intake.stop();
    Intake2.stop();
    Brain.Screen.printAt(10,40,"Intake stopped.");
}

void liftUp() {
	Intake.spin(vex::directionType::fwd, 600, vex::velocityUnits::rpm);
    Intake2.spin(vex::directionType::rev, 600, vex::velocityUnits::rpm);
}

void liftDown() {
	Intake.spin(vex::directionType::rev, 600, vex::velocityUnits::rpm);
    Intake2.spin(vex::directionType::fwd, 600, vex::velocityUnits::rpm);
}

void liftStop() {
	intakeStop();
}

void catgo() {
    Catapult.spin(vex::directionType::fwd,200,vex::velocityUnits::rpm);
}

void catstop() {
    Catapult.stop();
}

void flipUp() {
    Flipper.spin(vex::directionType::fwd, 200, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
}

void flipDown() {
    Flipper.spin(vex::directionType::rev, 200, vex::velocityUnits::rpm);
    Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
}

void flipStop() {
    Flipper.stop();
    flipRot = Flipper.rotation(vex::rotationUnits::deg);
}

void flipOne() {
    
}

void drive(int vel) { // + fwd - rev
    FL_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    FR_Base.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
    BL_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    BR_Base.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
}

void driveRot(int deg, bool waitForCompletion=false, int vel=200) {
    if(waitForCompletion){
        FL_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        FL_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void driveTo(int deg, bool waitForCompletion=false, int vel=200) {
    if(waitForCompletion){
        FL_Base.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.rotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.rotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        FL_Base.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.startRotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.startRotateTo(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void strafe(int vel) { //+ right - left
    FL_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    FR_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    BL_Base.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
    BR_Base.spin(vex::directionType::rev,isBallMode*vel,vex::velocityUnits::rpm);
}

void strafeRot(int deg, bool waitForCompletion=false, int vel=200) {
    if(waitForCompletion){
        FL_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.rotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        FL_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.startRotateFor(-deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

int stepVel;
int stepDelay;

void accel(int targetVel, int time=70, int step=10, bool isStrafe=false) { //speed up from zero to goal
    stepVel = 0;
    stepDelay = time/step;
    if(targetVel<0) step = -step;
    if(isStrafe){
        while (abs(stepVel) < abs(targetVel)) {
            strafe(stepVel);
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

void driveDecel(int vel, int timeAcc) {
    //decel from target speed in timeAcc ms
    accel(vel, timeAcc, -10);
    drive(0);
}

void strafeAccel(int vel, int timeAcc, int timeDrive) {
    accel(vel, timeAcc, 10, true);
    sleep(timeDrive);
    drive(0);
}

void rotFast(int vel) { //+ right - left
    FL_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    FR_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    BL_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
    BR_Base.spin(vex::directionType::fwd,isBallMode*vel,vex::velocityUnits::rpm);
}

void rot(int deg, bool waitForCompletion=false, int vel=200) { 
    if(waitForCompletion){
        FL_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.rotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        FL_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.startRotateFor(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void rotTo(int deg, bool waitForCompletion=false, int vel=200) { 
    if(waitForCompletion){
        FL_Base.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.rotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    } else {
        FL_Base.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        FR_Base.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BL_Base.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
        BR_Base.startRotateTo(deg,vex::rotationUnits::deg,vel,vex::velocityUnits::rpm);
    }
}

void stopBase() {
    FL_Base.stop();
    FR_Base.stop();
    BL_Base.stop();
    BR_Base.stop();
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
	//auto reload
	Catapult.spin(vex::directionType::fwd, 200, vex::velocityUnits::rpm);
	Brain.Screen.printAt(10,40, "Catapult Running");
	sleep(500);
	while(CatBumper.value() == 1){
		sleep(20);
	}
	Catapult.stop();
	
    /* UNCOMMENT for regular catapult function
	if(isCatapultReady) {
        //catapult down, fire
       // Catapult.RotateFor(1,vex::rotationUnits::rev,200,vex::velocityUnits::rpm);
        Catapult.spin(vex::directionType::fwd, 200,vex::velocityUnits::rpm);
        Brain.Screen.printAt(10,40, "Catapult Firing");
        sleep(500);
        Catapult.stop();
        isCatapultReady = false;
    } else {
        //catapult up, start moving arm
        Catapult.spin(vex::directionType::fwd, 200, vex::velocityUnits::rpm);
        Brain.Screen.printAt(10,40, "Catapult Running");
        while(CatBumper.value() == 1){
            sleep(20);
        }
        Catapult.stop();
        isCatapultReady = true;
    }*/
	
}

void smack() {
   if(Catapult.isSpinning()){
	   // catapult is in motion, do not activate
       return;
   } else if(isCollectorReady) {
	   Catapult.spin(vex::directionType::rev, 200,vex::velocityUnits::rpm);
		sleep(500);
		Catapult.stop();
		isCollectorReady = false;
   } else {
      Catapult.spin(vex::directionType::rev, 200, vex::velocityUnits::rpm);
      while(ColBumper.value() == 1) {
		  sleep(20);
	  }
	  Catapult.stop(vex::brakeType::hold);
	  isCollectorReady = true;
   }
   
}

void trebStop() {
    //called when bumper is pressed
    Catapult.stop();
    Brain.Screen.printAt(10,40, "Ready Position Reached");
    isCatapultReady = true;
}
///////////////////////////////////////// TASKS /////////////////////////////////////////////////////////////////

int launchCallback() {
    for(;;) {
        if(isBallMode) Controller.ButtonR1.pressed(trebuchet);
        Controller.ButtonL1.pressed(smack);
        //Brain.Screen.printAt(10,150,"Launch Callback Running...");
        vex::task::sleep(100);
    }
    return 1;
}

int FRCallback() {
    for(;;) {
        FR_Base.spin(vex::directionType::rev,isBallMode*1.6*(FB - isBallMode*T - LR),vex::velocityUnits::rpm);
		vex::task::sleep(20);
    }
	return 1;
}
int FLCallback() {
    for(;;) {
        FL_Base.spin(vex::directionType::fwd,isBallMode*1.6*(FB + isBallMode*T + LR),vex::velocityUnits::rpm);
		vex::task::sleep(20);
    }
	return 1;
}
int BRCallback() {
    for(;;) {
        BR_Base.spin(vex::directionType::rev,isBallMode*1.6*(FB - isBallMode*T + LR),vex::velocityUnits::rpm);
		vex::task::sleep(20);
    }
	return 1;
}
int BLCallback() {
    for(;;) {
        BL_Base.spin(vex::directionType::fwd,isBallMode*1.6*(FB + isBallMode*T - LR),vex::velocityUnits::rpm);
		vex::task::sleep(20);
    }
	return 1;
}

int brakeBase() {
    for(;;){
        if(isBraking){
            FL_Base.startRotateTo(BLpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            FR_Base.startRotateTo(BRpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            BL_Base.startRotateTo(CRpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            BR_Base.startRotateTo(CLpos,vex::rotationUnits::deg, 200, vex::velocityUnits::rpm);
            Brain.Screen.printAt(10, 120, "BL: %.2f", FL_Base.rotation(vex::rotationUnits::deg));
        }
        sleep(10);
    }
    return 1;
}

int mainCallback() {
    for(;;){
      Controller.ButtonUp.pressed(toggleLEDs);
        
      Controller.ButtonRight.pressed(toggleMode);
      Controller.ButtonRight.released(delayLEDsOff);

      Controller.ButtonUp.pressed(flipUp);
      Controller.ButtonUp.released(flipStop);
	  Controller.ButtonDown.pressed(flipDown);
      Controller.ButtonDown.released(flipStop);
	  Controller.ButtonB.pressed(flipOne);
	  
	  Controller.ButtonA.pressed(catgo);
	  Controller.ButtonA.released(catstop);
      
	  if (isBallMode) {
		Controller.ButtonR2.pressed(intake);
		Controller.ButtonR2.released(intakeStop);
		Controller.ButtonL2.pressed(outtake);
		Controller.ButtonL2.released(intakeStop);
	  } else {
		Controller.ButtonR2.pressed(liftDown);
		Controller.ButtonR2.released(liftStop);
		Controller.ButtonR1.pressed(liftUp);
		Controller.ButtonR1.released(liftStop);
		Controller.ButtonL2.pressed(flipOne);
	  }
	  
      
    }
}

vex::task launchTask = vex::task(launchCallback);
vex::task mainTask = vex::task(mainCallback);
vex::task BLTask = vex::task(BLCallback);
vex::task BRTask = vex::task(BRCallback);
vex::task FRTask = vex::task(FRCallback);
vex::task FLTask = vex::task(FLCallback);

void toggleBrake() {
    if(Brain.timer(vex::timeUnits::msec) > 300){
        isBraking = !isBraking;
        if(isBraking) {
            BLpos = FL_Base.rotation(vex::rotationUnits::deg);
            BRpos = FR_Base.rotation(vex::rotationUnits::deg);
            CRpos = BL_Base.rotation(vex::rotationUnits::deg);
            CLpos = BR_Base.rotation(vex::rotationUnits::deg);
            //baseTask.resume();
            Brain.Screen.printAt(10,70,"Braking: %f", BLpos);
            Brain.Screen.printAt(10,100,"rotation: %f", FL_Base.rotation(vex::rotationUnits::deg));
        } else {
            //baseTask.suspend();
            Brain.Screen.printAt(10,70,"Braking: %b", isBraking);
        }
        Brain.resetTimer();
    }
}

///////////////////////////////////////// AUTONS /////////////////////////////////////////////////////////////////

void oleReliable(int side = RIGHT) {
    // shoot preload, toggle bottom flag
    isCatapultReady = true;
    intake();
    if(side == RIGHT) driveAccel(150, 1000, 600); // nab 2nd ball
    else if(side == LEFT) driveAccel(150, 1000, 400); // nab 2nd ball
    sleep(300);
    driveAccel(-150, 1000, 600);    // drive back to wall
    // IF RED HITTING THE WALL COMING BACK IS AN ISSUE: 
    // COMMENT ABOVE AND UNCOMMENT BELOW
    //if(side == RIGHT) driveAccel(-150, 1000, 600); // drive back to wall
    //else if(side == LEFT) driveAccel(-150, 1000, 400); // drive back to wall
    sleep(300);
    intakeStop();
    rot(side * 380);                // turn to wall
    sleep(1000);
    strafeFor(side * 200, 400);     // align against wall
    strafeAccel(side * -100, 700, 0);                   // come out from wall
    if(side == RIGHT) driveAccel(100, 300, 700);        // fwd to double shot posn
    else if(side == LEFT) driveAccel(100, 300, 550);    // fwd to double shot posn
    rot(side * -30);                // turn slightly to aim
    sleep(200);
    trebuchet();                    // launch ball
    if(side == RIGHT) rot(35);      // turn to toggle bottom flag
    else if(side == LEFT) rot(-25); // turn to toggle bottom flag
    trebuchet();                    // ready catapult
    sleep(200);
    intake();
    driveAccel(150, 1000, 500);     // forward to toggle flag
    sleep(200);
    driveAccel(-150, 1000, 500);    // drive back
    
    // PHASE II :P
    strafeFor(side * 200, 400);     // align to wall
    intakeStop();
    strafeAccel(side * -100, 700, 0);// move out from wall
    sleep(200);
    rot(-side * 190);               // turn -45deg
    sleep(1000);
    outtake();
    driveAccel(150, 1000, 0);       // accelerate to 150rpm
    driveDecel(150, 1000);       // decelerate to 150rpm
    //trebuchet(); uncomment this if bot ends in correct posn
}

//experimental
void reeeeverdale(int side = RIGHT) {
    // shoot preload, toggle bottom flag
    isCatapultReady = true;
    intake();
    if(side == RIGHT) driveAccel(200, 1000, 100); // nab 2nd ball (1.1s)
    else if(side == LEFT) driveAccel(200, 1000, 0); // nab 2nd ball
    sleep(150);                     //---- ADD 100-150ms if ball is not intaken
    if(side == RIGHT) driveAccel(-200, 1000, 100); // drive back to wall (2.35s)
    else if(side == LEFT) driveAccel(-200, 1000, 0); // drive back to wall
    sleep(100);                     //---- ADD 100ms if overturning
    intakeStop();
    rot(side * 380);                // turn to wall
    sleep(750);                     //---- ADD 50 ms IF TURN IS INCOMPLETE (3.2s)
    strafeFor(side * 200, 400);     // align against wall
    strafeAccel(side * -100, 700, 0);                   // come out from wall (4.3s)
    if(side == RIGHT) driveAccel(100, 300, 700);        // fwd to double shot posn (5.3s)
    else if(side == LEFT) driveAccel(100, 300, 550);    // fwd to double shot posn
    rot(side * -30);                // turn slightly to aim
    sleep(200); 
    trebuchet();                    // launch ball, 500ms (6s)
    if(side == RIGHT) rot(35);      // turn to toggle bottom flag
    else if(side == LEFT) rot(-25); // turn to toggle bottom flag
    trebuchet();                    // ready catapult, 500ms
    sleep(100);
    intake();
    driveAccel(200, 1000, 100);     // forward to toggle flag (7.7s)
    // later, add left/right strafe to collect balls
    sleep(100);
    driveAccel(-200, 1000, 100);    // drive back (8.9s)
    
    // PHASE II :P
    strafeFor(side * 200, 400);     // align to wall (9.3s)
    intakeStop();
    strafeAccel(side * -100, 700, 0);// move out from wall (10s)
    sleep(200);
    rot(-side * 190);               // turn -45deg
    sleep(375);                     //---- ADD 25 ms IF TURN IS INCOMPLETE (10.6s)
    outtake();
    driveAccel(150, 1000, 0);       // accelerate to 150rpm
    driveDecel(150, 1000);       // decelerate to 150rpm (12.6s)
    //trebuchet(); uncomment this if bot ends in correct posn (13.1s)
}

void backCapPark(int side = RIGHT){
    intake(); // intake ball
    driveAccel(150, 1000, 600);
    sleep(100);
    /*
    rot(side * 380);
    sleep(1000);
    strafeAccel(side * -80, 200, 350); //strafe in front of cap
    sleep(100);
    driveAccel(-160, 1000, 0); //drive back into cap
    intakeStop();
    Flipper.spin(vex::directionType::fwd, -100, vex::velocityUnits::pct);
    sleep(300);
    Flipper.stop();
    flipRot = -360;
    trebuchet();
    strafeAccel(side * 80, 400, 600);*/
    
    //go straight to park
    driveAccel(-80, 200, 160);
    rot(side * 380);
    sleep(1000);
    intake(); //start climber wheels
    driveFor(200, 900); //climb tile
    sleep(500);
    intakeStop();
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
    
    oleReliable(RIGHT); //BF
    //backCapPark(RIGHT); //BB
    //oleReliable(LEFT);  //RF
    //backCapPark(LEFT);  //RB
    
    //EXPERIMENTAL, KEEP COMMENTED!!!
    //reeeeverdale(RIGHT);  //BF
    //reeeeverdale(LEFT);   //RF
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

void usercontrol( void ) {
  // User control code here, inside the loop
  while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo 
    // values based on feedback from the joysticks.
      
      //Controller.ButtonL2.pressed(toggleBrake);

      FB = Controller.Axis3.value();
      LR = Controller.Axis4.value();
      T = (int)(Controller.Axis1.value());
	  Lift = Controller.Axis2.position(vex::percentUnits::pct);
	  
	  if (abs(Lift)> 80) {
		Intake.spin(vex::directionType::fwd, Lift, vex::percentUnits::pct);
		Intake2.spin(vex::directionType::rev, Lift, vex::percentUnits::pct);
	  }
      
      /* FL_Base.spin(vex::directionType::fwd,isBallMode*2*(FB + isBallMode*T + LR),vex::velocityUnits::rpm);
      FR_Base.spin(vex::directionType::rev,isBallMode*2*(FB - isBallMode*T - LR),vex::velocityUnits::rpm);
      BL_Base.spin(vex::directionType::fwd,isBallMode*2*(FB + isBallMode*T - LR),vex::velocityUnits::rpm);
      BR_Base.spin(vex::directionType::rev,isBallMode*2*(FB - isBallMode*T + LR),vex::velocityUnits::rpm); */

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