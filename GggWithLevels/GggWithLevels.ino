//GhostHunters
// Try to survive with a friend as your mansion is haunted
// by Ghosts, Ghouls, and Goblins
//With Levvels



#define PALE makeColorHSB(200,60,60)
#define lightHue 45
#define laserHue 135
#define bossHue 75
#define ghoulHue 8

// 100-these gives you the chance of spawn
int BOSS_SPAWN_CHANCE;   //95 seems good 
int GHOST_GHOUL_SPAWN_CHANCE;  //80 seems good

//Timing of Spawns
int BOSS_TIME;
int GHOST_WAIT_TIME;

//time to kill mobs
int BOSS_DEAD_TIME;
int DEAD_TIME;




#define SURVIVAL_TIME 60000 //one minute
#define GHOST_FADE_TIME 100 //time of breath() function


// A B C D E F
enum blinkType {EMPTY,GHOST,GHOUL,LIGHT,DEAD,WIN,FLASHLIGHT,LASER,BEAM,BOSS};
byte blinkType=WIN;
enum signalState {LEVELSELECT,PLAY,GO,RESOLVE};
byte signalState=LEVELSELECT;
byte levelDifficulty;

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








void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty=1;
  ghostWaitTimer.set(GHOST_WAIT_TIME);
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
      case BOSS:
        bossDisplay();
        break;
      case EMPTY:
        setColor(PALE);
        //bossAura();
        break;
      case GHOST:
        ghostDisplay();
        break;
      case GHOUL:
        ghoulDisplay();
        break;
      case LIGHT:
        lightDisplay();
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
        beamDisplay();
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
    if(blinkType==LIGHT || blinkType==BEAM){
      setValueSentOnFace(sendData,(receivingFace+3)%6);
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
      ghostWaitTimer.set(GHOST_WAIT_TIME);
      bossTimer.set(BOSS_TIME);
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
  
//WIN CONDITION
  if(gameTimer.isExpired()){
    if(blinkType!=DEAD && blinkType!=FLASHLIGHT && blinkType!=LASER){
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
      randomHaunting=random(100);
      ghoulOrGhost=(random(100)+random(100));
      if(noGhostNeighbors()){
        if(randomHaunting>=GHOST_GHOUL_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
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
      //ghostWaitTimer.set(random(500)+RANDOM_GHOST_TIME);
    }
  }

  //BOSS SPAWNING
  if(blinkType==EMPTY){
    if(bossTimer.isExpired()){
      randomHaunting=random(100);
      if(noGhostNeighbors()){
        if(randomHaunting>=BOSS_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          deadTimer.set(BOSS_DEAD_TIME);
          blinkType=BOSS;
        }
      }
      bossTimer.set(BOSS_TIME);
      //bossTimer.set(random(500)+RANDOM_BOSS_TIME);
    }
  }

  //BOSS KILLING (if getting flashlight and laser then boss dies
  if(blinkType==BOSS){
    if(isReceivingLaser() && isReceivingLight()){
      blinkType=EMPTY;
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
  

  //IF I DONT KILL THE GHOSTS OR GHOULS FAST ENOUGH I DIE
  if(blinkType==GHOST || blinkType==GHOUL){
    if(deadTimer.isExpired()){
      blinkType=DEAD;
    }
    if(buttonDoubleClicked()){
      blinkType=EMPTY;
    }
  }

  //IF I DONT KILL THE BOSS FAST ENOUGH I DIE
  if(blinkType==BOSS){
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
      BOSS_SPAWN_CHANCE=101;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      DEAD_TIME=8000;
      GHOST_WAIT_TIME=3000;
      ghostWaitTimer.set(3000);     
      break;
    case 2: //the og difficulty
      BOSS_SPAWN_CHANCE=95;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      BOSS_DEAD_TIME=4000;
      DEAD_TIME=3000;
      BOSS_TIME=3500;
      GHOST_WAIT_TIME=3000;
      ghostWaitTimer.set(3000);     
      bossTimer.set(3500);
      break;
    case 3:
      BOSS_SPAWN_CHANCE=90;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      BOSS_DEAD_TIME=4000;
      DEAD_TIME=3000;
      BOSS_TIME=3100;
      GHOST_WAIT_TIME=2500;
      ghostWaitTimer.set(2500);     
      bossTimer.set(3000);
      break;

  }
  gameTimer.set(SURVIVAL_TIME);
  blinkType=EMPTY;

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
      if (getBlinkType(getLastValueReceivedOnFace(f)) == LASER || getBlinkType(getLastValueReceivedOnFace(f)) == BEAM){
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
      if (getBlinkType(getLastValueReceivedOnFace(f)) == FLASHLIGHT || getBlinkType(getLastValueReceivedOnFace(f)) == LIGHT){
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

//returns true if there are no ghosts nearby
bool noGhostNeighbors(){
  byte ghosts=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == GHOST || getBlinkType(getLastValueReceivedOnFace(f)) == GHOUL || getBlinkType(getLastValueReceivedOnFace(f)) == BOSS){//This neighbor isn't in RESOLVE. Stay in RESOLVE
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
        

void ghostDisplay(){
  breath();
  setColor(makeColorHSB(0,0,dimness));
}

void ghoulDisplay(){
  breath();
  setColor(makeColorHSB(ghoulHue,255,dimness));
}

void bossDisplay(){
  setColorOnFace(makeColorHSB(bossHue,245,random(65)+190),random(5));
}

void bossAura(){
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == BOSS){
        setColorOnFace(makeColorHSB(bossHue,245,random(50)+100),f);
        setColorOnFace(makeColorHSB(bossHue,245,random(50)+100),(f+1)%6);
        setColorOnFace(makeColorHSB(bossHue,245,random(50)+100),(f-1)%6);
      }
    }
  }
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
    ghostFadeInTimer.set(GHOST_FADE_TIME);
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



byte getBlinkType(byte data){
  return (data>>2);
}

byte getSignalState(byte data){
  return (data & 3);
}

byte getLevelDifficulty(byte data){
  return (data>>4);
}