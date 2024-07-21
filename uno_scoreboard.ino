#include <FastLED.h>
#define NUM_LEDS 192   // Number of LEDs in your strip or matrix
#define DATA_PIN 3     // Data pin connected to your LED strip or matrix

#include <SPI.h>
// #include <nRF24L01.h>
#include "printf.h"
#include "RF24.h"

#define LED_TYPE SK6812  // LED chipset type

RF24 radio(7, 8); // CE, CSN
uint8_t address[][6] = { "0x3333337830LL", "0x3333337830LL" };

bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
bool role = false;  // true = TX role, false = RX role
int payload = 0;

CRGB leds[NUM_LEDS];  // Define an array of CRGB LEDs

int HOME_SCORE = 0;
int AWAY_SCORE = 0;

// Flag to track if LEDs need updating
bool ledsChanged = false;

// Bounce objects to read debounced button input

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
}

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);  // Initialize FastLED with the LED configuration
  delay(4000);

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
  // char input = Serial.parseInt();
  // radioNumber = input == 1;
  // Serial.print(F("radioNumber = "));
  // Serial.println((int)radioNumber);

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
  //radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }

  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data


  // ledsChanged = true;

  // setHomeScore(9);
  // setAwayScore(3);
  // delay(4000);
}

void loop() {
  if (role) {
    // This device is a TX node

    unsigned long start_timer = micros();                // start the timer
    bool report = radio.write(&payload, sizeof(float));  // transmit & save the report
    unsigned long end_timer = micros();                  // end the timer

    if (report) {
      Serial.print(F("Transmission successful! "));  // payload was delivered
      Serial.print(F("Time to transmit = "));
      Serial.print(end_timer - start_timer);  // print the timer result
      Serial.print(F(" us. Sent: "));
      Serial.println(payload);  // print payload sent
      payload += 1;          // increment float payload
    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }

    // to make this example readable in the serial monitor
    // delay(1000);  // slow transmissions down by 1 second

  } else {
    // This device is a RX node

    uint8_t pipe;
    if (radio.available()) {              // is there a payload? get the pipe number that recieved it
      // char clk[32] = "";
      // uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      char clk[8] = "";  // Reduced buffer size
      radio.read(clk, sizeof(clk));  // fetch payload from FIFO
      Serial.println((clk[0]));

      int scoreInt = atoi(clk + 1);  // Convert to int directly
      Serial.print(F("Received "));
      // Serial.print(bytes);  // print the size of the payload
      Serial.print(F(" bytes on pipe "));
      // Serial.print(pipe);  // print the pipe number
      Serial.print(F(": "));
      // Serial.println(payload);  // print the payload's value
      // Serial.println(payload);  // print the payload's value
      Serial.println(scoreInt);

      if (clk[0] == 'h') {
        setHomeScore(scoreInt);

        FastLED.show();
      } else if (clk[0] == 'a') {
        setAwayScore(scoreInt);

        FastLED.show();
      }
      // setAwayScore(payload);
      // tone(buzzer, 500);
      // delay(50); 
      // noTone(buzzer);
    }
  }  // role

    // HOME_SCORE++;
    // setHomeScore(HOME_SCORE);

    // AWAY_SCORE++;
    // setAwayScore(AWAY_SCORE);

    // FastLED.show();
    // delay(1000);

  // if (Serial.available()) {
  //   // change the role via the serial monitor

  //   char c = toupper(Serial.read());
  //   if (c == 'T' && !role) {
  //     // Become the TX node

  //     role = true;
  //     Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
  //     radio.stopListening();

  //   } else if (c == 'R' && role) {
  //     // Become the RX node

  //     role = false;
  //     Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
  //     radio.startListening();
  //   }
  // }
}

int numberOfCharacters (int score) {
  if (score < 10) {
    return 1;
  }
  if (score >= 10 && score <= 99) {
    return 2;
  }
  if (score >= 100 && score <= 199) {
    return 3;
  }
}

void setHomeScore(int homeScore) {
  char scoreChar[3];  // Make sure the array is large enough to hold the number and the null terminator
  itoa(homeScore, scoreChar, 10);  // Convert integer to string (base 10)
  
  int numDigits = numberOfCharacters(homeScore);  // Get the number of digits in the String
  int digits[numDigits];  // Create an array to store the digits

  // Extract each digit from the String and store it in the array
  for (int i = 0; i < numDigits; i++) { 
    digits[i] = scoreChar[i] - '0';  // Convert char to int
  }

  // // check if numDigits is 3, 2, or 1
  if (numDigits == 3) {
    drawChar(digits[2], 2);
    drawChar(digits[1], 1);
    drawChar(digits[0], 0);
  } else if (numDigits == 2) {
    drawChar(0, 0);
    drawChar(digits[1], 2);
    drawChar(digits[0], 1);
  } else if (numDigits == 1) {
    drawChar(0, 0);
    drawChar(digits[0], 2);
    drawChar(0, 1);
  }

  ledsChanged = true;
}

void setAwayScore(int homeScore) {
  char scoreChar[3];  // Make sure the array is large enough to hold the number and the null terminator
  itoa(homeScore, scoreChar, 10);  // Convert integer to string (base 10)
  
  int numDigits = numberOfCharacters(homeScore);  // Get the number of digits in the String
  int digits[numDigits];  // Create an array to store the digits

  // Extract each digit from the String and store it in the array
  for (int i = 0; i < numDigits; i++) { 
    digits[i] = scoreChar[i] - '0';  // Convert char to int
  }

  // // check if numDigits is 3, 2, or 1
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

  ledsChanged = true;
}


// void setAwayScore(int awayScore) {
//   String awayScoreString = String(awayScore);  // Convert int to String

//   int numDigits = awayScoreString.length();  // Get the number of digits in the String

//   int digits[numDigits];  // Create an array to store the digits

//   // Extract each digit from the String and store it in the array
//   for (int i = 0; i < numDigits; i++) {
//     digits[i] = awayScoreString.charAt(i) - '0';  // Convert char to int
//   }

//   // check if numDigits is 3, 2, or 1
//   if (numDigits == 3) {
//     drawChar(digits[2], 5);
//     drawChar(digits[1], 4);
//     drawChar(digits[0], 3);
//   } else if (numDigits == 2) {
//     drawChar(digits[1], 5);
//     drawChar(digits[0], 4);
//   } else if (numDigits == 1) {
//     drawChar(digits[0], 5);
//     drawChar(0, 4);
//   }

//   // displayScoreToLCD();
//   ledsChanged = true;
// }
