#include <Arduino.h>
#include <USB.h>
#include <USBHIDMouse.h>
#include <USBHIDConsumerControl.h>
#include <USBHIDKeyboard.h>

// ============================================================
// USB HID
// ============================================================

USBHIDMouse Mouse;
USBHIDConsumerControl Consumer;
USBHIDKeyboard Keyboard;

// ============================================================
// NEXTION
// ============================================================

HardwareSerial Nextion(0);

#define NEXTION_BAUD 9600

// ESP32-S3 UART0 pins
#define NEXTION_RX 44
#define NEXTION_TX 43

// ============================================================
// ROTARY ENCODER
// ============================================================

#define ENCODER_CLK 4
#define ENCODER_DT  5
#define ENCODER_SW  6

// ============================================================
// POTENTIOMETERS
// ============================================================

#define POT_VOLUME     7
#define POT_BRIGHTNESS 1
#define POT_ZOOM       2

// ============================================================
// MATRIX
// ============================================================

const int ROWS = 4;
const int COLS = 4;

const int rowPins[ROWS] = {
  38, 39, 40, 41
};

const int colPins[COLS] = {
  42, 8, 17, 18
};

const int keyMap[ROWS][COLS] = {
  { 1,  2,  3,  4},
  { 5,  6,  7,  8},
  { 9, 10, 11, 12},
  {13, 14, 15, 16}
};

// ============================================================
// MATRIX STATE
// ============================================================

bool keyRawState[ROWS][COLS] = {};
bool keyStableState[ROWS][COLS] = {};
unsigned long keyDebounceTime[ROWS][COLS] = {};

const unsigned long DEBOUNCE_MS = 15;

// ============================================================
// ENCODER STATE
// ============================================================

int lastEncoderCLK = HIGH;
bool lastEncoderButton = HIGH;

// ============================================================
// POT STATE
// ============================================================

int lastVolume = -1;
int lastBrightness = -1;
int lastZoom = -1;

const int POT_THRESHOLD = 2;

// ============================================================
// PROFILE
// ============================================================

enum Profile {
  PROFILE_GENERAL,
  PROFILE_WORD
};

Profile currentProfile = PROFILE_GENERAL;

// ============================================================
// NEXTION HELPERS
// ============================================================

void nextionEnd() {

  Nextion.write(0xFF);
  Nextion.write(0xFF);
  Nextion.write(0xFF);
}

void nextionCommand(const char *command) {

  Nextion.print(command);
  nextionEnd();
}

void nextionText(const char *component, const char *text) {

  Nextion.print(component);
  Nextion.print(".txt=\"");
  Nextion.print(text);
  Nextion.print("\"");

  nextionEnd();
}

// ============================================================
// GENERAL PAGE
// ============================================================

void showGeneralPage() {

  nextionCommand("page general");

  delay(50);

  nextionText("tProfile", "GENERAL");
}

// ============================================================
// WORD PAGE
// ============================================================

void showWordPage() {

  nextionCommand("page word");

  delay(50);

  nextionText("tProfile", "WORD");
}

// ============================================================
// PROFILE
// ============================================================

void setProfile(Profile profile) {

  currentProfile = profile;

  if (currentProfile == PROFILE_GENERAL) {

    Serial.println("PROFILE -> GENERAL");

    showGeneralPage();

  } else if (currentProfile == PROFILE_WORD) {

    Serial.println("PROFILE -> WORD");

    showWordPage();
  }
}

// ============================================================
// WINDOWS RUN COMMAND
// ============================================================

void winRun(const char *command) {

  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');

  delay(100);

  Keyboard.releaseAll();

  delay(400);

  Keyboard.print(command);

  delay(100);

  Keyboard.write(KEY_RETURN);

  delay(300);
}

// ============================================================
// KEYBOARD COMBINATION
// ============================================================

void combo(
  uint8_t modifier1,
  uint8_t modifier2,
  uint8_t key
) {

  Keyboard.press(modifier1);

  if (modifier2 != 0) {
    Keyboard.press(modifier2);
  }

  if (key != 0) {
    Keyboard.press(key);
  }

  delay(80);

  Keyboard.releaseAll();

  delay(50);
}

// ============================================================
// GENERAL PROFILE ACTIONS
// ============================================================

