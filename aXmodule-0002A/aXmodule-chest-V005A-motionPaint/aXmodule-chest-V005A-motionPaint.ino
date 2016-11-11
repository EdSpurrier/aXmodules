//////////////////////////////////////////////////////////////////////
//  DEBUGGING
//////////////////////////////////////////////////////////////////////
int wifiOff = false;

const int maxBrightness = 10;

//////////////////////////////////////////////////////////////////////
//  WIFI CONNECTION
//////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

const char* ssid = "Slayd";
const char* password = "sealord56";
boolean wifiConnected = false;

// UDP variables
unsigned int localPort = 8888;
WiFiUDP UDP;
boolean udpConnected = false;
byte packetBuffer[5]; //buffer to hold incoming packet,
int packetSize;

//  RANDOMIZED WIFI CONNECTION DELAY
int wifiDelayTime = random(300, 800);




//////////////////////////////////////////////////////////////////////
//  FILE SYSTEM
//////////////////////////////////////////////////////////////////////

bool filesLoaded = false;
#include "FS.h"
boolean fileSystemLoaded = false;
File workingFile;


//////////////////////////////////////////////////////////////////////
//  BUTTON
//////////////////////////////////////////////////////////////////////

//static const uint8_t buttonPin = 5;     //   THIS IS ACTUALLY D1
static const uint8_t button2Pin = 16;     //   THIS IS ACTUALLY D0

//   ONE SHOULDER BUILD OUT OF SYNC
static const uint8_t buttonPin = 12;     //   THIS IS ACTUALLY D6




// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
bool buttonDown = false;

// variables will change:
int button2State = 0;         // variable for reading the pushbutton status
bool button2Down = false;


// BUTTON ACTIONS
bool patternPause = false;


//////////////////////////////////////////////////////////////////////
//  SPI COMMUNICATION & FRAMES
//////////////////////////////////////////////////////////////////////
#include "SPI.h"

//This is 1 frame for 1 led consisting of 4 bytes: BRIGHTNESS,BLUE,GREEN,RED
byte brightness = 150;

//Confirm Connection Frames
byte confirmFlash[3] = {0,30,0};

//Confirm Connection Frames
byte startupFlash[3] = {30,0,0};

//Confirm Connection Frames
byte fileSysFlash[3] = {0,0,30};


//The start frame consists of 4 Bytes,each set to 0
byte startFrame[4] = {0,0,0,0};

void startFrames () {
  //send the start frame
  SPI.transfer(startFrame[0]);
  SPI.transfer(startFrame[1]);
  SPI.transfer(startFrame[2]);
  SPI.transfer(startFrame[3]); 
}

//The end frame consists of 4 bytes, each set to 255
byte endFrame[4] = {255,255,255,255};

void endFrames () {
  //send the end frame
  SPI.transfer(endFrame[0]);
  SPI.transfer(endFrame[1]);
  SPI.transfer(endFrame[2]);
  SPI.transfer(endFrame[3]);
}





//////////////////////////////////////////////////////////////////////
//  LED SETUP
//////////////////////////////////////////////////////////////////////
const int ledDirt = 5;
const int thisPartNoLEDs = 238;

//  PARTS
//  CHEST   = 238
//  LEG     = 158
//  SLEEVE  = 78
//  MASK V1 = ????
//  SHOULDER = 154


//  LED STRIPS
//  XL    = 42
//  L     = 37
//  M     = 26
//  S     = 20


const int noLEDs = (ledDirt + thisPartNoLEDs);

int red = 0;
int green = 0;
int blue = 0;

int currentLed = 0;
int currentLedData = 0;





//////////////////////////////////////////////////////////////////////
//  PATTERNS
//////////////////////////////////////////////////////////////////////

String patternFiles[2] = {
  "linesDown",
  "linesIn"
  };

int noMonoPatterns = 2;
int noColorPatterns = 0;

int noPatterns = (noMonoPatterns + noColorPatterns);


//  LINES DOWN 6 WIDE
const int monoPattern0frames = 35;
byte monoPattern0[monoPattern0frames][thisPartNoLEDs];

//  BARS ACROSS
const int monoPattern1frames = 16;
byte monoPattern1[monoPattern1frames][thisPartNoLEDs];

//  LINES DOWN CENTER FIRST
const int monoPattern2frames = 1;
byte monoPattern2[monoPattern2frames][thisPartNoLEDs];


