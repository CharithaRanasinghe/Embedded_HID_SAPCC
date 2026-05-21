#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDConsumerControl.h"
#include "USBHIDKeyboard.h"

USBHIDMouse    Mouse;
USBHIDConsumerControl Consumer;
USBHIDKeyboard Keyboard;

// ── Rotary encoder ──────────────────────────────────────────
#define CLK     4
#define DT      5
#define SW      6
#define POT_PIN 7

int  lastStateCLK;
int  lastVolume      = -1;
bool lastButtonState = HIGH;

// ── 4×4 Matrix ─────────────────────────────────────────────
const int ROWS = 4;
const int COLS = 4;

int rowPins[ROWS] = {38, 39, 40, 41};   // OUTPUT — left side, 4 adjacent
int colPins[COLS] = {42,  8, 17, 18};   // INPUT_PULLUP — debounce caps on these

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

bool          prevState[ROWS][COLS]     = {};
bool          rawState[ROWS][COLS]      = {};
unsigned long lastDebounce[ROWS][COLS]  = {};
const unsigned long DEBOUNCE_MS         = 15;

// ── Helpers ─────────────────────────────────────────────────

// Win+R → type command → Enter
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

// Simple combo: e.g. Ctrl+Shift+Esc
void combo(uint8_t mod1, uint8_t mod2, uint8_t key) {
  Keyboard.press(mod1);
  if (mod2) Keyboard.press(mod2);
  if (key)  Keyboard.press(key);
  delay(100);
  Keyboard.releaseAll();
}

// ── Key actions ─────────────────────────────────────────────
void runAction(char key) {
  switch (key) {

    // ── App launchers (Win+R) ──
    case '1': winRun("calc");       break;  // Calculator
    case '2': winRun("wps");        break;  // WPS Word
    case '3': winRun("et");         break;  // WPS Sheets
    case '4': winRun("wpp");        break;  // WPS Presentation
    case '6': winRun("notepad");    break;  // Notepad

    // ── Shortcuts ──
    case '5':  // File Explorer  Win+E
      combo(KEY_LEFT_GUI, 0, 'e');
      break;

    case '7':  // Task Manager  Ctrl+Shift+Esc
      combo(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_ESC);
      break;

    case '8':  // Snip & Sketch  Win+Shift+S
      combo(KEY_LEFT_GUI, KEY_LEFT_SHIFT, 's');
      break;

    case '9':  // Lock screen  Win+L
      combo(KEY_LEFT_GUI, 0, 'l');
      break;

    case 'D':  // Show desktop  Win+D
      combo(KEY_LEFT_GUI, 0, 'd');
      break;

    case '*':  // Copy  Ctrl+C
      combo(KEY_LEFT_CTRL, 0, 'c');
      break;

    case '#':  // Paste  Ctrl+V
      combo(KEY_LEFT_CTRL, 0, 'v');
      break;

    // ── Media (Consumer HID — no keyboard needed) ──
    case '0':
      Consumer.press(CONSUMER_CONTROL_PLAY_PAUSE);
      Consumer.release();
      break;

    case 'A':
      Consumer.press(CONSUMER_CONTROL_SCAN_NEXT);
      Consumer.release();
      break;

    case 'B':
      Consumer.press(CONSUMER_CONTROL_SCAN_PREVIOUS);
      Consumer.release();
      break;

    case 'C':
      Consumer.press(CONSUMER_CONTROL_MUTE);
      Consumer.release();
      break;
  }
}

// ── Matrix scan (non-blocking debounce) ─────────────────────
void scanMatrix() {
  unsigned long now = millis();

  for (int r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(10);          // let col RC filter settle

    for (int c = 0; c < COLS; c++) {
      bool pressed = (digitalRead(colPins[c]) == LOW);

      if (pressed != rawState[r][c]) {
        lastDebounce[r][c] = now;
        rawState[r][c]     = pressed;
      }

      if ((now - lastDebounce[r][c]) >= DEBOUNCE_MS) {
        if (pressed && !prevState[r][c]) {
          runAction(keys[r][c]);
        }
        prevState[r][c] = pressed;
      }
    }

    digitalWrite(rowPins[r], HIGH);
  }
}

// ── Setup ───────────────────────────────────────────────────
void setup() {
  // Rotary encoder
  pinMode(CLK, INPUT);
  pinMode(DT,  INPUT);
  pinMode(SW,  INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK);
  analogReadResolution(12);

  // Matrix rows
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  // Matrix cols
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  USB.begin();
  Mouse.begin();
  Consumer.begin();
  Keyboard.begin();
  delay(2000);
}

// ── Main loop ───────────────────────────────────────────────
void loop() {

  // ── Encoder scroll ──────────────────────────────
  int currentCLK = digitalRead(CLK);
  if (currentCLK != lastStateCLK) {
    if (digitalRead(DT) != currentCLK) {
      Mouse.move(0, 0,  1);
    } else {
      Mouse.move(0, 0, -1);
    }
    delay(2);
  }
  lastStateCLK = currentCLK;

  // ── Potentiometer volume ─────────────────────────
  int raw    = analogRead(POT_PIN);
  int volume = map(raw, 0, 4095, 0, 100);

  if (abs(volume - lastVolume) > 2) {
    if (lastVolume == -1) {
      lastVolume = volume;
    } else if (volume > lastVolume) {
      for (int i = 0; i < (volume - lastVolume); i++) {
        Consumer.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
        Consumer.release();
        delay(2);
      }
    } else {
      for (int i = 0; i < (lastVolume - volume); i++) {
        Consumer.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
        Consumer.release();
        delay(2);
      }
    }
    lastVolume = volume;
  }

  // ── Encoder button → Device Manager ─────────────
  bool currentButtonState = digitalRead(SW);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    winRun("devmgmt.msc");
  }
  lastButtonState = currentButtonState;

  // ── Matrix keypad ────────────────────────────────
  scanMatrix();

  delay(5);
}
