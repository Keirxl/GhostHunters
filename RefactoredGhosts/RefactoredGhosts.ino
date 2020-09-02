
#define PALE makeColorHSB(200,60,60)
#define DEAD_TIME 4500
#define GHOST_WAIT_TIME 2000
#define SURVIVAL_TIME 50000
#define BOSS_TIME 3000

byte beamColors[3] = {230, 8, 135};
byte beamSwitch = 1;
byte mobColors[4]={230,8,135,75};
byte mobType=0; //1=ghosts 2=ghouls 3=polters 4=goblins

//ABCDEF
enum state {SELECT, PLAY, RESULTS}; //AB
byte state;
enum blinkType {BOARD, BEAM_IN, BEAM_OUT}; //CD
byte faceBlinkType[6] = {BOARD, BOARD, BOARD, BOARD, BOARD, BOARD};
byte faceBeamType[6] = {0, 0, 0, 0, 0, 0}; //EF //light,geist,laser
byte levelDifficulty = 1; //DEF in level select state
byte receivedLevelDifficulty;
bool isBeam = false;
byte sendData;

byte randomHaunting;
byte ghoulOrGhost;
byte BOSS_SPAWN_CHANCE=101;   //95 seems good 
byte GHOST_GHOUL_SPAWN_CHANCE=80;  //80 seems good
byte POLTER_SPAWN_CHANCE=12;

Timer ghostWaitTimer;
Timer deadTimer;
Timer bossTimer;
Timer gameTimer;




void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty = 1;
  state = SELECT;
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case SELECT:
      levelSelectLoop();
      break;
    case PLAY:
      playLoop();
      break;
    case RESULTS:
      resultsLoop();
      break;
  }

  //SENDING DATA

  if (state == SELECT) {
    //{ab}cdef    //ab{cdef}
    sendData = (state << 4) + levelDifficulty;
    setValueSentOnAllFaces(sendData);
  } else {
    FOREACH_FACE(f) {
      //{ab}cdef    //ab{cd}ef             //abcd{ef}
      sendData = (state << 4) + (faceBlinkType[f] << 2) + (faceBeamType[f]);
      setValueSentOnFace(sendData, f);
    }
  }


  //DISPLAYING
  switch(state){
    case SELECT:
      levelSelectDisplay();
      break;
    case RESULTS:
      resultsDisplay();
      break;
    case PLAY:
      playDisplay();
      break;
  }
  
    
  
}

void levelSelectLoop() {

  if (buttonMultiClicked()) {
    byte clicks = buttonClickCount();
    if (clicks == 3) {
      ghostWaitTimer.set(GHOST_WAIT_TIME);
      bossTimer.set(BOSS_TIME);
      state = PLAY;
    }
  }

  if (buttonDoubleClicked()) {
    state = PLAY;
    isBeam = true;
    FOREACH_FACE(f) {
      faceBlinkType[f] = BEAM_OUT;
      faceBeamType[f] = 1;
    }
  }

  if (buttonSingleClicked()) {
    levelDifficulty++;
    if (levelDifficulty > 6) {
      levelDifficulty = 1;
    }
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      if (getLevelDifficulty(getLastValueReceivedOnFace(f)) != 0 && getLevelDifficulty(getLastValueReceivedOnFace(f)) < 7) {
        receivedLevelDifficulty = getLevelDifficulty(getLastValueReceivedOnFace(f));
        if (levelDifficulty == 1 && receivedLevelDifficulty == 6) {
          levelDifficulty = 1;
        } else if (levelDifficulty != 6 && levelDifficulty < receivedLevelDifficulty) {
          levelDifficulty = receivedLevelDifficulty;
        } else if (levelDifficulty == 6 && receivedLevelDifficulty == 1) {
          levelDifficulty = 1;
        }
      }
    }
  }

  //LISTEN FOR OTHERS IN PLAY STAGE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      if (getState(getLastValueReceivedOnFace(f)) == PLAY) {
        ghostWaitTimer.set(GHOST_WAIT_TIME);
        bossTimer.set(BOSS_TIME);
        state = PLAY;
      }
    }
  }
}