//  LINES DOWN 6 WIDE
const int colorPattern0frames = 1;
byte colorPattern0[colorPattern0frames][(thisPartNoLEDs*3)];

//  BARS ACROSS
const int colorPattern1frames = 1;
byte colorPattern1[colorPattern1frames][(thisPartNoLEDs*3)];

//  LINES DOWN CENTER FIRST
const int colorPattern2frames = 1;
byte colorPattern2[colorPattern2frames][(thisPartNoLEDs*3)];





//  PATTERN LOOP VARIABLES
int currentPatternNo = 0;
int currentFrameNo = 0;
int currentPatternLoopNo = 0;
int currentPatternFramesNo = 0;

bool motionPaintActive = false;
int initPatternNo = 1;

//////////////////////////////////////////////////////////////////////
//  COLOUR CHANGER SETUP
//////////////////////////////////////////////////////////////////////

bool redUp = false;
bool greenUp = false;
bool blueUp = true;

bool redGo = false;
bool greenGo = false;
bool blueGo = true;

bool red2Go = false;
bool green2Go = false;
bool blue2Go = false;

//  COLOUR CHANGER
int currentColour = 1;
const int maxColour = 6;

bool isColorReset = false;

//////////////////////////////////////////////////////////////////////
//  SETUP
//////////////////////////////////////////////////////////////////////

void setup() {
  // initialize serial:
  Serial.begin(250000);

  // SPI START
  SPI.begin();
  // SET BIT ORDER - MSBFIRST or LSBFIRST 
  SPI.setBitOrder(MSBFIRST);
  /*
  you may need to change this depending on the quality of signal you are getting. it controlls the speed of the SPI transfer
  start at the fastest speed and move down untill it works. if it works at the top speed, leave it!
  
  The modes are 16mhz/DIVIDER so
  SPI_CLOCK_DIV2 = 8mhz
  SPI_CLOCK_DIV4 = 4mhz
  SPI_CLOCK_DIV8 = 2mhz
  SPI_CLOCK_DIV16 = 1mhz
  SPI_CLOCK_DIV32 = 500khz
  SPI_CLOCK_DIV128 = 125khhz
  
  */
  //SPI.setClockDivider(SPI_CLOCK_DIV8);

  // START BUTTONS
  pinMode(buttonPin, INPUT_PULLUP); // Set pin 12 as an input w/ pull-up
  pinMode(button2Pin, INPUT_PULLUP); // Set pin 13 as an input w/ pull-up


  ledFlash(startupFlash, 5);

  
  // Initialise wifi connection
  wifiConnected = connectWifi();
  
  // only proceed if wifi connection successful
  if(wifiConnected){
    udpConnected = connectUDP();
      if (udpConnected){
        
      }
  };


  if(wifiOff) {
    wifiConnected = false;
  }

}




//////////////////////////////////////////////////////////////////////
//  WIFI CONNECTOR
//////////////////////////////////////////////////////////////////////

void wifiConnector () {
  if (!wifiOff && !wifiConnected) {
  
    // Initialise wifi connection
    wifiConnected = connectWifi();
    
    // only proceed if wifi connection successful
    if(wifiConnected){
      udpConnected = connectUDP();
        if (udpConnected){
          
        };
    } else {
      delay(3000);
    };
    
  };
};





//////////////////////////////////////////////////////////////////////
//  LOAD FILES
//////////////////////////////////////////////////////////////////////

