// #include <FastLED.h>
#include <Bounce2.h>
// Include Wire Library for I2C
#include <Wire.h>
// Include NewLiquidCrystal Library for I2C
// #include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

// MEGA BOARD
#define CE_PIN 49
#define CSN_PIN 47

#define NUM_LEDS 192   // Number of LEDs in your strip or matrix
#define DATA_PIN 2    // Data pin connected to your LED strip or matrix
#define LED_TYPE SK6812  // LED chipset type

// Define LCD pinout
const int  en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = { "0x3333337830LL", "0x3333337830LL" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 0;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
bool role = true;  // true = TX role, false = RX role

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
int payload = 0;

// Define I2C Address - change if reqiuired
const int i2c_addr = 0x27; // 0x3F;

// CRGB leds[NUM_LEDS];  // Define an array of CRGB LEDs

// TODO:: Add multiple strip support for faster processing
// #define NUM_STRIPS 2
// #define NUM_LEDS_PER_STRIP 96
// int NUM_LEDS_PER_STRIP = 96
// CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

int HOME_SCORE = 0;
int AWAY_SCORE = 0;

// input pins
const int pin5 = 5;
const int pin6 = 6;
const int pin8 = 8;
const int pin9 = 9;

// Define LCD display connections
// LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

// Flag to track if LEDs need updating
bool ledsChanged = false; 

// Bounce objects to read debounced button input
Bounce2::Button homeIncrementButton = Bounce2::Button();
Bounce2::Button homeDecrementButton = Bounce2::Button();

Bounce2::Button awayIncrementButton = Bounce2::Button();
Bounce2::Button awayDecrementButton = Bounce2::Button();

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  radio.setAutoAck(false);

  // print example's introductory prompt
  Serial.println(F("RF24/examples/GettingStarted"));

  // To set the radioNumber via the Serial monitor on startup
  // Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  // while (!Serial.available()) {
  //   // wait for user input
  // }
  char input = Serial.parseInt();
  radioNumber = input == 1;
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);

  // role variable is hardcoded to RX behavior, inform the user of this
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_MIN);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  //radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  //radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }

  // For debugging info
  printf_begin();             // needed only once for printing details
  radio.printDetails();       // (smaller) function that prints raw register values
  radio.printPrettyDetails(); // (larger) function that prints human readable data

  homeDecrementButton.attach(pin5, INPUT);
  homeIncrementButton.attach(pin6, INPUT);

  awayDecrementButton.attach(pin8, INPUT);
  awayIncrementButton.attach(pin9, INPUT);

  homeDecrementButton.interval(5);
  homeIncrementButton.interval(5);

  awayDecrementButton.interval(5);
  awayIncrementButton.interval(5);

  delay(4000);
  ledsChanged = true;

  // lcd.begin(16,2);
  setHomeScore();
  setAwayScore();
  // displayScoreToLCD();
}

void loop() {
  homeDecrementButton.update();
  homeIncrementButton.update();

  awayDecrementButton.update();
  awayIncrementButton.update();

  if (homeDecrementButton.fell() && HOME_SCORE > 0) {
    HOME_SCORE--;
    setHomeScore();
  }

  if (homeIncrementButton.fell()) {
    HOME_SCORE++;
    setHomeScore();
  }

  if (awayDecrementButton.fell() && AWAY_SCORE > 0) {
    AWAY_SCORE--;
    setAwayScore();
  }

  if (awayIncrementButton.fell()) {
    AWAY_SCORE++;
    setAwayScore();
  }



  if (Serial.available()) {
    // change the role via the serial monitor

    char c = toupper(Serial.read());
    if (c == 'T' && !role) {
      // Become the TX node

      role = true;
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      radio.stopListening();

    } else if (c == 'R' && role) {
      // Become the RX node

      role = false;
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      radio.startListening();
    }
  }
}

void displayScoreToLCD () {
  // lcd.clear();

  // lcd.setCursor(0,0);
  // lcd.print("Bulldogs: ");  
  // lcd.print(HOME_SCORE);  

  // lcd.setCursor(0,1);
  // lcd.print("Dragons: ");  
  // lcd.print(AWAY_SCORE);  
}
void setHomeScore() {
  // This device is a TX node
  String hTot;
  hTot = "h" + String(HOME_SCORE);
  char hSc[8];
  hTot.toCharArray(hSc, 8);
  bool report = radio.write(&hSc, sizeof(hSc));

  if (report) {
    Serial.print(F("Transmission successful! "));  // payload was delivered
    Serial.print(F("Time to transmit = "));
    // Serial.print(end_timer - start_timer);  // print the timer result
    Serial.print(F(" us. Sent: "));
    Serial.println(hSc);  // print payload sent
    //HOME_SCORE += 1;          // increment float payload
    if (HOME_SCORE >= 199) {
      HOME_SCORE = 0;
    }
  } else {
    Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
  }
}

void setAwayScore() {
  // This device is a TX node
  String hTot;
  hTot = "a" + String(AWAY_SCORE);
  char hSc[8];
  hTot.toCharArray(hSc, 8);
  bool report = radio.write(&hSc, sizeof(hSc));

  if (report) {
    Serial.print(F("Transmission successful! "));  // payload was delivered
    Serial.print(F("Time to transmit = "));
    // Serial.print(end_timer - start_timer);  // print the timer result
    Serial.print(F(" us. Sent: "));
    Serial.println(hSc);  // print payload sent
    //HOME_SCORE += 1;          // increment float payload
    if (AWAY_SCORE >= 199) {
      AWAY_SCORE = 0;
    }
  } else {
    Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
  }
}