void runGeneralAction(int key) {

  Serial.print("GENERAL ACTION -> KEY ");
  Serial.println(key);

  switch (key) {

    case 1:

      Serial.println("Calculator");

      winRun("calc");

      break;

    case 2:

      Serial.println("Microsoft Word");

      winRun("winword");

      break;

    case 3:

      Serial.println("PowerPoint");

      winRun("powerpnt");

      break;

    case 4:

      Serial.println("Next Track");

      Consumer.press(
        CONSUMER_CONTROL_SCAN_NEXT
      );

      Consumer.release();

      break;

    case 5:

      Serial.println("Microsoft Excel");

      winRun("excel");

      break;

    case 6:

      Serial.println("Notepad");

      winRun("notepad");

      break;

    case 7:

      Serial.println("Task Manager");

      combo(
        KEY_LEFT_CTRL,
        KEY_LEFT_SHIFT,
        KEY_ESC
      );

      break;

    case 8:

      Serial.println("Previous Track");

      Consumer.press(
        CONSUMER_CONTROL_SCAN_PREVIOUS
      );

      Consumer.release();

      break;

    case 9:

      Serial.println("File Explorer");

      combo(
        KEY_LEFT_GUI,
        0,
        'e'
      );

      break;

    case 10:

      Serial.println("Screenshot");

      combo(
        KEY_LEFT_GUI,
        KEY_LEFT_SHIFT,
        's'
      );

      break;

    case 11:

      Serial.println("Lock PC");

      combo(
        KEY_LEFT_GUI,
        0,
        'l'
      );

      break;

    case 12:

      Serial.println("Mute");

      Consumer.press(
        CONSUMER_CONTROL_MUTE
      );

      Consumer.release();

      break;

    case 13:

      Serial.println("Copy");

      combo(
        KEY_LEFT_CTRL,
        0,
        'c'
      );

      break;

    case 14:

      Serial.println("Play/Pause");

      Consumer.press(
        CONSUMER_CONTROL_PLAY_PAUSE
      );

      Consumer.release();

      break;

    case 15:

      Serial.println("Paste");

      combo(
        KEY_LEFT_CTRL,
        0,
        'v'
      );

      break;

    case 16:

      Serial.println("Show Desktop");

      combo(
        KEY_LEFT_GUI,
        0,
        'd'
      );

      break;

    default:

      Serial.println("Invalid General key");

      break;
  }
}

// ============================================================
// WORD PROFILE ACTIONS
// ============================================================

void runWordAction(int key) {

  Serial.print("WORD ACTION -> KEY ");
  Serial.println(key);

  switch (key) {

    // --------------------------------------------------------
    // 1 - NEW EQUATION
    // --------------------------------------------------------

    case 1:

      Serial.println("NEW EQUATION");

      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press('=');

      delay(100);

      Keyboard.releaseAll();

      break;

    // --------------------------------------------------------
    // 2 - DERIVATIVE
    // --------------------------------------------------------

    case 2:

      Serial.println("Derivative");

      Keyboard.print("d/dx");

      break;

    // --------------------------------------------------------
    // 3 - SECOND DERIVATIVE
    // --------------------------------------------------------

    case 3:

      Serial.println("Second Derivative");

      Keyboard.print("d^2/dx^2");

      break;

    // --------------------------------------------------------
    // 4 - INTEGRAL
    // --------------------------------------------------------

    case 4:

      Serial.println("Integral");

      Keyboard.print("int");

      break;

    // --------------------------------------------------------
    // 5 - DEFINITE INTEGRAL
    // --------------------------------------------------------

    case 5:

      Serial.println("Definite Integral");

      Keyboard.print("int_a^b");

      break;

    // --------------------------------------------------------
    // 6 - LIMIT
    // --------------------------------------------------------

    case 6:

      Serial.println("Limit");

      Keyboard.print("lim");

      break;

    // --------------------------------------------------------
    // 7 - SUMMATION
    // --------------------------------------------------------

    case 7:

      Serial.println("Summation");

      Keyboard.print("sum");

      break;

    // --------------------------------------------------------
    // 8 - FRACTION
    // --------------------------------------------------------

    case 8:

      Serial.println("Fraction");

      Keyboard.print("a/b");

      break;

    // --------------------------------------------------------
    // 9 - SQUARE ROOT
    // --------------------------------------------------------

    case 9:

      Serial.println("Square Root");

      Keyboard.print("sqrt(x)");

      break;

    // --------------------------------------------------------
    // 10 - SUPERSCRIPT
    // --------------------------------------------------------

    case 10:

      Serial.println("Superscript");

      Keyboard.print("^");

      break;

    // --------------------------------------------------------
    // 11 - SUBSCRIPT
    // --------------------------------------------------------

    case 11:

      Serial.println("Subscript");

      Keyboard.print("_");

      break;

    // --------------------------------------------------------
    // 12 - PI
    // --------------------------------------------------------

    case 12:

      Serial.println("Pi");

      Keyboard.print("pi");

      break;

    // --------------------------------------------------------
    // 13 - THETA
    // --------------------------------------------------------

    case 13:

      Serial.println("Theta");

      Keyboard.print("theta");

      break;

    // --------------------------------------------------------
    // 14 - MULTIPLY
    // --------------------------------------------------------

    case 14:

      Serial.println("Multiply");

      Keyboard.print("*");

      break;

    // --------------------------------------------------------
    // 15 - DIVIDE
    // --------------------------------------------------------

    case 15:

      Serial.println("Divide");

      Keyboard.print("/");

      break;

    // --------------------------------------------------------
    // 16 - PLUS/MINUS
    // --------------------------------------------------------

    case 16:

      Serial.println("Plus/Minus");

      Keyboard.print("+-");

      break;

    default:

      Serial.println("Invalid Word key");

      break;
  }
}