void loadFiles () {
  //  Initialize File System
  fileSystemLoaded = SPIFFS.begin();

  
  
  

  if (fileSystemLoaded) {
    ledFlash(fileSysFlash, 3);
    Serial.println("File system loaded");

    currentPatternNo = 0;
    while (currentPatternNo < noPatterns) {
     
      String fileName = "/";
      fileName += (String) currentPatternNo;
      fileName += ".txt";
      Serial.println(fileName);
  
      workingFile = SPIFFS.open(fileName, "r");
      if (!workingFile) {
          Serial.println("file open failed");
          ledFlash(fileSysFlash, 1);
      } else {
        Serial.println("file opened!!");
        
        String l_line = "";
  
        int frameLine = 0;
        int thisByte = 0;

        if(currentPatternNo < noMonoPatterns) {
          Serial.println("Mono Pattern");
        } else {
          Serial.println("Colour Pattern");
        };
        
        //open the file here
        while (workingFile.available() != 0) {  
            //A inconsistent line length may lead to heap memory fragmentation        
            l_line = workingFile.readStringUntil(',');        
            if (l_line == "") //no blank lines are anticipated        
              break;      
  
           int thisColor = l_line.toInt();
  
          if(currentPatternNo < noMonoPatterns) {
            if (thisByte >= thisPartNoLEDs) {
              frameLine++;
              thisByte = 0;
            };
  
            //  ADD TO PATTERNS
            if(currentPatternNo == 0) {
              monoPattern0[frameLine][thisByte] = thisColor;
            } else if(currentPatternNo == 1) {
              monoPattern1[frameLine][thisByte] = thisColor;
            } else if(currentPatternNo == 2) {
              monoPattern2[frameLine][thisByte] = thisColor;
            };
            
          } else {
            if (thisByte >= (thisPartNoLEDs*3)) {
              frameLine++;
              thisByte = 0;
            };

            //  ADD TO PATTERNS
            if((currentPatternNo-noMonoPatterns) == 0) {
              colorPattern0[frameLine][thisByte] = thisColor;
            } else if((currentPatternNo-noMonoPatterns) == 1) {
              colorPattern1[frameLine][thisByte] = thisColor;
            } else if((currentPatternNo-noMonoPatterns) == 2) {
              colorPattern2[frameLine][thisByte] = thisColor;
            };
          };
        
          


          
          thisByte++;
           delay(1);
        }; 
        //close the file here
  
  
        workingFile.close();
       
        String patternLineString = "Pattern Lines Loaded = ";
        patternLineString += frameLine;
        Serial.println(patternLineString);
        patternLineString = "This pattern internal ID = ";
        patternLineString += (currentPatternNo-noMonoPatterns);
        Serial.println(patternLineString);
        Serial.println("Pattern Loaded");
  
        loadingPatternsFlash(startupFlash, currentPatternNo);
        
        currentPatternNo++;
      } 
      

    };
  };
  
  ledFlash(fileSysFlash, 3);

  currentPatternNo = 1;
  
  filesLoaded = true;
}





//////////////////////////////////////////////////////////////////////
//  LOOP
//////////////////////////////////////////////////////////////////////

