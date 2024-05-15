#include <FastLED.h>
#include <Bounce2.h>

#define NUM_LEDS 192   // Number of LEDs in your strip or matrix
#define DATA_PIN 2    // Data pin connected to your LED strip or matrix
#define LED_TYPE SK6812  // LED chipset type

CRGB leds[NUM_LEDS];  // Define an array of CRGB LEDs

int HOME_SCORE = 0;
int AWAY_SCORE = 0;
int HOME_SEGMENTS[] = {0,1,2};
int AWAY_SEGMENTS[] = {3,4,5};

// input pins
// const byte pin7 = 7;
// const int pin6 = 6;
// const int pin5 = 5;
// const int pi64 = 4;
// const int INPUT_BUTTON_PIN = 4;


// Bounce objects to read debounced button input
// Bounce2::Button btnStart = Bounce2::Button();
Bounce2::Button homeIncrementButton = Bounce2::Button();
Bounce2::Button homeDecrementButton = Bounce2::Button();

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
        FastLED.show();
      }else {
        leds[i + offset[offsetY]] = CRGB::Black;
        FastLED.show();
      }
    }
  } else {
    for (int i = 0; i <= 41; i++) {
      if(charLayout[number][i] == 1) {
        leds[i + offset[offsetY]] = CRGB::Red;
        FastLED.show();
      }else {
        leds[i + offset[offsetY]] = CRGB::Black;
        FastLED.show();
      }
    }
  }
}

int buttonState = 0;

void setup() {
  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);  // Initialize FastLED with the LED configuration
  // Serial.begin(9600);
  // Serial.begin(115200);

  // Serial.println("STARTTTED");
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  // digitalWrite(pin6, LOW);


  // homeIncrementButton.attach(pi64, INPUT_PULLUP);
  // homeDecrementButton.attach(pin5, INPUT_PULLUP);
  delay(4000);
  setHomeScore(0);
  setAwayScore(0);
}

int pressed=0;
void loop() {
  // Serial.println(HOME_SCORE);
  // if (homeDecrementButton.pressed()) {
    // HOME_SCORE = HOME_SCORE - 1;
    // setHomeScore(HOME_SCORE);
  // }

  // if (homeIncrementButton.pressed()) {
    // HOME_SCORE = HOME_SCORE + 1;
    // Serial.println(HOME_SCORE);
    // setHomeScore(HOME_SCORE);
  // }

  // int buttonVal = digitalRead(pin6); // returns 0 (LOW) or 1 (HIGH)

  // // Serial.println(buttonVal);

  // // if (buttonVal == 1) {
  // // HOME_SCORE = HOME_SCORE + 1;
  // // Serial.println(HOME_SCORE);
  // // setHomeScore(HOME_SCORE);
  // // }

  if (digitalRead(6) == 1) {
    if(pressed == 0) {
      setHomeScore(HOME_SCORE++);
      pressed = 1;
    }
  } else {
    pressed = 0;
  }

  if (digitalRead(5) == 1) {
    if(pressed == 0) {
      setHomeScore(HOME_SCORE--);
      pressed = 1;
    }
  } else {
    pressed = 0;
  }

  if (digitalRead(9) == 1) {
    if(pressed == 0) {
      setAwayScore(AWAY_SCORE++);
      pressed = 1;
    }
  } else {
    pressed = 0;
  }

  if (digitalRead(8) == 1) {
    if(pressed == 0) {
      setAwayScore(AWAY_SCORE--);
      pressed = 1;
    }
  } else {
    pressed = 0;
  }

  //delay(500);
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
