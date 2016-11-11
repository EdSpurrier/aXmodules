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
const int thisPartNoLEDs = 78;

//  PARTS
//  CHEST   = 236
//  LEG     = 158
//  SLEEVE  = 78
//  MASK V1 = ????


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

int currentPatternNo = 0;
int currentFrameNo = 0;

int noPatterns = 10;

//  LINES DOWN - 5 FRAMES
byte newPattern0[5][thisPartNoLEDs];

//  LINES IN - 10 FRAMES
byte newPattern1[10][thisPartNoLEDs];




//////////////////////////////////////////////////////////////////////
//  SETUP
//////////////////////////////////////////////////////////////////////

void setup() {
  // initialize serial:
  Serial.begin(250000);
  Serial.println("Starting up");

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


  ledFlash(startupFlash, 5);

  
  // Initialise wifi connection
  wifiConnected = connectWifi();
  
  // only proceed if wifi connection successful
  if(wifiConnected){
    udpConnected = connectUDP();
      if (udpConnected){
        
      }
  };

}






//////////////////////////////////////////////////////////////////////
//  LOAD FILES
//////////////////////////////////////////////////////////////////////

void loadFiles () {
  //  Initialize File System
  fileSystemLoaded = SPIFFS.begin();

  
  int thisByte = 0;
  

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
      
      //open the file here
      while (workingFile.available() != 0) {  
          //A inconsistent line length may lead to heap memory fragmentation        
          l_line = workingFile.readStringUntil(',');        
          if (l_line == "") //no blank lines are anticipated        
            break;      

         int thisColor = l_line.toInt();


        if (thisByte >= 78) {
          frameLine++;
          thisByte = 0;
          
        };
        newPattern0[frameLine][thisByte] = thisColor;
        thisByte++;
         delay(1);
      } 
      //close the file here


      workingFile.close();
     
      String patternLineString = "Pattern Lines Loaded = ";
      patternLineString += frameLine;
      Serial.println(patternLineString);
      Serial.println("Pattern Loaded");

      loadingPatternsFlash(startupFlash, currentPatternNo);
      
      currentPatternNo++;
    } 
    
        
    };
  };
  
  ledFlash(fileSysFlash, 3);

  currentPatternNo = 0;
  filesLoaded = true;
}





//////////////////////////////////////////////////////////////////////
//  LOOP
//////////////////////////////////////////////////////////////////////

void loop() {

  if(!filesLoaded) {
    loadFiles();
  }




  
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



        /////////////////////////////////////////////////////////////////////////////////////
        //  PROGRAMS
        /////////////////////////////////////////////////////////////////////////////////////
        if(packetBuffer[3] == 0) {
            startFrames();
            allOn();
            endFrames();
        } else if (packetBuffer[3] == 1) {
        		if (packetBuffer[4] < 3) {
        		  startFrames();
              runPattern(packetBuffer[4]);
              endFrames();
        		};
        } else if (packetBuffer[3] == 2) {
           startFrames();
            allOn();
            endFrames();
        };
        /////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////
        
      };
    };
  } else {
    //  NOT CONNECTED TO WIFI

        String feedback = "Running loop without Wifi | ";

        feedback += "| Current Pattern No = ";        
        feedback += currentPatternNo;
        
        feedback += "| Current Frame No = ";        
        feedback += currentFrameNo;
        
        if (currentPatternNo == 0) {
            startFrames();
            allOn();
            endFrames();
            feedback += "| Pattern Run!";
            delay(10);
        } else if (packetBuffer[3] == 1) {
              startFrames();
              runPattern(currentFrameNo);
              endFrames();
              feedback += "| Pattern Run!";
              delay(100);
        };

        
        if (currentFrameNo < 5) {
          currentFrameNo++;
        } else {
          currentFrameNo = 0;
        };

        Serial.println(feedback);
        colourChanger ();
        
        
  };

  
};




/////////////////////////////////////////////////////////////////////////
//  RUN PATTERNS
/////////////////////////////////////////////////////////////////////////
bool redUp = false;
bool greenUp = false;
bool blueUp = true;

bool redGo = false;
bool greenGo = false;
bool blueGo = true;

bool red2Go = false;
bool green2Go = false;
bool blue2Go = false;

int maxValue = 50;
int fadeIncrease = 1;

int halfMaxValue = maxValue / 2;

void colourChanger () {


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

void runPattern(int frameNo) {
  currentLed = 0;
  currentLedData = 0;

  startFrames();
           while (currentLed < noLEDs) {   
            
            SPI.transfer(brightness); //  BRIGHTNESS

            thisBrightness = (float) (packetBuffer[2] / 127.0f) * newPattern0[frameNo][currentLedData];

            SPI.transfer((int) thisBrightness); //  BLUE


            thisBrightness = (float) (packetBuffer[1] / 127.0f) * newPattern0[frameNo][currentLedData];
            
            SPI.transfer((int) thisBrightness); //  GREEN


            thisBrightness = (float) (packetBuffer[0] / 127.0f) * newPattern0[frameNo][currentLedData];

            SPI.transfer((int) thisBrightness); //  RED

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
  //serial.println("Connecting to WiFi");
  
  // Wait for connection
  //serial.print("Connecting");
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
    //serial.print("Connected to ");
    //serial.println(ssid);
    //serial.print("IP address: ");
    //serial.println(WiFi.localIP());
  }
  else {
    //serial.println("");
    //serial.println("Connection failed.");

    packetBuffer[2] = 50;
    packetBuffer[1] = 0;
    packetBuffer[0] = 0;
    
  }
  return state;
}
 
