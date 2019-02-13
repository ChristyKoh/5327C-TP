#include "robot-config.h"
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*        Description: Competition template for VCS VEX V5                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//Creates a competition object that allows access to Competition methods.
vex::competition    Competition;

//Disable if you want the warning removed for working
 bool batterywarning = false;

/*/////////////////////////////////*/
/*       Rainbow Color Wheel       */
/*        By Connor, 1814D         */
/*/////////////////////////////////*/
  //Color Value
  int color1 = 0;
int rainbowscroll(){
    while(1==1){
        color1++;
        if(color1 >= 7) color1 = 0;
                vex::task::sleep(400);
    }
}


/*/////////////////////////*/
/*       Menu Screen       */
/*    By Connor, 1814D     */
/*/////////////////////////*/
int DeltaMenu(){
    
   //Menu Information
    int menuxup = 500;
    int menuyup = /*-250*/10;
    
    int defaultposy = 0;
    int defaultposx = 235;

    int menuposy = menuyup;
    int menuposx = menuxup;
    int width = 240;
    int height = 225;
    int middlex;
    int middley;
    
    int menu = 'H'; //H stands for home. The menu would go to this when menu starts up
    int MotorTemp;
    while(1==1){
        Brain.Screen.setPenColor(vex::color::black); 
        Brain.Screen.drawRectangle(menuposx+defaultposx,menuposy+defaultposy ,width,height,vex::color::black);      
        //menuposy = menuposy*0.95;
        menuposx = menuposx*0.93;

        Brain.Screen.setPenWidth(2);   
        /*if(color1 == 3)Brain.Screen.setPenColor(vex::color::red);   
        else if(color1 == 2) Brain.Screen.setPenColor(vex::color::green); 
        else if(color1 == 4) Brain.Screen.setPenColor(vex::color::blue);     
        else if(color1 == 6) Brain.Screen.setPenColor(vex::color::yellow);  
        else if(color1 == 5) Brain.Screen.setPenColor(vex::color::purple); 
        else if(color1 == 0) Brain.Screen.setPenColor(vex::color::orange);  
        else if(color1 == 1) Brain.Screen.setPenColor(vex::color::cyan); */
        Brain.Screen.setPenColor(vex::color::red);  
        Brain.Screen.drawRectangle(menuposx+defaultposx,menuposy+defaultposy ,width,height,vex::color::black);      
        
        middlex = (width/2) + (menuposx+defaultposx);
        middley = (height/2) + menuposy;  

        
            
            int buttonx;
            int buttony;
            int buttonheight;
            int buttonwidth;
            int buttonmiddlex;
            int buttonmiddley;
        
        //H is Home
        //S is Sensor
        //A is Auton
        
        //Home Menu
        if(menu == 'H'){
            
            //Battery Indication
             Brain.Screen.setPenWidth(20);   
             Brain.Screen.setPenColor(vex::color::red);            
             Brain.Screen.drawLine(middlex-110,middley+95,middlex+110,middley+95);  
            /*if(Brain.Battery.capacity(vex::percentUnits::pct)>75){
             Brain.Screen.setPenColor(vex::color::green);                 
            }
           else if(Brain.Battery.capacity(vex::percentUnits::pct)>50){
             //Brain.Screen.setPenColor(vex::color::yellow);                 
            }
            else{
             Brain.Screen.setPenColor(vex::color::orange);                 
            }*/
             Brain.Screen.setPenColor(vex::color::green);    
             Brain.Screen.drawLine(middlex-110,middley+95,middlex+(2.2*(Brain.Battery.capacity(vex::percentUnits::pct))-110) ,middley+95);  
            
          Brain.Screen.setPenWidth(2);             

            /*////////////////////////////////////////////////////////////////////
            //Back Button//
            ///////////////
            
            //Characteristics of Button
            buttonx = middlex+70;
            buttony = middley - 90;
            buttonheight = 30;
            buttonwidth = 40;
            
            //Don't Change
            buttonmiddlex = buttonx + (buttonwidth/2);
            buttonmiddley = buttony + (buttonheight/2);
            
            Brain.Screen.setPenColor(vex::color::red);
            if(Brain.Screen.pressing()) { //If the Brain is pressed...
                if(abs(Brain.Screen.xPosition()-buttonmiddlex) < 40 && abs(Brain.Screen.yPosition()-buttonmiddley) < 30){
                    
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::white);  
                    Brain.Screen.setPenColor(vex::color::black); 
                }
                 else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black); 
                     Brain.Screen.setPenColor(vex::color::white); 
                 } 
            }
            else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);   
                Brain.Screen.setPenColor(vex::color::white); 
            }    
            Brain.Screen.printAt(middlex+85,middley-70,false, ">");*/
            //////////////////////////////////////////////////////////////////////
            
            //Menu Heading
            Brain.Screen.setPenColor(vex::color::white);  
            //Brain.Screen.drawLine(middlex-110,middley-60,middlex+60,middley-60);
            Brain.Screen.drawLine(middlex-110,middley-60,middlex+110,middley-60);
            Brain.Screen.printAt(middlex-110,middley-70,false,"Home");
            
            ////////////////////////////////////////////////////////////////////
            //Sensor Value Button//
            ///////////////////////
            
            //Characteristics of Button
            buttonx = middlex-110;
            buttony = middley - 45;
            buttonwidth = 100;
            buttonheight = 50;
            
            //Don't Change
            buttonmiddlex = buttonx + (buttonwidth/2);
            buttonmiddley = buttony + (buttonheight/2);
            
            Brain.Screen.setPenColor(vex::color::red);
            if(Brain.Screen.pressing()) { //If the Brain is pressed...
                if(abs(Brain.Screen.xPosition()-buttonmiddlex) < 50 && abs(Brain.Screen.yPosition()-buttonmiddley) < 25){
                    menu = 'S';
                    
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::white);
                                Brain.Screen.setPenColor(vex::color::black);  
                    vex::task::sleep(200);
                }
                 else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);
                                 Brain.Screen.setPenColor(vex::color::white);  
                 } 
            }
            else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);
                            Brain.Screen.setPenColor(vex::color::white);  
            }    
            
            /*Brain.Screen.setPenColor(vex::color::red);
            Brain.Screen.drawRectangle(middlex-110,middley-45 ,100,50,vex::color::black); */ 
            Brain.Screen.printAt(middlex-90,middley-25,false,"Sensor");    
            Brain.Screen.printAt(middlex-90,middley-5,false,"Values"); 
            //////////////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////////////
            //Auton Select Button//
            ///////////////////////
            
            //Characteristics of Button
            buttonx = middlex+10;
            buttony = middley - 45;
            buttonwidth = 100;
            buttonheight = 50;
            
            //Don't Change
            buttonmiddlex = buttonx + (buttonwidth/2);
            buttonmiddley = buttony + (buttonheight/2);
            
            Brain.Screen.setPenColor(vex::color::red);
            if(Brain.Screen.pressing()) { //If the Brain is pressed...
                if(abs(Brain.Screen.xPosition()-buttonmiddlex) < 50 && abs(Brain.Screen.yPosition()-buttonmiddley) < 25){
                    menu = 'A';
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::white);
                                Brain.Screen.setPenColor(vex::color::black);             
                                        vex::task::sleep(200);
                }
                 else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);
                                 Brain.Screen.setPenColor(vex::color::white);  
                 } 
            }
            else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);
                            Brain.Screen.setPenColor(vex::color::white);  
            }    
            
            /*            Brain.Screen.setPenColor(vex::color::red);
            Brain.Screen.drawRectangle(middlex+10,middley-45 ,100,50,vex::color::black);  
            Brain.Screen.setPenColor(vex::color::white);   */
            Brain.Screen.printAt(middlex+35,middley-25,false,"Auton");    
            Brain.Screen.printAt(middlex+30,middley-5,false,"Select");
            //////////////////////////////////////////////////////////////////////           

            
            //SD Card 
            if(Brain.SDcard.isInserted()){
                Brain.Screen.setPenColor(vex::color::red);
                Brain.Screen.drawRectangle(middlex-110,middley+20 ,100,50,vex::color::green);  
                Brain.Screen.setPenColor(vex::color::black);  
                Brain.Screen.printAt(middlex-80,middley+40,false,"[SD]");  
                Brain.Screen.printAt(middlex-100,middley+60,false,"Inserted");             
            }
            else{
                Brain.Screen.setPenColor(vex::color::orange);
                Brain.Screen.drawRectangle(middlex-110,middley+20 ,100,50,vex::color::red);  
                Brain.Screen.setPenColor(vex::color::white);  
                Brain.Screen.printAt(middlex-80,middley+40,false,"[SD]");   
                Brain.Screen.printAt(middlex-95,middley+60,false,"Removed"); 
            }

            //Motor Temperature
            MotorTemp = /*Lift.temperature(vex::percentUnits::pct)*/+0;
            Brain.Screen.setPenColor(vex::color::red);
            if(MotorTemp<40){
             Brain.Screen.drawRectangle(middlex+10,middley+20 ,100,50,vex::color::green);   
                            Brain.Screen.setPenColor(vex::color::black); 
            }
            else if(MotorTemp<60){
              Brain.Screen.drawRectangle(middlex+10,middley+20 ,100,50,vex::color::orange); 
                            Brain.Screen.setPenColor(vex::color::white); 
            }
            else if(MotorTemp<100){
                Brain.Screen.setPenColor(vex::color::orange);
                Brain.Screen.drawRectangle(middlex+10,middley+20 ,100,50,vex::color::red); 
                            Brain.Screen.setPenColor(vex::color::white); 
            }
            Brain.Screen.printAt(middlex+15,middley+40,false,"Lift Temp");    
            Brain.Screen.printAt(middlex+50,middley+60,false,"%d",MotorTemp); 
            
        }//End of 'H' or Home

        //Sensor Value Menu
        if(menu == 'S'){
            
          Brain.Screen.setPenWidth(2);             

            ////////////////////////////////////////////////////////////////////
            //Back Button//
            ///////////////
            
            //Characteristics of Button
            buttonx = middlex + 70;
            buttony = middley - 90;
            buttonheight = 30;
            buttonwidth = 40;
            
            //Don't Change
            buttonmiddlex = buttonx + (buttonwidth/2);
            buttonmiddley = buttony + (buttonheight/2);
            
            Brain.Screen.setPenColor(vex::color::red);
            if(Brain.Screen.pressing()) { //If the Brain is pressed...
                if(abs(Brain.Screen.xPosition()-buttonmiddlex) < 40 && abs(Brain.Screen.yPosition()-buttonmiddley) < 30){
                    menu = 'H';
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::white);  
                    Brain.Screen.setPenColor(vex::color::black); 
                            vex::task::sleep(200);  
                }
                 else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black); 
                     Brain.Screen.setPenColor(vex::color::white); 
                 } 
            }
            else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);   
                Brain.Screen.setPenColor(vex::color::white); 
            }    
            Brain.Screen.printAt(middlex+85,middley-70,false, ">");
            //////////////////////////////////////////////////////////////////////
            
            //Menu Heading
            Brain.Screen.setPenWidth(2);
            Brain.Screen.setPenColor(vex::color::white);  
            Brain.Screen.drawLine(middlex-110,middley-60,middlex+60,middley-60);
            Brain.Screen.printAt(middlex-110,middley-70,false,"Sensor Values");
 
                /*
            //Lift Potentiometer
            Brain.Screen.setPenColor(vex::color::green); 
            Brain.Screen.printAt(middlex-110,middley-30,false,"LiftP:%d", LiftPot.value(vex::analogUnits::range12bit));
            Brain.Screen.setPenColor(vex::color::red); 
            Brain.Screen.drawLine(middlex-110,middley-20,middlex-10,middley-20);
             Brain.Screen.setPenColor(vex::color::blue); 
            Brain.Screen.drawLine(middlex-110,middley-20,middlex+((LiftPot.value(vex::analogUnits::range12bit))/14.8)-110,middley-20);      
            
            //Wrist Potentiometer
            Brain.Screen.setPenColor(vex::color::green); 
            Brain.Screen.printAt(middlex-110,middley-0,false,"Wrist:%d", WristPot.value(vex::analogUnits::range12bit));
            Brain.Screen.setPenColor(vex::color::red); 
            Brain.Screen.drawLine(middlex-110,middley+10,middlex-10,middley+10);
             Brain.Screen.setPenColor(vex::color::blue); 
            Brain.Screen.drawLine(middlex-110,middley+10,middlex+((WristPot.value(vex::analogUnits::range12bit)-445)/35)-110,middley+10);   
            
            //Lift Encoder Value
            Brain.Screen.setPenColor(vex::color::green); 
            int liftrot = Lift.rotation(vex::rotationUnits::deg);
            Brain.Screen.printAt(middlex-110,middley+90,false,"Lift Encoder:%d", liftrot);
            Brain.Screen.setPenColor(vex::color::red); 
            Brain.Screen.drawLine(middlex-110,middley+100,middlex+110,middley+100);
             Brain.Screen.setPenColor(vex::color::blue); 
            if(liftrot<=0){
                liftrot=0;
            }
            Brain.Screen.drawLine(middlex-110,middley+100,middlex+(-0.323*(liftrot))-110,middley+100); */ 
            
            
        }//End of 'S' or SensorValues
        
         //Auton Select Menu
        if(menu == 'A'){
            
          Brain.Screen.setPenWidth(2);             

            ////////////////////////////////////////////////////////////////////
            //Back Button//
            ///////////////
            
            //Characteristics of Button
            buttonx = middlex+70;
            buttony = middley - 90;
            buttonheight = 30;
            buttonwidth = 40;
            
            //Don't Change
            buttonmiddlex = buttonx + (buttonwidth/2);
            buttonmiddley = buttony + (buttonheight/2);
            
            Brain.Screen.setPenColor(vex::color::red);
            if(Brain.Screen.pressing()) { //If the Brain is pressed...
                if(abs(Brain.Screen.xPosition()-buttonmiddlex) < 40 && abs(Brain.Screen.yPosition()-buttonmiddley) < 30){
                    menu = 'H';
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::white);  
                    Brain.Screen.setPenColor(vex::color::black);
                            vex::task::sleep(200);  
                }
                 else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black); 
                     Brain.Screen.setPenColor(vex::color::white); 
                 } 
            }
            else{
                      Brain.Screen.drawRectangle(buttonx,buttony ,buttonwidth,buttonheight,vex::color::black);   
                Brain.Screen.setPenColor(vex::color::white); 
            }    
            Brain.Screen.printAt(middlex+85,middley-70,false, ">");
            //////////////////////////////////////////////////////////////////////
            
            //Menu Heading
            Brain.Screen.setPenColor(vex::color::white);  
            Brain.Screen.drawLine(middlex-110,middley-60,middlex+60,middley-60);
            Brain.Screen.printAt(middlex-110,middley-70,false,"Auton Select");
            
        }//End of 'A' or Auton Select
        
        
        
        //Battery Warning
        if(Brain.Battery.capacity(vex::percentUnits::pct)<30 && batterywarning == true){
            Brain.Screen.setPenColor(vex::color::red);
            Brain.Screen.drawRectangle(middlex-115,middley-105 ,230,210,vex::color::red);  
            Brain.Screen.setPenColor(vex::color::black);
            Brain.Screen.printAt(middlex-30,middley-5,false,"REPLACE");
            Brain.Screen.printAt(middlex-40,middley+30,false,"BATTERIES");
        }
        //Check to see if the brain has been pressed.
        /*
        if(Brain.Screen.pressing()) { //If the Brain is pressed...
            Brain.Screen.drawCircle(Brain.Screen.xPosition(), Brain.Screen.yPosition(), 10);
        }
        */
        
        vex::task::sleep(20);        
    }
}