void loop() {
  //  CHECK IF CONNECTED OR RECONNECT
  wifiConnector();


  if(!filesLoaded) {
    loadFiles();
  }

  checkButtonState();
  
  // check if the WiFi and UDP connections were successful
  if(wifiConnected && filesLoaded){
    if(udpConnected) { 
      // if there’s data available, read a packet
      packetSize = UDP.parsePacket();
      if(packetSize) {
        IPAddress remote = UDP.remoteIP();
        //Serial.println(Udp.remotePort());
        // read the packet into packetBufffer
        //UDP.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
        UDP.read(packetBuffer, 5);
  
        // send a reply, to the IP address and port that sent us the packet we received
        //UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
        //UDP.write(aXmoduleName, 10);

        //  packetBuffer[0] = RED
        //  packetBuffer[1] = GREEN
        //  packetBuffer[2] = BLUE
        //  packetBuffer[3] = PATTERN NUMBER
        //  packetBuffer[4] = FRAME NUMBER

        //  RESET MOTIONPAINT
        motionPaintActive = false;
        
        /////////////////////////////////////////////////////////////////////////////////////
        //  PROGRAMS
        /////////////////////////////////////////////////////////////////////////////////////
        if(packetBuffer[3] == 0) {
            startFrames();
            allOn();
            endFrames();
        } else if (packetBuffer[3] == 1) {
          if (packetBuffer[4] < monoPattern0frames) {
            startFrames();
            runPattern(monoPattern0[packetBuffer[4]]);
            endFrames();
          } else if (packetBuffer[4] == 127) {
            currentPatternNo = 1;
            currentPatternFramesNo = monoPattern0frames;
            motionPaintActive = true;
          };;
        } else if (packetBuffer[3] == 2) {
         if (packetBuffer[4] < monoPattern1frames) {
            startFrames();
            runPattern(monoPattern1[packetBuffer[4]]);
            endFrames();
          } else if (packetBuffer[4] == 127) {
            currentPatternNo = 2;
            currentPatternFramesNo = monoPattern1frames;
            motionPaintActive = true;
          };;
        } else if (packetBuffer[3] == 3) {
         if (packetBuffer[4] < monoPattern2frames) {
            startFrames();
            runPattern(monoPattern2[packetBuffer[4]]);
            endFrames();
          } else if (packetBuffer[4] == 127) {
            currentPatternNo = 3;
            currentPatternFramesNo = monoPattern2frames;
            motionPaintActive = true;
          };;
        } else if (packetBuffer[3] == 4) {
         if (packetBuffer[4] < colorPattern0frames) {
            startFrames();
            runPatternColor(colorPattern0[packetBuffer[4]]);
            endFrames();
          } else if (packetBuffer[4] == 127) {
            currentPatternNo = 4;
            currentPatternFramesNo = colorPattern0frames;
            motionPaintActive = true;
          };
        } else if (packetBuffer[3] == 5) {
         if (packetBuffer[4] < colorPattern1frames) {
            startFrames();
            runPatternColor(colorPattern1[packetBuffer[4]]);
            endFrames();
          } else if (packetBuffer[4] == 127) {
            currentPatternNo = 5;
            currentPatternFramesNo = colorPattern1frames;
            motionPaintActive = true;
          };
        } else if (packetBuffer[3] == 6) {
         if (packetBuffer[4] < colorPattern2frames) {
            startFrames();
            runPatternColor(colorPattern2[packetBuffer[4]]);
            endFrames();
          } else if (packetBuffer[4] == 127) {
            currentPatternNo = 6;
            currentPatternFramesNo = colorPattern2frames;
            motionPaintActive = true;
          };
        };
        /////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////
        
      };
    };
  } else {

    
    //  NOT CONNECTED TO WIFI
    
    //  CURRENT NO PATTERNS

//        String feedback = "Running loop without Wifi | ";

//        feedback += "| Current Pattern No = ";        
//        feedback += currentPatternNo;
        
//        feedback += "| Current Frame No = ";        
//        feedback += currentFrameNo;
        if(!patternPause) {
          if (currentPatternNo == 0) {
              autoPattern(10, 10, 10, currentColour, true, false);
          } else if (currentPatternNo == 1) {
              autoPattern(monoPattern0frames, 25 , 5, currentColour, true, false);
          } else if (currentPatternNo == 2) {
              autoPattern(monoPattern1frames, 35, 5, currentColour, true, false);
          } else if (currentPatternNo == 3) {
              autoPattern(monoPattern2frames, 70, 5, currentColour, true, false);
          } else if (currentPatternNo == 4) {
              autoPattern(colorPattern0frames, 0, 10, currentColour, true, false);
          } else if (currentPatternNo == 5) {
              autoPattern(colorPattern1frames, 0, 10, currentColour, true, false);
          } else if (currentPatternNo == 6) {
              autoPattern(colorPattern2frames, 0, 10, currentColour, true, false);

              
          };
        };


//        Serial.println(feedback);
        
  };

  if (motionPaintActive) {
    autoPattern(currentPatternFramesNo, 0, 1, 0, false, false);
    Serial.println("go");
  };
  
};


/////////////////////////////////////////////////////////////////////////
//  AUTO PATTERNS
/////////////////////////////////////////////////////////////////////////

//  DELAY WAIT = ANIMATION SPEED
//  NO CYCLES = NUMBER OR TIMES THE PATTERN RUNS BEFORE PROGRESSING TO NEXT
//  COLOUR TYPE
//  //  0 = Colour Fade
//  //  1 = Red
//  //  2 = Green
//  //  3 = Blue
//  //  4 = Purple
//  //  5 = Violet
//  //  6 = Orange


void changeColour () {
  if(currentColour < maxColour) {
    currentColour++;
  } else {
    currentColour = 0;
  }
      
  isColorReset = true;

  String output = "colour changed to: ";
  output += currentColour;
  Serial.println(output);
}

void changePatternType() {

}

