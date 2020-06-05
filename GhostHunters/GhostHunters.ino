//GhostHunters
// Try to survive with a friend as ghosts and ghouls rise up
// into your now hautned mansion!


#define PALE makeColorHSB(200,60,60)
#define lightHue 45
#define laserHue 75

#define SURVIVAL_TIME 60000 //one minute
#define GHOST_WAIT_TIME 5000
#define GHOST_FADE_TIME 100
#define DEAD_TIME 3000
#define LASER_TIME 300

enum blinkType {EMPTY,GHOST,GHOUL,LIGHT,DEAD,FLASHLIGHT,LASER,BEAM};
byte blinkType=EMPTY;
enum signalState {INERT,GO,RESOLVE};
byte signalState=INERT;
enum winStates {WIN,PLAY};
byte winState=PLAY;


Timer ghostWaitTimer;//when this runs out a new ghost may or may not spawn
Timer deadTimer; //whent this runs out you los
Timer laserTimer; //for kill animation
Timer ghostFadeInTimer;
Timer gameTimer;

byte receivingFace; //to orient the beam of light
byte dimness;
byte highest;
byte lowest;
bool isDecrease=false;// for the ghost fade in





void setup() {
  // put your setup code here, to run once:
  randomize();
  ghostWaitTimer.set(GHOST_WAIT_TIME);
  gameTimer.set(SURVIVAL_TIME);
}

