
const int buttonPin = 5;     //   THIS IS ACTUALLY D1
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
bool buttonDown = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Started up");
  Serial.print("Looping: "); 
  
    // initialize the pushbutton pin as an input:

  //initialize the buttonPin as output
pinMode(buttonPin, INPUT_PULLUP); // Set pin 12 as an input w/ pull-up


}

int loopNo = 0;
int maxLoopNo = 20;

void loop() {
  if (loopNo < maxLoopNo) {
    Serial.print(".");
    loopNo++;
  } else {
    Serial.println("");
    loopNo = 0;  
    Serial.print("Looping: ");
  };

   buttonState = digitalRead(buttonPin);
  
    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (buttonState == HIGH) {     
      // since we're writing HIGH to the pin when, if it's HIGH , the button isn't pressed, as in there is no connectivity between ground and pin 2:
  //so do whatever here that you want when the button is not pushed    

      Serial.println("button not pushed ");
      buttonDown = false;
    } 
    else if (!buttonDown) {
      // turn LED on, or do whatever else you want when your button is pushed

      Serial.println("button pushed");
      buttonDown = true;
 
    }
}

