//#include "DigiKeyboard.h"

/*** ASSIGNING PIN NUMBERS FOR ARDUINO I/O ***/
const int led = 13; //LED output for testing

// is this still used? -GL
const int opticalPin = A4; // analog

const int strobePin = 4; //digital output to strobe
// is the 'reverse' part important? might be confusing -GL
const int reverseBreakBeamPin = 2; //digital input from breakbeam IR receiver

//digital inputs from IR receivers
const int optical_In1 = 7; 
const int optical_In2 = 8; 
const int optical_In3 = 9; 
/*** ASSIGNING PIN NUMBERS FOR ARDUINO I/O ***/

/*** ASSIGNING THRESHOLDS TO BE USED IN CODE ***/
unsigned long STROBEFLASHTIME = 20;  //sets strobe 'on' time
const int CANDIDATETHRESHOLD = 50;  //sets minimum duration of candidate pulse
/*** ASSIGNING THRESHOLDS TO BE USED IN CODE***/

/*** ASSIGNING STATE VARIABLES ***/
bool breakbeam = false; //whether breakbeam has triggered
bool looking = false;   //whether user is looking at ball
bool lookingtemp = false; 

//states of individual 'looking' detectors
bool optical_In1_state = false; 
bool optical_In2_state = false; 
bool optical_In3_state = false; 

//is this distinct from 'breakbeam'? -GL
bool reverse_breakbeam_state = false; 
/*** ASSIGNING STATE VARIABLES ***/

/*** ASSIGNING ADDITIONAL VARIABLES ***/
//these are accessed during interrupts
//they track timing of events using millis()
volatile unsigned long last_breakbeam_change = NULL;
volatile unsigned long now = NULL; 

//these track timing of events using millis()
unsigned long candidate_change = NULL;      //time index of candidate 'looking' event
unsigned long last_looking_change = NULL;   //time index of the last time 'looking' has changed
unsigned long next = NULL;                  //time index of next scheduled strobe flash
unsigned long last_breakbeam_flash = NULL;  //time index of last time the breakbeam triggered a strobe flash
unsigned long last_flash = NULL;            //time index of last time any strobe flash was triggered

//this is an estimate of the spinning ball's period
unsigned long global_period = NULL; 

//N is the number of strobe states desired when not looking
int N = 5; 
int bonus_flash_counter = 0; 
/*** ASSIGNING ADDITIONAL VARIABLES ***/

//this is called once, to configure the arduino before it runs the main program
void setup() {
  /*** ASSIGNING PINMODES ***/
  pinMode(optical_In1, INPUT); 
  pinMode(optical_In2, INPUT); 
  pinMode(optical_In3, INPUT); 
  pinMode(reverseBreakBeamPin, INPUT); 
  pinMode(strobePin, OUTPUT); 
  pinMode(led, OUTPUT); 
  pinMode(5, OUTPUT);       //what is this one??? -GL
  /*** ASSIGNING PINMODES ***/

  // Attaching interrupt to breakbeam pin so that 'breakBeamCheck' is triggered each time stick passes across beam. 
  // Note that stick is off center, so that only one side of stick breaks beam
  // (thus, the breakbeam should trigger only once per revolution)
  attachInterrupt(digitalPinToInterrupt(reverseBreakBeamPin), breakBeamCheck, RISING); // was 'CHANGE'

  // Assigning initial values to output pins 
  digitalWrite(strobePin, LOW);
  digitalWrite(led, LOW);

  //Serial is used for debugging. Not needed for final  code
  Serial.begin(9600); 
  while(! Serial); 
  Serial.println("Here we go!"); 
}

//this is the main loop, which is called endlessly
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
//It checks whether the signal picked up by the breakbeam is legitimate, and sets the 'breakbeam' state variable if so.
//IMPORTANT NOTE: THE VALUES IN BREAK BEAM CHECK MUST BE CALIBRATED BASED ON THE MOTOR ROTATION RATE. (currently set to values 0 > x > 5)
//This is to distinguish signal from the strobe noise (since strobe also triggers breakbream) 
/******/

