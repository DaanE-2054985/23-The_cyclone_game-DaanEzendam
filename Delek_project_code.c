#include <Arduino.h>
#include <TFT_eSPI.h>

void printTitel(void);
void DisplayValue(int, int, int);


TFT_eSPI tft = TFT_eSPI(); // Constructor for the TFT library

#define DISPLAYTEXT "AC4 - B2 - ADDRESSABLE LED"

#include <Adafruit_NeoPixel.h>       //Installeer deze library, via de library manage
#define ADDLEDPIN 17
#define BTN_PIN 13
#define POT_PIN 15

int amount_leds = 3;
int led_on = 0;
int target_led = 1;
int total_score = 0;
int speed = 300;
int turns = 3;
boolean buttonDown = false;

Adafruit_NeoPixel LedStrip = Adafruit_NeoPixel(amount_leds, ADDLEDPIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  pinMode(ADDLEDPIN, OUTPUT);
  Serial.begin(250000);
  printTitel();
  LedStrip.begin();
  LedStrip.setBrightness(15);
  LedStrip.show(); // Initialize all pixels to 'off'
  pinMode(POT_PIN, INPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(BTN_PIN, button_pressed, RISING);
  
  speed = map(analogRead(POT_PIN),0,4096,50,500);
}

void loop()
{
  if(turns > 0){
    if(!buttonDown){
      executeButtonUp();
    }
    else{
      executeButtonDown();
    }
  }else{
    LedStrip.clear();
    turns = 3;
    tft.setCursor(0,0,4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("Total score: ");
    tft.println(total_score);
    total_score = 0;
    delay(5000);
    printTitel();
    speed = map(analogRead(POT_PIN),0,4096,50,500);
    Serial.println(speed);
  }
}

void executeButtonUp(){
  for (int LedNumber = led_on+1; LedNumber < amount_leds; LedNumber++)
    {
      if(buttonDown)
        break;
      led_on = LedNumber;
      LedStrip.clear();   //Turn all pixels off
      DisplayValue(255, 2, 1);
      DisplayValue(128, 1, LedNumber);
      delay(speed);
    }
    if(!buttonDown)
      led_on = -1;
}

void executeButtonDown(){
  Serial.println("button has been pressed");
  turns-=1;
  tft.setCursor(0,0,4);
  tft.setTextColor(TFT_BLACK, TFT_YELLOW);
  tft.print("beurten: ");
  tft.println(turns);
  
  tft.setCursor(0,50,4);
  tft.setTextColor(TFT_BLACK, TFT_YELLOW);
  tft.print("Total score: ");
  total_score += calc_score();
  tft.println(total_score);
  delay(3000);
  buttonDown = false;
}

int calc_score(){
  if(led_on == target_led)
    return map(speed,50,500,10,2);
  if(led_on == target_led+1 || led_on == target_led-1)
    return map(speed,50,500,5,1);
  return 0;
}
void DisplayValue(int ValIn, int color, int LedNum)
{
  uint32_t ColorPicked = 0;
  if (color == 0) {
    ColorPicked = LedStrip.Color(ValIn, 0, 0);
  }
  else if (color == 1) {
    ColorPicked = LedStrip.Color(0, ValIn, 0);
  }
  else if (color == 2) {
    ColorPicked = LedStrip.Color(0, 0, ValIn);
  }
  else {
    ColorPicked = LedStrip.Color(ValIn, ValIn, ValIn);
  }

  LedStrip.setPixelColor(LedNum, ColorPicked);
  LedStrip.show();
//  Serial.print(LedNum);
//  Serial.print(":");
//  Serial.print(ValIn);
//  Serial.print(",");
}

void button_pressed(){
  buttonDown = true;
}


void printTitel()
{
  tft.init();
  tft.setRotation(3);       //setRotation: 1: Screen in landscape(USB to the right), 3:  Screen in landscape(USB connector Left)
  tft.fillScreen(TFT_BLACK);//Fill screen with random colour
  
  tft.setCursor(0,0,4);
  tft.setTextColor(TFT_BLACK, TFT_YELLOW);
  tft.print("beurten: ");
  tft.println(turns);
  
  tft.setCursor(0,50,4);
  tft.setTextColor(TFT_BLACK, TFT_YELLOW);
  tft.print("Total score: ");
  tft.println(total_score);
}