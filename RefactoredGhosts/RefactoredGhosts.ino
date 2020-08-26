//l
#define PALE makeColorHSB(200,60,60)
byte beamColors[3]={45,135,8};
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




void setup() {
  // put your setup code here, to run once:
  randomize();
  levelDifficulty=1;
  state=PLAY;
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
                //{ab}cdef    //ab{cdef}
     sendData = (state<<4) + levelDifficulty;
     setValueSentOnAllFaces(sendData);
  }else{
    FOREACH_FACE(f){
                 //{ab}cdef    //ab{cd}ef             //abcd{ef}
      sendData = (state<<4) + (faceBlinkType[f]<<2)+(faceBeamType[f]);
      setValueSentOnFace(sendData,f);
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
      }else if(faceBlinkType[f]==LIGHT){
        beamsDisplay(faceBeamType[f]);
      }
    }
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
    isBeam=true;
    FOREACH_FACE(f){
      faceBlinkType[f]=BEAM;
      faceBeamType[f]=1;
    }
  }

  if(buttonSingleClicked()){
    levelDifficulty++;
    if(levelDifficulty>6){
      levelDifficulty=1;
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

  if(buttonDoubleClicked()){
    state=PLAY;
    isBeam=!isBeam;
    FOREACH_FACE(f){
      faceBlinkType[f]=LIGHT;
      faceBeamType[f]=1;
    }
  }

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
          if(getBlinkType(getLastValueReceivedOnFace(f))==LIGHT){
            faceBlinkType[f]=BEAM;
            faceBlinkType[(f+3)%6]=BEAM;
            faceBeamType[f]=getBeamType(getLastValueReceivedOnFace(f));
            faceBeamType[(f+3)%6]=getBeamType(getLastValueReceivedOnFace(f));
          }
        }
      }else if(faceBlinkType[f]==BEAM){
        if(getBlinkType(getLastValueReceivedOnFace(f))==BEAM || getBlinkType(getLastValueReceivedOnFace(f))==LIGHT){
          faceBlinkType[f]=BEAM;
        }else{
          faceBlinkType[f]=BOARD;
        }
      }
  }

  //LISTEN FOR OTHERS IN SELECT STAGE
  /*
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
    */
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

void resultsDisplay(){
  setColor(BLUE);
}

void beamsDisplay(byte beamColor){
  setColor(makeColorHSB(beamColors[beamColor],240,255));
}


//{ab}cdef    //ab{cd}ef             //abcd{ef}
//state       blinkType             beamType

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
  return (data&7);
}
