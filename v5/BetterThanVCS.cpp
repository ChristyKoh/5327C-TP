	#include "robot-config.h"
	
	// vision related define

	#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
	#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))
	#define ABS(N) ((N<0)?(-N):(N))
	#define XCAMMERAMAX 320
	#define YCAMMERAMAX 240
	#define CAMERAAVE 4 // this is the running average for all both x-pixel and y-pixels
	#define CAMERASHIFT 2 // 2^2 = 4 = CAMERAAVE, use a factor of 2 for integer shifts
	#define YBINSNUM 16 // use a course resolution to keep things simple, was 32 for object identifications
	#define YBINOFFSET 8 // computed from 0.5* YCAMMERAMAX/YBINSNUM, for Brain Screen printing
	#define MAXOBJBIN 3 // maximum number of for detected objects
	#define RED_BRIGHTNESS 100 // these are starting brightness, able to adjust during run time
	#define GREEN_BRIGHTNESS 40
	#define BLUE_BRIGHTNESS 60
	#define SCREEN_UDATE_COUNT 32
	#define SAMPLE_COUNT_TO_TRACKING 16


	#define YBINLOWERLIMIT 135 // define to use to pick the desired object parse by yhist()
	#define XTARGET_AUTON_ALLIANCE_BLUE 180
	#define XTARGET_AUTON_ALLIANCE_RED 155
	#define XTARGET_RED 165
	#define XTARGET_BLUE 180
	#define XTARGET_GREEN_SHOOT_RED 160 // blue alliance
	#define XTARGET_GREEN_SHOOT_BLUE 135 // red alliance
	#define XTARGET_SKILLS 155 // skills run
	#define STRAFE_RECENTER true
	#define	ROT_RECENTER false
	#define ALLIANCE  3// 0 is red, 1 is blue, 2 is testing, 3 is skills

	#define AUTON_SELECT 4
	// 0 is oleReliable
	// 1 is pReliable
	// 2 is backCap
	// 3 is backPark
	// 4 is progSkills
	// 5 is park

	#define VISIONDEBUG 1// print out for vision debug
	
	#define YHIST_SELECT 0 // 1 to enable, 0 disable histogram based object tracking, green color
	#define YTRACKING 1 // this is for in the y direction, using the camera models

	#define YTRACKMINBIN 2 // assuming 80 ypixel as max spearateion bet flags 
	#define YTRACKMAXBIN 8 // assuming 40 ypixel as min spearateion bet flags
	#define YTRACKMAXBINPIXSIZE 90
	#define YTRACKMINBINPIXSIZE 50
	#define YTRACKMINCOUNT 4

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

	enum class DetectedObjState { Reset, Histogram, Tracking };

	// struct definition

	typedef struct {
		// used during Histrogram state
		// these need to be converted to be actual screen pixel units
		// for y screen position it should be lowerBinIndex*YCAMMERAMAX/YBINSNUM+YBINOFFSET
		// for x screen position, TODO
		int lowerBinIndex; //update during Histogram State
		int higherBinIndex; // update during Histogram State
		// used during Tracking state
		int lowerPixelBoundary; // update during Tracking State
		int higherPixelBoundary; // update during Tracking State
		int numberOfAve; // size of running average window
		int total; // y pixel total for efficient running ave
		int ary[CAMERAAVE];
		int index; // index for running average
		int ave; // the average
		int aveRef; // this is the average to track movement, mainly for rebining
		int binOffset; // binoffset to be apply to lowerPixelBoundary and upperPixelBoundary
		uint32_t y1UpdateCount; // frequency of histogram bin
	}bin_boundary_t;

	typedef struct {
		int xave; // average x pixel values by the vision cam
		int yave;
		int numYHistObjs;
		int numYHistTracking;
		int index; // current index for x pixel running ave
		int numberOfAve; // size of running average window 
		int xTotal; // x pixel total for efficient running ave
		int yTotal; // y pixel total for efficient running ave
		int xary[CAMERAAVE]; // raw x pixel coordinates
		int yary[CAMERAAVE]; // raw y pixel coordinates
		int yhist[YBINSNUM]; // y histogram with YBINSNUM bins
		DetectedObjState state; // state for the State Machine
		int objIndex; //this is the index of yBinBoundary or xBinBoundary tracking

		bin_boundary_t yBinBoundary[MAXOBJBIN];
		bin_boundary_t xBinBoundary[MAXOBJBIN];
#if (YTRACKING == 1)
		int y1BinCount; // number of bins, do not use this for computations, only for comparisons
		int y1BinStep; // use this to compute bin count, index is computed from ave (in pixel) / y1BinStep
		int y1BinOffset;
		int activeY1Flag; // takes the value of 0 or 1 depending on y1BinBoundary or y2BinBoundary
		volatile bin_boundary_t *activeY1BinBoundary;// pointing to active binBoundary for updateVision to update
		volatile bin_boundary_t *activeX1BinBoundary;// pointing to active binBoundary for updateVision to update
		bin_boundary_t y1BinBoundary[YTRACKMAXBIN]; // 40 pixel is the min difference in pixel in y direction => there is 6 bins
		bin_boundary_t x1BinBoundary[YTRACKMAXBIN];
		bin_boundary_t y2BinBoundary[YTRACKMAXBIN]; // alternate for saving data
		bin_boundary_t x2BinBoundary[YTRACKMAXBIN];
#endif
	} visionObj_t;


	typedef struct {
		bool run;
		int delayInterval; // delay before next snapshot, not shorter than 100ms.
		int XlimLeft;
		int XlimRight;
		int Xtarget;

	}control_t;


	

	// pointers to member functions https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.cbclx01/cplr034.htm


	/////////////////////////////////////// VARIABLES /////////////////////////////////////////////////////////////
	// vision
	volatile visionObj_t DetectedObjs;
	volatile control_t VisionCtrl;
	volatile bin_boundary_t* xbin_pointer = NULL;// used for tracking
	volatile bin_boundary_t* ybin_pointer = NULL;// used for tracking
	volatile int selectYPix = YBINLOWERLIMIT; // this allows selection of the objs programatically for skills
	volatile int sigCounter = 1; // green is default
	volatile int redBrightness = RED_BRIGHTNESS;
	volatile int greenBrightness = GREEN_BRIGHTNESS;
	volatile int blueBrightness = BLUE_BRIGHTNESS;
	volatile int redXtarget = XTARGET_RED;
	volatile int blueXtarget = XTARGET_BLUE;
	#if (ALLIANCE == 0) // red alliance
		volatile int greenXtarget = XTARGET_GREEN_SHOOT_BLUE;
	#else
		volatile int greenXtarget = XTARGET_GREEN_SHOOT_RED;
	#endif


	//status
	int isBallMode = 1; //ball mode 1, cap mode -1
	bool amIBlue = true;
	bool areLEDsOn = false;

	//sensor
	int BaseR, BaseL, BaseF, BaseB, avgBaseFwd, avgBaseRot;
	
	//base
	bool isAutonBase = false;
	bool isBraking = false;
	bool runningBaseMethod = false;
	int FB, LR, T, Lift;
	int FL, FR, BL, BR;
	float initGyro;			//volatile gyro var
	
	//catapult
	bool isCatapultReady = false;
	bool isCatapultPriming = false;
	bool canPew = false;
	bool isCollectorPriming = false;
	bool isCollectorReady = false;
	bool canSmak = false;
	
	//lift
	bool isPlaceReady = false;
	bool canStopLift = true;
	
	//flipper
	int currPos = 0;
	int foldPos = 20;
	int downPos = 253;
	int placePos = 200;//192;
	int flipPos = 90;
	float torqueLimit = 1.0;
	
	//autonomous
	int driveThread_distance;
	int driveThread_max = 200;
	int driveThread_kP = 0.5;

	//timers
	double ledtime, modetime, drivetime, catprimetime, colprimetime, torquetime, lifttime;

	/////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////
	void swap(int *xp, int *yp)
	{
		int temp = *xp;
		*xp = *yp;
		*yp = temp;
	}

	void bubbleSort(int arr[], int n)
	{
		int i, j;
		for (i = 0; i < n - 1; i++)

			// Last i elements are already in place    
			for (j = 0; j < n - i - 1; j++)
				if (arr[j] > arr[j + 1])
					swap(&arr[j], &arr[j + 1]);
	}

	void printConfig() {
		Brain.Screen.setFont(vex::fontType::mono15);

		int alliance = ALLIANCE;
		Brain.Screen.setCursor(1, 1);
		switch (alliance) {
			case 0:
				Brain.Screen.print("Alliance: Red");
				break;
			case 1:
				Brain.Screen.print("Alliance: Blue");
				break;
			case 3:
				Brain.Screen.print("Alliance: Skills");
				break;
		}

		int auton_select = AUTON_SELECT;
		Brain.Screen.setCursor(2, 1);
		switch (auton_select) {
			case 0:
				Brain.Screen.print("Auton: oleReliable");
				break;
			case 1:
				Brain.Screen.print("Auton: pReliable");
				break;
			case 2:
				Brain.Screen.print("Auton: backCap");
				break;
			case 3:
				Brain.Screen.print("Auton: backPark");
				break;
			case 4:
				Brain.Screen.print("Auton: progSkills");
				break;
			}
	}

	void printTracking() {
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		int loop = 0;
		VisionCtrl.run = false;
		vex::task::sleep(100);
		//DetectedObjs.state = DetectedObjState::Histogram;
#if (VISIONDEBUG == 1)
		Brain.Screen.clearScreen();
#endif
		//reset everything
		DetectedObjs.xave = 0;
		DetectedObjs.yave = 0;
		DetectedObjs.numYHistObjs = 0;
		DetectedObjs.numYHistTracking = 0;
		DetectedObjs.index = 0;
		DetectedObjs.numberOfAve = CAMERAAVE;
		for (loop = 0; loop < CAMERAAVE; loop++) {
			DetectedObjs.xary[loop] = 0;
			DetectedObjs.yary[loop] = 0;
		}
		DetectedObjs.xTotal = 0;
		DetectedObjs.yTotal = 0;
		DetectedObjs.state = DetectedObjState::Histogram;
		DetectedObjs.objIndex = -1;

		// init the histogram
		for (loop = 0; loop < YBINSNUM; loop++) {
			DetectedObjs.yhist[loop] = 0;
		}

		for (loop = 0; loop < MAXOBJBIN; loop++) {

			DetectedObjs.yBinBoundary[loop].lowerBinIndex = 0; //update during Histogram State
			DetectedObjs.yBinBoundary[loop].higherBinIndex = 0; // update during Histogram State
			// used during Tracking state
			DetectedObjs.yBinBoundary[loop].lowerPixelBoundary = 0; // update during Tracking State
			DetectedObjs.yBinBoundary[loop].higherPixelBoundary = 0; // update during Tracking State
			DetectedObjs.yBinBoundary[loop].numberOfAve = 4; // size of running average window
			DetectedObjs.yBinBoundary[loop].total = 0; // y pixel total for efficient running ave
			DetectedObjs.yBinBoundary[loop].ave = 0;
			DetectedObjs.yBinBoundary[loop].index = 0;

			// clear xbin
			DetectedObjs.xBinBoundary[loop].lowerBinIndex = 0; //update during Histogram State
			DetectedObjs.xBinBoundary[loop].higherBinIndex = 0; // update during Histogram State
			// used during Tracking state
			DetectedObjs.xBinBoundary[loop].lowerPixelBoundary = 0; // update during Tracking State
			DetectedObjs.xBinBoundary[loop].higherPixelBoundary = 0; // update during Tracking State
			DetectedObjs.xBinBoundary[loop].numberOfAve = 4; // size of running average window
			DetectedObjs.xBinBoundary[loop].total = 0; // y pixel total for efficient running ave
			DetectedObjs.xBinBoundary[loop].ave = 0;
			DetectedObjs.xBinBoundary[loop].index = 0;
			for (int j = 0; j < CAMERAAVE; j++) {

				DetectedObjs.yBinBoundary[loop].ary[j] = 0;
				DetectedObjs.xBinBoundary[loop].ary[j] = 0;
			}

		}

#if (YTRACKING == 1)
		for (loop = 0; loop < YTRACKMAXBIN; loop++) {

			DetectedObjs.y1BinBoundary[loop].lowerBinIndex = 0; //update during Histogram State
			DetectedObjs.y1BinBoundary[loop].higherBinIndex = 0; // update during Histogram State
			// used during Tracking state
			DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary = 0; // update during Tracking State
			DetectedObjs.y1BinBoundary[loop].higherPixelBoundary = 0; // update during Tracking State
			DetectedObjs.y1BinBoundary[loop].numberOfAve = 4; // size of running average window
			DetectedObjs.y1BinBoundary[loop].total = 0; // y pixel total for efficient running ave
			DetectedObjs.y1BinBoundary[loop].ave = 0;
			DetectedObjs.y1BinBoundary[loop].index = 0;
			DetectedObjs.y1BinBoundary[loop].aveRef = 0; // this is the average to track movement, mainly for rebining
			DetectedObjs.y1BinBoundary[loop].binOffset = 0; // binoffset to be apply to lowerPixelBoundary and upperPixelBoundary
			DetectedObjs.y1BinBoundary[loop].y1UpdateCount = 0; // frequency of histogram bin


			// clear xbin
			DetectedObjs.x1BinBoundary[loop].lowerBinIndex = 0; //update during Histogram State
			DetectedObjs.x1BinBoundary[loop].higherBinIndex = 0; // update during Histogram State
			// used during Tracking state
			DetectedObjs.x1BinBoundary[loop].lowerPixelBoundary = 0; // update during Tracking State
			DetectedObjs.x1BinBoundary[loop].higherPixelBoundary = 0; // update during Tracking State
			DetectedObjs.x1BinBoundary[loop].numberOfAve = 4; // size of running average window
			DetectedObjs.x1BinBoundary[loop].total = 0; // y pixel total for efficient running ave
			DetectedObjs.xBinBoundary[loop].ave = 0;
			DetectedObjs.x1BinBoundary[loop].index = 0;
			DetectedObjs.x1BinBoundary[loop].aveRef = 0; // this is the average to track movement, mainly for rebining
			DetectedObjs.x1BinBoundary[loop].binOffset = 0; // binoffset to be apply to lowerPixelBoundary and upperPixelBoundary
			DetectedObjs.x1BinBoundary[loop].y1UpdateCount = 0; // frequency of histogram bin
			for (int j = 0; j < CAMERAAVE; j++) {

				DetectedObjs.y1BinBoundary[loop].ary[j] = 0;
				DetectedObjs.x1BinBoundary[loop].ary[j] = 0;
			}

		}

#endif
		switch (sigCounter) {
		case 0:
#if (VISIONDEBUG == 1)              
			Brain.Screen.printAt(10, 40, "blue");
#endif
			Controller.Screen.print("blue");
			break;
		case 1:
#if (VISIONDEBUG == 1)  
			Brain.Screen.printAt(10, 40, "green");
#endif
			Controller.Screen.print("green");
			break;
		case 2:
#if (VISIONDEBUG == 1)  
			Brain.Screen.printAt(10, 40, "red");
#endif
			Controller.Screen.print("red");
			break;
		}
		xbin_pointer = NULL;
		ybin_pointer = NULL;
		VisionCtrl.run = true;
	}

	void yhist() {
		//print y histogram
#if (VISIONDEBUG == 1)  
		Brain.Screen.clearScreen();
		Brain.Screen.setPenColor(vex::color::blue);
#endif
		int loop;
		int delta;
		double step = YCAMMERAMAX / YBINSNUM;
		int ypos = 0;
		int lastBinIndex = 0; // index into DetectedObjs.yhist[] with zero count, ie no object detected
		bool updateLowerBinIndex = true; // lower because the index val is smaller numerically

		int objectIndex = 0;
		int binStep = 0; // unit is pixel
		int binOffset = 0; // unit is pixel
		int numOfBins = 0;
		int binNumber = 0;
		int tp = 0;
		bin_boundary_t tpBoundary;
		int ind = 0;
		bool found = false;
		int ary[3] = { 0 ,0 ,0 };

		for (loop = 0; loop < YBINSNUM; loop++) {
			delta = 480 - DetectedObjs.yhist[loop];
			ypos = YBINOFFSET + step * loop; // position on the screen to draw line

			if (delta == 480) {
				// nothing detected in this histogram bin, DetectedObjs.yhist[loop] = 0;
#if (VISIONDEBUG == 1)  
				Brain.Screen.setPenColor(vex::color::yellow);
				Brain.Screen.drawLine(470, ypos, 480, ypos);
#endif
				if (loop > 0) {
					// check if the last zero and current bin is zero
					if (((loop - lastBinIndex) > 1) && (objectIndex < MAXOBJBIN)) {
						// non-consecutive zero value bin detected

						// update DetectedObjs lower and upper bin indices
						//Brain.Screen.setPenColor(vex::color::white);
						DetectedObjs.yBinBoundary[objectIndex].lowerBinIndex = lastBinIndex;//lastBinIndex+1;
							//Brain.Screen.drawLine(0,(lastBinIndex+1)*step+YBINOFFSET,480,(lastBinIndex+1)*step+YBINOFFSET);
						DetectedObjs.yBinBoundary[objectIndex].higherBinIndex = loop;
						// estimate the object y pixel value
						DetectedObjs.yBinBoundary[objectIndex].ave = (loop*step + YBINOFFSET + lastBinIndex * step + YBINOFFSET) / 2;
						//Brain.Screen.drawLine(0,loop*step+YBINOFFSET,480,loop*step+YBINOFFSET);
						//Brain.Screen.setPenColor(vex::color::blue);
						//updateLowerBinIndex = false;
						//updateHigherBinIndex = true;
						objectIndex++;

					}// non-consecutive zero, boundary
				}
				lastBinIndex = loop;
			} // when the DetectedObjs.yhist[loop] is zero
			else {
#if (VISIONDEBUG == 1)  
				Brain.Screen.setPenColor(vex::color::blue);
				Brain.Screen.drawLine(delta, ypos, 480, ypos);
#endif
			}
			// reset the histogram
			DetectedObjs.yhist[loop] = 0;

		}// end looping through all the bins

		DetectedObjs.numYHistObjs = objectIndex;

		objectIndex = 0;
		// replot to check
		int ypos_prev = 0;
		for (loop = 0; loop < (DetectedObjs.numYHistObjs - 1); loop++) {
			ypos = (DetectedObjs.yBinBoundary[loop].higherBinIndex + DetectedObjs.yBinBoundary[loop + 1].lowerBinIndex) / 2;
			ypos = ypos * step + YBINOFFSET; // convert to actual pixel
#if (VISIONDEBUG == 1)  
			Brain.Screen.setPenColor(vex::color::white);
			//Brain.Screen.drawLine(0,ypos,480,ypos);

			//Brain.Screen.setCursor(loop+2,6);
			//Brain.Screen.print("objs %d, %d, %d",loop,DetectedObjs.yBinBoundary[loop].lowerBinIndex,DetectedObjs.yBinBoundary[loop].higherBinIndex);
			//Brain.Screen.drawLine(0,DetectedObjs.yBinBoundary[loop].lowerBinIndex*step+YBINOFFSET,480,DetectedObjs.yBinBoundary[loop].lowerBinIndex*step+YBINOFFSET);
			//Brain.Screen.drawLine(0,DetectedObjs.yBinBoundary[loop].higherBinIndex*step+YBINOFFSET,480,DetectedObjs.yBinBoundary[loop].higherBinIndex*step+YBINOFFSET);
#endif        
		//update the pixel boundaries
			if (loop == 0) {
				DetectedObjs.yBinBoundary[loop].lowerPixelBoundary = (DetectedObjs.yBinBoundary[loop].lowerBinIndex*step + YBINOFFSET) / 2;

			}
			else {
				DetectedObjs.yBinBoundary[loop].lowerPixelBoundary = ypos_prev;
			}
			DetectedObjs.yBinBoundary[loop].higherPixelBoundary = ypos;
			ypos_prev = ypos;
			// debug if boundary is computed correctly
#if (VISIONDEBUG == 1)  
			Brain.Screen.drawLine(0, DetectedObjs.yBinBoundary[loop].lowerPixelBoundary, 480, DetectedObjs.yBinBoundary[loop].lowerPixelBoundary);
			Brain.Screen.printAt(40, DetectedObjs.yBinBoundary[loop].lowerPixelBoundary, "%d", DetectedObjs.yBinBoundary[loop].lowerPixelBoundary);
			Brain.Screen.drawLine(0, DetectedObjs.yBinBoundary[loop].higherPixelBoundary, 480, DetectedObjs.yBinBoundary[loop].higherPixelBoundary);
			Brain.Screen.printAt(40, DetectedObjs.yBinBoundary[loop].higherPixelBoundary, "%d", DetectedObjs.yBinBoundary[loop].higherPixelBoundary);
#endif
		}
		// update the last object, where loop is pointing to
		DetectedObjs.yBinBoundary[loop].lowerPixelBoundary = ypos_prev;
		DetectedObjs.yBinBoundary[loop].higherPixelBoundary = (DetectedObjs.yBinBoundary[loop].higherBinIndex*step + YBINOFFSET +
			YCAMMERAMAX) / 2;
#if (VISIONDEBUG == 1)  
		/*
		Brain.Screen.drawLine(0,DetectedObjs.yBinBoundary[loop].lowerPixelBoundary ,480,DetectedObjs.yBinBoundary[loop].lowerPixelBoundary);
		Brain.Screen.drawLine(0,DetectedObjs.yBinBoundary[loop].higherPixelBoundary ,480,DetectedObjs.yBinBoundary[loop].higherPixelBoundary);
		Brain.Screen.printAt(40,DetectedObjs.yBinBoundary[loop].higherPixelBoundary,"%d",DetectedObjs.yBinBoundary[loop].lowerPixelBoundary );
		Brain.Screen.printAt(40,DetectedObjs.yBinBoundary[loop].higherPixelBoundary,"%d",DetectedObjs.yBinBoundary[loop].higherPixelBoundary );
		*/
		Brain.Screen.setCursor(4, 3);
		Brain.Screen.print("yobjs is : %d", DetectedObjs.numYHistObjs);
#endif    
		// section to make the object selection
		// only green is enabled for object selection, ie sigCounter == 1 (0 = blue, 1 = green, 2 = red)
#if (YHIST_SELECT == 1)
		if ((DetectedObjs.numYHistObjs > 0) && (sigCounter == 1)) {
			for (loop = 0; loop < (DetectedObjs.numYHistObjs); loop++) {
				if ((DetectedObjs.yBinBoundary[loop].higherPixelBoundary > selectYPix) &&
					(DetectedObjs.yBinBoundary[loop].lowerPixelBoundary <= selectYPix)) {
					// selectYPix (global variable) or YBINLOWERLIMIT defines the y pixel value where the higherPixelBoundary and lowerPiexelBoundary
					// must fall in between
					DetectedObjs.objIndex = loop;
					xbin_pointer = &DetectedObjs.xBinBoundary[loop];
					ybin_pointer = &DetectedObjs.yBinBoundary[loop];
				}
			}
		}
#endif

#if (YTRACKING == 1)
		// this uses different bins with fixed lower and upper boundary to enable tracking
		//
		//Brain.Screen.setFont(vex::fontType::mono12);
		switch (DetectedObjs.numYHistObjs)
		{
		case 0:
			// nothing detected, break
			break;
		case 1:

			// 1 object detected, but use min 3 bins, this is arbitrary, might need to change
			// compute binOffset
			binStep = 60;//YCAMMERAMAX/YTRACKMINBIN; // 80
			DetectedObjs.y1BinCount = YCAMMERAMAX / binStep;
			DetectedObjs.y1BinStep = binStep;
			//figure out which bin the found object belongs
			binNumber = DetectedObjs.yBinBoundary[0].ave / binStep;// * YTRACKMINBIN/YCAMMERAMAX;
			tpBoundary.lowerPixelBoundary = binNumber * binStep;
			tpBoundary.higherPixelBoundary = (binNumber + 1)*binStep;
			tpBoundary.binOffset = tpBoundary.lowerPixelBoundary + binStep / 2 - DetectedObjs.yBinBoundary[0].ave;
			tpBoundary.ave = DetectedObjs.yBinBoundary[0].ave;

			DetectedObjs.y1BinOffset = tpBoundary.binOffset;

			for (loop = 0; loop < DetectedObjs.y1BinCount; loop++)
			{
				// compute offset from ideal boundaries based on detected object
				tp = loop * binStep - tpBoundary.binOffset;
				DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary = MAX(tp, 0);
				tp = (loop + 1)* binStep - tpBoundary.binOffset;
				DetectedObjs.y1BinBoundary[loop].higherPixelBoundary = MIN(tp, YCAMMERAMAX);
				DetectedObjs.y1BinBoundary[loop].binOffset = tpBoundary.binOffset;
				if (loop == binNumber)
				{
					DetectedObjs.y1BinBoundary[loop].ave = tpBoundary.ave;
					DetectedObjs.y1BinBoundary[loop].aveRef = DetectedObjs.y1BinBoundary[loop].ave;
					// check if the bin is in the right range, used for auton and match aiming
					if ((DetectedObjs.yBinBoundary[0].higherPixelBoundary > selectYPix) &&
						(DetectedObjs.yBinBoundary[0].lowerPixelBoundary <= selectYPix))
					{
						// selectYPix (global variable) , YBINLOWERLIMIT defines the y pixel value where the higherPixelBoundary and lowerPiexelBoundary
						// must fall in between
						DetectedObjs.objIndex = loop;
						xbin_pointer = &DetectedObjs.x1BinBoundary[loop];
						ybin_pointer = &DetectedObjs.y1BinBoundary[loop];
						DetectedObjs.activeY1BinBoundary = &(DetectedObjs.y1BinBoundary[0]);// pointing to active binBoundary for updateVision to update
						DetectedObjs.activeX1BinBoundary = &(DetectedObjs.x1BinBoundary[0]);// pointing to active binBoundary for updateVision to update
					}
				}
				else
				{
					DetectedObjs.y1BinBoundary[loop].ave = 0;
					DetectedObjs.y1BinBoundary[loop].aveRef = DetectedObjs.y1BinBoundary[loop].ave;
				}
#if (VISIONDEBUG == 1) 
				Brain.Screen.setPenColor(vex::color::green);
				Brain.Screen.drawLine(0, DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary, 360, DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary);
				Brain.Screen.printAt(40, DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary, "%d", DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary);
				Brain.Screen.drawLine(0, DetectedObjs.y1BinBoundary[loop].higherPixelBoundary, 360, DetectedObjs.y1BinBoundary[loop].higherPixelBoundary);
				Brain.Screen.printAt(40, DetectedObjs.y1BinBoundary[loop].higherPixelBoundary, "%d", DetectedObjs.y1BinBoundary[loop].higherPixelBoundary);
				Brain.Screen.setPenColor(vex::color::white);
#endif				
			}
			break;
		case 2:
		case 3:
			/*if (DetectedObjs.numYHistObjs ==2)
			{// top obj if there 2 detected}
				 selectYPix = DetectedObjs.yBinBoundary[0].ave;
			}
			if (DetectedObjs.numYHistObjs ==3)
			{// middle obj if there 2 detected}
				selectYPix = DetectedObjs.yBinBoundary[1].ave;
			}
			*/
			for (loop = 0; loop < DetectedObjs.numYHistObjs; loop++)
			{
				ary[loop] = DetectedObjs.yBinBoundary[loop].ave;	 // put in temp array for buble sort

			}

			// sort it so that it is increasing in y pixel
			bubbleSort(ary, DetectedObjs.numYHistObjs);

			for (loop = 0; loop < (DetectedObjs.numYHistObjs - 1); loop++)
			{
				binStep += (ary[loop + 1] - ary[loop]);
				/*
				Brain.Screen.clearLine(5,vex::color::black);
				Brain.Screen.setCursor(5,3);
				Brain.Screen.print("loop %d, binStep %d", loop, binStep);
				*/
			}
			binStep = binStep / (DetectedObjs.numYHistObjs - 1);

			//Brain.Screen.clearLine(5,vex::color::black);
			//Brain.Screen.setCursor(5,3);
			//Brain.Screen.print("binStep %d", binStep);
			//Brain.Screen.print("ind1 %d, ind0 %d",DetectedObjs.yBinBoundary[1].ave, DetectedObjs.yBinBoundary[0].ave);
			// limit size to about 80
			if (binStep > (YTRACKMAXBINPIXSIZE))
			{
				binStep = YTRACKMAXBINPIXSIZE;
			}
			if (binStep < YTRACKMINBINPIXSIZE)
			{
				binStep = YTRACKMINBINPIXSIZE;
			}
			numOfBins = (2 * YCAMMERAMAX + binStep) / 2 / binStep;
			DetectedObjs.y1BinCount = numOfBins; // update the bin size
			DetectedObjs.y1BinStep = binStep;

			//compute offset
			binOffset = 0;
			for (loop = 0; loop < (DetectedObjs.numYHistObjs - 1); loop++)
			{
				tp = (ary[loop + 1] + ary[loop]) / 2; // bin boundary in pixel

				binNumber = tp / binStep;
				int dummy1 = tp - binNumber * binStep;
				if (dummy1 >= binStep / 2)
				{
					binNumber = (tp + binStep - 1) / binStep; // round up of x/y is (x+y-1)/y for positive number
				}
				binOffset += tp - binNumber * binStep;
				/*
				Brain.Screen.clearLine(6+ loop, vex::color::black);
				Brain.Screen.setCursor(6+ loop,3);
				Brain.Screen.print("bin %d, #coff %d, Offset %d,ys %d, %d",binNumber,tp, binOffset,ary[loop],ary[loop+1]);
				*/

			}
			binOffset = binOffset / (DetectedObjs.numYHistObjs - 1);
			DetectedObjs.y1BinOffset = binOffset;

			Brain.Screen.clearLine(5, vex::color::black);
			Brain.Screen.setCursor(5, 3);
			Brain.Screen.print("binStep %d, #bin %d, binOffset %d", binStep, numOfBins, binOffset);

			// init all bins
			for (loop = 0; loop < numOfBins; loop++)
			{
				// initialize all bins
				tp = loop * binStep + binOffset;
				DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary = MAX(tp, 0);
				tp = (loop + 1)* binStep + binOffset;
				DetectedObjs.y1BinBoundary[loop].higherPixelBoundary = MIN(tp, YCAMMERAMAX);
				DetectedObjs.y1BinBoundary[loop].binOffset = binOffset;

				found = false;
				ind = 0;
				while ((ind < DetectedObjs.numYHistObjs) && (!found))
				{
					binNumber = (DetectedObjs.yBinBoundary[ind].ave - binOffset) / binStep;
					if (binNumber == loop)
					{
						DetectedObjs.y1BinBoundary[loop].ave = DetectedObjs.yBinBoundary[ind].ave;
						DetectedObjs.y1BinBoundary[loop].aveRef = DetectedObjs.y1BinBoundary[loop].ave;
						if ((DetectedObjs.yBinBoundary[ind].higherPixelBoundary > selectYPix) &&
							(DetectedObjs.yBinBoundary[ind].lowerPixelBoundary <= selectYPix))
						{
							// selectYPix (global variable) or YBINLOWERLIMIT defines the y pixel value where the higherPixelBoundary and lowerPiexelBoundary
							// must fall in between
							DetectedObjs.objIndex = loop;
							xbin_pointer = &DetectedObjs.x1BinBoundary[loop];
							ybin_pointer = &DetectedObjs.y1BinBoundary[loop];
							DetectedObjs.activeY1Flag = 0;
							DetectedObjs.activeY1BinBoundary = &(DetectedObjs.y1BinBoundary[0]);// pointing to active binBoundary for updateVision to update
							DetectedObjs.activeX1BinBoundary = &(DetectedObjs.x1BinBoundary[0]);// pointing to active binBoundary for updateVision to update
							Brain.Screen.clearLine(9, vex::color::black);
							Brain.Screen.setCursor(9, 3);
							Brain.Screen.print("select bin# %d, y: %3d, offset %3d", binNumber, DetectedObjs.y1BinBoundary[loop].ave, binOffset);

						}
						found = true;
					}

					ind++;
				}
				if (!found)
				{
					DetectedObjs.y1BinBoundary[loop].ave = 0;
					DetectedObjs.y1BinBoundary[loop].aveRef = 0;
				}
#if (VISIONDEBUG == 1) 
				// need to add binoffset because plotting in the raw, not mapped pixels
				Brain.Screen.setPenColor(vex::color::green);
				Brain.Screen.drawLine(0, DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary, 360, DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary);
				Brain.Screen.printAt(400, DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary, "%d", DetectedObjs.y1BinBoundary[loop].lowerPixelBoundary);
				Brain.Screen.drawLine(0, DetectedObjs.y1BinBoundary[loop].higherPixelBoundary, 360, DetectedObjs.y1BinBoundary[loop].higherPixelBoundary);
				Brain.Screen.printAt(400, DetectedObjs.y1BinBoundary[loop].higherPixelBoundary, "%d", DetectedObjs.y1BinBoundary[loop].higherPixelBoundary);
				Brain.Screen.setPenColor(vex::color::white);
#endif
			}

			break;

		default:
			break;
		}

#endif 	
#if (VISIONDEBUG == 1)
		// print info to controller
		Controller.Screen.clearScreen();
		Controller.Screen.setCursor(2, 1);
		Controller.Screen.print("target: %d", VisionCtrl.Xtarget);
		Controller.Screen.setCursor(1, 1);
		switch (sigCounter) {
		case 0:
			Controller.Screen.print("blue %d", DetectedObjs.numYHistObjs);
			Controller.Screen.setCursor(3, 1);
			Controller.Screen.clearLine(3);
			Controller.Screen.print("b brightness %d", blueBrightness);
			break;
		case 1:
			Controller.Screen.print("green %d", DetectedObjs.numYHistObjs);
			Controller.Screen.setCursor(3, 1);
			Controller.Screen.clearLine(3);
			Controller.Screen.print("g brightness %d", greenBrightness);
			break;
		case 2:
			Controller.Screen.print("red %d", DetectedObjs.numYHistObjs);
			Controller.Screen.setCursor(3, 1);
			Controller.Screen.clearLine(3);
			Controller.Screen.print("r brightness %d", redBrightness);
			break;
		}
		DetectedObjs.state = DetectedObjState::Tracking;
		//vex::task::sleep(500);
#endif
	}

	void updateVision() {
		int xScreen = 0;
		int yScreen = 0;
		int ave = 0;
		int loop = 0;
		static int counter = 0; // counter to print output
		static int counterTrack = 0;
		int yind = 0;
		int oldestval = 0;
		int j = 0; //index for histogram resolved objects
		bool match = false;
		int numObjs = 0;
		int binNum = 0;
		volatile bin_boundary_t *tp_bin_boundary = NULL;
		int tpVal = 0;
		int remapCounter = 0;
		int count = 0;

		Brain.Screen.setFont(vex::fontType::mono15);
		while (true) {

			if (VisionCtrl.run) {


				switch (sigCounter) {
				case 0:
					//move to increaseSigCounter
					//Vision.setBrightness( 60 );
					//Brain.Screen.printAt(10,40,"blue");
					numObjs = Vision.takeSnapshot(SIG_1); // blue
					break;
				case 1:
					//move to increaseSigCounter
					//Vision.setBrightness( 60 );
					//Brain.Screen.printAt(10,40,"green");
					numObjs = Vision.takeSnapshot(SIG_2); // green
					break;
				case 2:
					//Vision.setBrightness( 70 ); 
					//Brain.Screen.printAt(10,40,"red");
					numObjs = Vision.takeSnapshot(SIG_3); //red
					break;
				case 3:
					numObjs = Vision.takeSnapshot(CO_FLAGRED);
					break;
				case 4:
					numObjs = Vision.takeSnapshot(CO_FLAGBLUE);
					break;	
				}

				//if (Vision.largestObject.exists && Vision.largestObject.width>5) {
				if ((Vision.objects[0].width > 2) ||
					(Vision.objects[0].height > 2)) { // true , Vision.largestObject.width>3 

				//scaling, LCD 480x240, camera 640x400

				//numObjs = MIN(3,Vision.objectCount) ;
					ave = 0;
					count = 0;
					for (loop = 0; loop < numObjs; loop++) {
						xScreen = Vision.objects[loop].centerX * 480;
						xScreen /= XCAMMERAMAX; // 640
						yScreen = Vision.objects[loop].centerY * 240;
						yScreen /= YCAMMERAMAX;
						// y is reverse for both camera and screen
#if (VISIONDEBUG == 1)  
						if ((counter%SCREEN_UDATE_COUNT) == 0) {
							//limit screen printing
							if (loop == 0) {

								Brain.Screen.drawCircle(xScreen, yScreen, 10, vex::color::green); // y 240 max

							}
							else {
								Brain.Screen.drawCircle(xScreen, yScreen, 5, vex::color::white);
							}
							Brain.Screen.printAt(xScreen + 10, yScreen, "%d, %d", Vision.objects[loop].centerX, Vision.objects[loop].centerY);
						}
#endif
						if (Vision.objects[loop].centerX <= 270)
						{
							ave += Vision.objects[loop].centerX;
							count++;
						}
						// update histogram or not depending on state machine

						yind = (Vision.objects[loop].centerY*YBINSNUM);
						yind /= YCAMMERAMAX;
						if (loop == 0)
							DetectedObjs.yhist[int(yind)] += 4;
						else
							DetectedObjs.yhist[int(yind)]++;

						//
						if (DetectedObjs.state == DetectedObjState::Tracking) {
#if (YTRACKING == 1)                    
							// compute bin index, in this case j
							match = false;
							j = 0;

							//while ( j< DetectedObjs.y1BinStep && !match)
							//{
								/*tp_bin_boundary = (DetectedObjs.activeY1BinBoundary + j);
								if( (Vision.objects[loop].centerY >= DetectedObjs.lowerPixelBoundary)
								   && (Vision.objects[loop].centerY < tp_bin_boundary->higherPixelBoundary))*/
							if ((Vision.objects[loop].centerY >= DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].lowerPixelBoundary)
								&& (Vision.objects[loop].centerY < DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].higherPixelBoundary))
							{
								// y1 bin boundary
								/*tp_bin_boundary =(DetectedObjs.activeY1BinBoundary + j);
								oldestval = tp_bin_boundary->ary[tp_bin_boundary->index] ;
								tp_bin_boundary->total = tp_bin_boundary->total -oldestval + Vision.objects[loop].centerY;
								tp_bin_boundary->ave  = tp_bin_boundary->total >> CAMERASHIFT;
								tp_bin_boundary->ary[tp_bin_boundary->index] = Vision.objects[loop].centerY;
								tp_bin_boundary->index++;
								tp_bin_boundary->index %=CAMERAAVE;
								tp_bin_boundary->y1UpdateCount++; */

								/*
								if (sem.owner())
								{
									sem.unlock();
								}
								*/
								//sem.lock();
								oldestval = DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ary[DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].index];
								DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].total = DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].total - oldestval + Vision.objects[loop].centerY; // update total, ave is the latest val
								DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ave = DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].total >> CAMERASHIFT;
								DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ary[DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].index] = Vision.objects[loop].centerY;
								DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].index++;
								DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].index %= CAMERAAVE;
								DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].y1UpdateCount++;

								oldestval = DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].ary[DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].index];
								DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].total = DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].total - oldestval + Vision.objects[loop].centerX; // update total, ave is the latest val
								DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].ave = DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].total >> CAMERASHIFT;
								DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].ary[DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].index] = Vision.objects[loop].centerX;
								DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].index++;
								DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].index %= CAMERAAVE;
								DetectedObjs.x1BinBoundary[DetectedObjs.objIndex].y1UpdateCount++;
								//x1 bin boundary
								/*
								tp_bin_boundary = (DetectedObjs.activeX1BinBoundary +  j);

								oldestval = tp_bin_boundary->ary[tp_bin_boundary->index] ;
								tp_bin_boundary->total = tp_bin_boundary->total -oldestval + Vision.objects[loop].centerX;
								tp_bin_boundary->ave  = tp_bin_boundary->total >> CAMERASHIFT;
								tp_bin_boundary->ary[tp_bin_boundary->index] = Vision.objects[loop].centerX;
								tp_bin_boundary->index++;
								tp_bin_boundary->index %=CAMERAAVE;
								tp_bin_boundary->y1UpdateCount++;
								*/
								//sem.unlock();
							}
							//j++;    
						//}   // end while


							if (DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].y1UpdateCount > YTRACKMINCOUNT)
							{
								// check for delta and trigger the rebining
								tpVal = DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ave - DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].aveRef;
								if (ABS(tpVal) > 15)
								{
									DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].lowerPixelBoundary += tpVal;
									DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].higherPixelBoundary += tpVal;
									DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].aveRef = DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ave;
									DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].y1UpdateCount = 0;

									/*
										Brain.Screen.clearLine(15,vex::color::black);
										Brain.Screen.setCursor(15,3);
										Brain.Screen.print("bin# %1d,y:%3d,yf:%3d,lb:%d,ub:%d",DetectedObjs.objIndex,tp_bin_boundary->ave,tp_bin_boundary->aveRef,
													   tp_bin_boundary->lowerPixelBoundary ,tp_bin_boundary->higherPixelBoundary);

									*/
									//remapYbins();

								}

								//counterTrack =0						
							}


