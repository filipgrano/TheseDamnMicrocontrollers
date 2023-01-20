//Code for running a temperature sensor built on Arduino Nano
// D3 - CLK Encoder
// D4 - DT Encoder
// D5 - Button Encoder
// D9 - BUZZER (+)
// D10 - Temperature sensor + 4k7 ohm
// D11 - Temperature sensor + 4k7 ohm
// A4 - SDA OLED Display
// A5 - SCL OLED Display
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <YetAnotherPcInt.h>  //specific library for making any Digital pin as interrupt pin on Nano
//#include "pitches.h" //for option to play tunes
//#include "themes.h" //for option to play tunes

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define ONE_WIRE_BUS_1 11    //pin11
#define ONE_WIRE_BUS_2 10    //pin10
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define CLK 3                //rotary encoder pin D3
#define DT 4                 //rotary encoder pin D4
#define SW 5                 //rotary encoder pushbutton pin D5
const int buzzer = 9;        //buzzer to arduino pin 9

OneWire oneWire_in(ONE_WIRE_BUS_1);
OneWire oneWire_out(ONE_WIRE_BUS_2);
DallasTemperature sensor_inhouse(&oneWire_in);
DallasTemperature sensor_outhouse(&oneWire_out);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float TempA = 0;     //global variable for storing temp reading
float TempB = 0;     //global variable for storing temp reading
int AlarmA = 30;     //used to set temperature at which causes alarm to go off
int AlarmB = 30;     //used to set temperature at which causes alarm to go off
int EditMode = 1;    //used to check if button is pressed / edit mode or not
int editing = 0;     // determine which menu item is being edited A or B temperature alarm
int counter = 0;     // Used for rotary encoder pulse counts
int counterMAX = 1;  // Used for limiting rotary encoder input max/min values
int counterMIN = 0;  // Used for limiting rotary encoder input max/min values
int refresh = 0;     // used for screen refresh on need basis.
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;

int buzzerplay = 0;  //debug

void setup() {
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);  // Set buzzer - pin 9 as an output
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
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.cp437(true);                  // Use full 256 char 'Code Page 437' font
  display.clearDisplay();
  display.invertDisplay(false);


  GetTemp();  // request temperature reading

  display.setCursor(0, 0);  // Start at top-x corner
  display.print(F("A: "));
  display.setTextSize(1.5);  // Normal 1:1 pixel scale
  display.print(TempA);
  display.setTextSize(1);     // Normal 1:1 pixel scale
  display.setCursor(60, 10);  // Start for alarm temp:
  display.print("Alarm: ");
  display.print(AlarmA);
  if (counter == 0) {
    display.setCursor(50, 10);  // Start for alarm temp:
    display.print("*");
  }
  display.setCursor(0, 20);  // Start at top-x corner
  display.print("B: ");
  display.print(TempB);
  display.setCursor(60, 30);  // Start for alarm temp:
  display.print("Blarm: ");
  display.print(AlarmB);
  if (counter == 1) {
    display.setCursor(50, 30);  // Start for alarm temp:
    display.print("*");
  }
  display.setCursor(0, 40);  // Start at top-x corner
  display.print("Delta: ");
  display.print(TempA - TempB);

  // testing rotary encoder values:
  // display.setCursor(0, 55);  // Start at top-x corner
  // display.print(currentDir);
  // display.setCursor(30, 55);  // Start at top-x corner
  // display.print(counter);
  // display.setCursor(60, 55);  // Start at top-x corner
  // display.print(EditMode);
  display.display();  //print the things on the display'
  delay(100);

  //check if user wants to edit;
  if (EditMode == 0) {
    //User will edit A
    Edit();
  }

  //Check if alarm should ring; 
  if(TempA>=AlarmA){
    Alarm();
  }
}


////////////////////////////////>>Useful function only<</////////
void GetTemp() {  //Request temperature reading
  sensor_inhouse.requestTemperatures();
  sensor_outhouse.requestTemperatures();
  TempA = sensor_inhouse.getTempCByIndex(0);
  TempB = sensor_outhouse.getTempCByIndex(0);
}

void Alarm(){ //ring the alarm
// A > Alarm   || B > Blarm
// A < Alarm > A & B 
display.invertDisplay(true);
        tone(buzzer, 2000);  // Send 1KHz sound signal...
        delay(50);
         tone(buzzer, 3000);  // Send 1KHz sound signal...                           // ...for 1 sec
        delay(50);
         tone(buzzer, 4000);  // Send 1KHz sound signal...                           // ...for 1 sec
         delay(100);
        noTone(buzzer);                     // Stop sound...
        buzzerplay = 0;
}

