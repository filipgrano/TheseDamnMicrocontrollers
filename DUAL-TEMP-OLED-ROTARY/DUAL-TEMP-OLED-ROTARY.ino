#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define ONE_WIRE_BUS_1 11 //pin11
#define ONE_WIRE_BUS_2 10 //pin10
#include <YetAnotherPcInt.h> //specific library for making any Digital pin as interrupt pin on Nano

OneWire oneWire_in(ONE_WIRE_BUS_1);
OneWire oneWire_out(ONE_WIRE_BUS_2);
DallasTemperature sensor_inhouse(&oneWire_in);
DallasTemperature sensor_outhouse(&oneWire_out);

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
float TempA =0; //global variable for storing temp reading
float TempB = 0; //global variable for storing temp reading

// Rotary Encoder Inputs
#define CLK 3   //rotary encoder pin D4
#define DT 4    //rotary encoder pin D3
#define SW 5   ////rotary encoder pushbutton pin D5
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  // Call updateEncoder() when any high/low changed seen
  // on interrupt 0 (pin 2), or interrupt 1 (pin 3)
 // attachPCINT(digitalPinToINT(CLK), GetInput, CHANGE);
  //attachInterrupt(DT, GetInput, CHANGE);
   PcInt::attachInterrupt(CLK, GetInput, CHANGE);
    PcInt::attachInterrupt(DT, GetInput, CHANGE);
    PcInt::attachInterrupt(SW, GetInput, CHANGE);
  
}

void loop() {
   display.setTextSize(1);      // Normal 1:1 pixel scale
   display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
   display.clearDisplay();
   
  GetTemp(); // request temperature reading

    display.setCursor(0, 0);     // Start at top-x corner
    display.print("A: ");
    display.print(TempA);
    display.setCursor(0, 20);     // Start at top-x corner
    display.print("B: ");
    display.print(TempB);

// testing rotary encoder 
   display.setCursor(0, 40);     // Start at top-x corner
   display.print(currentDir);
   display.setCursor(30, 40);     // Start at top-x corner
   display.print(counter);
      display.setCursor(60, 40);     // Start at top-x corner
   display.print(lastButtonPress);

 display.display(); //print the things on the display'
 delay(1);
}


////////////////////////////////>>Useful function only<</////////
void GetTemp(){
  //requests temperature reading
    sensor_inhouse.requestTemperatures();
    sensor_outhouse.requestTemperatures();
     TempA = sensor_inhouse.getTempCByIndex(0);
     TempB = sensor_outhouse.getTempCByIndex(0);
}

void GetInput(){
//monitors the encoder for user input
 // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter ++;
      currentDir ="CW";
    } else {
      // Encoder is rotating CW so increment
      counter --;
      currentDir ="CCW";
    }
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      Serial.println("Button pressed!");
    }

    // Remember last button press event
    lastButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  //delay(1);
}


// void GetInput(){
// //monitors the encoder for user input
//  // Read the current state of CLK
//   currentStateCLK = digitalRead(CLK);

//   // If last and current state of CLK are different, then pulse occurred
//   // React to only 1 state change to avoid double count
//   if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

//     // If the DT state is different than the CLK state then
//     // the encoder is rotating CCW so decrement
//     if (digitalRead(DT) != currentStateCLK) {
//       counter --;
//       currentDir ="CCW";
//     } else {
//       // Encoder is rotating CW so increment
//       counter ++;
//       currentDir ="CW";
//     }
//   }
//   // Remember last CLK state
//   lastStateCLK = currentStateCLK;

//   // Read the button state
//   int btnState = digitalRead(SW);

//   //If we detect LOW signal, button is pressed
//   if (btnState == LOW) {
//     //if 50ms have passed since last LOW pulse, it means that the
//     //button has been pressed, released and pressed again
//     if (millis() - lastButtonPress > 50) {
//       Serial.println("Button pressed!");
//     }

//     // Remember last button press event
//     lastButtonPress = millis();
//   }

//   // Put in a slight delay to help debounce the reading
//   //delay(1);
// }