#if (VISIONDEBUG == 1)                        
							if (((counter%SCREEN_UDATE_COUNT) == 0) && (ybin_pointer != NULL) && (xbin_pointer != NULL))
							{
								//remapCounter++;
								//remapCounter %=  DetectedObjs.y1BinCount;
								remapCounter = DetectedObjs.objIndex;
								//tp_bin_boundary = (DetectedObjs.activeY1BinBoundary + remapCounter);
								Brain.Screen.clearLine(16, vex::color::black);//Brain.Screen.clearLine(10,vex::color::black);
								Brain.Screen.setCursor(16, 3);//Brain.Screen.setCursor(10,3);
								if (remapCounter == DetectedObjs.objIndex)
								{
									/*
									Brain.Screen.print("**b# %d, y:%d,yf:%d,ym:%d,cnt:%d",remapCounter,tp_bin_boundary->ave,tp_bin_boundary->aveRef,
													   (tp_bin_boundary->lowerPixelBoundary + tp_bin_boundary->higherPixelBoundary)/2,
													   tp_bin_boundary->y1UpdateCount);*/
									Brain.Screen.print("**b# %d, y:%d,yf:%d,ym:%d,cnt:%d", remapCounter, DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ave,
										DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].aveRef,
										(DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].lowerPixelBoundary +
											DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].higherPixelBoundary) / 2,
										DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].y1UpdateCount);
								}
								else
								{
									/*
									Brain.Screen.print("b# %d, y:%d,yf:%d,ym:%d,cnt:%d",remapCounter,tp_bin_boundary->ave,tp_bin_boundary->aveRef,
													   (tp_bin_boundary->lowerPixelBoundary + tp_bin_boundary->higherPixelBoundary)/2,
													   tp_bin_boundary->y1UpdateCount);*/
									Brain.Screen.print("b# %d, y:%d,yf:%d,ym:%d,cnt:%d", remapCounter, DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].ave,
										DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].aveRef,
										(DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].lowerPixelBoundary +
											DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].higherPixelBoundary) / 2,
										DetectedObjs.y1BinBoundary[DetectedObjs.objIndex].y1UpdateCount);
								}

							}