void loop() {
  
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

  if(winState==WIN){
    winDisplay();
  }else{
    switch(blinkType){
      case EMPTY:
        if(!laserTimer.isExpired()){
          setColor(dim(GREEN,180));
        }else{
          setColor(PALE);
        }
        break;
      case GHOST:
        ghostDisplay();
        break;
      case GHOUL:
        ghoulDisplay();
        break;
      case LIGHT:
        setColor(makeColorHSB(lightHue,240,100));
        setColorOnFace(makeColorHSB(lightHue,240,255),receivingFace);
        setColorOnFace(makeColorHSB(lightHue,240,255),(receivingFace+3)%6);
        break;
      case FLASHLIGHT:
        setColor(makeColorHSB(lightHue,240,255));
        break;
      case DEAD:
         deadDisplay();
         break;
      case LASER:
        setColor(makeColorHSB(laserHue,240,255));
        break;
      case BEAM:
        setColor(makeColorHSB(laserHue,240,100));
        setColorOnFace(makeColorHSB(laserHue,240,255),receivingFace);
        setColorOnFace(makeColorHSB(laserHue,240,255),(receivingFace+3)%6);
        break;
      default:
        setColor(BLUE);
        break;
    }
  }


  byte sendData = (blinkType<<2) +(winState<<5)+ signalState;
  if(blinkType==LIGHT || blinkType==BEAM){
    setValueSentOnFace(sendData,(receivingFace+3)%6);
  }else{
    setValueSentOnAllFaces(sendData);
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
      if(isAlone()){
        blinkType=FLASHLIGHT;
      }
    }
    if(blinkType==FLASHLIGHT){
      if(buttonDoubleClicked()){
        if(isAlone()){
          blinkType=LASER;
        }
      }
    }
    if(blinkType==LASER){
      if(buttonDoubleClicked()){
        if(isAlone()){
          blinkType=FLASHLIGHT;
        }
      }
    }
  

  if(gameTimer.isExpired()){
    if(blinkType!=DEAD && blinkType!=FLASHLIGHT && blinkType!=LASER){
      winState=WIN;
    }
  }


  if(blinkType!=DEAD){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getWinState(getLastValueReceivedOnFace(f))==WIN){
          winState=WIN;
        }
      }
    }
  }
  
  //if im not dead,check to see if someone near me is dead. if so. same.

  
  if(blinkType!=DEAD && blinkType!=LASER && blinkType!=FLASHLIGHT){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==DEAD){
          gameTimer.set(0);
          blinkType=DEAD;
        }
      }
    }
  }

  //GHOST AND GHOUL SPAWNING
  if(blinkType==EMPTY){
    if(ghostWaitTimer.isExpired()){
      //check for neighborGhosts. dont spawn a ghost if neighbor is a ghosts 
      byte randomHaunting=random(100);
      byte ghoulOrGhost=(random(100)+random(100));
      if(noGhostNeighbors()){
        if(randomHaunting>=75){
          if(ghoulOrGhost>=100){
            deadTimer.set(DEAD_TIME);
            highest=185;
            lowest=140;
            blinkType=GHOUL;
          }else{
            deadTimer.set(DEAD_TIME);
            highest=185;
            lowest=140;
            blinkType=GHOST;
          }
        }
      }
      ghostWaitTimer.set(GHOST_WAIT_TIME);
    }

    
  }

 
  
  //light sending
  if(blinkType==LIGHT){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==LIGHT || getBlinkType(getLastValueReceivedOnFace(receivingFace))==FLASHLIGHT){
        blinkType=LIGHT;
      }else{
        blinkType=EMPTY;
      }
    }else{
      blinkType=EMPTY;
    }
  }else{
    FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==LIGHT ||getBlinkType(getLastValueReceivedOnFace(f))==FLASHLIGHT){
          if(blinkType==GHOST){
            blinkType=EMPTY;
          }else if(blinkType==EMPTY){
            receivingFace=f;
            blinkType=LIGHT;
          }
        }
     }
  }
  }
  
  //laser sending
  if(blinkType==BEAM){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==BEAM || getBlinkType(getLastValueReceivedOnFace(receivingFace))==LASER){
        blinkType=BEAM;
      }else{
        blinkType=EMPTY;
      }
    }else{
      blinkType=EMPTY;
    }
  }else{
    FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==BEAM ||getBlinkType(getLastValueReceivedOnFace(f))==LASER){
          //kill the ghost!
          if(blinkType==GHOUL){
            blinkType=EMPTY;
          }else if(blinkType==EMPTY){
            receivingFace=f;
            blinkType=BEAM;
          }
        }
     }
  }
  }
  

  
  if(blinkType==GHOST || blinkType==GHOUL){
    //if the time is up i die
    if(deadTimer.isExpired()){
      blinkType=DEAD;
    }
    if(buttonDoubleClicked()){
      laserTimer.set(LASER_TIME);
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
  winState=PLAY;
  ghostWaitTimer.set(GHOST_WAIT_TIME);
  gameTimer.set(SURVIVAL_TIME);


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

//returns true if there are no ghosts nearby
bool noGhostNeighbors(){
  byte ghosts=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == GHOST) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        ghosts++;
      }
    }
  }
  if(ghosts==0){
    return true;
  }else{
    return false;
  }
}

void breath(){
  if(ghostFadeInTimer.isExpired()){
    if(!isDecrease){
      dimness+=10;
    }else{
      dimness-=10;
    }
    if(dimness>250){
      dimness=250;
      isDecrease=true;
    }else if(dimness<180){
      dimness=180;
      isDecrease=false;
    }
    ghostFadeInTimer.set(GHOST_FADE_TIME);
  }
}

void ghostDisplay(){
  breath();
  setColor(makeColorHSB(0,0,dimness));
}

void ghoulDisplay(){
  breath();
  setColor(makeColorHSB(12,255,dimness));
}

void deadDisplay(){
  setColor(makeColorHSB(8,230,random(55)+200));
}

void winDisplay(){
  if(ghostFadeInTimer.isExpired()){
    byte maybeFlash=random(50);
      if(maybeFlash>=25){
        setColor(WHITE);
      }else{
      setColor(dim(WHITE,175));
    }
    ghostFadeInTimer.set(GHOST_FADE_TIME);
  }
}

byte getBlinkType(byte data){
  return ((data>>2)&7);
}

byte getSignalState(byte data){
  return (data & 3);
}

byte getWinState(byte data){
  return (data>>5);
}
