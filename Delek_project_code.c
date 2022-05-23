#include <Arduino.h>
#include <TFT_eSPI.h>

#define WIFI_SSID /*"telenet-91A71"*/"Biblos"
#define WIFI_PASS /*"rhxQ5AVyG1EW"*/"87654321"

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include "AdafruitIO_WiFi.h"
#define IO_USERNAME  "Daan321"
#define IO_KEY       "aio_eeqr25qhgV1LsxAUPK5zxQAK3el7"

#if defined(USE_AIRLIFT) || defined(ADAFRUIT_METRO_M4_AIRLIFT_LITE) ||         \
    defined(ADAFRUIT_PYPORTAL)
// Configure the pins used for the ESP32 connection
#if !defined(SPIWIFI_SS) // if the wifi definition isnt in the board variant
// Don't change the names of these #define's! they match the variant ones
#define SPIWIFI SPI
#define SPIWIFI_SS 10 // Chip select pin
#define NINA_ACK 9    // a.k.a BUSY or READY pin
#define NINA_RESETN 6 // Reset pin
#define NINA_GPIO0 -1 // Not connected
#endif
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS, SPIWIFI_SS,
    NINA_ACK, NINA_RESETN, NINA_GPIO0, &SPIWIFI);
#else
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
#endif


// set up the feeds

AdafruitIO_Feed* speedFeed = io.feed("change_led_speed");
AdafruitIO_Feed* pointFeed = io.feed("current_points");
AdafruitIO_Feed* scoreboardFeed = io.feed("scoreboard");

void printTitel(void);
void DisplayValue(int, int, int);
void button_pressed();
void letter_up();
void letter_down();
void printName();

TFT_eSPI tft = TFT_eSPI(); // Constructor for the TFT library

#define DISPLAYTEXT "AC4 - B2 - ADDRESSABLE LED"

#include <Adafruit_NeoPixel.h>       //Installeer deze library, via de library manage
#define ADDLEDPIN 17
#define BTN_PIN 13
#define POT_PIN 15
#define LED_PIN1 32
#define LED_PIN2 33
#define LED_PIN3 25
#define BTN_UP 0
#define BTN_DOWN 35
#define LETTERS_IN_NAME 4

int amount_leds = 29;
int led_on = 0;
int target_led = 15;
int total_score = 0;
int speed = 100;
int max_speed = 100;
int min_speed = 10;
int turns = 3;
unsigned long last_time = 0;
boolean buttonDown = false;
boolean nextRound = true;
boolean name_set = true;

char alfabet[26] = { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };
char name[LETTERS_IN_NAME + 1] = "    ";
int letter = 0;
int current_name_letter = 0;
boolean letter_changed = true;
char board[10][LETTERS_IN_NAME + 10];
int current_board = 0;



