//GhostHunters
// Try to survive with a friend as your mansion is haunted
// by Ghosts, Ghouls, and Goblins
//With Levvels



#define PALE makeColorHSB(200,60,60)
#define lightHue 45
#define laserHue 135
#define bossHue 75
#define ghoulHue 20 //orange
#define redHue 8

// 100-these gives you the chance of spawn
int GREENTARGET_SPAWN_CHANCE;   //95 seems good 
int PURPLETARGET_ORANGETARGET_SPAWN_CHANCE;  //80 seems good

//Timing of Spawns
int GREENTARGET_TIME;
int PURPLETARGET_WAIT_TIME;

//time to kill mobs
int GREENTARGET_DEAD_TIME;
int DEAD_TIME;




#define SURVIVAL_TIME 60000 //one minute
#define PURPLETARGET_FADE_TIME 100 //time of breath() function


//---------------
//  Signals
//---------------

// A B C D E F
enum blinkType {EMPTY,PURPLETARGET,ORANGETARGET,DEAD,WIN,GREENTARGET,YELLOWY,BLUEY,REDY};
byte blinkType=WIN;
enum signalState {LEVELSELECT,PLAY,GO,RESOLVE};
byte signalState=LEVELSELECT;
byte levelDifficulty;
bool source=false;

Timer ghostWaitTimer;//when this runs out a new ghost may or may not spawn
Timer deadTimer; //whent this runs out you los
Timer laserTimer; //for kill animation
Timer ghostFadeInTimer;
Timer gameTimer;
Timer bossTimer;

byte receivingFace; //to orient the beam of light
byte dimness;
byte highest;
byte lowest;
bool isDecrease=false;// for the ghost fade in
byte randomHaunting; //to see if haunted
byte ghoulOrGhost; //decides ghoul or ghost
byte receivedLevelDifficulty;
byte paintBallColor=1;








void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty=1;
  ghostWaitTimer.set(PURPLETARGET_WAIT_TIME);
  gameTimer.set(SURVIVAL_TIME);
}

void loop() {
  
   switch (signalState) {
    case LEVELSELECT:
      levelSelectLoop();
      break;
    case PLAY:
      PLAYLoop();
      break;
    case GO:
      goLoop();
      break;
    case RESOLVE:
      resolveLoop();
      break;
  }

  if(signalState!= LEVELSELECT){
    switch(blinkType){
      case WIN:
        winDisplay();
        break;
      case GREENTARGET:
        bossDisplay();
        break;
      case EMPTY:
        setColor(PALE);
        //bossAura();
        break;
      case PURPLETARGET:
        ghostDisplay();
        break;
      case ORANGETARGET:
        ghoulDisplay();
        break;
      case YELLOWY:
        if(source){
          setColor(makeColorHSB(lightHue,240,255));
        }else{
          lightDisplay();
        }
        break;
      case BLUEY:
        if(source){
          setColor(makeColorHSB(laserHue,240,255));
        }else{
          beamDisplay();
        }
        break;
      case REDY:
        if(source){
          setColor(makeColorHSB(redHue,250,255));
        }else{
          redyDisplay();
        }
        break;
      case DEAD:
         deadDisplay();
         break;
      default:
        setColor(BLUE);
        break;
    }
  }else{
    levelSelectDisplay();
  }


  if(signalState==LEVELSELECT){
    byte sendData = (levelDifficulty<<4) + signalState;
    setValueSentOnAllFaces(sendData);
  }else{
    byte sendData = (blinkType<<2) + signalState;
    if(blinkType==YELLOWY || blinkType==BLUEY || blinkType==REDY){
      if(source){
        setValueSentOnAllFaces(sendData);
      }else{
        setValueSentOnFace(sendData,(receivingFace+3)%6);
      }
    }else{
      setValueSentOnAllFaces(sendData);
    }
  }
 
}

