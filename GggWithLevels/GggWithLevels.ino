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
enum blinkType {EMPTY,GHOST,GHOUL,LIGHT,BEAM,DEAD,WIN,BOSS,EMP,BURST};
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
Timer empBurstTimer;

byte receivingFace; //to orient the beam of light
byte dimness;
byte highest;
byte lowest;
bool isDecrease=false;// for the ghost fade in
byte randomHaunting; //to see if haunted
byte ghoulOrGhost; //decides ghoul or ghost
byte receivedLevelDifficulty;
byte hp=3;
byte weaponType=1;



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
        if(source){
          setColor(makeColorHSB(lightHue,240,255));
        }else{
          lightDisplay();
        }
        break;
      case DEAD:
         deadDisplay();
         break;
      case BEAM:
        if(source){
          setColor(makeColorHSB(laserHue,240,255));
        }else{
          beamDisplay();
        }
        break;
      case EMP:
         if(source){
          setColor(MAGENTA);
         }else{
          setColor(makeColorHSB(245,240,random(125)+130));
         }
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
  
//------------------------------------
//   FLASHLIGHT AND LASER HANDLING
//------------------------------------

    if(buttonLongPressed()){
      if(isAlone()){
        source=true;
        weaponType=1;
        weaponHandling();
      }
    }
    if(source==true){
      if(buttonSingleClicked()){
        if(isAlone()){
          source=true;
          weaponType++;
          if(weaponType>3){
             weaponType=1;
          }
          weaponHandling();
        }
      }
    }
    

   if(source){
    if(hp<=0){
      blinkType=DEAD;
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
//----------------
//   KILLING MOBS 
//----------------
  if(blinkType==BOSS){
    if(isReceivingLaser() && isReceivingLight()){
      blinkType=EMPTY;
    }else if(isReceivingEmpBurst()){
      blinkType=EMPTY;
    }
  }

  if(blinkType==GHOST){
    if(isReceivingLight()){
      blinkType=EMPTY;
    }else if(isReceivingEmpBurst()){
      blinkType=EMPTY;
    }
  }

  if(blinkType==GHOUL){
    if(isReceivingLaser()){
      blinkType=EMPTY;
    }else if(isReceivingEmpBurst()){
      blinkType=EMPTY;
    }
  }

 
//-----------------------------------------
// Super Clunky Signal Sending Chunk (SCSC)
//-----------------------------------------

  //light sending
  if(blinkType==LIGHT && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==LIGHT){
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
        if(getBlinkType(getLastValueReceivedOnFace(f))==LIGHT){
          if(blinkType==EMPTY){
            receivingFace=f;
            blinkType=LIGHT;
          }
        }
     }
    }
  }
  
  //laser sending
  if(blinkType==BEAM && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==BEAM){
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
        if(getBlinkType(getLastValueReceivedOnFace(f))==BEAM){
          if(blinkType==EMPTY){
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
  source=false;
  hp=3;

  
  
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
      if (getBlinkType(getLastValueReceivedOnFace(f)) == BEAM){
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
      if (getBlinkType(getLastValueReceivedOnFace(f)) == LIGHT){
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

bool isReceivingEmpBurst(){
  byte emp=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == EMP){
        emp++;
      }
    }
  }
  if(emp==0){
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

void weaponHandling(){
  switch(weaponType){
    case 1:
      blinkType=LIGHT;
      break;
    case 2:
      blinkType=BEAM;
      break;
    case 3:
      blinkType=EMP;
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