//Rectangles has an x y at the right corner, then width, then height, then color
/*/////////////////////////////////*/
/*       Rainbow Delta Logo        */
/*        By Connor, 1814D         */
/*/////////////////////////////////*/

int rainbowlogo(){
     //Rainbow Delta Logo
    int middle = 0;
    int logoxposition = middle - 125;

    int a;
    int b;
    int c;
    int d;
    int y1 = -300;
    
    //Go down from top
    while(/*y1 < -1*/1==1){
                    Brain.Screen.render(true,true);
                        Brain.Screen.setPenColor(vex::color::black); 
        Brain.Screen.drawRectangle(130+logoxposition,5 + y1,220,220,vex::color::black);
        //        Brain.Screen.setPenColor(vex::color::white); 
        //Brain.Screen.drawRectangle(240 + logoxposition, 120 + y1, 105,105, vex::color::black);
        if(color1 == 0)Brain.Screen.setPenColor(vex::color::red);   
        else if(color1 == 1) Brain.Screen.setPenColor(vex::color::green); 
        else if(color1 == 2) Brain.Screen.setPenColor(vex::color::blue);     
        else if(color1 == 3) Brain.Screen.setPenColor(vex::color::yellow);  
        else if(color1 == 4) Brain.Screen.setPenColor(vex::color::purple); 
        else if(color1 == 5) Brain.Screen.setPenColor(vex::color::orange);  
        else if(color1 == 6) Brain.Screen.setPenColor(vex::color::cyan); 

        a = b = c = d = 0;
         //Draw Rainbow Circle
        Brain.Screen.setPenWidth(5);     
        Brain.Screen.drawCircle(240 + logoxposition, 120 + y1, 105, vex::color::transparent);
         Brain.Screen.setPenWidth(2);
        
  
        //Draw Rainbow Line
        for(int j = 1; j <= 6; j ++){
            Brain.Screen.drawLine(170 - a + logoxposition, 160 + y1, 240 + b + logoxposition, 40 + y1 - a);
            a -= 2;
            b++;
        }
     
        Brain.Screen.setPenColor(vex::color::white); 
     
        //Draw 2 White Lines
        for(int k = 1; k <= 6; k ++){
            Brain.Screen.drawLine(250 - c + logoxposition, 60 + y1 + d, 310 - d + logoxposition, 160 + y1);
            Brain.Screen.drawLine(190 + c + logoxposition, 161 + y1 - d, 301 + logoxposition, 161 + y1 - d);
            d++;
            Brain.Screen.drawLine(250 - c + logoxposition, 60 + y1 + d, 310 - d + logoxposition, 160 + y1);
            Brain.Screen.drawLine(190 + c + logoxposition, 161 + y1 - d, 301 + logoxposition, 161 + y1 - d);
            c++;
            d++;
        }
        
      vex::task::sleep(20);   
       // y1+=5;
        y1*=0.85;
        /*if(y1>=-1){
            y1 =0; 
        }*/

    }
}