void levelSelectLoop(){


  //CHANGE TO PLAY STAGE
  if(buttonMultiClicked()){
    byte clicks=buttonClickCount();
    if(clicks==3){
      blinkType=EMPTY;
      ghostWaitTimer.set(PURPLETARGET_WAIT_TIME);
      bossTimer.set(GREENTARGET_TIME);
      gameTimer.set(SURVIVAL_TIME);
      signalState=GO;
    }
  }

  //LEVEL TOGGLE
  if(buttonSingleClicked()){
    levelDifficulty++;
    if(levelDifficulty>3){
      levelDifficulty=1;
    }
  }


// use % here to make it so increase always happens and it just goes around the bend

    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getLevelDifficulty(getLastValueReceivedOnFace(f)) != 0){
          receivedLevelDifficulty = getLevelDifficulty(getLastValueReceivedOnFace(f));
          switch(levelDifficulty){
            case 1:
              if(receivedLevelDifficulty==2){
                levelDifficulty=2;
              }
              break;
            case 2:
              if(receivedLevelDifficulty==3){
                levelDifficulty=3;
              }
              break;
            case 3:
              if(receivedLevelDifficulty==1){
                levelDifficulty=1;
              }
              break;
              
          }
        }
      }
    }

    //LISTEN FOR OTHERS IN PLAY STAGE
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getSignalState(getLastValueReceivedOnFace(f))==GO){
          signalState=GO;
        }
      }
    }
  
}

void PLAYLoop() {
  
  
  
  //set myself to LEVELSELECT
  if (buttonMultiClicked()) {
    byte clicks=buttonClickCount();
    if(clicks==3){
      blinkType=WIN;
      signalState = RESOLVE;
      levelDifficulty=1;
    }
  }

//FLASHLIGHT AND LASER HANDLING
    if(buttonLongPressed()){
      if(isAlone()){
        source=true;
        paintBallColor=1;
        colorHandling();
      }
    }
    if(source==true){
      if(buttonSingleClicked()){
        if(isAlone()){
          paintBallColor++;
          colorHandling();
        }
      }
    }
    if(blinkType==BLUEY && source==true){
      if(buttonSingleClicked()){
        if(isAlone()){
          blinkType=YELLOWY;
        }
      }
    }
  
//WIN CONDITION
  if(gameTimer.isExpired()){
    if(blinkType!=DEAD && source==false){
      blinkType=WIN;
    }
  }

  if(blinkType!=DEAD){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==WIN){
          blinkType==WIN;
        }
      }
    }
  }
  
 //if im not dead,check to see if someone near me is dead. if so. same.
  
  if(blinkType!=DEAD && source==false){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==DEAD){
          gameTimer.set(0);
          blinkType=DEAD;
        }
      }
    }
  }

  //PURPLETARGET AND ORANGETARGET SPAWNING
  if(blinkType==EMPTY){
    if(ghostWaitTimer.isExpired()){
      //check for neighborGhosts. dont spawn a ghost if neighbor is a ghosts 
      randomHaunting=random(100);
      ghoulOrGhost=(random(100)+random(100));
      if(noGhostNeighbors()){
        if(randomHaunting>=PURPLETARGET_ORANGETARGET_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          if(ghoulOrGhost>=100){
            deadTimer.set(DEAD_TIME);
            highest=185;
            lowest=140;
            blinkType=ORANGETARGET;
          }else{
            deadTimer.set(DEAD_TIME);
            highest=185;
            lowest=140;
            blinkType=PURPLETARGET;
          }
        }
      }
      ghostWaitTimer.set(PURPLETARGET_WAIT_TIME);
      //ghostWaitTimer.set(random(500)+RANDOM_PURPLETARGET_TIME);
    }
  }

  //GREENTARGET SPAWNING
  if(blinkType==EMPTY){
    if(bossTimer.isExpired()){
      randomHaunting=random(100);
      if(noGhostNeighbors()){
        if(randomHaunting>=GREENTARGET_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          deadTimer.set(GREENTARGET_DEAD_TIME);
          blinkType=GREENTARGET;
        }
      }
      bossTimer.set(GREENTARGET_TIME);
      //bossTimer.set(random(500)+RANDOM_GREENTARGET_TIME);
    }
  }

  //GREENTARGET KILLING (if getting flashlight and laser then boss dies
  if(blinkType==GREENTARGET){
    if(isReceivingLaser() && isReceivingLight()){
      blinkType=EMPTY;
    }
  }

  //PURPLE KILLING
  if(blinkType==PURPLETARGET){
    if(isReceivingLaser() && isReceivingRED()){
      blinkType=EMPTY;
    }
  }

  //ORANGE KILLING
  if(blinkType==ORANGETARGET){
    if(isReceivingLight() && isReceivingRED()){
      blinkType=EMPTY;
    }
  }

