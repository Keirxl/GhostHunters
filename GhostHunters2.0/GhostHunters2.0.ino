//2.00000
#define PALE makeColorHSB(200,60,60)
#define lightHue 230
#define geistHue 135
#define bossHue 75
#define ghoulHue 8
#define DEAD_TIME 5000
#define GHOST_WAIT_TIME 2000
#define BOSS_TIME 3000
#define PERIOD 2000
#define SURVIVAL_TIME 50000 //one minute

// 100-these gives you the chance of spawn
byte BOSS_SPAWN_CHANCE;   //95 seems good 
byte GHOST_GHOUL_SPAWN_CHANCE;  //80 seems good
byte POLTER_SPAWN_CHANCE;

// A B C D E F
enum blinkType {EMPTY,GHOST,GHOUL,DEAD,WIN,LIGHT,BEAM,EMP,BOSS,POLTER,GEISTGUN};
byte blinkType=EMPTY;
enum signalState {LEVELSELECT,PLAY,GO,RESOLVE};
byte signalState=LEVELSELECT;
byte levelDifficulty;
bool source=false;
byte sendData;

Timer ghostWaitTimer;//when this runs out a new ghost may or may not spawn
Timer deadTimer; //whent this runs out you lose
Timer gameTimer;
Timer bossTimer;

byte receivingFace; //to orient the beam of light
byte dimness;
byte randomHaunting; //to see if haunted
byte ghoulOrGhost; //decides ghoul or ghost
byte receivedLevelDifficulty;
byte weaponType=1;
byte badBoiType;
byte badBoiHue[4]={lightHue,ghoulHue,geistHue,bossHue};

