//l
#define PALE makeColorHSB(200,60,60)
byte beamColors[3]={45,135,8};
<<<<<<< Updated upstream

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
=======
byte beamSwitch=1;

//ABCDEF
enum state {SELECT,PLAY,RESULTS}; //AB
byte state;
enum blinkType {BOARD,LIGHT,BEAM}; //CD
byte faceBlinkType[6]={BOARD,BOARD,BOARD,BOARD,BOARD,BOARD};
byte faceBeamType[6]={0,0,0,0,0,0}; //EF //light,geist,laser
byte levelDifficulty=1; //DEF in level select state
byte receivedLevelDifficulty;

bool isBeam=false;

byte sendData;
byte recievingFace;



>>>>>>> Stashed changes

void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty=1;
<<<<<<< Updated upstream
=======
  state=SELECT;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
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
=======
     sendData = (state<<4) + levelDifficulty;
     setValueSentOnAllFaces(sendData);
  }else{
    FOREACH_FACE(f){
      sendData = (state<<4) + (faceBlinkType[f]<<2)+(faceBeamType[f]);
      setValueSentOnFace(f,sendData);
    }
  }
  

  //DISPLAYING
  if(state==SELECT){
    levelSelectDisplay();
  }else if(state==RESULTS){
    resultsDisplay();
  }else{
    FOREACH_FACE(f){
      if(faceBlinkType[f]==BOARD){
        setColor(PALE);
      }else if(faceBlinkType[f]==BEAM){
        beamsDisplay(faceBeamType[f]);
      }
    }
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    blinkType=BEAM;
    beamType=1;
  }

  if(blinkType==BEAM){
    if(buttonPressed()){
      beamType++;
      if(beamType>3){
        beamType=1;
      }
=======
    isBeam=!isBeam;
    FOREACH_FACE(f){
      faceBlinkType[f]=BEAM;
      faceBeamType[f]=1;
    }
  }

  if(buttonSingleClicked()){
    levelDifficulty++;
    if(levelDifficulty>6){
      levelDifficulty=1;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
  
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
=======

  if(isBeam){
    if(buttonSingleClicked()){
      beamSwitch++; //could be changed to save space
      if(beamSwitch>2){
        beamSwitch=0;
      }
      FOREACH_FACE(f){
        faceBeamType[f]=beamSwitch;
      }
    }
  }

  if(buttonMultiClicked()){
    byte clicks=buttonClickCount();
    if(clicks==3){
      state=SELECT;
      FOREACH_FACE(f){
        faceBlinkType[f]=BOARD;
        faceBeamType[f]=0;
      }
    }
  }
  
    FOREACH_FACE(f){
      if(faceBlinkType[f]==BOARD){
        if(!isValueReceivedOnFaceExpired(f)){
          if(getBlinkType(getLastValueReceivedOnFace(f))==BEAM){
            faceBlinkType[f]=BEAM;
            faceBlinkType[(f+3)%6]=BEAM;
            faceBeamType[f]=getBeamType(getLastValueReceivedOnFace(f));
            faceBeamType[(f+3)%6]=getBeamType(getLastValueReceivedOnFace(f));
          }
        }
      }
  }

  //LISTEN FOR OTHERS IN SELECT STAGE
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getState(getLastValueReceivedOnFace(f))==PLAY){
          state=SELECT;
          FOREACH_FACE(f){
            faceBlinkType[f]=BOARD;
            faceBeamType[f]=0;
            isBeam=false;
          }
        }
      }
    }
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
=======
void resultsDisplay(){
  setColor(BLUE);
}

>>>>>>> Stashed changes
void beamsDisplay(byte beamColor){
  setColor(makeColorHSB(beamColors[beamColor],240,255));
}

<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
  return (data&15);
=======
  return (data&7);
>>>>>>> Stashed changes
}
