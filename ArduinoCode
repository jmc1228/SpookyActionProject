//#include "DigiKeyboard.h"

/*** ASSIGNING PIN NUMBERS ***/
const int led = 13; //for testing

const int opticalPin = A4; //analog

const int strobePin = 4; //digital
const int reverseBreakBeamPin = 2; //digital

const int optical_In1 = 7; 
const int optical_In2 = 8; 
const int optical_In3 = 9; 
/*** ASSIGNING PIN NUMBERS ***/

/*** ASSIGNING THRESHOLDS TO BE USED IN CODE ***/
const int LOOKINGTIMETHRESHOLD = 300; 
unsigned long STROBEFLASHTIME = 30; 
const int CANDIDATETHRESHOLD = 100; 
/*** ASSIGNING THRESHOLDS TO BE USED IN CODE***/

/*** ASSIGNING STATE VARIABLES ***/
bool breakbeam = false; 
bool looking = false;
bool lookingtemp = false; 


bool optical_In1_state = false; 
bool optical_In2_state = false; 
bool optical_In3_state = false; 

bool reverse_breakbeam_state = false; 
/*** ASSIGNING STATE VARIABLES ***/

/*** ASSIGNING ADDITIONAL VARIABLES ***/
volatile unsigned long last_breakbeam_change = NULL;
volatile unsigned long now = NULL; 

unsigned long candidate_change = NULL; 
unsigned long last_looking_change = NULL; 
unsigned long next = NULL; 
unsigned long global_period = NULL; 
unsigned long last_breakbeam_flash = NULL; 
unsigned long last_flash = NULL; 
/*** ASSIGNING ADDITIONAL VARIABLES ***/

//N is the number of strobe states desired
int N = 5; 


 
void setup() {
  /*** ASSIGNING PINMODES ***/
  pinMode(optical_In1, INPUT); 
  pinMode(optical_In2, INPUT); 
  pinMode(optical_In3, INPUT); 
  pinMode(reverseBreakBeamPin, INPUT); 
  pinMode(strobePin, OUTPUT); 
  pinMode(led, OUTPUT); 
  pinMode(5, OUTPUT); 
  /*** ASSIGNING PINMODES ***/

  // Attaching interrupt to breakbeam pin to be triggered each time stick passes across beam. Note that stick is off center, so that only one side of stick breaks beam
  attachInterrupt(digitalPinToInterrupt(reverseBreakBeamPin), breakBeamCheck, CHANGE);

  // Assigning initial values to pins 
  digitalWrite(strobePin, LOW);
  digitalWrite(led, LOW);

  /**
   * Serial is used for debugging. Not needed for final  code
   * **/
  Serial.begin(9600); 
  while(! Serial); 
  Serial.println("Here we go!"); 
}

void loop() {

  //assign 'now' to current time, for consistency across all functions 
  now = millis(); 

  //function to check if the user is looking at the ball or not
  lookingCheck();   

  //function to schedule strobe flashes 
  flashScheduler(); 

  //function to trigger strobe flashes
  flash(); 
  
}

/*******/
//The breakBeamCheck() function is triggered via an interrupt 
//IMPORTANT NOTE: THE VALUES IN BREAK BEAM CHECK MUST BE CALLIBRATED BASED ON THE MOTOR SPINNING RATE. (currently set to values 0 > x > 5)
//This is to distinguish signal from the strobe noise (since strobe also triggers breakbream) 
/******/
void breakBeamCheck(){    
     if((now - last_breakbeam_change) > 0 && (now - last_breakbeam_change) < 5){
      breakbeam = true; 
      //Serial.println((now - last_breakbeam_change)); 
   }
   last_breakbeam_change = now; 
   
}

/*******/
//ToDo: Add Coments
/******/
void lookingCheck(){
  //lookingtemp = !optical_In1_state;
  
  optical_In1_state = digitalRead(optical_In1); 
  optical_In2_state = digitalRead(optical_In2); 
  optical_In3_state = digitalRead(optical_In3); 
 
  if(!optical_In1_state || !optical_In2_state || !optical_In3_state){
    lookingtemp = true;  
  }
  else{
    lookingtemp = false; 
  }

  if(lookingtemp != looking){
    if(!candidate_change){
          candidate_change = now; 
    }
    if(((now - candidate_change) > CANDIDATETHRESHOLD)){
        last_looking_change = now; 
        looking = lookingtemp;  
        Serial.print("Looking:"); 
        Serial.println(looking); 
    }
    
    
    
  }
  else if(lookingtemp == looking){
    candidate_change = NULL; 
  }
}


/*******/
//ToDo: Add Coments
/******/
void flashScheduler(){
  //if the breakbeam is triggered, the next flash to schedule is /always/ 'now', whether or not the person is looking
  if(breakbeam){
          if(last_breakbeam_flash){
                    global_period = now - last_breakbeam_flash; 
          }
          next = now; 
          last_breakbeam_flash = now;  
          breakbeam = false; 
  }
  else if(!looking && global_period && !next){
     next = last_flash + global_period / N;         
  }
 
}

/*******/
//ToDo: Add Coments
/******/
void flash(){
  if(next){
        if(now >= next){
           digitalWrite(strobePin, HIGH);
           digitalWrite(led, HIGH);
           last_flash = next;  
        }
       
        if(now - next >= STROBEFLASHTIME){
           digitalWrite(strobePin, LOW);
           digitalWrite(led, LOW);
           next = NULL; 
        }  
  } 
}

