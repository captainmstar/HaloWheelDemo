/* This is for lighting Halo leds via 
 *  VL53L0X sensors
The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>
#include <FastLED.h>

//VL53L0X Sensors
#define XSHUT_pin1 7
#define XSHUT_pin2 6
#define XSHUT_pin3 5
#define XSHUT_pin4 4

//LEDs
#define DATA_PIN_TOP 2//Top of wheel
#define DATA_PIN_BOTTOM 3//Bottom of wheel
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 61
#define BRIGHTNESS 250
#define FRAMES_PER_SECOND 120

//Sensors
VL53L0X sensorTop;
VL53L0X sensorBottom;
VL53L0X sensorRight;
VL53L0X sensorLeft;

//LEDs
CRGB ledsTop[NUM_LEDS];
CRGB ledsBottom[NUM_LEDS];

void setup()
{
  delay(3000); // 3 second delay for recovery
  
  Serial.begin(9600);
  // wait until serial port opens for native USB devices
  while (! Serial) { delay(1); }
  
  Serial.println(F("setup..."));

  setupSensors();
  setupLEDs();
}

void loop()
{
  runBottom();
  Serial.print(sensorTop.readRangeContinuousMillimeters());
  Serial.print(F(","));
  Serial.print(sensorBottom.readRangeContinuousMillimeters());
  Serial.print(F(","));
  Serial.print(sensorRight.readRangeContinuousMillimeters());
  Serial.print(',');
  Serial.print(sensorLeft.readRangeContinuousMillimeters());
  Serial.println();

  
}

void setupSensors()
{
  Serial.println(F("setupSensors..."));
    
  /*WARNING*/
  //Shutdown pins of VL53L0X ACTIVE-LOW-ONLY NO TOLERANT TO 5V will fry them
  pinMode(XSHUT_pin1, OUTPUT);
  pinMode(XSHUT_pin2, OUTPUT);
  pinMode(XSHUT_pin3, OUTPUT);
  pinMode(XSHUT_pin4, OUTPUT);
  
  Serial.println(F("Shutdown pins inited..."));

  digitalWrite(XSHUT_pin1, LOW);
  digitalWrite(XSHUT_pin2, LOW);
  digitalWrite(XSHUT_pin3, LOW);
  digitalWrite(XSHUT_pin4, LOW);
  Serial.println(F("XSHUT pins in reset mode...(pins are low)"));
  
  delay(500);
  
  Wire.begin();
  Serial.println(F("Wire.begin()"));
  
  //Power up sensorTop
  pinMode(XSHUT_pin1, INPUT);
  delay(150);
  Serial.println(F("XSHUT_pin1 pinMode INPUT"));
  
  if(!sensorTop.init(true)) {
    Serial.println(F("Error starting sensorTop"));
  } else {
    Serial.println(F("Succes starting sensorTop"));
  }
  delay(100);

  //Set I2C address
  sensorTop.setAddress((uint8_t)22);
  delay(10);

  //Power up sensorBottom
  pinMode(XSHUT_pin2, INPUT);
  delay(150);
  Serial.println(F("XSHUT_pin2 pinMode INPUT"));
  
  if(!sensorBottom.init(true)) {
    Serial.println(F("Error starting sensorBottom"));
  } else {
    Serial.println(F("Succes starting sensorBottom"));
  }
  delay(100);

  //Set I2C address
  sensorBottom.setAddress((uint8_t)25);
  delay(10);

  //Power up sensorRight
  pinMode(XSHUT_pin3, INPUT);
  delay(150);
  Serial.println(F("XSHUT_pin3 pinMode INPUT"));
  
  if(!sensorRight.init(true)) {
    Serial.println(F("Error starting sensorRight"));
  } else {
    Serial.println(F("Succes starting sensorRight"));
  }
  delay(100);

  //Set I2C address
  sensorRight.setAddress((uint8_t)28);
  delay(10);

  //Power up sensorLeft
  pinMode(XSHUT_pin4, INPUT);
  delay(150);
  Serial.println(F("XSHUT_pin4 pinMode INPUT"));
  
  if(!sensorLeft.init(true)) {
    Serial.println(F("Error starting sensorLeft"));
  } else {
    Serial.println(F("Succes starting sensorLeft"));
  }
  delay(100);

  //Set I2C address
  sensorLeft.setAddress((uint8_t)31);
  delay(10);
  
  sensorTop.setTimeout(100);
  sensorBottom.setTimeout(100);
  sensorRight.setTimeout(100);
  sensorLeft.setTimeout(100);
  Serial.println(F("Sensor timeouts set"));

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  //Read every 500 ms
  sensorTop.startContinuous(500);
  sensorBottom.startContinuous(500);
  sensorRight.startContinuous(500);
  sensorLeft.startContinuous(500);
  delay(10);
  Serial.println(F("Sensors startContinuous()"));
}