void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty=1;
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
      case EMPTY:
        setColor(PALE);
        break;
      case GHOUL:
      case POLTER:
      case BOSS:
      case GHOST:
        badBoiDisplay();
        break;
      case LIGHT:       
        lightDisplay();
        break;
      case DEAD:
         deadDisplay();
         break;
      case BEAM:
          beamDisplay();
        break;
      case GEISTGUN:
          geistGunDisplay();
         break;
      default:
        setColor(BLUE);
        break;
    }
  }else{
    levelSelectDisplay();
  }

  if(signalState==LEVELSELECT){
    sendData = (levelDifficulty<<2) + signalState;
    setValueSentOnAllFaces(sendData);
  }else{
    sendData = (blinkType<<2) + signalState;
    if(blinkType==LIGHT || blinkType==BEAM || blinkType==GEISTGUN){
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

  //WEAPON MAKER (that also cahnges you to play stage
  if(buttonDoubleClicked()){
      if(isAlone()){
        signalState=GO;
        source=true;
        weaponType=1;
        weaponHandling();
      }
    }

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
    if(levelDifficulty>6){
      levelDifficulty=1;
    }
  }

// use % here to make it so increase always happens and it just goes around the bend

    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getSignalState(getLastValueReceivedOnFace(f))==LEVELSELECT){
          if(getLevelDifficulty(getLastValueReceivedOnFace(f)) != 0 && getLevelDifficulty(getLastValueReceivedOnFace(f))<7){
            receivedLevelDifficulty = getLevelDifficulty(getLastValueReceivedOnFace(f));
            if(levelDifficulty==1 && receivedLevelDifficulty==6){
              levelDifficulty = 1;
            }else if(levelDifficulty!=6 && levelDifficulty < receivedLevelDifficulty){
              levelDifficulty = receivedLevelDifficulty;
            }else if(levelDifficulty==6 && receivedLevelDifficulty==1){
              levelDifficulty=1;
            }
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
    if(!source){
      byte clicks=buttonClickCount();
      if(clicks==3){
        blinkType=WIN;
        signalState = RESOLVE;
        levelDifficulty=1;
      }
    }
  }
  
//------------------------------------
//   FLASHLIGHT AND LASER HANDLING
//------------------------------------

    if(buttonDoubleClicked()){
      if(isAlone()){
        source=true;
        weaponType=1;
        weaponHandling();
      }
    }

    //WHAT WEAPONS ARE AVAILABLE
     
    if(source==true){
      if(buttonPressed()){
          weaponType++;
          if(levelDifficulty<2 || levelDifficulty==4){
            if(weaponType>2){
               weaponType=1;
            }
          }else{
            if(weaponType>3){
               weaponType=1;
            }
          }
          
          weaponHandling();
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

  //GHOST AND GHOUL SPAWNING AND ALSO SOME POLTERGEISTS!
  if(blinkType==EMPTY){
    if(ghostWaitTimer.isExpired()){
      //check for neighborGhosts. dont spawn a ghost if neighbor is a ghosts 
      randomHaunting=random(100);
      ghoulOrGhost=(random(100));
        if(randomHaunting>=GHOST_GHOUL_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          if(55<ghoulOrGhost){
            blinkType=GHOUL;
            badBoiType=1;
            deadTimer.set(DEAD_TIME);
          }else{
            blinkType=GHOST;
            badBoiType=0;
            deadTimer.set(DEAD_TIME);
          }
            deadTimer.set(DEAD_TIME);
        }else if(randomHaunting<POLTER_SPAWN_CHANCE){
          blinkType=POLTER;
          deadTimer.set(DEAD_TIME);
          badBoiType=2;
        }
      
      ghostWaitTimer.set(GHOST_WAIT_TIME);
      //ghostWaitTimer.set(random(500)+RANDOM_GHOST_TIME);
    }
  }
    //BOSS SPAWNIN
    if(bossTimer.isExpired()){
      randomHaunting=random(100);
        if(randomHaunting>=BOSS_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          deadTimer.set(DEAD_TIME);
          blinkType=BOSS;
          badBoiType=3;
      
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
    }
  }

  if(blinkType==GHOST){
    if(isReceivingLight()){
      blinkType=EMPTY;
    }
  }

  if(blinkType==POLTER){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==GEISTGUN){
          blinkType=EMPTY;
        }
     }
    }
  }

  if(blinkType==GHOUL){
    if(isReceivingLaser()){
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
  }

  //GEIST GUN SENDING
  if(blinkType==GEISTGUN && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==GEISTGUN){
        blinkType=GEISTGUN;
      }else{
        blinkType=EMPTY;
      }
    }else{
      blinkType=EMPTY;
    }
  }

  //From empty to beam
  if(blinkType==EMPTY){
    FOREACH_FACE(f){
        if(!isValueReceivedOnFaceExpired(f)){
          byte beamType = getBlinkType(getLastValueReceivedOnFace(f));
          if(beamType == LIGHT || beamType == BEAM || beamType == GEISTGUN){
            receivingFace=f;
            blinkType=beamType;
          }
       }
     }
  }

  //IF I DONT KILL THE GHOSTS OR GHOULS FAST ENOUGH I DIE
  if(blinkType==GHOST || blinkType==GHOUL || blinkType==BOSS){
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//###############################################
//       LEVEL DIFFICULTY MODIFIERS ARE HERE
//###############################################
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void goLoop() {
  signalState = PLAY;
  
  bossTimer.set(BOSS_TIME);
  ghostWaitTimer.set(GHOST_WAIT_TIME); 
  
  switch(levelDifficulty){
    case 1: // just ghosts and ghouls... classic
      BOSS_SPAWN_CHANCE=101;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      POLTER_SPAWN_CHANCE=0;   
      break;
    case 2: //Ghosts ghouls and poltergeists.... oh boy
      BOSS_SPAWN_CHANCE=101;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      POLTER_SPAWN_CHANCE=15;    
      break;
    case 3:   // ghosts ghouls and poltergeists, but harder
      BOSS_SPAWN_CHANCE=101;
      GHOST_GHOUL_SPAWN_CHANCE=75;
      POLTER_SPAWN_CHANCE=17;    
      break;
    case 4: //og difficulty (ghosts ghouls, goblins)
      BOSS_SPAWN_CHANCE=95;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      POLTER_SPAWN_CHANCE=0;
      break;
   case 5:                       //oh boy...
      BOSS_SPAWN_CHANCE=95;
      GHOST_GHOUL_SPAWN_CHANCE=80;
      POLTER_SPAWN_CHANCE=15;  
      break;
   case 6:                      //good luck :)
      BOSS_SPAWN_CHANCE=90;
      GHOST_GHOUL_SPAWN_CHANCE=75;
      POLTER_SPAWN_CHANCE=20;  
      break;
  }

  if(!source){
    blinkType=EMPTY;
  }
  if(levelDifficulty!=6){
    gameTimer.set(SURVIVAL_TIME);
  }

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

void breath(){
  byte breathProgress = map(millis()%PERIOD,0,PERIOD,0,255);
  dimness = (sin8_C(breathProgress)/2)+127;
}

void lightDisplay(){
  setColor(makeColorHSB(lightHue,255,255));
}

void beamDisplay(){
  setColor(makeColorHSB(ghoulHue,random(50)+190,random(70)+70));
  setColorOnFace(makeColorHSB(ghoulHue,240,255),receivingFace);
  setColorOnFace(makeColorHSB(ghoulHue,240,255),(receivingFace+3)%6);
}

void geistGunDisplay(){
  setColor(makeColorHSB(geistHue,random(50)+190,random(65)+190));
}


void deadDisplay(){
  breath();
  setColor(makeColorHSB(15,random(70)+170,dimness));
}

void winDisplay(){
     breath();
     setColor(makeColorHSB(45,random(70)+170,dimness));
}


void badBoiDisplay(){
  breath();
  byte badFaces=map(deadTimer.getRemaining(),0,DEAD_TIME,0,5);
  setColor(makeColorHSB(badBoiHue[badBoiType],255,dimness));
  
  FOREACH_FACE(f){
    if(f>badFaces){
      setColorOnFace(makeColorHSB(badBoiHue[badBoiType],255,dimness-130),f);
    }
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
      blinkType=GEISTGUN;
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
  return (data>>2);
}