// ============================================================
// GENERIC PHYSICAL KEY ACTION
// ============================================================

void runAction(int key) {

  if (key < 1 || key > 16) {
    return;
  }

  if (currentProfile == PROFILE_GENERAL) {

    runGeneralAction(key);

  } else {

    runWordAction(key);
  }
}

// ============================================================
// MATRIX SCAN
// ============================================================

void scanMatrix() {

  unsigned long now = millis();

  for (int r = 0; r < ROWS; r++) {

    for (int i = 0; i < ROWS; i++) {

      digitalWrite(
        rowPins[i],
        HIGH
      );
    }

    digitalWrite(
      rowPins[r],
      LOW
    );

    delayMicroseconds(20);

    for (int c = 0; c < COLS; c++) {

      bool pressed =
        digitalRead(colPins[c]) == LOW;

      if (pressed != keyRawState[r][c]) {

        keyRawState[r][c] = pressed;

        keyDebounceTime[r][c] = now;
      }

      if (
        (now - keyDebounceTime[r][c])
        >= DEBOUNCE_MS
      ) {

        if (
          keyStableState[r][c] != pressed
        ) {

          keyStableState[r][c] = pressed;

          if (pressed) {

            int key = keyMap[r][c];

            Serial.print(
              "PHYSICAL KEY -> "
            );

            Serial.println(key);

            runAction(key);
          }
        }
      }
    }
  }

  for (int i = 0; i < ROWS; i++) {

    digitalWrite(
      rowPins[i],
      HIGH
    );
  }
}

// ============================================================
// ROTARY ENCODER
// ============================================================

void handleEncoder() {

  int currentCLK =
    digitalRead(ENCODER_CLK);

  if (currentCLK != lastEncoderCLK) {

    if (currentCLK == LOW) {

      if (
        digitalRead(ENCODER_DT)
        != currentCLK
      ) {

        Mouse.move(
          0,
          0,
          1
        );

        Serial.println(
          "ENCODER -> SCROLL UP"
        );

      } else {

        Mouse.move(
          0,
          0,
          -1
        );

        Serial.println(
          "ENCODER -> SCROLL DOWN"
        );
      }
    }
  }

  lastEncoderCLK = currentCLK;

  bool currentButton =
    digitalRead(ENCODER_SW);

  if (
    lastEncoderButton == HIGH &&
    currentButton == LOW
  ) {

    Serial.println(
      "ENCODER BUTTON -> DEVICE MANAGER"
    );

    winRun("devmgmt.msc");
  }

  lastEncoderButton = currentButton;
}

// ============================================================
// VOLUME POT
// ============================================================

void handleVolumePot() {

  int raw =
    analogRead(POT_VOLUME);

  int value =
    map(raw, 0, 4095, 0, 100);

  if (lastVolume < 0) {

    lastVolume = value;

    return;
  }

  int difference =
    value - lastVolume;

  if (
    abs(difference)
    >= POT_THRESHOLD
  ) {

    if (difference > 0) {

      for (
        int i = 0;
        i < difference;
        i++
      ) {

        Consumer.press(
          CONSUMER_CONTROL_VOLUME_INCREMENT
        );

        Consumer.release();
      }

    } else {

      for (
        int i = 0;
        i < -difference;
        i++
      ) {

        Consumer.press(
          CONSUMER_CONTROL_VOLUME_DECREMENT
        );

        Consumer.release();
      }
    }

    lastVolume = value;
  }
}

