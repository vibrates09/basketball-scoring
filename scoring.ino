#include <FastLED.h>
#include <Bounce2.h>

#define NUM_LEDS 192   // Number of LEDs in your strip or matrix
#define DATA_PIN 2    // Data pin connected to your LED strip or matrix
#define LED_TYPE SK6812  // LED chipset type

CRGB leds[NUM_LEDS];  // Define an array of CRGB LEDs

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

// Flag to track if LEDs need updating
bool ledsChanged = false; 

// Bounce objects to read debounced button input
Bounce2::Button homeIncrementButton = Bounce2::Button();
Bounce2::Button homeDecrementButton = Bounce2::Button();

Bounce2::Button awayIncrementButton = Bounce2::Button();
Bounce2::Button awayDecrementButton = Bounce2::Button();

int charLayout[11][42] = {
    {
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0
    }, // 0
    {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0
    }, // 1
    {
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1
    }, // 2
    {
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1
    }, // 3
    {
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1
    }, // 4
    {
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1
    }, // 5
    {  
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1
    }, // 6
    {  
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,

    }, // 7
    {  
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
    }, // 8
    {  
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1,
    }, // 9
    {  
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
    }, // CLEAR
}; 

int twoSegment[2][12] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 1
};

int offset[] = { 0, 12, 54, 96, 108, 150 };

void drawChar(int number, int offsetY) {
  if (offset[offsetY] == 0 || offset[offsetY] == 96) {
    for (int i = 0; i <= 11; i++) {
      if(twoSegment[number][i] == 1) {
        leds[i + offset[offsetY]] = CRGB::Red;
      }else {
        leds[i + offset[offsetY]] = CRGB::Black;
      }
    }
  } else {
    for (int i = 0; i <= 41; i++) {
      if(charLayout[number][i] == 1) {
        leds[i + offset[offsetY]] = CRGB::Red;
      }else {
        leds[i + offset[offsetY]] = CRGB::Black;
      }
    }
  }
  // FastLED.show();
}

void setup() {
  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);  // Initialize FastLED with the LED configuration

  // TODO:: Add multiple strip support for faster processing
  // FastLED.addLeds<LED_TYPE, DATA_PIN2>(leds[0], NUM_LEDS_PER_STRIP);
  // FastLED.addLeds<LED_TYPE, LED_PIM3>(leds[1], NUM_LEDS_PER_STRIP);

  homeDecrementButton.attach(pin5, INPUT);
  homeIncrementButton.attach(pin6, INPUT);

  awayDecrementButton.attach(pin8, INPUT);
  awayIncrementButton.attach(pin9, INPUT);

  homeDecrementButton.interval(5);
  homeIncrementButton.interval(5);

  awayDecrementButton.interval(5);
  awayIncrementButton.interval(5);

  delay(3000);
  setHomeScore(0);
  setAwayScore(0);
}

void loop() {
  homeDecrementButton.update();
  homeIncrementButton.update();

  awayDecrementButton.update();
  awayIncrementButton.update();

  if (homeDecrementButton.fell()) {
    HOME_SCORE--;
    setHomeScore(HOME_SCORE);
    ledsChanged = true;
  }

  if (homeIncrementButton.fell()) {
    HOME_SCORE++;
    setHomeScore(HOME_SCORE);
    ledsChanged = true;
  }

  if (awayDecrementButton.fell()) {
    AWAY_SCORE--;
    setAwayScore(AWAY_SCORE);
    ledsChanged = true;
  }

  if (awayIncrementButton.fell()) {
    AWAY_SCORE++;
    setAwayScore(AWAY_SCORE);
    ledsChanged = true;
  }

  if (ledsChanged) {
    FastLED.show();
    ledsChanged = false; // Reset the flag after updating LEDs
  }
}

void setHomeScore(int homeScore) {
  String homeScoreString = String(homeScore);  // Convert int to String

  int numDigits = homeScoreString.length();  // Get the number of digits in the String

  int digits[numDigits];  // Create an array to store the digits

  // Extract each digit from the String and store it in the array
  for (int i = 0; i < numDigits; i++) {
    digits[i] = homeScoreString.charAt(i) - '0';  // Convert char to int
  }

  // check if numDigits is 3, 2, or 1
  if (numDigits == 3) {
    drawChar(digits[2], 2);
    drawChar(digits[1], 1);
    drawChar(digits[0], 0);
  } else if (numDigits == 2) {
    drawChar(digits[1], 2);
    drawChar(digits[0], 1);
  } else if (numDigits == 1) {
    drawChar(digits[0], 2);
    drawChar(0, 1);
  }
}

void setAwayScore(int awayScore) {
  String awayScoreString = String(awayScore);  // Convert int to String

  int numDigits = awayScoreString.length();  // Get the number of digits in the String

  int digits[numDigits];  // Create an array to store the digits

  // Extract each digit from the String and store it in the array
  for (int i = 0; i < numDigits; i++) {
    digits[i] = awayScoreString.charAt(i) - '0';  // Convert char to int
  }

  // check if numDigits is 3, 2, or 1
  if (numDigits == 3) {
    drawChar(digits[2], 5);
    drawChar(digits[1], 4);
    drawChar(digits[0], 3);
  } else if (numDigits == 2) {
    drawChar(digits[1], 5);
    drawChar(digits[0], 4);
  } else if (numDigits == 1) {
    drawChar(digits[0], 5);
    drawChar(0, 4);
  }
}