/*
void breakBeamCheck(){
     // breakbeam signal should have well-defined duration (see above)
     // check that the time since the last breakbeam change fits within expected window 
     if((now - last_breakbeam_change) > 0 && (now - last_breakbeam_change) < 5){
      breakbeam = true; 
      //Serial.println((now - last_breakbeam_change)); 
   }
   last_breakbeam_change = now; 
   
}
*/
void breakBeamCheck(){
  breakbeam = true; 
}

/*******/
//The lookingCheck() function is called by the main loop
//It reads the state of the 3 optical detectors and determines whether they indicate that the user is looking at the ball
//Note that lookingCheck() uses the CANDIDATETHRESHOLD value to filter out spurious signals
/******/
void lookingCheck(){
  //temporarily use only one optical input, for debugging
  //lookingtemp = !optical_In1_state;

  //read the optical inputs
  optical_In1_state = digitalRead(optical_In1); 
  optical_In2_state = digitalRead(optical_In2); 
  optical_In3_state = digitalRead(optical_In3); 

  //perform logical OR of optical inputs, so that any one will trigger a change
  //note that the IR receiver outputs go low when a signal is present, so we invert each sensor's state to maintain the idea that 'true' = 'signal present'
  //also note that we only populate a temporary variable until the state change is verified as legitimate
  if(!optical_In1_state || !optical_In2_state || !optical_In3_state){
    lookingtemp = true;  
  }
  else{
    lookingtemp = false; 
  }

  //if the looking state has changed
  if(lookingtemp != looking){
    //and if there isn't a candidate change we're waiting to evaluate
    if(!candidate_change){
          //record a new candidate change
          candidate_change = now; 
    }
    //if there is a candidate change already, and it it longer than the threshold
    if(((now - candidate_change) > CANDIDATETHRESHOLD)){
        //they're really looking!
        //record the new time, and update looking state
        last_looking_change = now; 
        looking = lookingtemp;  
        Serial.print("Looking:"); 
        Serial.println(looking); 
    }    
  }
  //either nothing new has happened, or there was a very fast change between sensor reads
  //in either case, there probably wasn't a legitimate signal, so reset the candidate change (start looking for a new one)
  else if(lookingtemp == looking){
    candidate_change = NULL; 
  }
}


/*******/
//The flashScheduler() function is called by the main loop
//It decides when the next strobe flash should happen, based on the breakbeam and 'looking' states
/******/
void flashScheduler(){
  //if the breakbeam is triggered, the next flash to schedule is /always/ 'now', whether or not the person is looking
  if(breakbeam){
          //if the breakbeam has flashed at least once before, then calculate the ball's period
          if(last_breakbeam_flash){
                    global_period = now - last_breakbeam_flash; 
          }
          //schedule a flash, update the last breakbeam flash time, and reset breakbeam state
          next = now; 
          last_breakbeam_flash = now;  
          breakbeam = false;
          bonus_flash_counter = 0; 
  }
  //if breakbeam wasn't triggered, schedule 'superposition state' flashes, only if user isn't looking
  //also make sure there isn't a breakbeam flash about to happen, and that the ball's period has been determined
  else if((!looking) && global_period && (!next) && (bonus_flash_counter < (N-1))){
     next = last_flash + global_period / N;     //add flash corresponding to next of N superposition states
     //Serial.println(bonus_flash_counter); 
     //Serial.println(now); 
     //Serial.println(next); 
     bonus_flash_counter = bonus_flash_counter + 1; 
  }
 
}

/*******/
//The flash() function is called by the main loop
//It controls the 'on' time when the strobe is flashed, so that the lighting effect is consistent
//Note that it references STROBEFLASHTIME, which sets the minimum flash 'on' time
/******/
void flash(){
  //if a flash has been scheduled
  if(next){
        //and it's time for the strobe to be on
        if(now >= next){ 
          //Serial.println("here"); 
          //turn the strobe on and update the last flash time
           digitalWrite(strobePin, HIGH);
           digitalWrite(led, HIGH);
           last_flash = next;  
        }
       //or it's time for the strobe to be off (minimum 'on' time has elapsed)
        if(now >= (STROBEFLASHTIME + next)){
          //Serial.println("There"); 
          //turn the strobe off, and clear 'next', which indicates no new flashes are scheduled
           digitalWrite(strobePin, LOW);
           digitalWrite(led, LOW);
           next = NULL; 
        }  
  } 
}