void setupLEDs() {
  
  // tell FastLED about the LED strip configuration
  //top led strip
  FastLED.addLeds<LED_TYPE,DATA_PIN_TOP,COLOR_ORDER>(ledsTop, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //bottom led strip
  FastLED.addLeds<LED_TYPE,DATA_PIN_BOTTOM,COLOR_ORDER>(ledsBottom, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

int MIN_D = 100;
int MAX_D = 500;
int mapSensorToDelay(uint16_t sensorReading) {
  int constrainedInput = constrain((int)sensorReading, MIN_D, MAX_D);
  if(constrainedInput == MIN_D) {
    return MIN_D;
  } else if(constrainedInput == MAX_D) {
    return MAX_D;
  } else {
    //Round up to next int then convert to hundreds for delay
    return ceil(sensorReading / 100) * 100;
  }
}



void blinkBottom() {
  for(int i = 300; i > 0; i-=10) {
    blinkSectionBottom(11, 47, CRGB::Red, i); 
  }
  solidSectionBottom(11, 47, CRGB::Red); 
  delay(5000);
}
int oldBottomDelay = 0;

void runBottom() {
  int newBottomDelay = mapSensorToDelay(sensorBottom.readRangeContinuousMillimeters());
  if(newBottomDelay != oldBottomDelay) {
    runBottomLEDs(newBottomDelay);
    oldBottomDelay = newBottomDelay;
  }
}

void runBottomLEDs(int delay) {
  if(MAX_D == delay) {
    //turn off bottom
    solidSectionBottom(11, 47, CRGB::Black); 
  } else if(MIN_D == delay) {
    //solid light
    solidSectionBottom(11, 47, CRGB::Red); 
  } else {
    //flash lights with delay
    blinkSectionBottom(11, 47, CRGB::Red, delay); 
  }
}

void blinkSectionAll(int startIndex, int stopIndex, CRGB::HTMLColorCode color, int blinkDelay) {
  for(int dot = startIndex; dot < stopIndex; dot++) { 
    ledsTop[dot] = color;
    ledsBottom[dot] = color;
  }
  FastLED.show();
  delay(blinkDelay);
  for(int dot = startIndex; dot < stopIndex; dot++) {
    ledsTop[dot] = CRGB::Black;
    ledsBottom[dot] = CRGB::Black;
  }
  FastLED.show();
  delay(blinkDelay);
}

void blinkSectionTop(int startIndex, int stopIndex, CRGB::HTMLColorCode color, int blinkDelay) {
  for(int dot = startIndex; dot < stopIndex; dot++) { 
    ledsTop[dot] = color;
  }
  FastLED.show();
  delay(blinkDelay);
  for(int dot = startIndex; dot < stopIndex; dot++) {
    ledsTop[dot] = CRGB::Black;
  }
  FastLED.show();
  delay(blinkDelay);
}

void blinkSectionBottom(int startIndex, int stopIndex, CRGB::HTMLColorCode color, int blinkDelay) {
  for(int dot = startIndex; dot < stopIndex; dot++) { 
    ledsBottom[dot] = color;
  }
  FastLED.show();
  delay(blinkDelay);
  for(int dot = startIndex; dot < stopIndex; dot++) {
    ledsBottom[dot] = CRGB::Black;
  }
  FastLED.show();
  delay(blinkDelay);
}

void solidSectionBottom(int startIndex, int stopIndex, CRGB::HTMLColorCode color) {
  for(int dot = startIndex; dot < stopIndex; dot++) { 
    ledsBottom[dot] = color;
  }
  FastLED.show();
}

/**
 * Utility method to print number of I2C devices
 * connected to arduino.
 */
void printNumI2CDevices() {
  Serial.println(F("I2C scanner. Scanning ..."));
  byte count = 0;

  for (byte i = 1; i < 240; i++)
  {

    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Serial.print(F("Found address: "));
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");

  delay(3000);
}
