//GhostHunters
// Try to survive with a friend as your mansion is haunted
// by Ghosts, Ghouls, and Goblins



#define PALE makeColorHSB(200,60,60)
#define lightHue 45
#define laserHue 135
#define bossHue 75
#define ghoulHue 8

// 100-these gives you the chance of spawn
#define BOSS_SPAWN_CHANCE 95   //95 seems good 
#define GHOST_GHOUL_SPAWN_CHANCE 80  //80 seems good

byte RANDOM_BOSS_TIME=3000;
byte RANDOM_GHOST_TIME=2500;
#define BOSS_TIME 3500 //time before boss tries to spawn
#define GHOST_WAIT_TIME 3000 //time before ghosts and ghouls try to spawn

#define SURVIVAL_TIME 60000 //one minute
#define GHOST_FADE_TIME 100 //time of breath() function
#define BOSS_DEAD_TIME 4000 //time beofre boss kills ya
#define DEAD_TIME 3000 //time before ghosts or ghouls kill ya

// A B C D E F
enum blinkType {EMPTY,GHOST,GHOUL,LIGHT,DEAD,WIN,BEAM,BOSS};
byte blinkType=WIN;
enum signalState {INERT,GO,RESOLVE};
byte signalState=INERT;
bool source=false;

byte sendData = (blinkType<<2) + signalState;

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
      default:
        setColor(BLUE);
        break;
    }
  
  //if(gameTimer.getRemaining()<=SURVIVAL_TIME/2){
    //RANDOM_GHOST_TIME -= 800;
   // RANDOM_BOSS_TIME -= 800;
  //}

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

void inertLoop() {
  
  //set myself to GO
  if (buttonMultiClicked()) {
    byte clicks=buttonClickCount();
    if(clicks==3){
      blinkType=EMPTY;
      signalState = GO;
    }
  }

//FLASHLIGHT AND LASER HANDLING
    if(buttonLongPressed()){
      if(isAlone()){
        source=true;
        blinkType=LIGHT;
      }
    }
    if(blinkType==LIGHT && source==true){
      if(buttonDoubleClicked()){
        if(isAlone()){
          blinkType=BEAM;
        }
      }
    }
    if(blinkType==BEAM && source==true){
      if(buttonDoubleClicked()){
        if(isAlone()){
          blinkType=LIGHT;
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
  source=false;
  ghostWaitTimer.set(GHOST_WAIT_TIME);
  bossTimer.set(BOSS_TIME);
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



byte getBlinkType(byte data){
  return (data>>2);
}

byte getSignalState(byte data){
  return (data & 3);
}