/*////////////////////////////////*/
/*       Controller Screem        */
/*        By Connor, 1814D        */
/*////////////////////////////////*/
int controllerscreen(){
    
/*Controller1.Screen.clearScreen();
    
    //Load because it looks cool
    for(int p = 1; p < 3; p++){
        Controller1.Screen.clearLine(2);
        Controller1.Screen.setCursor(2,0);
        Controller1.Screen.print("       Loading.");
        vex::task::sleep(200); 
        Controller1.Screen.clearLine(2);
        Controller1.Screen.setCursor(2,0);
        Controller1.Screen.print("       Loading..");
        vex::task::sleep(200); 
        Controller1.Screen.clearLine(2);
        Controller1.Screen.setCursor(2,0);
        Controller1.Screen.print("       Loading...");
        vex::task::sleep(200); 
    }*/
Controller1.Screen.clearScreen();
    while(1==1){                                

        Controller1.Screen.clearLine(1);
        Controller1.Screen.setCursor(1,0);
        Controller1.Screen.print("Batt Percent: %d", Brain.Battery.capacity(vex::percentUnits::pct));
        vex::task::sleep(5000);                 
       
    }
 
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
  // ..........................................................................
  // Insert autonomous user code here.
  // ..........................................................................

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
    vex::task scrollthrough(rainbowscroll);
    vex::task screenmenu(DeltaMenu);
    vex::task rainbow(rainbowlogo);  //Starts task rainbow
    vex::task controlscreen(controllerscreen);  //Starts task controlscreen
    //rainbow.stop(); //Stops task rainbow
while(1){
    vex::task::sleep(100);
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

    //Prevent main from exiting with an infinite loop.                        
    while(1) {
      vex::task::sleep(100);//Sleep the task for a short amount of time to prevent wasted resources.
    }    
       
}