//-----------------------------------------
// Super Clunky Signal Sending Chunk (SCSC)
//-----------------------------------------
  
  // YELLOWY
  if(blinkType==YELLOWY && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==YELLOWY){
        blinkType=YELLOWY;
      }else{
        blinkType=EMPTY;
      }
    }else{
      blinkType=EMPTY;
    }
  }else{
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==YELLOWY){
          if(blinkType==EMPTY){
            receivingFace=f;
            blinkType=YELLOWY;
          }
        }
     }
    }
  }
  
  // BLUEY
  
  if(blinkType==BLUEY && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==BLUEY){
        blinkType=BLUEY;
      }else{
        blinkType=EMPTY;
      }
    }else{
      blinkType=EMPTY;
    }
  }else{
    FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==BLUEY){
          if(blinkType==EMPTY){
            receivingFace=f;
            blinkType=BLUEY;
          }
        }
     }
  }
  }

  // REDY
  
  if(blinkType==REDY && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==REDY){
        blinkType=REDY;
      }else{
        blinkType=EMPTY;
      }
    }else{
      blinkType=EMPTY;
    }
  }else{
    FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==REDY){
          if(blinkType==EMPTY){
            receivingFace=f;
            blinkType=REDY;
          }
        }
     }
  }
  }
  

  //IF I DONT KILL THE PURPLETARGETS OR ORANGETARGETS FAST ENOUGH I DIE
  if(blinkType==PURPLETARGET || blinkType==ORANGETARGET){
    if(deadTimer.isExpired()){
      blinkType=DEAD;
    }
    if(buttonDoubleClicked()){
      blinkType=EMPTY;
    }
  }

  //IF I DONT KILL THE GREENTARGET FAST ENOUGH I DIE
  if(blinkType==GREENTARGET){
    if(deadTimer.isExpired()){
      blinkType=DEAD;
    }
  }
  

  


  //listen for neighbors in resolve
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == RESOLVE) {//a neighbor saying GO!
        signalState = RESOLVE;

      }
    }
  }
}

//--------------------------
//     GO LOOP transitions to PLAY
//--------------------------

void goLoop() {
  signalState = PLAY;

  switch(levelDifficulty){
    case 1: //no bosses(goblins) and low spawn rate
      GREENTARGET_SPAWN_CHANCE=101;
      PURPLETARGET_ORANGETARGET_SPAWN_CHANCE=80;
      DEAD_TIME=8000;
      PURPLETARGET_WAIT_TIME=3000;
      ghostWaitTimer.set(3000);     
      break;
    case 2: //the og difficulty
      GREENTARGET_SPAWN_CHANCE=95;
      PURPLETARGET_ORANGETARGET_SPAWN_CHANCE=80;
      GREENTARGET_DEAD_TIME=4000;
      DEAD_TIME=3000;
      GREENTARGET_TIME=3500;
      PURPLETARGET_WAIT_TIME=3000;
      ghostWaitTimer.set(3000);     
      bossTimer.set(3500);
      break;
    case 3:
      GREENTARGET_SPAWN_CHANCE=90;
      PURPLETARGET_ORANGETARGET_SPAWN_CHANCE=80;
      GREENTARGET_DEAD_TIME=4000;
      DEAD_TIME=3000;
      GREENTARGET_TIME=3100;
      PURPLETARGET_WAIT_TIME=2500;
      ghostWaitTimer.set(2500);     
      bossTimer.set(3000);
      break;

  }
  gameTimer.set(SURVIVAL_TIME);
  blinkType=EMPTY;
  source=false;

  //look for neighbors who have not heard the news
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == LEVELSELECT) {
        signalState = GO;
      }
    }
  }
}

