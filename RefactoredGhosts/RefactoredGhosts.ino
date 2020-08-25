
#define PALE makeColorHSB(200,60,60)
byte beamColors[3]={45,135,8};

//ABCDEF
enum state {SELECT,PLAY,RESULTS}; //AB
byte state=SELECT;
enum blinkType {BOARD,LIGHT,BEAM}; //CD
byte blinkType=BOARD;
byte beamType=1; //EF //light,geist,laser
byte levelDifficulty=1; //DEF in level select state
byte receivedLevelDifficulty;

byte sendData;
byte recievingFace;

//face data
byte faceData[6]={BOARD,BOARD,BOARD,BOARD,BOARD,BOARD};

void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty=1;
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
  
  if(state==SELECT){
     sendData= (state<<4) + levelDifficulty;
  }else{
     sendData= (state<<4) + (blinkType<<2) + beamType;
  }
  setValueSentOnAllFaces(sendData);

  //DISPLAYING
  switch(blinkType){
    case BEAM:
      beamsDisplay(beamType);
      break;
  }
  
}

void levelSelectLoop(){

  if(buttonMultiClicked()){
    byte clicks=buttonClickCount();
    if(clicks==3){
      state=PLAY;
    }
  }

  if(buttonDoubleClicked()){
    state=PLAY;
    blinkType=BEAM;
    beamType=1;
  }

  if(blinkType==BEAM){
    if(buttonPressed()){
      beamType++;
      if(beamType>3){
        beamType=1;
      }
    }
  }
  
  FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
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
  
  //LISTEN FOR OTHERS IN PLAY STAGE
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getState(getLastValueReceivedOnFace(f))==PLAY){
          state=PLAY;
        }
      }
    }
}

void playLoop(){
  
  if(blinkType==BOARD){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==BEAM){
          recievingFace=f;
          blinkType=BEAM;
          beamType=getBeamType(getLastValueReceivedOnFace(f));
        }
      }
    }
  }
}

void resultsLoop(){
  
}

void levelSelectDisplay(){
  setColor(PALE);
  FOREACH_FACE(f){
    if(f<levelDifficulty){
      setColorOnFace(WHITE,f);
    }
  }
}

void beamsDisplay(byte beamColor){
  setColor(makeColorHSB(beamColors[beamColor],240,255));
}

byte getBlinkType(byte data){
  return (data>>2 & 3);
}

byte getState(byte data){
  return (data>>4);
}

byte getBeamType(byte data){
  return (data & 3);
}

byte getLevelDifficulty(byte data){
  return (data&15);
}
