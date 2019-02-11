	#include "robot-config.h"
	#define RIGHT 1
	#define LEFT -1
	#define RED -1
	#define BLUE 1
	#define FRONT 1
	#define BACK -1

	using namespace vex;

	/*---------------------------------------------------------------------------*/
	/*                                                                           */
	/*        Description: Competition template for VCS VEX V5                    */
	/*                                                                           */
	/*---------------------------------------------------------------------------*/

	//Creates a competition object that allows access to Competition methods.
	competition    Competition;
	
	// pointers to member functions https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.cbclx01/cplr034.htm


	/////////////////////////////////////// VARIABLES /////////////////////////////////////////////////////////////

	//mode
	bool isCatapultReady = false;
    bool isCatapultPriming = false;
    bool isCollectorPriming = false;
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
	int FB, LR, T, Lift;
	int FL, FR, BL, BR;
	double brakePosition[4] = {0.0, 0.0, 0.0, 0.0}; //BL, BR, CR, CL
	int avgBaseR, avgBaseL, avgBaseFwd, avgBaseRot;

	double BLpos, BRpos, CRpos, CLpos;

	int flipRot = 0;

    //timers
    double ledtime, modetime, basetime;

	/////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////

	void sleep(int t) {
		task::sleep(t);
	}

	void toggleLEDs() {
		if((Brain.timer(timeUnits::msec)-ledtime) > 300){
			areLEDsOn = !areLEDsOn;
			if(areLEDsOn){
				LED.state(100,percentUnits::pct);
				//LEDa.state(100,percentUnits::pct);
				//LEDb.state(100,percentUnits::pct);
				//LEDc.state(100,percentUnits::pct);
				//LEDd.state(100,percentUnits::pct);
				//LEDe.state(100,percentUnits::pct);
				//LEDf.state(100,percentUnits::pct);
			} else {
				LED.state(0,percentUnits::pct);
				//LEDa.state(0,percentUnits::pct);
				//LEDb.state(0,percentUnits::pct);
			    //LEDc.state(0,percentUnits::pct);
				//LEDd.state(0,percentUnits::pct);
				//LEDe.state(0,percentUnits::pct);
				//LEDf.state(0,percentUnits::pct);
			}
			//Brain.resetTimer();
            ledtime = Brain.timer(timeUnits::msec);
		}
	}

	void toggleMode() {
		if((Brain.timer(timeUnits::msec)-modetime) > 350){
			isBallMode = -isBallMode;
			if(isBallMode == 1) {
				Brain.Screen.printAt(10,40,"Mode: Ball Mode %b", isBallMode);
			} else {
				Brain.Screen.printAt(10,40,"Mode: Cap Mode %b", isBallMode);
			}
			LED.state(100,percentUnits::pct);
			areLEDsOn = true;
			modetime = Brain.timer(timeUnits::msec);
            //Brain.resetTimer();
		}
	}

	void delayLEDsOff() {
		sleep(500);
		LED.state(0,percentUnits::pct);
		areLEDsOn = false;
	}

	void intake() {
		Intake.spin(directionType::fwd, 600, velocityUnits::rpm);
		Intake2.spin(directionType::rev, 600, velocityUnits::rpm);
		Brain.Screen.printAt(10,40,"Intake running at 600rpm");
	}

	void outtake() {
		Intake.spin(directionType::rev, 600, velocityUnits::rpm);
		Intake2.spin(directionType::fwd, 600, velocityUnits::rpm);
		Brain.Screen.printAt(10,40,"Intake running at 600rpm");
	}

	void intakeStop() {
		Intake.stop();
		Intake2.stop();
		Brain.Screen.printAt(10,40,"Intake stopped.");
	}

	void liftUp() {
		Intake.spin(directionType::fwd, 600, velocityUnits::rpm);
		Intake2.spin(directionType::rev, 600, velocityUnits::rpm);
	}

	void liftDown() {
		Intake.spin(directionType::rev, 600, velocityUnits::rpm);
		Intake2.spin(directionType::fwd, 600, velocityUnits::rpm);
	}

	void liftStop() {
		intakeStop();
	}

	void catgo() {
		Catapult.spin(directionType::fwd,200,velocityUnits::rpm);
	}

	void catstop() {
		Catapult.stop(brakeType::coast);
	}

	void flipUp() {
		Flipper.spin(directionType::fwd, 200, velocityUnits::rpm);
		Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
	}

	void flipDown() {
		Flipper.spin(directionType::rev, 200, velocityUnits::rpm);
		Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
	}

	void flipStop() {
		Flipper.stop();
		flipRot = Flipper.rotation(rotationUnits::deg);
	}

	void flipOne() {
		
	}
	
	void resetBaseEnc() {
		FL_Base.resetRotation();
		FR_Base.resetRotation();
		BL_Base.resetRotation();
		BR_Base.resetRotation();
	}

	void drive(int vel) { // + fwd - rev
        BL = vel;
        BR = -vel;
        FR = -vel;
        FL = vel;
        
		/*FL = vel;
		FR = -vel;
		BL = vel;
		BR = -vel;*/
		/*
		FL_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		FR_Base.spin(directionType::rev,isBallMode*vel,velocityUnits::rpm);
		BL_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		BR_Base.spin(directionType::rev,isBallMode*vel,velocityUnits::rpm);
		*/
	}

    void driveLR(int L, int R) {
        BL = L;
        BR = -R;
        FR = -R;
        FL = L;
    }

    void driveAxes(int ladder, int snake) {
        BL = ladder;
        BR = -snake;
        FR = -snake;
        FL = ladder;
    }

	void driveRot(int deg, bool waitForCompletion=false, int vel=200) {
		if(waitForCompletion){
			FL_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.rotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.rotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
		} else {
			FL_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.startRotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.startRotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
		}
	}

	void driveTo(int deg, bool waitForCompletion=false, int vel=200) {
		if(waitForCompletion){
			FL_Base.rotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.rotateTo(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.rotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.rotateTo(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
		} else {
			FL_Base.startRotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.startRotateTo(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.startRotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.startRotateTo(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
		}
	}

	void strafe(int vel) { //+ right - left
		FL_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		FR_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		BL_Base.spin(directionType::rev,isBallMode*vel,velocityUnits::rpm);
		BR_Base.spin(directionType::rev,isBallMode*vel,velocityUnits::rpm);
	}

	void strafeRot(int deg, bool waitForCompletion=false, int vel=200) {
		if(waitForCompletion){
			FL_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.rotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.rotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
		} else {
			FL_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.startRotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.startRotateFor(-deg,rotationUnits::deg,vel,velocityUnits::rpm);
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
		FL_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		FR_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		BL_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
		BR_Base.spin(directionType::fwd,isBallMode*vel,velocityUnits::rpm);
	}

    void rot(int vel) {
		FL_Base.spin(directionType::fwd,vel,velocityUnits::rpm);
		FR_Base.spin(directionType::fwd,vel,velocityUnits::rpm);
		BL_Base.spin(directionType::rev,vel,velocityUnits::rpm);
		BR_Base.spin(directionType::rev,vel,velocityUnits::rpm);
	}

	void rotRot(int deg, bool waitForCompletion=false, int vel=200) { 
		if(waitForCompletion){
			FL_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.rotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
		} else {
			FL_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.startRotateFor(deg,rotationUnits::deg,vel,velocityUnits::rpm);
		}
	}

	void rotTo(int deg, bool waitForCompletion=false, int vel=200) { 
		if(waitForCompletion){
			FL_Base.rotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.rotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.rotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.rotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
		} else {
			FL_Base.startRotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			FR_Base.startRotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BL_Base.startRotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
			BR_Base.startRotateTo(deg,rotationUnits::deg,vel,velocityUnits::rpm);
		}
	}

	void stopBase() {
		FR = 0;
		FL = 0;
		BR = 0;
		BL = 0;
		
		FL_Base.stop();
		FR_Base.stop();
		BL_Base.stop();
		BR_Base.stop();
	}

	void rotTime(int vel, int time){
		rotFast(vel);
		sleep(time);
		stopBase();
	}

	void driveTime(int vel, int time) {
		drive(vel);
		sleep(time);
		stopBase();
	}

	void strafeTime(int vel, int time) {
		strafe(vel);
		sleep(time);
		stopBase();
	}
	
	int derr, power, goal, aStep, initRot;
	double kP;
	
	void driveFor(int dist, bool isStrafe=false) {
		// diameter 2.75 in, radius 1.375 in
		// inches to degrees
		// 2.75*pi= 360 deg
		// d/2.75/pi*360
		// thus multiplier is 41.6697, round to 42
		// because wheels are angled, multiply by sqrt(2)
		// so multiplier is 58.9298, approx 59
		resetBaseEnc();
		initRot = GyroYaw.value(rotationUnits::deg);
		
		kP = 0.5; //.555 want to slow down at 360deg
		goal = dist * 33;
		derr = goal - avgBaseFwd;
		//Brain.Screen.printAt(0,120,"DriveFor initiated.");
        basetime = Brain.timer(timeUnits::msec);
        
        //accelerate to 200rpm in 100ms
        aStep = copysign(1,dist)*5;
        for(int c=0; c<40; c++) {
            //drive(c*aStep);
            //driveAxes(c*aStep, c*aStep+10);
			driveLR(c*aStep, c*aStep+10);
            task::sleep(6);
        }
        
        //p to goal
		while(abs(derr) > 3) {
			Brain.Screen.printAt(0,140,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
            //if ((Brain.timer(timeUnits::msec)-basetime) < 16) driveAxes(0,power);
            //else drive(power);
			drive(power);
			//driveAxes();
            //if(!isStrafe) drive(power);
			//else strafe(power);
			derr = goal - avgBaseFwd;
            sleep(5); //otherwise taking differnce is meaningless
		}
		stopBase();
        Brain.Screen.printAt(0,120,"Drive distance reached.");
	}
    
    
    void rotFor(int deg) {
        // radius of turning is approx 8 inches
		// given degrees, convert to radians and * r
		// multiplier = /180*pi*8 ~ 458
		
		resetBaseEnc();
		
		kP = 0.4;
        goal = deg * 458;
		derr = goal - avgBaseRot;
		
		while(abs(derr) > 3) {
			//Brain.Screen.printAt(0,180,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
            //if ((Brain.timer(timeUnits::msec)-basetime) < 16) driveAxes(0,power);
            //else drive(power);
			rot(power);
            //if(!isStrafe) drive(power);
			//else strafe(power);
			derr = goal - avgBaseRot;
            sleep(5); //otherwise taking differnce is meaningless
		}
		stopBase();
        Brain.Screen.printAt(0,140,"Drive distance reached.");
        
    }

	void align() {
		if(amIBlue) Vision.takeSnapshot(CAPBLUE);
		else Vision.takeSnapshot(CAPRED);
		if (Vision.largestObject.exists && Vision.largestObject.width>5) {   
		}
	}

    int sensor() {
		Brain.Screen.printAt(0,80, "sensor task running");
		for(;;) {
			avgBaseL = FL_Base.rotation(rotationUnits::deg) + BL_Base.rotation(rotationUnits::deg);
			avgBaseR = FR_Base.rotation(rotationUnits::deg) + BR_Base.rotation(rotationUnits::deg);
			avgBaseFwd = (avgBaseL - avgBaseR)>>2;
			avgBaseRot = (avgBaseL + avgBaseR)>>2;
			
			//Brain.Screen.clearScreen();
			Brain.Screen.printAt(0,100, "Gyro: %.2f", GyroYaw.value(rotationUnits::deg));
            //Brain.Screen.printAt(0,100, "Average Base Drive: %d", avgBaseFwd);
			//Brain.Screen.printAt(0,120, "Average Base Rotation: %d", avgBaseRot);
			sleep(20);
		}
		return 1;
	}

	void trebuchet() {
        if (isCollectorPriming) return;
		//auto reload
        isCatapultPriming = true;
		Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
		Brain.Screen.printAt(10,40, "Catapult Running");
		sleep(500);
		while(CatBumper.value() == 1){
			sleep(20);
		}
		Catapult.stop();
		isCatapultPriming = false;
		/* UNCOMMENT for regular catapult function
		if(isCatapultReady) {
			//catapult down, fire
		   // Catapult.RotateFor(1,rotationUnits::rev,200,velocityUnits::rpm);
			Catapult.spin(directionType::fwd, 200,velocityUnits::rpm);
			Brain.Screen.printAt(10,40, "Catapult Firing");
			sleep(500);
			Catapult.stop();
			isCatapultReady = false;
		} else {
			//catapult up, start moving arm
			Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
			Brain.Screen.printAt(10,40, "Catapult Running");
			while(CatBumper.value() == 1){
				sleep(20);
			}
			Catapult.stop();
			isCatapultReady = true;
		}*/
		
	}

    int smackTillButton() {
        while(ColBumper.value() == 1) {
              if (Catapult.torque(torqueUnits::Nm) >= 1.0) {
                  Catapult.stop(brakeType::coast);
                  isCollectorReady = false;
                  return -1;
              }
			  sleep(2);
		  }
        return 1;
    }

	void smack() {
	   if(isCatapultPriming){
		   // catapult is in motion, do not activate
           Brain.Screen.clearScreen();
           Brain.Screen.printAt(0,80,"Smack pressed, Catapult is spinning");
	   } else if(isCollectorReady) {
           isCollectorPriming = true;
           Brain.Screen.clearScreen();
           Brain.Screen.printAt(0,80,"Smack pressed");
		   Catapult.spin(directionType::rev, 200,velocityUnits::rpm);
		   sleep(300);
		   Catapult.stop(brakeType::coast);
           isCollectorReady = false;
           isCollectorPriming = false;
	   } else {
          isCollectorPriming = true;
          Brain.Screen.clearScreen();
          Brain.Screen.printAt(0,80,"Smack pressed, winding down");
		  Catapult.spin(directionType::rev, 200, velocityUnits::rpm);
          if (smackTillButton() > 0){
              Catapult.stop(brakeType::hold);
              isCollectorReady = true;
          } else {
              Catapult.stop(brakeType::coast);
              isCollectorReady = false;
          }
          isCollectorPriming = false;
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
			task::sleep(100);
		}
		return 1;
	}

	int FRCallback() {
		for(;;) {
			//FR_Base.spin(directionType::rev,isBallMode*1.6*(FB - isBallMode*T - LR),velocityUnits::rpm);
			FR_Base.spin(directionType::fwd, FR, velocityUnits::rpm);
            if (FR == 0) FR_Base.stop();
			task::sleep(2);
		}
		return 1;
	}
	int FLCallback() {
		for(;;) {
			//FL_Base.spin(directionType::fwd,isBallMode*1.6*(FB + isBallMode*T + LR),velocityUnits::rpm);
			FL_Base.spin(directionType::fwd, FL, velocityUnits::rpm);
            if (FL == 0) FL_Base.stop();
			task::sleep(2);
		}
		return 1;
	}
	int BRCallback() {
		for(;;) {
			//BR_Base.spin(directionType::rev,isBallMode*1.6*(FB - isBallMode*T + LR),velocityUnits::rpm);
			BR_Base.spin(directionType::fwd, BR, velocityUnits::rpm);
            if (BR == 0) BR_Base.stop();
			task::sleep(2);
		}
		return 1;
	}
	int BLCallback() {
		for(;;) {
			//BL_Base.spin(directionType::fwd,isBallMode*1.6*(FB + isBallMode*T - LR),velocityUnits::rpm);
			BL_Base.spin(directionType::fwd, BL, velocityUnits::rpm);
            if (BL == 0) BL_Base.stop();
			task::sleep(2);
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

	task launchTask = task(launchCallback);
	task mainTask = task(mainCallback);
    task sensorTask = task(sensor);
    task BLTask = task(BLCallback);
    task FRTask = task(FRCallback);
    task BRTask = task(BRCallback);
    task FLTask = task(FLCallback);

	///////////////////////////////////////// AUTONS /////////////////////////////////////////////////////////////////	

	void park() {
		// drives forward until parks
		GyroPitch.startCalibration();
		sleep(1500);
		while(GyroPitch.value(rotationUnits::deg) < 12) {
			drive(200);
		}
		while(GyroPitch.value(rotationUnits::deg) > 2) {
			drive(150);
		}
		stopBase();
	}

    void fetch() {
        //nabs bol
        intake();
        driveFor(40);
        sleep(500);
        driveFor(-40);
        intakeStop();
    }

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
		strafeTime(side * 200, 400);     // align against wall
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
		strafeTime(side * 200, 400);     // align to wall
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

	void ezBackAuton(int side = RIGHT) {
		intake();
		driveRot(1125);
		sleep(1500);
		rot(side * 260);
		sleep(1100);
		intakeStop();
		park(); //uses gyro
		//driveTime(150, 1000); //park
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
		strafeTime(side * 200, 400);     // align against wall
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
		strafeTime(side * 200, 400);     // align to wall (9.3s)
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
		Flipper.spin(directionType::fwd, -100, velocityUnits::pct);
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
		driveTime(200, 900); //climb tile
		sleep(500);
		intakeStop();
	}

    void testing(motor *m, int vel) {
		m->spin(directionType::fwd, vel, velocityUnits::rpm);
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
        isCollectorReady = false;
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
		BL_Base.setStopping(brakeType::hold);
        BR_Base.setStopping(brakeType::hold);
        FL_Base.setStopping(brakeType::hold);
        FR_Base.setStopping(brakeType::hold);
        
        //fetch();
        rotFor(90);
        sleep(400);
        rotFor(-90);
        
        //testing(&Catapult, 200);
        //sleep(100);
        //Catapult.stop();
        
		//oleReliable(RIGHT); //BF
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
	  
		//sensorTask.stop();
        /*BLTask.resume();
        BRTask.resume();
        FLTask.resume();
        FRTask.resume();*/
        
        BL_Base.setStopping(brakeType::coast);
        BR_Base.setStopping(brakeType::coast);
        FL_Base.setStopping(brakeType::coast);
        FR_Base.setStopping(brakeType::coast);
        
	  while (1) {
		// This is the main execution loop for the user control program.
		// Each time through the loop your program should update motor + servo 
		// values based on feedback from the joysticks.
		  
		  //Controller.ButtonL2.pressed(toggleBrake);

		  FB = Controller.Axis3.position(percentUnits::pct);
		  LR = Controller.Axis4.position(percentUnits::pct);
		  
		  FR = isBallMode*-2*(FB - isBallMode*T - LR);
		  FL = isBallMode*2*(FB + isBallMode*T + LR);
		  BR = isBallMode*-2*(FB - isBallMode*T + LR);
		  BL = isBallMode*2*(FB + isBallMode*T - LR);
		  
		  Lift = Controller.Axis2.position(percentUnits::pct);
		  
		  if (abs(Lift)> 80) {
              T = 0;
              Intake.spin(directionType::fwd, Lift, percentUnits::pct);
              Intake2.spin(directionType::fwd, Lift, percentUnits::pct);
		  } else {
              T = Controller.Axis1.value();
              if (!(Controller.ButtonR2.pressing() || Controller.ButtonL2.pressing())) {
                  Intake.stop();
                  Intake2.stop();
              }
          }
		  
		  /* FL_Base.spin(directionType::fwd,isBallMode*2*(FB + isBallMode*T + LR),velocityUnits::rpm);
		  FR_Base.spin(directionType::rev,isBallMode*2*(FB - isBallMode*T - LR),velocityUnits::rpm);
		  BL_Base.spin(directionType::fwd,isBallMode*2*(FB + isBallMode*T - LR),velocityUnits::rpm);
		  BR_Base.spin(directionType::rev,isBallMode*2*(FB - isBallMode*T + LR),velocityUnits::rpm); */

		task::sleep(20); //Sleep the task for a short amount of time to prevent wasted resources. 
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