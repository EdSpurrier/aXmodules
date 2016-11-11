#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

//  FILE SYSTEM
bool filesLoaded = false;
#include "FS.h"
boolean fileSystemLoaded = false;
File myFile;

byte aXmoduleName[10] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 1};

// wifi connection variables
const char* ssid = "Slayd";
const char* password = "sealord56";
boolean wifiConnected = false;

// UDP variables
unsigned int localPort = 8888;
WiFiUDP UDP;
boolean udpConnected = false;
byte packetBuffer[5]; //buffer to hold incoming packet,

int wifiDelayTime = random(300, 800);


//  SPI COMMUNICATION
#include "SPI.h"

//The start frame consists of 4 Bytes,each set to 0
byte startFrame[4] = {0,0,0,0};
//This is 1 frame for 1 led consisting of 4 bytes: BRIGHTNESS,BLUE,GREEN,RED

byte brightness = 150;

//The end frame consists of 4 bytes, each set to 255

byte endFrame[4] = {255,255,255,255};

//Confirm Connection Frames
byte confirmFlash[3] = {0,255,0};



//Confirm Connection Frames
byte startupFlash[3] = {255,0,0};

const int ledDirt = 5;
const int thisPartNoLEDs = 236;


//  PARTS
//  CHEST   = 236
//  LEG     = 158
//  SLEEVE  = 78
//  MASK V1 = ????


//  LED STRIPS
//  XL    = 42
//  L     = 37
//  M     = 26
//  S     = 


const int noLEDs = 78;

byte red = 0;
byte green = 0;
byte blue = 0;

int currentLed = 0;
int currentLedData = 0;

int packetSize;