void autoPattern(int numberFrames, int delayWait, int noCycles, int colourType, bool patternChanger, bool reversible) {

  bool patternDirection = true;
  
  String thisFrameLoop = "";
  currentFrameNo = 0;
  
  
  while (currentFrameNo < numberFrames) {
        checkButtonState();

        if (patternChanger) {
          if (colourType == 0) {
            colourChanger (1);
          } else if (colourType == 1) {
            packetBuffer[0] = maxBrightness;
            packetBuffer[1] = 0;
            packetBuffer[2] = 0;
          } else if (colourType == 2) {
            packetBuffer[0] = 0;
            packetBuffer[1] = maxBrightness;
            packetBuffer[2] = 0;
          } else if (colourType == 3) {
            packetBuffer[0] = 0;
            packetBuffer[1] = 0;
            packetBuffer[2] = maxBrightness;
          } else if (colourType == 4) {
            packetBuffer[0] = 0;
            packetBuffer[1] = (int) maxBrightness/2;
            packetBuffer[2] = (int) maxBrightness/2;
          } else if (colourType == 5) {
            packetBuffer[0] = (int) maxBrightness/2;
            packetBuffer[1] = 0;
            packetBuffer[2] = (int) maxBrightness/2;
          } else if (colourType == 6) {
            packetBuffer[0] = (int) maxBrightness/2;
            packetBuffer[1] = (int) maxBrightness/2;
            packetBuffer[2] = 0;
          };
        };
        
        


        if (currentPatternNo == 0) {
            startFrames();
            allOn();
            endFrames();
        } else if (currentPatternNo == 1) {
            runPattern(monoPattern0[currentFrameNo]);
        } else if (currentPatternNo == 2) {
            runPattern(monoPattern1[currentFrameNo]);
        } else if (currentPatternNo == 3) {
            runPattern(monoPattern2[currentFrameNo]);

        } else if (currentPatternNo == 4) {
            runPatternColor(colorPattern0[currentFrameNo]);
        } else if (currentPatternNo == 5) {
            runPatternColor(colorPattern1[currentFrameNo]);
        } else if (currentPatternNo == 6) {
            runPatternColor(colorPattern2[currentFrameNo]);
        };


    
//    thisFrameLoop = "frameLoop Run = ";
    thisFrameLoop += currentFrameNo;
    


    currentFrameNo++;
    delay(delayWait);
  };
  
  if (patternChanger) {
    if (currentPatternLoopNo < noCycles) {
      currentPatternLoopNo++;
    } else {
      currentPatternLoopNo = 0;
      if (currentPatternNo < noPatterns) {
        currentPatternNo++;
      } else {
        currentPatternNo = initPatternNo;
      }
    };
  };
  
  
  
}






/////////////////////////////////////////////////////////////////////////
//  COLOUR FADER
/////////////////////////////////////////////////////////////////////////

void resetColors () {
    redUp = false;
    greenUp = false;
    blueUp = true;
    
    redGo = false;
    greenGo = false;
    blueGo = true;
    
    red2Go = false;
    green2Go = false;
    blue2Go = false;

    packetBuffer[0] = 0;
    packetBuffer[1] = 0;
    packetBuffer[2] = 0;
}

int maxValue = maxBrightness;


int halfMaxValue = maxValue / 2;