void playLoop() {

  if (isBeam) {//flashlight behavior
    if (buttonSingleClicked()) {
      beamSwitch++; //could be changed to save space
      if (beamSwitch > 2) {
        beamSwitch = 0;
      }
      FOREACH_FACE(f) {
        faceBeamType[f] = beamSwitch;
      }
    }
  }else {//board behavior
    if (buttonDoubleClicked()) {
      state = PLAY;
      isBeam = true;
      FOREACH_FACE(f) {
        faceBlinkType[f] = BEAM_OUT;
        faceBeamType[f] = 1;
      }
    }

      //Mob Spawning
  if(isAllBoard){
    if(ghostWaitTimer.isExpired()){
      randomHaunting=random(100);
      ghoulOrGhost=(random(100));
        if(randomHaunting>=GHOST_GHOUL_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          if(55<ghoulOrGhost){
            mobType=2;
          }else{
            mobType=1;
          }
            deadTimer.set(DEAD_TIME);
        }else if(randomHaunting<POLTER_SPAWN_CHANCE){
          mobType=3;
          deadTimer.set(DEAD_TIME);
        }
      ghostWaitTimer.set(GHOST_WAIT_TIME);
    }
     if(bossTimer.isExpired()){
      randomHaunting=random(100);
        if(randomHaunting>=BOSS_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          deadTimer.set(DEAD_TIME);
          mobType=4;
          bossTimer.set(BOSS_TIME);
        }
      }
  }
  //mob killing
  if(mobType!=0){
    FOREACH_FACE(f){
      if (!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))== BEAM_OUT){
          byte recievedLaserType=getBeamType(getLastValueReceivedOnFace(f));
          if((recievedLaserType+1)==mobType){
            mobType=0;
          }
        }
      }
    }
  }

    FOREACH_FACE(f) {
      switch (faceBlinkType[f]) {
        case BOARD:
          boardLoop(f);
          break;
        case BEAM_IN:
          beamInLoop(f);
          break;
      }
    }
  }


  //HERE YOU DO THE LOOPS TO CHANGE TO RESULTS STATE

  //LISTEN FOR OTHERS IN SELECT STAGE
}

void boardLoop(byte face) {
  if(isAllBoard()){
    if (!isValueReceivedOnFaceExpired(face)) {//neighbor!
      byte neighborData = getLastValueReceivedOnFace(face);
      if (getBlinkType(neighborData) == BEAM_OUT) {
  
        faceBlinkType[face] = BEAM_IN;
        faceBeamType[face] = getBeamType(neighborData);
  
        faceBlinkType[(face + 3) % 6] = BEAM_OUT;
        faceBeamType[(face + 3) % 6] = getBeamType(neighborData);
      }
    }
  }
}

void beamInLoop(byte face) {
  if (!isValueReceivedOnFaceExpired(face)) {//neighbor!
    byte neighborData = getLastValueReceivedOnFace(face);
    if (getBlinkType(neighborData) != BEAM_OUT) {//neighbor not sending beam
      faceBlinkType[face] = BOARD;
      faceBlinkType[(face + 3) % 6] = BOARD;
    } else if (getBlinkType(neighborData) == BEAM_OUT) {//neighbor still sending beam
      faceBeamType[face] = getBeamType(neighborData);
      faceBeamType[(face + 3) % 6] = getBeamType(neighborData);
    }
  } else {//no neighbor
    faceBlinkType[face] = BOARD;
    faceBlinkType[(face + 3) % 6] = BOARD;
  }
}

void resultsLoop() {

}

//DISPLAYS
void playDisplay(){
  if(mobType==0){
    FOREACH_FACE(f) {
        if(faceBlinkType[f]==BOARD){
          setColorOnFace(PALE,f);
        }else{
          setColorOnFace(makeColorHSB(beamColors[faceBeamType[f]], 255, 255), f);
        }
     }
  }else{
    byte badFaces=map(deadTimer.getRemaining(),0,DEAD_TIME,0,5);
    setColor(makeColorHSB(mobColors[mobType-1],255,255));
    FOREACH_FACE(f){
      if(f>badFaces){
        setColorOnFace(makeColorHSB(mobColors[mobType-1],255,125),f);
      }
    }
  }
}
void levelSelectDisplay() {
  setColor(PALE);
  FOREACH_FACE(f) {
    if (f < levelDifficulty) {
      setColorOnFace(WHITE, f);
    }
  }
}

void resultsDisplay() {
  setColor(BLUE);
}

//makes sure beams don't cross and mobs don't spawn in beams
bool isAllBoard(){
  byte check=0;
  FOREACH_FACE(f){
    if(faceBlinkType[f]==BEAM_IN){
      check++;
    }
  }
  if(check>0){
    return false;
  }else{
    return true;
  }
}



//{ab}cdef    //ab{cd}ef             //abcd{ef}
//state       blinkType             beamType

byte getBlinkType(byte data) {
  return (data >> 2 & 3);
}

byte getState(byte data) {
  return (data >> 4);
}

byte getBeamType(byte data) {
  return (data & 3);
}

byte getLevelDifficulty(byte data) {
  return (data & 7);
}