Adafruit_NeoPixel LedStrip = Adafruit_NeoPixel(amount_leds, ADDLEDPIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    pinMode(ADDLEDPIN, OUTPUT);
    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
    pinMode(LED_PIN3, OUTPUT);
    pinMode(POT_PIN, INPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    Serial.begin(250000);
    printTitel();
    LedStrip.begin();
    LedStrip.setBrightness(15);
    LedStrip.show(); // Initialize all pixels to 'off'

    attachInterrupt(BTN_PIN, button_pressed, FALLING);
    attachInterrupt(BTN_UP, letter_up, FALLING);
    attachInterrupt(BTN_DOWN, letter_down, FALLING);
    speed = map(analogRead(POT_PIN), 0, 4096, min_speed, max_speed);


    //speed only wants to measure when there is no wifi
    Serial.print("Connecting to Adafruit IO");
    io.connect();

    while (io.status() < AIO_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    speedFeed->onMessage(handleSpeedMessage);
    pointFeed->save(0);
    speedFeed->save(speed);
}

void loop()
{
    int data = analogRead(POT_PIN);
    Serial.print("speed: ");
    Serial.println(data);
    if (nextRound && name_set) {
        if (turns > 0) {
            if (!buttonDown) {
                executeButtonUp();
            }
            else {
                executeButtonDown();
            }
        }
        else {
            speed = map(data, 0, 4096, min_speed, max_speed);
            LedStrip.clear();
            turns = 4;
            tft.setCursor(0, 0, 4);
            tft.setTextColor(TFT_BLACK, TFT_YELLOW);
            tft.print("Total score: ");
            tft.println(total_score);
            total_score = 0;
            pointFeed->save(0);
            nextTurn();
            printTitel();
            speedFeed->save(speed);
        }
    }
    else if (name_set == false) {
        if (nextRound) {
            if (current_name_letter < LETTERS_IN_NAME) {
                name[current_name_letter] = alfabet[letter];
                current_name_letter += 1;
                nextRound = false;
            }
            else {
                save_score();
                current_name_letter = 0;
            }
            printName();
        }
        if (letter_changed) {
            printName();
            letter_changed = false;
        }
    }
}

void save_score() {
    name_set = true;
    for (int i = 0; i < LETTERS_IN_NAME; i++) {
        board[current_board][i] = name[i];
        name[i] = ' ';
    }
    board[current_board][LETTERS_IN_NAME] = ':';
    char num[10];
    itoa(total_score, num, 10);
    for (int i = LETTERS_IN_NAME + 1; i < LETTERS_IN_NAME + 10; i++) {
        board[current_board][i] = num[i - (LETTERS_IN_NAME + 1)];
    }
    char scorename[(current_board + 1) * (LETTERS_IN_NAME + 10)];
    int let = 0;
    for (int i = 0; i < current_board + 1; i++) {
        for (int j = 0; j < LETTERS_IN_NAME + 10; j++) {
            if (board[i][j] != '\0') {
                scorename[let] = board[i][j];
                let += 1;
            }
            else {
                break;
            }
        }
        scorename[let] = '\n';
        let += 1;
    }
    current_board = (current_board + 1) % 10;
    scoreboardFeed->save(scorename);
}

void nextTurn() {
    turns -= 1;
    tft.setCursor(0, 0, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("beurten: ");
    tft.println(turns);
    if (turns < 3) {
        digitalWrite(LED_PIN3, LOW);
    }
    else {
        digitalWrite(LED_PIN3, HIGH);
    }
    if (turns < 2) {
        digitalWrite(LED_PIN2, LOW);
    }
    else {
        digitalWrite(LED_PIN2, HIGH);
    }
    if (turns < 1) {
        digitalWrite(LED_PIN1, LOW);
    }
    else {
        digitalWrite(LED_PIN1, HIGH);
    }
}

void executeButtonUp() {
    for (int LedNumber = led_on + 1; LedNumber < amount_leds; LedNumber++)
    {
        if (buttonDown)
            break;
        led_on = LedNumber;
        LedStrip.clear();   //Turn all pixels off
        DisplayValue(255, 3, target_led);
        DisplayValue(128, 2, (target_led + 1) % amount_leds);
        DisplayValue(128, 2, (target_led - 1) % amount_leds);
        DisplayValue(64, 0, (target_led + 2) % amount_leds);
        DisplayValue(64, 0, (target_led - 2) % amount_leds);
        DisplayValue(128, 1, LedNumber);
        delay(speed);
    }
    if (!buttonDown)
        led_on = -1;
}

void executeButtonDown() {
    nextTurn();

    tft.setCursor(0, 50, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("Total score: ");
    total_score += calc_score();
    tft.println(total_score);
    pointFeed->save(total_score);
    nextRound = false;
    buttonDown = false;
    if (turns == 0) {
        name_set = false;
        printName();
    }
}

int calc_score() {
    if (led_on == target_led)
        return 3 * max_speed / speed;
    if (led_on == target_led + 1 || led_on == target_led - 1)
        return 2 * max_speed / speed;
    if (led_on == target_led + 2 || led_on == target_led - 2)
        return max_speed / speed;
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

void handleSpeedMessage(AdafruitIO_Data* data) {
    Serial.println("speed changed");
    speed = data->toInt();
}

void button_pressed() {
    unsigned long current_time = millis();
    if (current_time - last_time > 200) {
        if (name_set) {
            if (nextRound)
                buttonDown = true;
            else
                nextRound = true;
        }
        else {
            nextRound = true;
        }
    }
    last_time = current_time;
}

void letter_up() {
    unsigned long current_time = millis();
    if (current_time - last_time > 200) {
        letter += 1;
        if (letter > 25)
            letter = 0;
        letter_changed = true;
    }
    last_time = current_time;
}

void letter_down() {
    unsigned long current_time = millis();
    if (current_time - last_time > 200) {
        letter -= 1;
        if (letter < 0)
            letter = 25;
        letter_changed = true;
    }
    last_time = current_time;
}

void printTitel()
{
    tft.init();
    tft.setRotation(3);       //setRotation: 1: Screen in landscape(USB to the right), 3:  Screen in landscape(USB connector Left)
    tft.fillScreen(TFT_BLACK);//Fill screen with random colour

    tft.setCursor(0, 0, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("beurten: ");
    tft.println(turns);

    tft.setCursor(0, 50, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("Total score: ");
    tft.println(total_score);
}

void printName() {
    tft.init();
    tft.setRotation(3);       //setRotation: 1: Screen in landscape(USB to the right), 3:  Screen in landscape(USB connector Left)
    tft.fillScreen(TFT_BLACK);//Fill screen with random colour

    tft.setCursor(0, 0, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("naam: ");
    tft.println(name);

    tft.setCursor(0, 50, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.println(alfabet[letter]);

    tft.setCursor(0, 100, 4);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.print("Total score: ");
    tft.println(total_score);
}