void colourChanger (int fadeIncrease) {

  if (isColorReset) {
    resetColors();
    isColorReset = false;
  }

  if(blueGo) {
    if (packetBuffer[2] < maxValue && blueUp || packetBuffer[2] == 0) {
      if (redGo) {
        blueGo = false;
      } else {
        packetBuffer[2] +=fadeIncrease;
        blueUp = true;
      };
    } else if (packetBuffer[2] == maxValue || packetBuffer[2] > maxValue) {
      blueUp = false;
      packetBuffer[2] -=fadeIncrease;
    } else if (packetBuffer[2] > 0 && !blueUp) {
      packetBuffer[2] -=fadeIncrease; 
      
      if(packetBuffer[2] < halfMaxValue) {
        redGo = true;
      };
    };

  };
  
  if(redGo) {
    if (packetBuffer[0] < maxValue && redUp || packetBuffer[0] == 0) {
      if (greenGo) {
        redGo = false;
      } else {
        packetBuffer[0] +=fadeIncrease;
        redUp = true;
      };
    } else if (packetBuffer[0] == maxValue || packetBuffer[0] > maxValue) {
      redUp = false;
      packetBuffer[0] -=fadeIncrease;
    } else if (packetBuffer[0] > 0 && !redUp) {
      packetBuffer[0] -=fadeIncrease;
      
      if(packetBuffer[0] < halfMaxValue) {
        greenGo = true;
      };
    };


    
  };
  
  if(greenGo) {
    if (packetBuffer[1] < maxValue && greenUp || packetBuffer[1] == 0) {
      if (blue2Go) {
        greenGo = false;
      } else {
        packetBuffer[1] +=fadeIncrease;
        greenUp = true;
      };
    } else if (packetBuffer[1] == maxValue || packetBuffer[1] > maxValue) {
      greenUp = false;
      packetBuffer[1] -=fadeIncrease;
    } else if (packetBuffer[1] > 0 && !greenUp) {
      packetBuffer[1] -=fadeIncrease;
      
      if(packetBuffer[1] < halfMaxValue) {
        blue2Go = true;
      };
    };
    
  };

  if(blue2Go) {
    if (packetBuffer[2] < maxValue && blueUp || packetBuffer[2] == 0) {
      if (green2Go) {
        blue2Go = false;
      } else {
        packetBuffer[2] +=fadeIncrease;
        blueUp = true;
      };
    } else if (packetBuffer[2] == maxValue || packetBuffer[2] > maxValue) {
      blueUp = false;
      packetBuffer[2] -=fadeIncrease;
    } else if (packetBuffer[2] > 0 && !blueUp) {
      packetBuffer[2] -=fadeIncrease; 
      
      if(packetBuffer[2] < halfMaxValue) {
        green2Go = true;
      };
    };
    
  };
  
  if(red2Go) {
    if (packetBuffer[0] < maxValue && redUp || packetBuffer[0] == 0) {
      if (blueGo) {
        red2Go = false;
      } else {
        packetBuffer[0] +=fadeIncrease;
        redUp = true;
      };
    } else if (packetBuffer[0] == maxValue || packetBuffer[0] > maxValue) {
      redUp = false;
      packetBuffer[0] -=fadeIncrease;
    } else if (packetBuffer[0] > 0 && !redUp) {
      packetBuffer[0] -=fadeIncrease;
      
      if(packetBuffer[0] < halfMaxValue) {
        blueGo = true;
      };
    };
  };
  
  if(green2Go) {
    if (packetBuffer[1] < maxValue && greenUp || packetBuffer[1] == 0) {
      if (red2Go) {
        green2Go = false;
      } else {
        packetBuffer[1] +=fadeIncrease;
        greenUp = true;
      };
    } else if (packetBuffer[1] == maxValue || packetBuffer[1] > maxValue) {
      greenUp = false;
      packetBuffer[1] -=fadeIncrease;
    } else if (packetBuffer[1] > 0 && !greenUp) {
      packetBuffer[1] -=fadeIncrease;
      
      if(packetBuffer[1] < halfMaxValue) {
        red2Go = true;
      };
    };
  };


    
  Serial.println(packetBuffer[0]);
  Serial.println(packetBuffer[1]);
  Serial.println(packetBuffer[2]);
}




/////////////////////////////////////////////////////////////////////////
//  CONFIRM FLASHES
/////////////////////////////////////////////////////////////////////////

void ledFlash (byte* flashBytes, int amount) {
  for(int i = 0; i < amount; i++) {
    startFrames();
    currentLed = 0;
    while (currentLed < noLEDs) {
      SPI.transfer(brightness);
      SPI.transfer(flashBytes[2]);  //  BLUE
      SPI.transfer(flashBytes[1]);  //  GREEN
      SPI.transfer(flashBytes[0]);  //  RED
      currentLed++;
    };
    endFrames();
    
    delay(50);
    
    startFrames();
    currentLed = 0;
    while (currentLed < noLEDs) {
      SPI.transfer(brightness);
      SPI.transfer(0);  //  BLUE
      SPI.transfer(0);  //  GREEN
      SPI.transfer(0);  //  RED
      currentLed++;
    };
    endFrames();
    
    delay(50);
  };
}


//////////////////////////////////////////////////////////////////////
//  LOADING PATTERNS FLASH
//////////////////////////////////////////////////////////////////////

void loadingPatternsFlash (byte* flashBytes, int fileNumber) {
    startFrames();
    currentLed = 0;
    
    while (currentLed < noLEDs) {
      
        if (currentLed < fileNumber) {
          SPI.transfer(brightness);
          SPI.transfer(flashBytes[2]);  //  BLUE
          SPI.transfer(flashBytes[1]);  //  GREEN
          SPI.transfer(flashBytes[0]);  //  RED
        } else {
          SPI.transfer(brightness);
          SPI.transfer(0);  //  BLUE
          SPI.transfer(0);  //  GREEN
          SPI.transfer(0);  //  RED
        };
        
        currentLed++;
    };
    endFrames();
}