void GetInput() {  //Monitors the encoder for user input
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK) {
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so increment
    if (digitalRead(DT) != currentStateCLK) {
      counter++;
      if (counter > counterMAX) {
        counter = counterMAX;
      }
      currentDir = "CW";
    } else {
      // Encoder is rotating CW so decrement
      counter--;
      if (counter < counterMIN) {
        counter = counterMIN;
      }
      currentDir = "CCW";
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
    if (millis() - lastButtonPress > 80) {
     // Serial.println("Button pressed!");
      switch (EditMode) {  //toggle edit mode on each button press
        case 1:
          EditMode = 0;
          break;
        case 0:
          EditMode = 1;
          break;
      }
    }
  }
  // Remember last button press event
  lastButtonPress = millis();
  // Put in a slight delay to help debounce the reading
  delay(5);
  buzzerplay = 1;  // Sound on input
  refresh = 1;     // screen refresh needed
}

//Edit Alarm A temperature
void Edit() {
  if (counter == 0) {
    editing = 0;
    counter = AlarmA;        // set alarmA value to counter start
  } else {
    editing = 1;
     counter = AlarmB;        // set alarmA value to counter start
  }
  counterMAX = 100;
  counterMIN = -99;
  
  while (EditMode == 0) {  //as long as button is not pressed again, stay in the loop
    // if screen needs update with new values, do it:
    if (refresh = 1) {
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.clearDisplay();
      display.print(F("Set temp "));   /////////////////////////////////jlskdjalksjdalksdj
      //  display.print(%f, editing);
      display.print(F(" alarm:"));
      display.setCursor(15, 15);  // Start at top-x corner
      display.setTextSize(5);
      display.print(counter);
      display.display();  //print the things on the display'
                          //delay(20);
                          //play buzzertone on change;
      if (buzzerplay == 1) {
        tone(buzzer, 2000 + counter * 15);  // Send 1KHz sound signal...
        delay(5);                           // ...for 1 sec
        noTone(buzzer);                     // Stop sound...
        buzzerplay = 0;
      }
      refresh = 0;
    }
  }
  counterMAX = 1;  // reset counterMAX
  counterMIN = 0;  //reset counterMIN
  if (editing == 0) {
    AlarmA = counter;
    counter = 0;  //return * selector to initial location
  } else {
    AlarmB = counter;
    counter = 1;  //return * selector to initial location
  }

  delay(250);  //no bounce effect
}

// //Edit Alarm A temperature
// void EditA() {
//  // display.setTextColor(SSD1306_WHITE);  // Draw white text
//  // display.cp437(true);                  // Use full 256 char 'Code Page 437' font
//   counterMAX = 100;
//   counterMIN = -99;
//   counter = AlarmA;  // set alarmA value to counter start
//   while (EditMode == 0) {  //as long as button is not pressed again, stay in the loop
//     // if screen needs update with new values, do it:
//     if(refresh = 1){
//     display.setCursor(0, 0);
//     display.setTextSize(1);
//     display.clearDisplay();
//     display.print("Set temp A alarm:");
//     display.setCursor(15, 15);  // Start at top-x corner
//     display.setTextSize(5);
//     display.print(counter);
//     display.display();  //print the things on the display'
//     //delay(20);
//       //play buzzertone on change;
//         if(buzzerplay ==1){
//         tone(buzzer, 2000+counter*15); // Send 1KHz sound signal...
//         delay(5);        // ...for 1 sec
//         noTone(buzzer);     // Stop sound...
//         buzzerplay=0;
//         }
//         refresh = 0;
//       }
//   }
//   counterMAX = 1;  // reset counterMAX
//   counterMIN = 0;  //reset counterMIN
//   AlarmA = counter;

//   counter = 0; //return * selector to initial location
//   delay(250);  //no bounce effect
//}
//Edit Alarm B temperature
// void EditB() {
//  // display.setTextColor(SSD1306_WHITE);  // Draw white text
//  // display.cp437(true);                  // Use full 256 char 'Code Page 437' font
//   counterMAX = 100;
//   counterMIN = -99;
//   counter = AlarmB;  // set alarmA value to counter start

//   while (EditMode == 0) {  //as long as button is not pressed again
//     display.setTextSize(1);               // Normal 1:1 pixel scale
//     display.setCursor(0, 0);  // Start at top-x corner
//     display.clearDisplay();
//     display.print("Set temp B alarm:");
//     display.setTextSize(5);               // Normal 1:1 pixel scale
//     display.setCursor(15, 15);  // Start at top-x corner
//     display.print(counter);
//     delay(20);
//     display.display();  //print the things on the display'

//   }
//   counterMAX = 1;  // reset counterMAX
//   counterMIN = 0;  //reset counterMIN
//   AlarmB = counter;
//   counter = 1; //return * selector to initial location
//   delay(250);  //no bounce effect
// }