// ============================================================
// BRIGHTNESS POT
// ============================================================

void handleBrightnessPot() {

  int raw =
    analogRead(POT_BRIGHTNESS);

  int value =
    map(raw, 0, 4095, 0, 100);

  if (lastBrightness < 0) {

    lastBrightness = value;

    return;
  }

  int difference =
    value - lastBrightness;

  if (
    abs(difference)
    >= POT_THRESHOLD
  ) {

    if (difference > 0) {

      for (
        int i = 0;
        i < difference;
        i++
      ) {

        Consumer.press(
          CONSUMER_CONTROL_BRIGHTNESS_INCREMENT
        );

        Consumer.release();
      }

    } else {

      for (
        int i = 0;
        i < -difference;
        i++
      ) {

        Consumer.press(
          CONSUMER_CONTROL_BRIGHTNESS_DECREMENT
        );

        Consumer.release();
      }
    }

    lastBrightness = value;
  }
}

// ============================================================
// ZOOM POT
// ============================================================

void handleZoomPot() {

  int raw =
    analogRead(POT_ZOOM);

  int value =
    map(raw, 0, 4095, 0, 100);

  if (lastZoom < 0) {

    lastZoom = value;

    return;
  }

  int difference =
    value - lastZoom;

  if (
    abs(difference)
    >= POT_THRESHOLD
  ) {

    Keyboard.press(
      KEY_LEFT_CTRL
    );

    if (difference > 0) {

      Mouse.move(
        0,
        0,
        1
      );

    } else {

      Mouse.move(
        0,
        0,
        -1
      );
    }

    Keyboard.releaseAll();

    lastZoom = value;
  }
}

// ============================================================
// NEXTION RECEIVE
// ============================================================

void processNextion() {

  while (Nextion.available()) {

    uint8_t command =
      Nextion.read();

    Serial.print(
      "NEXTION -> 0x"
    );

    if (command < 0x10) {
      Serial.print("0");
    }

    Serial.println(
      command,
      HEX
    );

    // ========================================================
    // GENERAL PROFILE
    // 0x20
    // ========================================================

    if (command == 0x20) {

      Serial.println(
        "NEXTION -> GENERAL PROFILE"
      );

      setProfile(
        PROFILE_GENERAL
      );

      continue;
    }

    // ========================================================
    // WORD PROFILE
    // 0x30
    // ========================================================

    if (command == 0x30) {

      Serial.println(
        "NEXTION -> WORD PROFILE"
      );

      setProfile(
        PROFILE_WORD
      );

      continue;
    }

    // ========================================================
    // GENERAL BUTTONS
    // 0x01 - 0x10
    // ========================================================

    if (
      command >= 0x01 &&
      command <= 0x10
    ) {

      int key = command;

      Serial.print(
        "NEXTION -> GENERAL KEY "
      );

      Serial.println(key);

      runGeneralAction(key);

      continue;
    }

    // ========================================================
    // WORD BUTTONS
    // 0x31 - 0x40
    // ========================================================

    if (
      command >= 0x31 &&
      command <= 0x40
    ) {

      int key =
        command - 0x30;

      Serial.print(
        "NEXTION -> WORD KEY "
      );

      Serial.println(key);

      runWordAction(key);

      continue;
    }

    // ========================================================
    // UNKNOWN
    // ========================================================

    Serial.println(
      "UNKNOWN NEXTION COMMAND"
    );
  }
}

// ============================================================
// SERIAL COMMANDS
// ============================================================