//  PATTERNS
byte newPattern0[5][78];
void setup() {
  // Initialise //serial connection
  //Serial.begin(115200);
 

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







  //  START UP FLASH
  for(int i = 0; i < 10; i++) {
    startFrames();
    currentLed = 0;
    while (currentLed < noLEDs) {
      SPI.transfer(brightness);
      SPI.transfer(startupFlash[2]);  //  BLUE
      SPI.transfer(startupFlash[1]);  //  GREEN
      SPI.transfer(startupFlash[0]);  //  RED
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
  

  
  
  
  // Initialise wifi connection
  wifiConnected = connectWifi();
  
  // only proceed if wifi connection successful
  if(wifiConnected){
    udpConnected = connectUDP();
      if (udpConnected){
        
      }
  };



  // initialize serial:
  Serial.begin(9600);

  //  Initialize File System
  fileSystemLoaded = SPIFFS.begin();

   
}


void loadFiles () {
  
 
  int thisByte = 0;

  if (fileSystemLoaded) {
    fileSystemConfirm();
    Serial.println("File system loaded");
    
    myFile = SPIFFS.open("/test.txt", "r");
    if (!myFile) {
        Serial.println("file open failed");
    } else {
      Serial.println("file opened!!");
      
      String l_line = "";

      int frameLine = 0;
      
      //open the file here
      while (myFile.available() != 0) {  
          //A inconsistent line length may lead to heap memory fragmentation        
          l_line = myFile.readStringUntil(',');        
          if (l_line == "") //no blank lines are anticipated        
            break;      
          //  
          Serial.println(l_line);
          
         //parse l_line here
        // if (thisByte < 3) {
          
          
         //};

        
        
          int thisColor = l_line.toInt();
          Serial.println("adding byte: ");
          Serial.println(thisColor);
          Serial.println("byte id: ");
          Serial.println(thisByte);

        if (thisByte >= 78) {

          currentLed = 0;
          currentLedData = 0;
         startFrames();
          while (currentLed < noLEDs) {   
            
            SPI.transfer(brightness); //  BRIGHTNESS
            SPI.transfer(newPattern0[frameLine][currentLedData]); //  BLUE
            currentLedData++;
        
            SPI.transfer(0); //  GREEN

        
            SPI.transfer(0); //  RED

        
            currentLed++;
          };
    
        endFrames();


      
        Serial.println("sending color");
          
          frameLine++;
          thisByte = 0; 
          Serial.println("new LINE _________________________________________________________");
        };
        newPattern0[frameLine][thisByte] = thisColor;
        thisByte++;
         delay(1);
      } 
      //close the file here




        
      myFile.close();

        
    };
  };

  filesLoaded = true;
}



void startFrames () {
  //send the start frame
  SPI.transfer(startFrame[0]);
  SPI.transfer(startFrame[1]);
  SPI.transfer(startFrame[2]);
  SPI.transfer(startFrame[3]); 
}

void endFrames () {
  //send the end frame
  SPI.transfer(endFrame[0]);
  SPI.transfer(endFrame[1]);
  SPI.transfer(endFrame[2]);
  SPI.transfer(endFrame[3]);
}


//  CONFIRM FILESYSTEM
void fileSystemConfirm() {
  for(int i = 0; i < 5; i++) {
    startFrames();
    currentLed = 0;
    while (currentLed < noLEDs) {
      SPI.transfer(brightness);
      SPI.transfer(255);  //  BLUE
      SPI.transfer(0);  //  GREEN
      SPI.transfer(0);  //  RED
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



void connectConfirm() {
  for(int i = 0; i < 10; i++) {
    startFrames();
    currentLed = 0;
    while (currentLed < noLEDs) {
      SPI.transfer(brightness);
      SPI.transfer(confirmFlash[2]);  //  BLUE
      SPI.transfer(confirmFlash[1]);  //  GREEN
      SPI.transfer(confirmFlash[0]);  //  RED
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

void sendPatternLine (byte* newLine) {
    
    SPI.transfer(brightness); //  BRIGHTNESS
    
    SPI.transfer(newLine[currentLedData]); //  BLUE
    currentLedData++;


    SPI.transfer(newLine[currentLedData]); //  GREEN
    currentLedData++;

    SPI.transfer(newLine[currentLedData]); //  RED
    currentLedData++;

    currentLed++;
 }

void loop() {

  if(!filesLoaded) {
    loadFiles();
  }
  
  // check if the WiFi and UDP connections were successful
  if(wifiConnected){
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

        //packetBuffer[3] = packetBuffer[3]/2;
        //packetBuffer[4] = packetBuffer[4]/2;
        
        //  PROGRAMS
        if(packetBuffer[3] == 0) {
            startFrames();
            allOn();
            endFrames();
        } else if (packetBuffer[3] == 1) {
        		startFrames();
        		linesDown();
        		endFrames();
        } else if (packetBuffer[3] == 2) {
            if (packetBuffer[4] < 3) {
              startFrames();
              newPattern0Go();
              endFrames();
            };
        };
      };
    };
  };
};


//
//  26 LEDS PER LINE
//  
//




//  RUN FUNCTION
void newPattern0Go() {
  currentLed = 0;
  currentLedData = 0;

  while (currentLed < noLEDs) {   
    
    SPI.transfer(brightness); //  BRIGHTNESS
    SPI.transfer(newPattern0[packetBuffer[4]][currentLedData]); //  BLUE
    currentLedData++;

    SPI.transfer(newPattern0[packetBuffer[4]][currentLedData]); //  GREEN
    currentLedData++;

    SPI.transfer(newPattern0[packetBuffer[4]][currentLedData]); //  RED
    currentLedData++;

    currentLed++;
  };

}


int linesDownFrameCut = 28;
int linesDownNoFrames = 255/linesDownFrameCut;

//  LINES DOWN PATTERN VARIABLES
byte linesDownPattern[5] = {15, 80, 255, 80, 15};
int linesDownPatternLength = 4;

byte thisBrightness = 0;

int striplength = 26;

//  PATTERN = LINES DOWN
void linesDown() {
  currentLed = 0;

  int currentPatternPosition = 0;

//48
  
  int currentPatternRow01Position = 0;
  int currentPatternRow02Position = 0;
  
  while (currentLed < noLEDs) {

    if (currentLed < striplength) {
      if( currentLed >= (packetBuffer[4]-linesDownPatternLength) && currentLed < (packetBuffer[4]) ) {
        SPI.transfer(brightness);
        SPI.transfer(packetBuffer[2]);  //  BLUE
        SPI.transfer(packetBuffer[1]);  //  GREEN
        SPI.transfer(packetBuffer[0]);  //  RED      
        currentPatternPosition++;
      
      } else {
        SPI.transfer(brightness);
        SPI.transfer(0);  //  BLUE
        SPI.transfer(0);  //  GREEN
        SPI.transfer(0);  //  RED
      };

    } else if (currentLed < (striplength*2)) {
      if( currentLed >= ((striplength*2)-(packetBuffer[4])) && currentLed < ((striplength*2)-(packetBuffer[4]-linesDownPatternLength)) ) {
        SPI.transfer(brightness);
        SPI.transfer(packetBuffer[2]);  //  BLUE
        SPI.transfer(packetBuffer[1]);  //  GREEN
        SPI.transfer(packetBuffer[0]);  //  RED      
        currentPatternPosition++;

      } else {
        SPI.transfer(brightness);
        SPI.transfer(0);  //  BLUE
        SPI.transfer(0);  //  GREEN
        SPI.transfer(0);  //  RED
      };
    } else if (currentLed < (striplength*3)) {
      if( currentLed >= (striplength*2) + (packetBuffer[4]-linesDownPatternLength) && currentLed < (striplength*2) + (packetBuffer[4]) ) {
        SPI.transfer(brightness);
        SPI.transfer(packetBuffer[2]);  //  BLUE
        SPI.transfer(packetBuffer[1]);  //  GREEN
        SPI.transfer(packetBuffer[0]);  //  RED      
        currentPatternPosition++;
  
      } else {
        SPI.transfer(brightness);
        SPI.transfer(0);  //  BLUE
        SPI.transfer(0);  //  GREEN
        SPI.transfer(0);  //  RED
      };
    };
    
    
    currentLed++; 
  };
};



//  PROGRAMS

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
    connectConfirm();
    //serial.println("");
    //serial.print("Connected to ");
    //serial.println(ssid);
    //serial.print("IP address: ");
    //serial.println(WiFi.localIP());
  }
  else {
    //serial.println("");
    //serial.println("Connection failed.");
  }
  return state;
}
 