#endif  

							// counterTrack is used to trigger the first computation in DetectedObjs.state == DetectedObjState::Tracking






#else                        
							// first check if for y-coordinates
							match = false;
							j = 0;
							while (j < DetectedObjs.numYHistObjs && !match) {
								if ((Vision.objects[loop].centerY >= DetectedObjs.yBinBoundary[j].lowerPixelBoundary)
									&& (Vision.objects[loop].centerY < DetectedObjs.yBinBoundary[j].higherPixelBoundary)) {
									match = true;

									oldestval = DetectedObjs.yBinBoundary[j].ary[DetectedObjs.yBinBoundary[j].index];
									DetectedObjs.yBinBoundary[j].total = DetectedObjs.yBinBoundary[j].total - oldestval + Vision.objects[loop].centerY; // update total, ave is the latest val
									DetectedObjs.yBinBoundary[j].ave = DetectedObjs.yBinBoundary[j].total >> CAMERASHIFT;
									DetectedObjs.yBinBoundary[j].ary[DetectedObjs.yBinBoundary[j].index] = Vision.objects[loop].centerY;
									DetectedObjs.yBinBoundary[j].index++;
									DetectedObjs.yBinBoundary[j].index %= CAMERAAVE;

									// updata xbin
									oldestval = DetectedObjs.xBinBoundary[j].ary[DetectedObjs.xBinBoundary[j].index];
									DetectedObjs.xBinBoundary[j].total = DetectedObjs.xBinBoundary[j].total - oldestval + Vision.objects[loop].centerX; // update total, ave is the latest val
									DetectedObjs.xBinBoundary[j].ave = DetectedObjs.xBinBoundary[j].total >> CAMERASHIFT;
									DetectedObjs.xBinBoundary[j].ary[DetectedObjs.xBinBoundary[j].index] = Vision.objects[loop].centerX;
									DetectedObjs.xBinBoundary[j].index++;
									DetectedObjs.xBinBoundary[j].index %= CAMERAAVE;
								}
								//---print out for debug-----
								/*
								if ((counter%16) == 0){
									Brain.Screen.printAt(10,DetectedObjs.yBinBoundary[j].higherPixelBoundary,"y: %d",DetectedObjs.yBinBoundary[j].ave);
								}
								//---print out for debug----
								*/
								j++;
							}// end while