void processSerial() {

  if (!Serial.available()) {
    return;
  }

  String input =
    Serial.readStringUntil('\n');

  input.trim();

  if (input.length() == 0) {
    return;
  }

  // ----------------------------------------------------------
  // GENERAL
  // ----------------------------------------------------------

  if (
    input.equalsIgnoreCase(
      "general"
    )
  ) {

    setProfile(
      PROFILE_GENERAL
    );

    return;
  }

  // ----------------------------------------------------------
  // WORD
  // ----------------------------------------------------------

  if (
    input.equalsIgnoreCase(
      "word"
    )
  ) {

    setProfile(
      PROFILE_WORD
    );

    return;
  }

  // ----------------------------------------------------------
  // HEX COMMANDS
  // ----------------------------------------------------------

  if (
    input.startsWith("0x") ||
    input.startsWith("0X")
  ) {

    long value =
      strtol(
        input.c_str(),
        nullptr,
        16
      );

    // General profile
    if (value == 0x20) {

      setProfile(
        PROFILE_GENERAL
      );

      return;
    }

    // Word profile
    if (value == 0x30) {

      setProfile(
        PROFILE_WORD
      );

      return;
    }

    // General buttons
    if (
      value >= 0x01 &&
      value <= 0x10
    ) {

      runGeneralAction(
        (int)value
      );

      return;
    }

    // Word buttons
    if (
      value >= 0x31 &&
      value <= 0x40
    ) {

      runWordAction(
        (int)(value - 0x30)
      );

      return;
    }

    Serial.println(
      "Invalid hexadecimal command"
    );

    return;
  }

  // ----------------------------------------------------------
  // DECIMAL COMMANDS
  // ----------------------------------------------------------

  int value =
    input.toInt();

  // General profile
  if (value == 20) {

    setProfile(
      PROFILE_GENERAL
    );

    return;
  }

  // Word profile
  if (value == 30) {

    setProfile(
      PROFILE_WORD
    );

    return;
  }

  // General buttons 1-16
  if (
    value >= 1 &&
    value <= 16
  ) {

    runGeneralAction(value);

    return;
  }

  // Word buttons 31-46
  if (
    value >= 31 &&
    value <= 46
  ) {

    runWordAction(
      value - 30
    );

    return;
  }

  Serial.println(
    "Invalid command."
  );
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  // ----------------------------------------------------------
  // USB CDC Serial
  // ----------------------------------------------------------

  Serial.begin(115200);

  // ----------------------------------------------------------
  // Nextion UART
  // ----------------------------------------------------------

  Nextion.begin(
    NEXTION_BAUD,
    SERIAL_8N1,
    NEXTION_RX,
    NEXTION_TX
  );

  // ----------------------------------------------------------
  // Matrix
  // ----------------------------------------------------------

  for (int r = 0; r < ROWS; r++) {

    pinMode(
      rowPins[r],
      OUTPUT
    );

    digitalWrite(
      rowPins[r],
      HIGH
    );
  }

  for (int c = 0; c < COLS; c++) {

    pinMode(
      colPins[c],
      INPUT_PULLUP
    );
  }

  // ----------------------------------------------------------
  // Encoder
  // ----------------------------------------------------------

  pinMode(
    ENCODER_CLK,
    INPUT
  );

  pinMode(
    ENCODER_DT,
    INPUT
  );

  pinMode(
    ENCODER_SW,
    INPUT_PULLUP
  );

  // ----------------------------------------------------------
  // ADC
  // ----------------------------------------------------------

  analogReadResolution(12);

  // ----------------------------------------------------------
  // USB HID
  // ----------------------------------------------------------

  USB.begin();

  Mouse.begin();
  Consumer.begin();
  Keyboard.begin();

  // ----------------------------------------------------------
  // Initial states
  // ----------------------------------------------------------

  lastEncoderCLK =
    digitalRead(
      ENCODER_CLK
    );

  lastEncoderButton =
    digitalRead(
      ENCODER_SW
    );

  // ----------------------------------------------------------
  // Startup
  // ----------------------------------------------------------

  delay(2000);

  Serial.println();
  Serial.println(
    "=============================="
  );

  Serial.println(
    "KEYFORGE ESP32-S3 CONTROLLER"
  );

  Serial.println(
    "=============================="
  );

  Serial.println(
    "USB CDC: READY"
  );

  Serial.println(
    "USB HID: READY"
  );

  Serial.println(
    "NEXTION: 9600 BAUD"
  );

  Serial.println(
    "MATRIX: READY"
  );

  Serial.println(
    "ENCODER: READY"
  );

  Serial.println(
    "POTS: READY"
  );

  Serial.println(
    "=============================="
  );

  Serial.println(
    "NEXTION PROTOCOL:"
  );

  Serial.println(
    "01-10 = GENERAL BUTTONS"
  );

  Serial.println(
    "20 = GENERAL PROFILE"
  );

  Serial.println(
    "30 = WORD PROFILE"
  );

  Serial.println(
    "31-40 = WORD BUTTONS"
  );

  Serial.println(
    "=============================="
  );

  // Start on home page
  nextionCommand("page home");
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  processNextion();

  processSerial();

  scanMatrix();

  handleEncoder();

  handleVolumePot();

  handleBrightnessPot();

  handleZoomPot();

  delay(2);
}