//////////////////////////////////////////////////////////////////////
//  RUN PATTERN
//////////////////////////////////////////////////////////////////////

float thisBrightness;

void runPattern(byte* patternFrames) {
  currentLed = 0;
  currentLedData = 0;
  
  
  
  startFrames();
           while (currentLed < noLEDs) {   
            
            SPI.transfer(brightness); //  BRIGHTNESS

            thisBrightness = (float) (packetBuffer[2] / 127.0f) * patternFrames[currentLedData];

            SPI.transfer((int) thisBrightness); //  BLUE


            thisBrightness = (float) (packetBuffer[1] / 127.0f) * patternFrames[currentLedData];
            
            SPI.transfer((int) thisBrightness); //  GREEN


            thisBrightness = (float) (packetBuffer[0] / 127.0f) * patternFrames[currentLedData];

            SPI.transfer((int) thisBrightness); //  RED

            currentLedData++;
        
            currentLed++;
          };

  endFrames();
}





void runPatternColor(byte* patternFrames) {
  currentLed = 0;
  currentLedData = 0;



  startFrames();
           while (currentLed < noLEDs) {   
            
            SPI.transfer(brightness); //  BRIGHTNESS


            SPI.transfer((patternFrames[currentLedData]/2)); //  BLUE
            
            currentLedData++;

            
            SPI.transfer((patternFrames[currentLedData]/2)); //  GREEN
            
            currentLedData++;


            SPI.transfer((patternFrames[currentLedData]/2)); //  RED

            currentLedData++;
        
            currentLed++;
          };

  endFrames();
}




////////////////////////////////////////////////////////////////////////////////////////////
//  PROGRAMS
////////////////////////////////////////////////////////////////////////////////////////////


//  ALL ON
void allOn () {
  //  LEDS
  currentLed = 0;
  
  while (currentLed < noLEDs) {
    SPI.transfer(brightness);
    SPI.transfer(packetBuffer[2]);  //  BLUE
    SPI.transfer(packetBuffer[1]);  //  GREEN
    SPI.transfer(packetBuffer[0]);  //  RED
    currentLed++;
  };
}


////////////////////////////////////////////////////////////////////////////////////////////
//  BUTTON STATE CHECK
////////////////////////////////////////////////////////////////////////////////////////////


void checkButtonState () {

   buttonState = digitalRead(buttonPin);
  
    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (buttonState == HIGH) {     
      // since we're writing HIGH to the pin when, if it's HIGH , the button isn't pressed, as in there is no connectivity between ground and pin 2:
  //so do whatever here that you want when the button is not pushed    

      buttonDown = false;
    } 
    else if (!buttonDown) {
      // turn LED on, or do whatever else you want when your button is pushed


      //changeColour();
      if (patternPause) {
        patternPause = false;
      } else {
        patternPause = true;
      };

      Serial.println("button 2 pushed");
      buttonDown = true;
 
    }


   button2State = digitalRead(button2Pin);
  
    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (button2State == HIGH) {     
      // since we're writing HIGH to the pin when, if it's HIGH , the button isn't pressed, as in there is no connectivity between ground and pin 2:
  //so do whatever here that you want when the button is not pushed    

      button2Down = false;
    } 
    else if (!button2Down) {
      // turn LED on, or do whatever else you want when your button is pushed


      changeColour();

      Serial.println("button pushed");
      button2Down = true;
 
    }
}





/////////////////////////////////////////////////////////////////////////////////////
//  UDP
/////////////////////////////////////////////////////////////////////////////////////
  

// connect to UDP – returns true if successful or false if not
boolean connectUDP(){
  boolean state = false;
  
  //serial.println("");
  //serial.println("Connecting to UDP");
  
  if(UDP.begin(localPort) == 1){
    //serial.println("Connection successful");
    state = true;
  }
  else{
    //serial.println("Connection failed");
  }

  return state;
}


// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.begin(ssid, password);
  
  //serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(wifiDelayTime);
    //serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
    
  if (state){
      ledFlash(confirmFlash, 5);
    //serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    //serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}
 