#endif     
#if (YHIST_SELECT == 1) 
							if (((counter%SCREEN_UDATE_COUNT) == 0) && (xbin_pointer != NULL) && (ybin_pointer != NULL)) {
								//int dummy1 = DetectedObjs.xBinBoundary[DetectedObjs.objIndex].lowerPixelBoundary;//(*bin_pointer).lowerPixelBoundary;
								int dummy1 = (*ybin_pointer).ave;
								int dummy2 = (*xbin_pointer).ave; //DetectedObjs.xBinBoundary[DetectedObjs.objIndex].ave;
#if (VISIONDEBUG == 1)  

								Brain.Screen.printAt(40, 100, "obj %d xave %3d, yave: %3d", DetectedObjs.objIndex, dummy2, dummy1);
								// additional trouble-shooting code for distance estimation
								if (DetectedObjs.numYHistObjs == 2)
								{
									// assume index is 1
									dummy1 = DetectedObjs.yBinBoundary[1].ave;
									dummy2 = DetectedObjs.xBinBoundary[1].ave;
									Brain.Screen.printAt(40, 140, "obj %d xave %3d, yave: %3d", 1, dummy2, dummy1);
								}
#endif                            
							}
#endif                  
						} // end check for tracking mode
					}
					//ave /= loop;
					ave /= count;
					// plot the largest obj, to confirm visually it is always index 0
					// this has been confirm, thus commenting out the code below
					/*
					Brain.Screen.setCursor(3,6);
					Brain.Screen.print("ax: %d",ave);
					xScreen = Vision.largestObject.centerX*480;
					xScreen /= XCAMMERAMAX;

					yScreen = Vision.largestObject.centerY*240;
					yScreen /= YCAMMERAMAX;
					//Brain.Screen.setCursor(2,6);
					//Brain.Screen.print("oX: %d, oY: %d",xScreen,yScreen);
					Brain.Screen.drawCircle(xScreen,yScreen,5,vex::color::red);
					*/

					// update the detected object in x
					oldestval = DetectedObjs.xary[DetectedObjs.index];
					DetectedObjs.xTotal = DetectedObjs.xTotal - oldestval + ave; // update total, ave is the latest val
					DetectedObjs.xave = DetectedObjs.xTotal >> CAMERASHIFT;
					DetectedObjs.xary[DetectedObjs.index] = ave;
					DetectedObjs.index++;
					DetectedObjs.index %= CAMERAAVE;

