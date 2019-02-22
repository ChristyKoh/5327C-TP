	#include "robot-config.h"
	#define RIGHT 1
	#define LEFT -1
	#define RED -1
	#define BLUE 1
	#define FRONT 1
	#define BACK -1
	#define POS 1
	#define NEG -1

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
	bool isAutonBase = false;
	bool isCatapultReady = false;
	bool isCatapultPriming = false;
	bool canPew = false;
	bool isCollectorPriming = false;
	bool isCollectorReady = false;
	bool canSmak = false;
	bool isBraking = false;
	bool canStopLift = true;
	bool amIBlue = true;
	bool areLEDsOn = false;
	int isBallMode = 1; //ball mode

	//sensor
	int BaseR, BaseL, BaseF, BaseB, avgBaseFwd, avgBaseRot;

	//base
	int thresh = 10;
	int FB, LR, T, Lift;
	int FL, FR, BL, BR;
	float initGyro;
	
	//lift

	//flipper
	//start position upward
	int foldPos = 10;
	int downPos = 240;
	int placePos = 192;
	int flipPos = 90;
	float torqueLimit = 1.0;

	//timers
	double ledtime, modetime, drivetime, catprimetime, colprimetime, torquetime, lifttime;

	/////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////

	void sleep(int t) {
		task::sleep(t);
	}

	void toggleLEDs() {
		if((Brain.timer(timeUnits::msec)-ledtime) > 300){
			areLEDsOn = !areLEDsOn;
			if(areLEDsOn){
				LED.state(100,percentUnits::pct);
			} else {
				LED.state(0,percentUnits::pct);
			}
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
		canStopLift = false;
		Intake.spin(directionType::fwd, 600, velocityUnits::rpm);
		Intake2.spin(directionType::rev, 600, velocityUnits::rpm);
	}

	void intakeSpeed(int s) {
		canStopLift = false;
		Intake.spin(directionType::fwd, s, velocityUnits::rpm);
		Intake2.spin(directionType::rev, s, velocityUnits::rpm);
	}

	void outtake() {
		canStopLift = false;
		Intake.spin(directionType::rev, 600, velocityUnits::rpm);
		Intake2.spin(directionType::fwd, 600, velocityUnits::rpm);
	}

	void intakeStop() {
		Intake.stop(brakeType::coast);
		Intake2.stop(brakeType::coast);
		//Brain.Screen.printAt(10,40,"Intake stopped.");
	}

	void liftUp() {
		canStopLift = false;
		Intake.spin(directionType::fwd, 200, velocityUnits::rpm);
		Intake2.spin(directionType::fwd, 200, velocityUnits::rpm);
	}

	void liftDown() {
		canStopLift = false;
		Intake.spin(directionType::rev, 200, velocityUnits::rpm);
		Intake2.spin(directionType::rev, 200, velocityUnits::rpm);
	}
	
	void liftSpeed(int s, bool stopLift=false) {
		canStopLift = stopLift;
		Intake.spin(directionType::fwd, s, velocityUnits::rpm);
		Intake2.spin(directionType::fwd, s, velocityUnits::rpm);
	}

	void liftStop() {
		intakeStop();
		canStopLift = true;
	}
	
	void liftHold() {
		Intake.stop(brakeType::hold);
		Intake2.stop(brakeType::hold);
		canStopLift = false;
	}
	
	void liftTop() {
		liftUp();
		double initLift = Intake.rotation(rotationUnits::deg);
		sleep(200);
		lifttime = Brain.timer(timeUnits::msec);
		while (abs(Intake.rotation(rotationUnits::deg) - initLift) > 0 && Brain.timer(timeUnits::msec) - lifttime < 5000) {
			//Brain.Screen.printAt(0,140,"initFlip is %d", initFlip - (int)Flipper.rotation(rotationUnits::deg));
			initLift = Intake.rotation(rotationUnits::deg);
			sleep(20);
		}
		// Controller.rumble("-");
		liftSpeed(50);
		liftHold();				//hold lift at topmost position
	}

	void catgo() {
		Catapult.spin(directionType::fwd,16,velocityUnits::rpm);
	}

	void catstop() {
		Catapult.stop(brakeType::coast);
	}

	void foldUp() {
		//Flipper.spin(directionType::fwd, 200, velocityUnits::rpm);
		//Brain.Screen.printAt(10,40,"Flipper running at 200rpm");
		liftSpeed(-200);
		torquetime = Brain.timer(timeUnits::msec);
		while (Intake.torque(torqueUnits::Nm) < .35 || (Brain.timer(timeUnits::msec) - torquetime) > 2000) {
			//Brain.Screen.printAt(0,180,"Torque is %0.1f", Intake.torque(torqueUnits::Nm));
			sleep(20);
		}
		sleep(400);
		liftHold();
		Flipper.rotateTo(foldPos, rotationUnits::deg, 100, velocityUnits::rpm);
		liftStop();
	}
	
	float delta_t;
	
	void flipReset() {
		liftSpeed(-100);
		double initLift = Intake.rotation(rotationUnits::deg);
		sleep(200);
		while (abs(Intake.rotation(rotationUnits::deg) - initLift) > 0) {
			//Brain.Screen.printAt(0,140,"initFlip is %d", initFlip - (int)Flipper.rotation(rotationUnits::deg));
			initLift = Intake.rotation(rotationUnits::deg);
			sleep(20);
		}
		Controller.rumble("-");
		liftHold();				//hold lift at bottommost position
		
		double initFlip = Flipper.rotation(rotationUnits::deg);
		Flipper.spin(directionType::rev, 100, velocityUnits::rpm);
		sleep(200);
		torquetime = Brain.timer(timeUnits::msec);
		while (abs(Flipper.rotation(rotationUnits::deg) - initFlip) > 3 && Brain.timer(timeUnits::msec) - torquetime < 2000) {
			initFlip = Flipper.rotation(rotationUnits::deg);
			sleep(100);
		}
		Flipper.stop(brakeType::hold);
		Flipper.resetRotation();
		sleep(500);
		Controller.rumble("-");
		liftStop(); // stop hold, allow reg control of lift
		
		Flipper.rotateTo(foldPos, rotationUnits::deg, 5, velocityUnits::rpm);
	}

	void flipDown() {
		Flipper.rotateTo(downPos, rotationUnits::deg, 100, velocityUnits::rpm);
	}

	void flipStop() {
		Flipper.stop(brakeType::hold);
	}

	void flipOne() {
		Flipper.rotateTo(flipPos, rotationUnits::deg, 100, velocityUnits::rpm);
		//sleep(100);
		Flipper.rotateTo(placePos, rotationUnits::deg, 100, velocityUnits::rpm);
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
	}

	void driveIndiv(int fl, int br, int fr, int bl){
		FL = fl;
		FR = fr;
		BL = bl;
		BR = br;
	}

	void driveLR(int L, int R) {
		BL = L;
		BR = -R;
		FR = -R;
		FL = L;
	}

	void driveFB(int F, int B) {
		BL = -B;
		BR = -B;
		FR = F;
		FL = F;
	}

	void driveAxes(int snek, int lad) {
		//snek drives right lad drives left       
		BL = lad;
		BR = -snek;
		FR = -lad;
		FL = snek;
	}

	void strafe(int vel) { //+ right - left
		FL = vel;
		FR = vel;
		BL = -vel;
		BR = -vel;
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

	void rot(int vel) {
		FL = vel;
		FR = vel;
		BL = vel;
		BR = vel;
	}

	void rotRot(int deg, int vel=200, bool waitForCompletion=false) { 
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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////   ODOMETRY   /////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int derr, power, goal, aSign, aStep, initRot, rotDiff;
	double kPr;

	void driveFor(int dist, int max=200, double kP=0.5, bool noStop=false, bool keepRot=true) {
		// diameter 2.75 in, radius 1.375 in
		// inches to degrees
		resetBaseEnc();
		initRot = GyroYaw.value(rotationUnits::deg);
		drivetime = Brain.timer(timeUnits::msec);
		//avgBaseFwd = (int) (FL_Base.rotation(rotationUnits::deg) - FR_Base.rotation(rotationUnits::deg) + BL_Base.rotation(rotationUnits::deg) - BR_Base.rotation(rotationUnits::deg))>>2;
		
		//kP = 0.5;
		kPr = 3;
		goal = dist * 33; //multiplier in to deg
		derr = goal - ((BaseL - BaseR)>>2);
		
		//accelerate to 200rpm in 100ms
		aStep = copysign(1,dist)*5;
		aSign = copysign(1,dist);
		if(dist > 0) {
			for(int c=0; c<40; c++) {
				driveLR(c*aStep, c*aStep+aSign*20);
				task::sleep(6);
			}
		} else {
			Brain.Screen.printAt(0,180,"Negative");
			for(int c=0; c<40; c++) {
				driveLR(c*aStep+aSign*30, c*aStep);
				task::sleep(6);
			}
		}
		
		
		//p to goal
		while(abs(derr) > 3) {
			//Brain.Screen.printAt(0,140,"%d %d", power, FL);
			power = derr * kP;
			power = power>max ? max : power;
			if(keepRot) {
				rotDiff = (GyroYaw.value(rotationUnits::deg)-initRot) * kPr;
				driveLR(power - rotDiff, power + rotDiff);
			}
			else drive(power); 
			derr = goal - ((BaseL - BaseR)>>2);
			sleep(5); //otherwise taking difference is meaningless
		}
		if(!noStop) stopBase();
		//Brain.Screen.printAt(0,120,"Drive distance reached.");
	}
	
	void strafeFor(int dist, double kP=0.7, bool keepRot=true) {
		
		//kP = 0.5;
		kPr = 3;
		goal = dist * 33;
		
		resetBaseEnc();
		initRot = GyroYaw.value(rotationUnits::deg);
		derr = goal - ((BaseF - BaseB)>>2);
		
		//p to goal
		while(abs(derr) > 3) {
			//Brain.Screen.printAt(0,140,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
			if(keepRot) {
				rotDiff = (GyroYaw.value(rotationUnits::deg)-initRot) * kPr;
				driveFB(power - rotDiff, power + rotDiff);
			}
			else strafe(power);   
			derr = goal - ((BaseF - BaseB)>>2);
			sleep(5); //otherwise taking difference is meaningless
		}
		stopBase();
	}

	void diagFor(int dist, int side, double kP=0.6, bool keepRot=false) {
		//only drives one axis (Snek falling, Lad rising)
		//Lad: BL, FR (/)		Snek: FL, BR (\)
		//distance directly prop to rotations
		//dist/2.75/pi*360 = 41.6696579
		//kP = 0.5;
		kPr = 3;
		goal = dist * 42;
		
		resetBaseEnc();
		initRot = GyroYaw.value(rotationUnits::deg);
		
		if (side == POS) { //drive snek 
			derr = goal - ((int)(FL_Base.rotation(rotationUnits::deg) - BR_Base.rotation(rotationUnits::deg))>>1);
			
			while(abs(derr) > 3) {
			//Brain.Screen.printAt(0,140,"snakes!");
				power = derr * kP;
				power = power>200 ? 200 : power;
				if(keepRot) {
					rotDiff = (GyroYaw.value(rotationUnits::deg)-initRot) * kPr;
					//driveIndiv(int fl, int br, int fr, int bl)
					driveIndiv(power - rotDiff, -power - rotDiff,0,0);
				} else driveAxes(power, 0);
				derr = goal - ((int)(FL_Base.rotation(rotationUnits::deg) - BR_Base.rotation(rotationUnits::deg))>>1);
				sleep(5); //otherwise taking difference is meaningless
			}
			
		} else { //NEG, drive lad 
			derr = goal - ((int)(BL_Base.rotation(rotationUnits::deg) - FR_Base.rotation(rotationUnits::deg))>>1);
			
			while(abs(derr) > 3) {
				//Brain.Screen.printAt(0,140,"ladders!");
				power = derr * kP;
				power = power>200 ? 200 : power;
				if(keepRot) {
					rotDiff = (GyroYaw.value(rotationUnits::deg)-initRot) * kPr;
					//driveIndiv(int fl, int br, int fr, int bl)
					driveIndiv(0,0, -power + rotDiff, power + rotDiff);
				} else driveAxes(0, power);   
				derr = goal - ((int)(BL_Base.rotation(rotationUnits::deg) - FR_Base.rotation(rotationUnits::deg))>>1);
				sleep(5); //otherwise taking difference is meaningless
			}
		}
		stopBase();
	}

	void rotFor(int deg, double kP=0.9) {
		// radius of turning is approx 8 inches
		// given degrees, convert to radians and * r to find dist
		// from dist, /2.75/pi to get degrees motor
		// multiplier = 1/180*pi*8/2.75/pi*360 = 5.8181818181818
		
		resetBaseEnc();
		
		//kP = 0.9;
		goal = deg * 5.7;
		derr = goal - ((BaseR + BaseL)>>2);
		
		while(abs(derr) > 3) {
			//Brain.Screen.printAt(0,180,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
			rot(power);
			derr = goal - ((BaseR + BaseL)>>2);
			sleep(5); //otherwise taking difference is meaningless
		}
		stopBase();
		//Brain.Screen.printAt(0,140,"Drive distance reached.");
	}

	void rotTo(int deg, double kP=4){ //using gyro
		goal = deg;
		derr = goal*1.1 - GyroYaw.value(rotationUnits::deg);
		
		while(abs(derr) > 0.05) {
			//Brain.Screen.printAt(0,180,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
			rot(power);
			derr = goal*1.055 - GyroYaw.value(rotationUnits::deg); //constant: 1.07
			sleep(5); //otherwise taking difference is meaningless
		}
		stopBase();
	}
	
	void rotForGyro(int deg, double kP=4){
		initGyro = GyroYaw.value(rotationUnits::deg);
		goal = deg;
		derr = goal*1.1 - (GyroYaw.value(rotationUnits::deg) - initGyro);
		
		while(abs(derr) > 0.05) {
			//Brain.Screen.printAt(0,180,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
			rot(power);
			derr = goal*1.055 - (GyroYaw.value(rotationUnits::deg) - initGyro); //constant: 1.07
			sleep(5); //otherwise taking difference is meaningless
		}
		stopBase();
	}

	void align() {
		if(amIBlue) Vision.takeSnapshot(CAPBLUE);
		else Vision.takeSnapshot(CAPRED);
		if (Vision.largestObject.exists && Vision.largestObject.width>5) {   
		}
	}

	int sensor() {
		//Brain.Screen.printAt(0,80, "sensor task running");
		for(;;) {
			BaseL = FL_Base.rotation(rotationUnits::deg) + BL_Base.rotation(rotationUnits::deg);
			BaseR = FR_Base.rotation(rotationUnits::deg) + BR_Base.rotation(rotationUnits::deg);
			BaseF = FL_Base.rotation(rotationUnits::deg) + FR_Base.rotation(rotationUnits::deg);
			BaseB = BR_Base.rotation(rotationUnits::deg) + BL_Base.rotation(rotationUnits::deg);
			
			//Brain.Screen.clearScreen();
			Brain.Screen.printAt(0,100, "Gyro: %.2f", GyroYaw.value(rotationUnits::deg));
			Brain.Screen.printAt(0,120, "GyroPitch: %.2f", GyroPitch.value(rotationUnits::deg));
			//Brain.Screen.printAt(0,180,"flipper rotation is %f", Flipper.rotation(rotationUnits::deg));
			//Brain.Screen.printAt(10,40, "%d", TopColBumper.value());
			
			
			
			//Brain.Screen.printAt(0,100, "Average Base Drive: %d", avgBaseFwd);
			//Brain.Screen.printAt(0,120, "Average Base Rotation: %d", avgBaseRot);
			sleep(5); //used to be 20
		}
		return 1;
	}

	void trebuchet() {
		canPew = false;
		
		//THIS IS RELYING ON NOT REENGAGING
		if (isCollectorPriming || isBallMode == -1) return;
		//auto reload
		isCatapultPriming = true;
		catprimetime = Brain.timer(timeUnits::msec);
		Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
		//Brain.Screen.printAt(10,40, "Catapult Running");
		sleep(400);
		if(CatBumper.value() == 0) {
		//if button is pressed immedately, catapult is already lowered
			//firing at a lower speed
			Catapult.spin(directionType::fwd, 10, velocityUnits::rpm);
			while(CatBumper.value() == 0 && Brain.timer(timeUnits::msec)-catprimetime < 800){
				sleep(2);
			}
			Catapult.stop();
			sleep(300);
			Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
			
		}//if not, catapult will simply prime
		//sleep(400);
		catprimetime = Brain.timer(timeUnits::msec);
		while(CatBumper.value() == 1 && Brain.timer(timeUnits::msec)-catprimetime < 2000){
			sleep(10);
		}
		Catapult.stop(brakeType::coast);
		isCatapultPriming = false;
		
	}
	
	void smackFromButton(int deg) {
		Catapult.spin(directionType::rev, 100, velocityUnits::rpm); //lower smack
		
		while(TopColBumper.value() == 0){
			//Catapult.spin(directionType::rev, 100, velocityUnits::rpm); //lower smack
			sleep(2);
		}
		Catapult.stop();
		//sleep(400); // not necessary
		Catapult.rotateFor(-deg*3, rotationUnits::deg, 200, velocityUnits::rpm); //lower smack 90deg
	}

	int smackTillButton() {
		colprimetime = Brain.timer(timeUnits::msec);
		while(ColBumper.value() == 1) {
			  if (Catapult.torque(torqueUnits::Nm) >= torqueLimit) {
				  return -1;
			  }
			  if (Brain.timer(timeUnits::msec)-colprimetime > 2000) {
				  return -1;
			  }
			  sleep(2);
		  }
		return 1;
	}

	void smack() {
		canSmak = false;
	   if(isCatapultPriming || isBallMode == -1){
		   return;
		   // catapult is in motion, do not activate
		   //Brain.Screen.clearScreen();
		   //Brain.Screen.printAt(0,80,"Smack pressed, Catapult is spinning");
	   } else if(isCollectorReady) {
		   isCollectorPriming = true;
		   colprimetime = Brain.timer(timeUnits::msec);
		   //Brain.Screen.clearScreen();
		   //Brain.Screen.printAt(0,80,"Smack pressed");
		   Catapult.spin(directionType::rev, 200,velocityUnits::rpm);
		   sleep(300);
		   Catapult.stop(brakeType::coast);
		   isCollectorReady = false;
		   isCollectorPriming = false;
	   } else {
		  isCollectorPriming = true;
		  colprimetime = Brain.timer(timeUnits::msec);
		  //Brain.Screen.clearScreen();
		  //Brain.Screen.printAt(0,80,"Smack pressed, winding down");
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
		//Brain.Screen.printAt(10,40, "Ready Position Reached");
		isCatapultReady = true;
	}

	void resetCat() {
		smack();
		trebuchet();
	}

	void pew() {
		canPew = true;
	}

	void smak() {
		canSmak = true;
	}
	
	void park() {
		//TODO add calibration check
		//drives forward until parks
		//GyroPitch.startCalibration();
		//sleep(1500);
		canStopLift = false;
		isAutonBase = true;
		initGyro = GyroPitch.value(rotationUnits::deg);
		
		intake();
		drive(200);
		while(abs(GyroPitch.value(rotationUnits::deg) - initGyro) < 12) {
			sleep(20);
		}
		while(abs(GyroPitch.value(rotationUnits::deg) - initGyro) > 2) {
			sleep(20);
		}
		sleep(200);
		intakeStop();
		stopBase();
		
		canStopLift = true;
		isAutonBase = false;
	}
	
	void placeInit() {
		canStopLift = false;
		isAutonBase = true;
		
		Brain.Screen.printAt(0,140, "cap place init");
		
		//intake();
		//init cap place task
		sleep(300);
		while (!Controller.ButtonLeft.pressing()){
			task::sleep(20); //wait, testing button press
			
		}
		//stop cap place task
		Brain.Screen.printAt(0,140, "cap place fin");
		intakeStop();
		stopBase();
		
		canStopLift = true;
		isAutonBase = false;
	}
	
	void placeCap() {
		canStopLift = false;
		isAutonBase = true;
		
		Brain.Screen.printAt(0,140, "cap place init");
		
		//lift to top
		liftTop();
		
		double initLift = Intake.rotation(rotationUnits::deg);
		liftDown();
		drive(90);
		sleep(650);
		stopBase();
		//liftStop();
		foldUp();
		//Controller.rumble("-");
		canStopLift = true;
		isAutonBase = false;
	}
	
	int placeCallback() {
		return 1;
	}
	

	///////////////////////////////////////// TASKS /////////////////////////////////////////////////////////////////

	int launchCallback() {
		for(;;) {
			// if(isBallMode) Controller.ButtonR1.pressed(trebuchet);
			// Controller.ButtonL1.pressed(smack);
			if(isBallMode && canPew) trebuchet();
			if(canSmak) smack();
			//Brain.Screen.printAt(10,150,"Launch Callback Running...");
			task::sleep(40);
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
		  
		  //Controller.ButtonRight.pressed(toggleLEDs);
			
		  //Controller.ButtonRight.pressed(toggleMode);
		  //Controller.ButtonRight.released(delayLEDsOff);
			
		  //COMMENT OUT DURING COMPETITION
		  //Controller.ButtonLeft.pressed(resetCat);
		  Controller.ButtonLeft.pressed(liftTop);
		  Controller.ButtonRight.pressed(placeCap);
		  //Controller.ButtonRight.pressed(flipReset);

		  Controller.ButtonUp.pressed(foldUp);
		  Controller.ButtonDown.pressed(flipDown);
		  Controller.ButtonB.pressed(flipOne);
		  
		  Controller.ButtonR1.pressed(pew);
		  Controller.ButtonL1.pressed(smak);
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
	//task placeTask = task(placeCallback);
	
	///////////////////////////////////////// AUTONS /////////////////////////////////////////////////////////////////	


	
	
	void parkmatch() {
		
	}

	void fetch() {
		//nabs bol
		intake();
		driveFor(33);
		//sleep(500);
		driveFor(-39);
		//driveFor(-32,200,0.5,false, false);
		//intakeStop();
	}

	void fetchFlip(bool plsPrimeCatapult=true) {		
		smackFromButton(130);
		intake();
		
		driveFor(-12, 200, 0.8);				//drive back before lowering
		//intakeStop();				//balls often not fully intaken
		isCollectorReady=false;
		torqueLimit = 1.5;
		smak(); 					//lower collector
		sleep(300);
		driveFor(12);
		if(plsPrimeCatapult)pew(); 	//flip cap and lower cat
		else smak(); 				//just flip cap
		torqueLimit = 1.0;
		sleep(300);
		//intake(); 					//continue intaking balls
	}

	void anticross(int side=RIGHT){
		//fire mid first, our column last
		intake();
		driveFor(33);
		//rotFor(side * 73); //aim for middle flags
		//sleep(300);
		//trebuchet();
		//intakeStop();
		rotForGyro(side * 45); //45 degrees
		diagFor(side*13, side*NEG);
		strafeFor(side*20);
		
		intakeSpeed(100); 	//spinning at 40rpm
		driveFor(6,50,0.7); //slow drive nom cap
		sleep(700);			//wait for balls to settle
		
		
		//pew(); //reloads, no work for some reason
		//sleep(100); //wait for fire
		Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
		sleep(400);
		Catapult.stop();
		//sleep(200); //allow button to settle
		//smack lower until resistance
		fetchFlip();		//get balls, flip cap
		
		driveFor(-6); 		//back up from cap
		strafeFor(side*15);
		rotForGyro(-47);
		driveFor(22, 200, 0.8);
		intakeStop();
		driveFor(-39);
		// rotFor(side*44);	//turn and face 
		// driveFor(-8);		//back up a lil
		// pew();				//fire column
		// sleep(200);			//wait for fire
		// rotTo(-89);			//turn 90 rel to start
		//driveFor();		//
		sleep(1500);
		intakeStop();
		
		//Controller.rumble("- -");
		
		sleep(500);
		stopBase();
	}


	void oleReliable(int side=RIGHT) {
		fetch();
		if(side == LEFT) rotForGyro(-86); 		// face flags
		else rotForGyro(83);
		//driveFor(-3);
		sleep(400);
		
		Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
		sleep(500);
		Catapult.stop();
		
		intakeStop();
		sleep(400);
		if(side == LEFT) {
			rotForGyro(side * 9);
			driveFor(40, 200, 0.8);			// toggle bottom flag
			driveFor(-26);
		}
		else {
			rotForGyro(side * 8);
			driveFor(44, 200, 0.8);
			driveFor(-21);
		}
		rotFor(side * -43);
		if(side == LEFT)strafeFor(side*-15); 	// strafe to cap
		else strafeFor(-17);
		intakeSpeed(100);
		driveFor(2,50,0.7); 	//slow drive nom cap
		fetchFlip();			// get balls, flip cap*/
		driveFor(-6);
		sleep(1500);
		//strafeFor(side*-10);
		//sleep(1500);
		//pew();
		intakeStop();
	}

	void testing(motor *m, int vel) {
		m->spin(directionType::fwd, vel, velocityUnits::rpm);
	}
	
	void backPark(int side=RIGHT) {
		intake();
		driveFor(34);
		//driveFor(-2);
		rotFor(side * 90);
		park();
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
	    isAutonBase = true;
		isBallMode = true;
		isCatapultReady = true;
		isCollectorReady = false;
		isBraking = false;
		amIBlue = true;
		areLEDsOn = false;
		
		
		Flipper.setStopping(brakeType::hold);
		
		BL_Base.setStopping(brakeType::hold);
		BR_Base.setStopping(brakeType::hold);
		FL_Base.setStopping(brakeType::hold);
		FR_Base.setStopping(brakeType::hold);
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
        //placeCap();
		//oleReliable(RIGHT);
        backPark(LEFT);
        //anticross(LEFT);
		//intakeSpeed(100);
		//park();
		//fetchFlip(true);
		//driveFor(-32);
		//testing(&Catapult, 200);
		//sleep(100);
		//Catapult.stop();
		
		sleep(500);
		BL_Base.setStopping(brakeType::coast);
		BR_Base.setStopping(brakeType::coast);
		FL_Base.setStopping(brakeType::coast);
		FR_Base.setStopping(brakeType::coast);
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
		Brain.Screen.print("Robot is in Driver mode");
		isAutonBase = false;
		isCollectorPriming = false;
		isCatapultPriming = false;
		isCatapultReady = false;
		isCollectorReady = false;
		isBallMode = true;
		
		BL_Base.setStopping(brakeType::coast);
		BR_Base.setStopping(brakeType::coast);
		FL_Base.setStopping(brakeType::coast);
		FR_Base.setStopping(brakeType::coast);
		
		Flipper.setStopping(brakeType::hold);
		flipReset();
		
	  while (1) {
		// This is the main execution loop for the user control program.
		// Each time through the loop your program should update motor + servo 
		// values based on feedback from the joysticks.
		  
		  //Controller.ButtonL2.pressed(toggleBrake);
		  
		  FB = Controller.Axis3.position(percentUnits::pct);
		  LR = Controller.Axis4.position(percentUnits::pct);
		  
		  if (!isAutonBase) {
			  FR = isBallMode*-2*(FB - isBallMode*T - LR);
			  FL = isBallMode*2*(FB + isBallMode*T + LR);
			  BR = isBallMode*-2*(FB - isBallMode*T + LR);
			  BL = isBallMode*2*(FB + isBallMode*T - LR);
		  }
		  
		  Lift = Controller.Axis2.position(percentUnits::pct);
		  
		  if (Lift > 80) {
			  T = 0;
			  Intake.spin(directionType::fwd, Lift, velocityUnits::pct);
			  Intake2.spin(directionType::fwd, Lift, velocityUnits::pct);
			  //if(Flipper.rotation(rotationUnits::deg) != placePos) Flipper.rotateTo(placePos, rotationUnits::deg, 30, velocityUnits::rpm); //fold flipper to hold
			  canStopLift = true;
		  } else if (Lift < -80) {
			  Intake.spin(directionType::fwd, Lift, velocityUnits::pct);
			  Intake2.spin(directionType::fwd, Lift, velocityUnits::pct);
			  //if(Flipper.rotation(rotationUnits::deg) != downPos) Flipper.rotateTo(downPos, rotationUnits::deg, 30, velocityUnits::rpm); //fold flipper to hold
			  canStopLift = true;
		  } else {
			  T = Controller.Axis1.value();
			  if (canStopLift) {
				  Intake.setVelocity(10, velocityUnits::rpm);
				  Intake2.setVelocity(10, velocityUnits::rpm); //set hold velocity
				  Intake.stop(brakeType::hold);					// hold lift
				  Intake2.stop(brakeType::hold);
			  }
		  }
		  
		  /* FL_Base.spin(directionType::fwd,isBallMode*2*(FB + isBallMode*T + LR),velocityUnits::rpm);
		  FR_Base.spin(directionType::rev,isBallMode*2*(FB - isBallMode*T - LR),velocityUnits::rpm);
		  BL_Base.spin(directionType::fwd,isBallMode*2*(FB + isBallMode*T - LR),velocityUnits::rpm);
		  BR_Base.spin(directionType::rev,isBallMode*2*(FB - isBallMode*T + LR),velocityUnits::rpm); */

		task::sleep(10); //Sleep the task for a short amount of time to prevent wasted resources. 
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