//------------------------------------------
//    RESOLVE transitions to LEVELSELECT
//------------------------------------------

void resolveLoop() {
  signalState = LEVELSELECT;//I default to this at the start of the loop. Only if I see a problem does this not happen

  blinkType=EMPTY;
  source=false;
  
  
  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == PLAY) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        signalState = RESOLVE;
      }
    }
  }

}

//For lasers
bool isReceivingLaser(){
  byte lasers=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == BLUEY){
        lasers++;
      }
    }
  }
  if(lasers==0){
    return false;
  }else{
    return true;
  }
}

//For lights
bool isReceivingLight(){
  byte lights=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == YELLOWY){
        lights++;
      }
    }
  }
  if(lights==0){
    return false;
  }else{
    return true;
  }
}

bool isReceivingRED(){
  byte reds=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == REDY){
        reds++;
      }
    }
  }
  if(reds==0){
    return false;
  }else{
    return true;
  }
}

//returns true if there are no ghosts nearby
bool noGhostNeighbors(){
  byte ghosts=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == PURPLETARGET || getBlinkType(getLastValueReceivedOnFace(f)) == ORANGETARGET || getBlinkType(getLastValueReceivedOnFace(f)) == GREENTARGET){//This neighbor isn't in RESOLVE. Stay in RESOLVE
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
    ghostFadeInTimer.set(PURPLETARGET_FADE_TIME);
  }
}

void lightDisplay(){
  setColor(makeColorHSB(lightHue,random(50)+190,random(70)+70));
  setColorOnFace(makeColorHSB(lightHue,240,255),receivingFace);
  setColorOnFace(makeColorHSB(lightHue,240,255),(receivingFace+3)%6);
}

void beamDisplay(){
  setColor(makeColorHSB(laserHue,random(50)+190,random(70)+70));
  setColorOnFace(makeColorHSB(laserHue,240,255),receivingFace);
  setColorOnFace(makeColorHSB(laserHue,240,255),(receivingFace+3)%6);
}

void redyDisplay(){
  setColor(makeColorHSB(redHue,random(50)+190,random(70)+70));
  setColorOnFace(makeColorHSB(redHue,240,255),receivingFace);
  setColorOnFace(makeColorHSB(redHue,240,255),(receivingFace+3)%6);
}
        

void ghostDisplay(){
  breath();
  setColor(makeColorHSB(195,255,dimness));
}

void ghoulDisplay(){
  breath();
  setColor(makeColorHSB(ghoulHue,255,dimness));
}

void bossDisplay(){
  setColorOnFace(makeColorHSB(bossHue,245,random(65)+190),random(5));
}


void deadDisplay(){
  breath();
  setColor(makeColorHSB(15,random(55)+200,dimness));
}

void winDisplay(){
  if(ghostFadeInTimer.isExpired()){
    byte maybeFlash=random(50);
      if(maybeFlash>=25){
        setColor(WHITE);
      }else{
      setColor(dim(WHITE,175));
    }
    ghostFadeInTimer.set(PURPLETARGET_FADE_TIME);
  }
}

//DISPLAY CURRENT LEVEL
void levelSelectDisplay(){
  setColor(PALE);
  FOREACH_FACE(f){
    if(f<levelDifficulty){
      setColorOnFace(WHITE,f);
    }
  }
}

//PAINT BALL COLOR HANDLING

void colorHandling(){
 byte paintBallColorModded = (paintBallColor%3) + 1;
  switch(paintBallColorModded){
    case 1:
      blinkType=YELLOWY;
      break;
    case 2:
      blinkType=BLUEY;
      break;
    case 3:
      blinkType=REDY;
      break;
  }
}



byte getBlinkType(byte data){
  return (data>>2);
}

byte getSignalState(byte data){
  return (data & 3);
}

byte getLevelDifficulty(byte data){
  return (data>>4);
}