#if (VISIONDEBUG == 1)               
					if ((counter%SCREEN_UDATE_COUNT) == 0) {
						//Brain.Screen.drawLine(VisionCtrl.XlimLeft,0,VisionCtrl.XlimLeft, 240); //left line
						//Brain.Screen.drawLine(VisionCtrl.XlimRight,0,VisionCtrl.XlimRight,240); //right line
						Brain.Screen.drawLine(VisionCtrl.Xtarget, 0, VisionCtrl.Xtarget, 240);
						/*
						Brain.Screen.clearLine(1,vex::color::black);
						Brain.Screen.clearLine(2,vex::color::black);
						Brain.Screen.clearLine(3,vex::color::black);
						Brain.Screen.setPenColor(vex::color::white);

						Brain.Screen.setCursor(1,3);
						Brain.Screen.setFont(vex::fontType::mono15);
						Brain.Screen.print("x: %d, y: %d, count:%d",Vision.largestObject.centerX,Vision.largestObject.centerY,
										   Vision.objectCount);
						xScreen = Vision.largestObject.centerX*480;
						xScreen /= XCAMMERAMAX;

						yScreen = Vision.largestObject.centerY*240;
						yScreen /= YCAMMERAMAX;
						Brain.Screen.setCursor(2,3);
						Brain.Screen.print("ax: %d",DetectedObjs.xave);
						*/
						Brain.Screen.setCursor(11, 3);
						Brain.Screen.clearLine(11, vex::color::black);
						Brain.Screen.print("xave: %d, Err: %d", DetectedObjs.xave, DetectedObjs.xave - VisionCtrl.Xtarget);
						counter = 0;


					}
#endif                
					//auto kick off yhist if current state is Histogram
					if ((DetectedObjs.state == DetectedObjState::Histogram) && (counterTrack%SAMPLE_COUNT_TO_TRACKING == 0)) {
						yhist(); // this must move the state machine to Tracking, or else resulted in multiple calls
						counterTrack = 0;
					}
#if (VISIONDEBUG == 1)  
					counter++;
