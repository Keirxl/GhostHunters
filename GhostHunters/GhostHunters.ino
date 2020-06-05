
#define PALE makeColorHSB(200,60,60)

#define GHOST_WAIT_TIME 5000
#define DEAD_TIME 3000

enum blinkType {EMPTY,GHOST,LIGHT,DEAD};
byte blinkType=EMPTY;
enum signalState {INERT,GO,RESOLVE};
byte signalState=INERT;


Timer ghostWaitTimer;//when this runs out a new ghost may or may not spawn
Timer deadTimer; //whent this runs out you los
Timer laserTimer; //for kill animation

byte receivingFace; //to orient the beam of light




void setup() {
  // put your setup code here, to run once:
}

void loop() {

  randomize();
  
   switch (signalState) {
    case INERT:
      inertLoop();
      break;
    case GO:
      goLoop();
      break;
    case RESOLVE:
      resolveLoop();
      break;
  }

  switch(blinkType){
    case EMPTY:
      if(!laserTimer.isExpired()){
        setColor(dim(GREEN,100));
      }else{
        setColor(PALE);
      }
      break;
    case GHOST:
      setColor(WHITE);
      break;
    case LIGHT:
      setColor(YELLOW);
      break;
    case DEAD:
       setColor(RED);
       break;
    default:
      setColor(BLUE);
      break;
  }


  byte sendData = (blinkType<<2) + signalState;
  if(blinkType!=LIGHT){
    setValueSentOnAllFaces(sendData);
  }else{
    setValueSentOnFace(sendData,(receivingFace+3)%6);
  }
}

void inertLoop() {
  
  //set myself to GO
  if (buttonMultiClicked()) {
    byte clicks=buttonClickCount();
    if(clicks==3){
      blinkType=EMPTY;
      signalState = GO;
    }
  }

  if(buttonLongPressed()){
    blinkType=LIGHT;
  }

  //if im not dead,check to see if someone near me is dead. if so. same.
  if(blinkType!=DEAD){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==DEAD){
          blinkType=DEAD;
        }
      }
    }
  }

  
  if(blinkType==EMPTY){
    //ghost spawning
    if(ghostWaitTimer.isExpired()){ 
      byte randomHaunting=random(100);
      if(randomHaunting>=80){
        deadTimer.set(DEAD_TIME);
        blinkType=GHOST;
      }
      ghostWaitTimer.set(GHOST_WAIT_TIME);
    }

    
  }

  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==LIGHT){
          if(blinkType==GHOST){
            deadTimer.set(DEAD_TIME);
          }
        }
     }
  }
  

  
  if(blinkType==GHOST){
    //if the time is up i die
    if(deadTimer.isExpired()){
      blinkType=DEAD;
    }
    if(buttonDoubleClicked()){
      laserTimer.set(150);
      blinkType=EMPTY;
    }
  }
  



  

  //listen for neighbors in GO
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == GO) {//a neighbor saying GO!
        signalState = GO;

      }
    }
  }
}

void goLoop() {
  signalState = RESOLVE;//I default to this at the start of the loop. Only if I see a problem does this not happen

  blinkType=EMPTY;


  //look for neighbors who have not heard the GO news
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == INERT) {//This neighbor doesn't know it's GO time. Stay in GO
        signalState = GO;
      }
    }
  }
}

void resolveLoop() {
  signalState = INERT;//I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == GO) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        signalState = RESOLVE;
      }
    }
  }
}

byte getBlinkType(byte data){
  return ((data>>2)&3);
}

byte getSignalState(byte data){
  return (data & 3);
}
