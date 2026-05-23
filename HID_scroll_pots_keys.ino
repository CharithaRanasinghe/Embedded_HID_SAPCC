#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDConsumerControl.h"
#include "USBHIDKeyboard.h"

USBHIDMouse Mouse;
USBHIDConsumerControl Consumer;
USBHIDKeyboard Keyboard;

#define CLK 4
#define DT  5
#define SW  6

int lastStateCLK;
bool lastButtonState = HIGH;

#define POT_VOLUME     7
#define POT_BRIGHTNESS 1
#define POT_ZOOM       2

int lastVolume     = -1;
int lastBrightness = -1;
int lastZoom       = -1;

const int ROWS = 4;
const int COLS = 4;

int rowPins[ROWS] = {38, 39, 40, 41};
int colPins[COLS] = {42, 8, 17, 18};

int keys[ROWS][COLS] = {
  {1,  2,  3,  4},
  {5,  6,  7,  8},
  {9, 10, 11, 12},
  {13,14, 15, 16}
};

bool prevState[ROWS][COLS] = {};
bool rawState[ROWS][COLS] = {};

unsigned long lastDebounce[ROWS][COLS] = {};
const unsigned long DEBOUNCE_MS = 15;

void winRun(const char* cmd) {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');

  delay(150);

  Keyboard.releaseAll();

  delay(400);

  Keyboard.print(cmd);

  delay(100);

  Keyboard.write(KEY_RETURN);

  delay(300);
}

void combo(uint8_t mod1, uint8_t mod2, uint8_t key) {

  Keyboard.press(mod1);

  if(mod2)
    Keyboard.press(mod2);

  if(key)
    Keyboard.press(key);

  delay(100);

  Keyboard.releaseAll();
}

void runAction(int key) {

  switch(key) {

    case 1:
      winRun("calc");
      break;

    case 2:
      winRun("winword");
      break;

    case 3:
      winRun("powerpnt");
      break;

    case 4:
      Consumer.press(CONSUMER_CONTROL_SCAN_NEXT);
      Consumer.release();
      break;

    case 5:
      winRun("excel");
      break;

    case 6:
      winRun("notepad");
      break;

    case 7:
      combo(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_ESC);
      break;

    case 8:
      Consumer.press(CONSUMER_CONTROL_SCAN_PREVIOUS);
      Consumer.release();
      break;

    case 9:
      combo(KEY_LEFT_GUI, 0, 'e');
      break;

    case 10:
      combo(KEY_LEFT_GUI, KEY_LEFT_SHIFT, 's');
      break;

    case 11:
      combo(KEY_LEFT_GUI, 0, 'l');
      break;

    case 12:
      Consumer.press(CONSUMER_CONTROL_MUTE);
      Consumer.release();
      break;

    case 13:
      combo(KEY_LEFT_CTRL, 0, 'c');
      break;

    case 14:
      Consumer.press(CONSUMER_CONTROL_PLAY_PAUSE);
      Consumer.release();
      break;

    case 15:
      combo(KEY_LEFT_CTRL, 0, 'v');
      break;

    case 16:
      combo(KEY_LEFT_GUI, 0, 'd');
      break;
  }
}

void scanMatrix() {

  unsigned long now = millis();

  for(int r = 0; r < ROWS; r++) {

    digitalWrite(rowPins[r], LOW);

    delayMicroseconds(10);

    for(int c = 0; c < COLS; c++) {

      bool pressed = (digitalRead(colPins[c]) == LOW);

      if(pressed != rawState[r][c]) {

        lastDebounce[r][c] = now;
        rawState[r][c] = pressed;
      }

      if((now - lastDebounce[r][c]) >= DEBOUNCE_MS) {

        if(pressed && !prevState[r][c]) {

          runAction(keys[r][c]);
        }

        prevState[r][c] = pressed;
      }
    }

    digitalWrite(rowPins[r], HIGH);
  }
}

void setup() {

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  lastStateCLK = digitalRead(CLK);

  analogReadResolution(12);

  for(int r = 0; r < ROWS; r++) {

    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }

  for(int c = 0; c < COLS; c++) {

    pinMode(colPins[c], INPUT_PULLUP);
  }

  USB.begin();

  Mouse.begin();
  Consumer.begin();
  Keyboard.begin();

  delay(2000);
}

void loop() {

  int currentCLK = digitalRead(CLK);

  if(currentCLK != lastStateCLK) {

    if(digitalRead(DT) != currentCLK) {

      Mouse.move(0, 0, 1);

    } else {

      Mouse.move(0, 0, -1);
    }

    delay(2);
  }

  lastStateCLK = currentCLK;

  int rawVolume = analogRead(POT_VOLUME);

  int volume = map(rawVolume, 0, 4095, 0, 100);

  if(abs(volume - lastVolume) > 2) {

    if(lastVolume == -1) {

      lastVolume = volume;

    } else if(volume > lastVolume) {

      for(int i = 0; i < (volume - lastVolume); i++) {

        Consumer.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
        Consumer.release();

        delay(2);
      }

    } else {

      for(int i = 0; i < (lastVolume - volume); i++) {

        Consumer.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
        Consumer.release();

        delay(2);
      }
    }

    lastVolume = volume;
  }

  int rawBrightness = analogRead(POT_BRIGHTNESS);

  int brightness = map(rawBrightness, 0, 4095, 0, 100);

  if(abs(brightness - lastBrightness) > 2) {

    if(lastBrightness == -1) {

      lastBrightness = brightness;

    } else if(brightness > lastBrightness) {

      for(int i = 0; i < (brightness - lastBrightness); i++) {

        Consumer.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
        Consumer.release();

        delay(2);
      }

    } else {

      for(int i = 0; i < (lastBrightness - brightness); i++) {

        Consumer.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
        Consumer.release();

        delay(2);
      }
    }

    lastBrightness = brightness;
  }

  int rawZoom = analogRead(POT_ZOOM);

  int zoom = map(rawZoom, 0, 4095, 0, 100);

  if(abs(zoom - lastZoom) > 2) {

    if(lastZoom == -1) {

      lastZoom = zoom;

    } else if(zoom > lastZoom) {

      Keyboard.press(KEY_LEFT_CTRL);

      Mouse.move(0, 0, 1);

      Keyboard.releaseAll();

    } else {

      Keyboard.press(KEY_LEFT_CTRL);

      Mouse.move(0, 0, -1);

      Keyboard.releaseAll();
    }

    lastZoom = zoom;
  }

  bool currentButtonState = digitalRead(SW);

  if(lastButtonState == HIGH && currentButtonState == LOW) {

    winRun("devmgmt.msc");
  }

  lastButtonState = currentButtonState;

  scanMatrix();

  delay(5);
}