#endif
					counterTrack++;
				}
				vex::task::sleep(VisionCtrl.delayInterval);
				//Brain.Screen.clearScreen();
			}
			vex::task::sleep(VisionCtrl.delayInterval * 2); // make sure we give control to other threads
		}
	}

	void increaseSigCounter() {

		// check for shift button
		if (Controller.ButtonLeft.pressing()) {
			return;
		}

		// fist save the old target values, might have been updated

		switch (sigCounter) {
		case 0:       // blue
			blueXtarget = VisionCtrl.Xtarget;
			break;
		case 1: // green
			greenXtarget = VisionCtrl.Xtarget;
			break;
		case 2: // red
			redXtarget = VisionCtrl.Xtarget;
			break;
		}
		// increase counter
#if(ALLIANCE == 0) // red, sigCounter == 0 and 1 only
		if (sigCounter == 0) {
			sigCounter = 1;
		}
		else {
			sigCounter = 0;
		}
#elif (ALLIANCE == 1) // blue, sigCounter == 1 and 2 only
		if (sigCounter == 1) {
			sigCounter = 2;
		}
		else {
			sigCounter = 1;
		}
#else
		sigCounter++;
		sigCounter %= 3;
#endif
#if (VISIONDEBUG == 1)
		Controller.Screen.clearScreen();
		Controller.Screen.setCursor(1, 1);
		switch (sigCounter) {
		case 0:
			Vision.setBrightness(blueBrightness);
			Brain.Screen.printAt(10, 40, "blue");
			Controller.Screen.print("blue");

			Controller.Screen.setCursor(3, 1);
			Controller.Screen.clearLine(3);
			Controller.Screen.print("b brightness %d", blueBrightness);
			VisionCtrl.Xtarget = blueXtarget;
			break;
		case 1:
			Vision.setBrightness(greenBrightness); //60
			Brain.Screen.printAt(10, 40, "green");
			Controller.Screen.print("green");
			Controller.Screen.setCursor(3, 1);
			Controller.Screen.clearLine(3);
			Controller.Screen.print("g brightness %d", greenBrightness);
			VisionCtrl.Xtarget = greenXtarget;
			break;
		case 2:
			Vision.setBrightness(redBrightness); //70
			Brain.Screen.printAt(10, 40, "red");
			Controller.Screen.print("red");
			Controller.Screen.setCursor(3, 1);
			Controller.Screen.clearLine(3);
			Controller.Screen.print("r brightness %d", redBrightness);
			VisionCtrl.Xtarget = redXtarget;
			break;
		}
		Controller.Screen.setCursor(2, 1);
		Controller.Screen.print("target: %d", VisionCtrl.Xtarget);

		printTracking();
		// provide info to the controller
#endif

		vex::task::sleep(100);
	}

	void adjustXTarget(int val) {
		Controller.Screen.clearLine(2);
		Controller.Screen.setCursor(2, 1);
		VisionCtrl.Xtarget = VisionCtrl.Xtarget + val;
		if (VisionCtrl.Xtarget > 230)
			VisionCtrl.Xtarget = 230;
		if (VisionCtrl.Xtarget < 90)
			VisionCtrl.Xtarget = 90;
		Controller.Screen.print("target: %d", VisionCtrl.Xtarget);

	}
	void targetIncrease() {
		adjustXTarget(5);

	}
	void targetDecrease() {
		adjustXTarget(-5);

	}

	void adjustBrightness(int val) {
		Controller.Screen.clearLine(3);
		Controller.Screen.setCursor(3, 1);
		switch (sigCounter) {
		case 0:
			blueBrightness += val;
			Vision.setBrightness(blueBrightness);
			Brain.Screen.printAt(10, 40, "blue brightnes %d", blueBrightness);
			Controller.Screen.print("b brightness %d", blueBrightness);
			break;
		case 1:
			greenBrightness += val;
			Vision.setBrightness(greenBrightness); //60
			Brain.Screen.printAt(10, 40, "green brightness %d", greenBrightness);
			Controller.Screen.print("g brightness %d", greenBrightness);
			break;
		case 2:
			redBrightness += val;
			Vision.setBrightness(redBrightness); //70
			Brain.Screen.printAt(10, 40, "red brightness %d", redBrightness);
			Controller.Screen.print("r brightness %d", redBrightness);
			break;
		}
		vex::task::sleep(100);

	}
	void brightnessIncrease() {
		adjustBrightness(5);
	}
	void brightnessDecrease() {
		adjustBrightness(-5);
	}

	void strafe(int vel) { //+ right - left
		FL = vel;
		FR = vel;
		BL = -vel;
		BR = -vel;
	}
	void rot(int vel) {
		FL = vel;
		FR = vel;
		BL = vel;
		BR = vel;
	}
	void rotRot(int deg, int vel = 200, bool waitForCompletion = false) {
		if (waitForCompletion) {
			FL_Base.rotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			FR_Base.rotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BL_Base.rotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BR_Base.rotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
		}
		else {
			FL_Base.startRotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			FR_Base.startRotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BL_Base.startRotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BR_Base.startRotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
		}
	}
	void strafeRot(int deg, int vel = 200, bool waitForCompletion = false) {
		if (waitForCompletion) {
			FL_Base.rotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			FR_Base.rotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BL_Base.rotateFor(-deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BR_Base.rotateFor(-deg, rotationUnits::deg, vel, velocityUnits::rpm);
		}
		else {
			FL_Base.startRotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			FR_Base.startRotateFor(deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BL_Base.startRotateFor(-deg, rotationUnits::deg, vel, velocityUnits::rpm);
			BR_Base.startRotateFor(-deg, rotationUnits::deg, vel, velocityUnits::rpm);
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
	//drive functions for recentering

	void recenterBot(volatile bin_boundary_t* bin_pointer, bool strafeCorrection = true) {
		// routine to re-center the bot using the camera position
		int loop = 0;
		bool done = false;
		int xErr = 0;
		isAutonBase = true;
		runningBaseMethod = true;

		FL_Base.setStopping(vex::brakeType::brake);
		FR_Base.setStopping(vex::brakeType::brake);
		BL_Base.setStopping(vex::brakeType::brake);
		BR_Base.setStopping(vex::brakeType::brake);

		//xErr = DetectedObjs.xave - VisionCtrl.Xtarget;
		xErr = (*bin_pointer).ave - VisionCtrl.Xtarget;
		Brain.Screen.setCursor(2, 6);
		Brain.Screen.print("Err: %d", xErr);
		while ((ABS(xErr) >= 5) && loop < 8) {

			if (xErr > 35) {
				xErr = 35;
			}
			if (xErr < -35) {
				xErr = -35;
			}
			if ((xErr < 10) && (xErr > 0))
				xErr = 10;
			if ((xErr > -10) && (xErr < 0))
				xErr = -10;

			//botRotateLeft(-xErr, 25);
			if (strafeCorrection) {
				strafeRot(xErr / 2, 40);
				vex::task::sleep(100);
			}
			else {
				rotRot(xErr / 2, 40);
				vex::task::sleep(100); // wait for half a sec
			}
			xErr = DetectedObjs.xave - VisionCtrl.Xtarget;
			Brain.Screen.clearLine(2, vex::color::black);
			Brain.Screen.setCursor(2, 6);
			Brain.Screen.print("Err: %d", xErr);
			loop++;
		}
		Brain.Screen.clearLine(2, vex::color::black);
		Brain.Screen.setCursor(2, 6);
		Brain.Screen.print("Err: %d xave: %d", xErr, DetectedObjs.xave);
		// when done, set the motors to coast
		stopBase();
		isAutonBase = false;
		runningBaseMethod = false;
		/*BASELEFTB.stop(vex::brakeType::coast);
		BASERIGHTB.stop(vex::brakeType::coast);*/
	}


	void recenterBot(bool strafeCorrection=true) {
		// routine to re-center the bot using the camera position
		int loop = 0;
		bool done = false;
		int xErr = 0;
		isAutonBase = true;
		runningBaseMethod = true;

		FL_Base.setStopping(vex::brakeType::brake);
		FR_Base.setStopping(vex::brakeType::brake);
		BL_Base.setStopping(vex::brakeType::brake);
		BR_Base.setStopping(vex::brakeType::brake);

		xErr = DetectedObjs.xave - VisionCtrl.Xtarget;
		Brain.Screen.setCursor(2,6);
		Brain.Screen.print("Err: %d",xErr);
		while ((ABS(xErr) >= 5) && loop < 8) {

			if (xErr > 35) {
				xErr = 35;
			}
			if (xErr < -35) {
				xErr = -35;
			}
			if ((xErr < 10) && (xErr > 0))
				xErr = 10;
			if ((xErr > -10) && (xErr < 0))
				xErr = -10;

			//botRotateLeft(-xErr, 25);
			if (strafeCorrection) {
				strafeRot(xErr/2, 40);
				vex::task::sleep(100);
			}
			else {
				rotRot(xErr / 2, 40);
				vex::task::sleep(100); // wait for half a sec
			}
			xErr = DetectedObjs.xave - VisionCtrl.Xtarget;
			Brain.Screen.clearLine(2,vex::color::black);
			Brain.Screen.setCursor(2,6);
			Brain.Screen.print("Err: %d",xErr);
			loop++;
		}
		Brain.Screen.clearLine(2, vex::color::black);
		Brain.Screen.setCursor(2, 6);
		Brain.Screen.print("Err: %d xave: %d", xErr, DetectedObjs.xave);
		// when done, set the motors to coast
		stopBase();
		isAutonBase = false;
		runningBaseMethod = false;
		/*BASELEFTB.stop(vex::brakeType::coast);
		BASERIGHTB.stop(vex::brakeType::coast);*/
	}

	void shiftKeySwitch() {
		// map to left button of the controller
		while (Controller.ButtonLeft.pressing()) 
		{
			if (Controller.ButtonL1.pressing()) {
				targetIncrease();
				vex::task::sleep(250);
			}
			if (Controller.ButtonL2.pressing()) {
				targetDecrease();
				vex::task::sleep(250);
			}
			if (Controller.ButtonR1.pressing()) {
				runningBaseMethod = true;
				rotRot(200);

				//brightnessIncrease();
				vex::task::sleep(250);
			}
			if (Controller.ButtonR2.pressing()) {
				runningBaseMethod = true;
				rotRot(-200);
				//brightnessDecrease();
				vex::task::sleep(250);
			}
			if (Controller.ButtonA.pressing()) {
				recenterBot(xbin_pointer, ROT_RECENTER);
				vex::task::sleep(250);
			}
			if (Controller.ButtonB.pressing()) {
				recenterBot(ROT_RECENTER);
				vex::task::sleep(250);
			}
			vex::task::sleep(100);
		}
	}


	// end vision functions/methods

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
			toggleLEDs(); //on when capping, off when balling
			//LED.state(100,percentUnits::pct);
			//areLEDsOn = true;
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
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
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
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		canStopLift = false;
		Intake.spin(directionType::rev, 600, velocityUnits::rpm);
		Intake2.spin(directionType::fwd, 600, velocityUnits::rpm);
	}

	void intakeStop() {
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
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
		isPlaceReady = false;
		Intake.spin(directionType::rev, 200, velocityUnits::rpm);
		Intake2.spin(directionType::rev, 200, velocityUnits::rpm);
	}
	
	void liftSpeed(int s, bool stopLift=false) {
		canStopLift = stopLift;
		isPlaceReady = false;
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
		Intake.setVelocity(50, velocityUnits::rpm);
		Intake2.setVelocity(50, velocityUnits::rpm);
		canStopLift = false;
	}
	
	void liftTop(bool goPlacePos = true) {
		if (goPlacePos) {
			Flipper.startRotateTo(placePos, rotationUnits::deg, 100, velocityUnits::rpm);
			currPos = placePos;
		}
		liftUp();
		double initLift = Intake.rotation(rotationUnits::deg);
		sleep(200);
		lifttime = Brain.timer(timeUnits::msec);
		while (abs(Intake.rotation(rotationUnits::deg) - initLift) > 0 && Brain.timer(timeUnits::msec) - lifttime < 5000) {
			//Brain.Screen.printAt(0,140,"initFlip is %d", initFlip - (int)Flipper.rotation(rotationUnits::deg));
			initLift = Intake.rotation(rotationUnits::deg);
			sleep(20);
		}
		liftSpeed(50);
		liftHold();				//hold lift at topmost position
		isPlaceReady = true;
	}
	
	void liftBottom(bool goDownPos = false) {
		if (goDownPos) {
			Flipper.startRotateTo(downPos, rotationUnits::deg, 100, velocityUnits::rpm);
			currPos = downPos;
		}
		liftDown();
		double initLift = Intake.rotation(rotationUnits::deg);
		sleep(200);
		lifttime = Brain.timer(timeUnits::msec);
		while (abs(Intake.rotation(rotationUnits::deg) - initLift) > 0 && Brain.timer(timeUnits::msec) - lifttime < 5000) {
			//Brain.Screen.printAt(0,140,"initFlip is %d", initFlip - (int)Flipper.rotation(rotationUnits::deg));
			initLift = Intake.rotation(rotationUnits::deg);
			sleep(5);
		}
		liftSpeed(-20);
		liftHold();				//hold lift at topmost position
		//Controller.rumble("-");
		isPlaceReady = false;
	}

	void catgo() {
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		Catapult.spin(directionType::fwd,16,velocityUnits::rpm);
	}

	void catstop() {
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		Catapult.stop(brakeType::coast);
	}

	void foldUp() {
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		liftSpeed(-100);
		double initLift = Intake.rotation(rotationUnits::deg);
		sleep(200);
		while (abs(Intake.rotation(rotationUnits::deg) - initLift) > 0) {
			initLift = Intake.rotation(rotationUnits::deg);
			sleep(20);
		}
		Controller.rumble("-");
		liftHold();				//hold lift at bottommost position
		Flipper.startRotateTo(foldPos, rotationUnits::deg, 50, velocityUnits::rpm);
		currPos = foldPos;
		liftStop();
	}
	
	float delta_t;
	
	void flipReset() {
		liftSpeed(-100);
		double initLift = Intake.rotation(rotationUnits::deg);
		sleep(200);
		while (abs(Intake.rotation(rotationUnits::deg) - initLift) > 0) {
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
		
		Flipper.startRotateTo(foldPos, rotationUnits::deg, 5, velocityUnits::rpm);
		currPos = foldPos;
	}

	void flipDown() {
		Flipper.startRotateTo(downPos, rotationUnits::deg, 50, velocityUnits::rpm);
		currPos = downPos;
	}

	void flipStop() {
		Flipper.stop(brakeType::hold);
	}

	void flipOne() { //flip, then hold in placing pos
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		Flipper.startRotateFor(-90, rotationUnits::deg, 100, velocityUnits::rpm);
		sleep(500);
		Flipper.startRotateTo(downPos, rotationUnits::deg, 100, velocityUnits::rpm);
		sleep(300);
		Flipper.startRotateTo(placePos, rotationUnits::deg, 50, velocityUnits::rpm);
		currPos = placePos;
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
	double initRotTo;
	double kPr;

	void driveFor(int dist, int max=200, double kP=0.5, bool noStop=false, bool keepRot=false) {
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
			//Brain.Screen.printAt(0,180,"Negative");
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
		derr = goal*1.1 - GyroYaw.value(rotationUnits::deg) + initRotTo;
		
		while(abs(derr) > 0.05) {
			//Brain.Screen.printAt(0,180,"%d %d", power, FL);
			power = derr * kP;
			power = power>200 ? 200 : power;
			rot(power);
			derr = goal*1.055 - GyroYaw.value(rotationUnits::deg) + initRotTo; //constant: 1.07
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
		if(amIBlue) Vision.takeSnapshot(SIG_1); //blue
		else Vision.takeSnapshot(SIG_3);//red
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
			Brain.Screen.printAt(0,100, "Gyro: %.2f", GyroYaw.value(rotationUnits::deg) - initRotTo);
			Brain.Screen.printAt(0,120, "GyroPitch: %.2f", GyroPitch.value(rotationUnits::deg));
			//Brain.Screen.printAt(0,180,"flipper rotation is %f", Flipper.rotation(rotationUnits::deg));
			//Brain.Screen.printAt(10,40, "%d", TopColBumper.value());
			//Brain.Screen.printAt( 10, 50, "Distance %6.1f in", Sonar.distance(distanceUnits::in));
			//if(Competition.isAutonomous() && Brain.timer(timeUnits::msec) > 15000) Controller.rumble("-");
			
			
			
			//Brain.Screen.printAt(0,100, "Average Base Drive: %d", avgBaseFwd);
			//Brain.Screen.printAt(0,120, "Average Base Rotation: %d", avgBaseRot);
			sleep(50); //used to be 20
		}
		return 1;
	}

	void trebuchet() {
		canPew = false;

		if (isCollectorPriming || isBallMode == -1) return;
		//if (Sonar.distance(distanceUnits::in) < 10) { //something blocking the way
		//	return;
		//}
		//auto reload
		isCatapultPriming = true;
		catprimetime = Brain.timer(timeUnits::msec);
		Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
		//Brain.Screen.printAt(10,40, "Catapult Running");
		sleep(400);
		if(CatBumper.value() == 0) {
		//Brain.Screen.printAt(10,40, "Button was pressed");
		//if button is pressed immedately, catapult is already lowered
			//firing at a lower speed
			Catapult.spin(directionType::fwd, 10, velocityUnits::rpm);
			while(CatBumper.value() == 0 && Brain.timer(timeUnits::msec)-catprimetime < 500){
				task::sleep(2);
			}
			Catapult.rotateTo(Catapult.rotation(rotationUnits::deg)-20, rotationUnits::deg, 200, velocityUnits::rpm);
			sleep(300);
			Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
			
		} //if not, catapult will simply prime 
		else {
			Brain.Screen.printAt(10,40, "Button not pressed");
		}
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
			  torqueLimit = 1.0;
		  } else {
			  Catapult.stop(brakeType::coast);
			  isCollectorReady = false;
			  torqueLimit = 1.5;
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
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
		canPew = true;
	}

	void smak() {
		if (Controller.ButtonLeft.pressing())
		{
			return;
		}
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
		
		//liftSpeed(-100);
		liftDown();
		drive(90);
		sleep(1000);
		stopBase();
		//liftStop();
		foldUp();
		//Controller.rumble("-");
		currPos = foldPos;
		canStopLift = true;
		isAutonBase = false;
	}
	
	void cap() { //called to toggle between ready states
		if (isPlaceReady) {
			placeCap();
			isPlaceReady = false;
		} else {
			liftTop();
			isPlaceReady = true;
		}
	}

	///////////////////////////////////////// TASKS /////////////////////////////////////////////////////////////////

	int launchCallback() {
		for(;;) {
			// if(isBallMode) Controller.ButtonR1.pressed(trebuchet);
			// Controller.ButtonL1.pressed(smack);
			if(isBallMode && canPew) trebuchet();
			if(canSmak) smack();
			task::sleep(40);
		}
		return 1;
	}

	int FRCallback() {
		for(;;) {
			//FR_Base.spin(directionType::rev,isBallMode*1.6*(FB - isBallMode*T - LR),velocityUnits::rpm);
			if (!runningBaseMethod) FR_Base.spin(directionType::fwd, FR, velocityUnits::rpm);
			if (!runningBaseMethod && FR == 0) FR_Base.stop();
			task::sleep(4);
		}
		return 1;
	}
	int FLCallback() {
		for(;;) {
			//FL_Base.spin(directionType::fwd,isBallMode*1.6*(FB + isBallMode*T + LR),velocityUnits::rpm);
			if (!runningBaseMethod) FL_Base.spin(directionType::fwd, FL, velocityUnits::rpm);
			if (!runningBaseMethod && FL == 0) FL_Base.stop();
			task::sleep(4);
		}
		return 1;
	}
	int BRCallback() {
		for(;;) {
			//BR_Base.spin(directionType::rev,isBallMode*1.6*(FB - isBallMode*T + LR),velocityUnits::rpm);
			if (!runningBaseMethod) BR_Base.spin(directionType::fwd, BR, velocityUnits::rpm);
			if (!runningBaseMethod && BR == 0) BR_Base.stop();
			task::sleep(4);
		}
		return 1;
	}
	int BLCallback() {
		for(;;) {
			//BL_Base.spin(directionType::fwd,isBallMode*1.6*(FB + isBallMode*T - LR),velocityUnits::rpm);
			if (!runningBaseMethod) BL_Base.spin(directionType::fwd, BL, velocityUnits::rpm);
			if (!runningBaseMethod && BL == 0) BL_Base.stop();
			task::sleep(4);
		}
		return 1;
	}

	int mainCallback() {
		for(;;){
			
		  
		  //Controller.ButtonRight.pressed(toggleLEDs);
			
		  Controller.ButtonRight.pressed(cap);
		  //Controller.ButtonRight.released(delayLEDsOff);
			
		  //COMMENT OUT DURING COMPETITION
		  //Controller.ButtonLeft.pressed(resetCat);
		  //Controller.ButtonLeft.pressed(toggleMode);
		  //Controller.ButtonRight.pressed(placeCap);
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

		  sleep(100);
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

	//AUTON TASKS
	int smakDownThread() {
		Catapult.spin(directionType::rev, 100, velocityUnits::rpm); //lower smack
		
		colprimetime = Brain.timer(timeUnits::msec);
		while(TopColBumper.value() == 0 && Brain.timer(timeUnits::msec)-colprimetime < 2000){
			this_thread::sleep_for(10);
		}
		Catapult.stop();
		Catapult.rotateFor(-390, rotationUnits::deg, 200, velocityUnits::rpm); //lower smack 90deg
		
		return 1;
	}
	
	int driveThread() {
		driveFor(driveThread_distance, driveThread_max, driveThread_kP);
		return 1;
	}
		
	void fetch() {
		//nabs bol
		intake();
		driveFor(33);
		driveFor(-39);
	}

	void fetchFlip(bool plsPrimeCatapult=true) {		
		smackFromButton(130);		//MAKE SIMULTANEOUS
		intake();
		
		driveFor(-12, 200, 0.8);	// CAN PROBABLY LOWER //drive back before lowering
		//intakeStop();				//balls often not fully intaken
		isCollectorReady=false;
		torqueLimit = 1.5;			//global torque var increase to allow smak to run
		smak(); 					//lower collector
		sleep(300);
		driveFor(12);
		if(plsPrimeCatapult)pew(); 	//flip cap and lower cat
		else smak(); 				//just flip cap
		torqueLimit = 1.0;
		sleep(300);
		//intake(); 					//continue intaking balls
	}	
 
	void fetchFlip2(int dist, bool plsPrimeCatapult=true, int delayDrive=300) {
		intakeSpeed(300);
		thread smackDown(smakDownThread);	//start thread before driving
		sleep(delayDrive);
		driveFor(dist, 60, 0.6);
		sleep(100);		
		driveFor(-10, 200, 0.8);	// CAN PROBABLY LOWER //drive back before lowering
		intake();
		isCollectorReady=false;
		torqueLimit = 1.5;			//global torque var increase to allow smak to run
		smak(); 					//lower collector
		driveFor(8, 200, 0.8);
		if(plsPrimeCatapult)pew(); 	//flip cap and lower cat
		else smak(); 				//just flip cap
		torqueLimit = 1.0;
		sleep(300); 
	}
	
	void backCapRight() {
		intake();
		driveFor(33);				//intake ball
		driveFor(-12, 200, 1);
		sleep(200);
		flipDown();
		rotForGyro(135);		//face cap
		intakeStop();
		liftBottom();
		driveFor(-17, 200, 1);		//under cap
		//Flipper.startRotateTo(placePos, rotationUnits::deg);
		//sleep(200);
		
		liftTop();
		sleep(300);
		flipOne();
		//Flipper.startRotateTo(110, rotationUnits::deg);		//hold up
		driveFor(5);
		rotForGyro(-73,3);			//turn to pole
		sleep(200);
		Flipper.startRotateTo(downPos, rotationUnits::deg);
		sleep(300);
		Flipper.startRotateTo(placePos, rotationUnits::deg);
		
		driveFor(-16); 				//drive to pole
		drive(-50);
		sleep(200);
		stopBase();
		placeCap();					//place cap
		rotForGyro(30);
		strafe(-5);
		driveFor(5,100,0.8);
		//drive(100);
		//sleep(400);
		
		// intake();				//park
		// park();
	}

	void backCapLeft() {
		intake();
		driveFor(33);				//intake ball
		driveFor(-14, 200, 0.7);
		//sleep(200);
		flipDown();
		//rotForGyro(-133);		//face cap
		//intakeStop();
		liftBottom();
		//driveFor(-17, 200, 0.7);		//under cap
		//sleep(2000);
		liftTop();
		//sleep(5000);
		//sleep(300);
		flipOne();
		sleep(300);
		//Flipper.startRotateTo(110, rotationUnits::deg);		//hold up
		//driveFor(5);
		//rotForGyro(70, 3);			//turn to pole
		//sleep(200);
		//Flipper.startRotateTo(downPos, rotationUnits::deg);
		//sleep(300);
		//Flipper.startRotateTo(placePos, rotationUnits::deg);

		//driveFor(-16,200,0.6); 				//drive to pole
		//drive(-50);
		//sleep(200);
		//stopBase();
		//placeCap();					//place cap
		////rotForGyro(-18);
		//rotTo(-93);
		//strafe(5);
		//driveFor(5, 100, 0.8);
		//drive(100);
		//sleep(400);

		// intake();				//park
		// park();
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
		else rotForGyro(80);//rotForGyro(84);
		
		pew();
		
		intakeStop();
		sleep(200);
		if(side == LEFT) {
			rotForGyro(side * 9);
			driveFor(40, 200, 0.8);			// toggle bottom flag
			driveFor(-26);
		}
		else {
			rotForGyro(side * 7);
			driveFor(49, 200, 0.8);
			driveFor(-21);
		}
		rotFor(side * -43);
		if(side == LEFT)strafeFor(side*-15); 	// strafe to cap
		else strafeFor(-20);
		intakeSpeed(100);
		driveFor(3,50,0.7); 	//slow drive nom cap
		fetchFlip(false);			// get balls, flip cap*/
		driveFor(-6);
		sleep(1500);
		intakeStop();
	}
	
	void wannabeReliableRight() {
		intake();
		driveFor(37, 200, .7);
		driveFor(-41, 200, .5);
		sleep(200);
		//rotForGyro(93, 4);	
		rotForGyro(84);
		/*printTracking();
		sleep(500);
		recenterBot(ROT_RECENTER);	// rotate to recenter*/
		pew();					//toggle near column

		intakeStop();
		sleep(400);
		rotForGyro(4);
		driveFor(49, 200, .8);	//toggle bottom flag
		driveFor(-20, 200, 1);
		//rotFor(-43);
		rotTo(45);
		strafeFor(-17);		//strafe to cap
		//driveFor(3,50,0.7); 		//slow drive nom cap
		fetchFlip2(5,false);		// get balls, flip cap
		strafeFor(-6);		//go more center to hit toggled center flags
		driveFor(6);				//position to shoot
		rotTo(45);

		VisionCtrl.Xtarget = 160;
		//recenterBot(); //strafe to recenter
		//while (Brain.timer(timeUnits::msec) < 14200) {
		//	task::sleep(5);
		//}
		//intakeStop();
		//pew(); //launch only after 14.5 second mark*/
	}
	
	void wannabeReliableLeft() {
		intake();
		driveFor(36, 200, 1);
		driveFor(-40, 200, .7);
		//sleep(200);
		rotForGyro(-84);			//rotForGyro(84);
		printTracking();
		sleep(300);
		recenterBot(ROT_RECENTER);
		//sleep(200);
		pew();					//toggle near column
		
		intakeStop();
		sleep(400);
		//rotForGyro(-4);
		rotTo(-93);
		driveFor(46, 200, .8);		//toggle bottom flag
		driveFor(-24, 200, 1);
		//rotFor(-43);
		rotTo(-60);
		strafeFor(12);				//strafe to cap
		fetchFlip2(5,false);		// get balls, flip cap
		//rotTo(-60);
		
		strafeFor(5);				//go more center to hit toggled center flags
		//Brain.Screen.printAt(10, 50, "Time %6.1f ", Brain.timer(timeUnits::msec));
		////driveFor(-8);				//position to shoot

		/*printTracking();
		sleep(200);
		recenterBot(ROT_RECENTER);*/
		//Brain.Screen.printAt(10, 40, "Distance %6.1f in", Sonar.distance(distanceUnits::in));
		while (Brain.timer(timeUnits::msec) < 14200) {
			task::sleep(5);
		}
		intakeStop();
		//Brain.Screen.printAt(10, 50, "Time %6.1f ", Brain.timer(timeUnits::msec));
		pew(); //launch only after 14.5 second mark
	}
	
	void progskills() {
	////START BACK LEFT
		
		//smak();
		//sleep(500);
		//intake();
		//driveFor(25,200,.8);		//drive to flip cap
		//smak();
		//driveFor(10);		//intake ball
		//smak();				//prime smak
		//sleep(200);
		//driveFor(-14);		//back up to turn
		//intakeStop();
		//rotTo(42);			//face cap
		//driveFor(16);		//drive to cap
		//smak();
		//sleep(400);
		//driveFor(-50);
		//rotTo(-92);			//face flag
		//driveFor(24);

		//printTracking();		//vision align
		//sleep(300);
		//recenterBot(ROT_RECENTER);
		//pew();					//fire!
		//sleep(400); 

	////CHECKPOINT 1
		strafe(-50);			//align to wall
		sleep(1000);
		stopBase();
		sleep(300);
		initRotTo = GyroYaw.value(rotationUnits::deg);	//reset gyro reference
		sleep(200);

		strafeFor(6);			//move away from wall
		driveFor(50);			//toggle bottom flag
		driveFor(-28, 150);
		rotTo(45);
		strafeFor(12);				//strafe to cap, nab balls
		fetchFlip2(5,false,100);
		strafeFor(20);
		rotForGyro(-15);

		VisionCtrl.Xtarget = 165;
		printTracking();		//vision align
		sleep(500);
		recenterBot(ROT_RECENTER);
		intakeStop();
		//sleep(300);

		Catapult.spin(directionType::fwd, 200, velocityUnits::rpm);
		sleep(400);
		Catapult.stop();
		
		//pew();					//fire!
		//sleep(400);

	////CHECKPOINT 2
		strafeFor(-2);
		rotTo(170);				//go for ball on red tile
		//sleep(1000);
		smak();					
		sleep(100);
		driveFor(3);
		intake();
		driveFor(-2);
		rotTo(85);				//go for front cap
		smak();
		driveFor(8);
		pew();					//flip front cap and draw back catapult
		driveFor(11);			//nab ball
		intakeStop();
		rotTo(-4);			//face bottom middle flag
		driveFor(25);
		printTracking();
		VisionCtrl.Xtarget = 195;
		sleep(300); // will be in the vision tracking state
		if (xbin_pointer != NULL)
		{
			recenterBot(xbin_pointer, ROT_RECENTER);
		}
		else 
		{	//if vision err, average assuming flag is toggled
			VisionCtrl.Xtarget = 243;
			recenterBot(ROT_RECENTER);
		}
		driveFor(20);		//toggle middle bottom flag

	////CHECKPOINT 3
		driveFor(-25);
		rotTo(83);			//face far cap
		isCollectorReady = false;
		isCatapultPriming = false;		//make su
		smak();
		driveFor(36);
		smak();				//flip far cap
		sleep(300);
		strafeFor(16);
		rotTo(5);

		VisionCtrl.Xtarget = 165;
		printTracking();		//vision align
		sleep(500);
		recenterBot(ROT_RECENTER);
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
		
		//flipReset();
		//Flipper.setVelocity(5, velocityUnits::rpm);
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
		//Brain.Screen.print("Robot is in Autonomous mode");
		Brain.resetTimer();

		#if (AUTON_SELECT == 0) // select oleReliable
			#if (ALLIANCE == 0) // red
					oleReliable(LEFT);
					return;
			#elif (ALLIANCE ==1) //blue
					oleReliable(RIGHT);
					return;
			#endif
		#elif (AUTON_SELECT == 1) // select pReliable
			#if (ALLIANCE == 0) // red
					wannabeReliableLeft();
					return;
			#elif (ALLIANCE ==1) //blue
					wannabeReliableRight();
					return;
			#endif
		#elif (AUTON_SELECT == 2) // select backCap
			#if (ALLIANCE == 0) // red
					backCapLeft();
					return;
			#elif (ALLIANCE ==1) //blue
					backCapRight();
					return;
			#endif
		#elif (AUTON_SELECT == 3) // select backPark
			#if (ALLIANCE == 0) // red
					backPark(LEFT);
					return;
			#elif (ALLIANCE ==1) //blue
					backPark(RIGHT);
					return;
			#endif
		#elif (AUTON_SELECT == 4) // select progSkills
			progskills();
		#elif (AUTON_SELECT == 5)	//select park
			park();
		#endif
        //sleep(1500);
		
		//fetchFlip2(5, false);
		//sleep(10000);
		//oleReliable(RIGHT);
        //wannabeReliable();
		
		//backCap(RIGHT);
		//backPark(RIGHT);
		//progskills();
        //wannabeReliableLeft();
		//park();
		//testing(&Catapult, 200);
		//sleep(100);
		//Catapult.stop();
		
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
		//Brain.Screen.print("Robot is in Driver mode");
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
			  Intake.spin(directionType::fwd, 150, velocityUnits::rpm);
			  Intake2.spin(directionType::fwd, 150, velocityUnits::rpm);
			  if(Flipper.rotation(rotationUnits::deg) != placePos) Flipper.startRotateTo(placePos, rotationUnits::deg, 30, velocityUnits::rpm); //fold flipper to hold
			  //Flipper.startRotateTo(placePos, rotationUnits::deg, 30, velocityUnits::rpm); //fold flipper to hold
			  canStopLift = true;
		  } else if (Lift < -80) {
			  Intake.spin(directionType::fwd, -150, velocityUnits::rpm);
			  Intake2.spin(directionType::fwd, -150, velocityUnits::rpm);
			  if(Flipper.rotation(rotationUnits::deg) != downPos) Flipper.startRotateTo(downPos, rotationUnits::deg, 30, velocityUnits::rpm); //fold flipper to hold
			  //Flipper.startRotateTo(downPos, rotationUnits::deg, 30, velocityUnits::rpm); //fold flipper to down
			  canStopLift = true;
			  isPlaceReady = false;
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

		task::sleep(20); //Sleep the task for a short amount of time to prevent wasted resources. 
	  }
	}
	//

	// Main will set up the competition functions and callbacks.
	//
	int main() {
		
		//Run the pre-autonomous function. 
		pre_auton();
	
		// vision related

		//Controller.Screen.clearScreen();
#if (ALLIANCE ==0)
		Vision.setLedColor(255, 0, 0);  //red
#elif (ALLIANCE ==1)
		Vision.setLedColor(0, 0, 255); // blue
#else
		Vision.setLedColor(0, 255, 0); //green debug
#endif

		VisionCtrl.delayInterval = 20;

#if (ALLIANCE == 1) // Blue Alliance, shoot Red
		VisionCtrl.Xtarget = XTARGET_AUTON_ALLIANCE_BLUE;
#endif
#if (ALLIANCE == 0) // Red Alliance, shoot Blue
		VisionCtrl.Xtarget = XTARGET_AUTON_ALLIANCE_RED;
#endif
#if (ALLIANCE == 2) // Debug
		VisionCtrl.Xtarget = XTARGET_AUTON_ALLIANCE_RED;
#endif
#if (ALLIANCE == 3) // Skills
		VisionCtrl.Xtarget = XTARGET_SKILLS;
#endif
		VisionCtrl.XlimLeft = VisionCtrl.Xtarget - 10; //screen plotting
		VisionCtrl.XlimRight = VisionCtrl.Xtarget + 10; // screen plotting

		int ave = 0;
		int loop = 0;

		DetectedObjs.xave = 0;
		DetectedObjs.yave = 0;
		DetectedObjs.numYHistObjs = 0;
		DetectedObjs.numYHistTracking = 0;
		DetectedObjs.index = 0;
		DetectedObjs.numberOfAve = CAMERAAVE;
		for (loop = 0; loop < CAMERAAVE; loop++) {
			DetectedObjs.xary[loop] = 0;
			DetectedObjs.yary[loop] = 0;
		}
		DetectedObjs.xTotal = 0;
		DetectedObjs.yTotal = 0;
		DetectedObjs.state = DetectedObjState::Histogram;
		DetectedObjs.objIndex = -1;

		// init the histogram
		for (loop = 0; loop < YBINSNUM; loop++) {
			DetectedObjs.yhist[loop] = 0;
		}

		for (loop = 0; loop < MAXOBJBIN; loop++) {

			//clear ybin
			DetectedObjs.yBinBoundary[loop].lowerBinIndex = 0; //update during Histogram State
			DetectedObjs.yBinBoundary[loop].higherBinIndex = 0; // update during Histogram State
			// used during Tracking state
			DetectedObjs.yBinBoundary[loop].lowerPixelBoundary = 0; // update during Tracking State
			DetectedObjs.yBinBoundary[loop].higherPixelBoundary = 0; // update during Tracking State
			DetectedObjs.yBinBoundary[loop].numberOfAve = CAMERAAVE; // size of running average window
			DetectedObjs.yBinBoundary[loop].total = 0; // y pixel total for efficient running ave
			DetectedObjs.yBinBoundary[loop].ave = 0;
			DetectedObjs.yBinBoundary[loop].index = 0;

			// clear xbin
			DetectedObjs.xBinBoundary[loop].lowerBinIndex = 0; //update during Histogram State
			DetectedObjs.xBinBoundary[loop].higherBinIndex = 0; // update during Histogram State
			// used during Tracking state
			DetectedObjs.xBinBoundary[loop].lowerPixelBoundary = 0; // update during Tracking State
			DetectedObjs.xBinBoundary[loop].higherPixelBoundary = 0; // update during Tracking State
			DetectedObjs.xBinBoundary[loop].numberOfAve = CAMERAAVE; // size of running average window
			DetectedObjs.xBinBoundary[loop].total = 0; // y pixel total for efficient running ave
			DetectedObjs.xBinBoundary[loop].ave = 0;
			DetectedObjs.xBinBoundary[loop].index = 0;

			for (int j = 0; j < CAMERAAVE; j++) {

				DetectedObjs.yBinBoundary[loop].ary[j] = 0;
				DetectedObjs.xBinBoundary[loop].ary[j] = 0;
			}
		}
		
		vex::thread visiontask = vex::thread(updateVision);
		
		vex::task::sleep(200);


		Controller.ButtonX.pressed(printTracking);
		VisionCtrl.run = true;
		Controller.ButtonLeft.pressed(shiftKeySwitch);
		//vision related

		//Set up callbacks for autonomous and driver control periods.
		Competition.autonomous( autonomous );
		Competition.drivercontrol( usercontrol );   

		printConfig();
	}	