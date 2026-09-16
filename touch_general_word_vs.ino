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
  PROFILE_WORD,
  PROFILE_VSCODE
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
// VS CODE PAGE
// ============================================================

void showVSCodePage() {

  nextionCommand("page vscode");

  delay(50);

  nextionText("tProfile", "VS CODE");
}

// ============================================================
// PROFILE
// ============================================================

void setProfile(Profile profile) {

  currentProfile = profile;

  if (currentProfile == PROFILE_GENERAL) {

    Serial.println("PROFILE -> GENERAL");

    showGeneralPage();
  }

  else if (currentProfile == PROFILE_WORD) {

    Serial.println("PROFILE -> WORD");

    showWordPage();
  }

  else if (currentProfile == PROFILE_VSCODE) {

    Serial.println("PROFILE -> VS CODE");

    showVSCodePage();
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

    case 1:

      Serial.println("NEW EQUATION");

      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press('=');

      delay(100);

      Keyboard.releaseAll();

      break;

    case 2:

      Serial.println("Derivative");

      Keyboard.print("d/dx");

      break;

    case 3:

      Serial.println("Second Derivative");

      Keyboard.print("d^2/dx^2");

      break;

    case 4:

      Serial.println("Integral");

      Keyboard.print("int");

      break;

    case 5:

      Serial.println("Definite Integral");

      Keyboard.print("int_a^b");

      break;

    case 6:

      Serial.println("Limit");

      Keyboard.print("lim");

      break;

    case 7:

      Serial.println("Summation");

      Keyboard.print("sum");

      break;

    case 8:

      Serial.println("Fraction");

      Keyboard.print("a/b");

      break;

    case 9:

      Serial.println("Square Root");

      Keyboard.print("sqrt(x)");

      break;

    case 10:

      Serial.println("Superscript");

      Keyboard.print("^");

      break;

    case 11:

      Serial.println("Subscript");

      Keyboard.print("_");

      break;

    case 12:

      Serial.println("Pi");

      Keyboard.print("pi");

      break;

    case 13:

      Serial.println("Theta");

      Keyboard.print("theta");

      break;

    case 14:

      Serial.println("Multiply");

      Keyboard.print("*");

      break;

    case 15:

      Serial.println("Divide");

      Keyboard.print("/");

      break;

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
// VS CODE PROFILE ACTIONS
// ============================================================

void runVSCodeAction(int key) {

  Serial.print("VS CODE ACTION -> KEY ");
  Serial.println(key);

  switch (key) {

    // ========================================================
    // 1 - OPEN VS CODE
    // ========================================================

    case 1:

      Serial.println("Open VS Code");

      winRun("code");

      break;


    // ========================================================
    // 2 - ESP32 BASIC SKELETON
    // ========================================================

    case 2:

      Serial.println("ESP32 setup/loop skeleton");

      Keyboard.print(
        "#include <Arduino.h>\n"
        "\n"
        "void setup() {\n"
        "  Serial.begin(115200);\n"
        "}\n"
        "\n"
        "void loop() {\n"
        "  \n"
        "}\n"
      );

      break;


    // ========================================================
    // 3 - GPIO SETUP
    // ========================================================

    case 3:

      Serial.println("GPIO setup");

      Keyboard.print(
        "// GPIO setup\n"
        "const int LED_PIN = 2;\n"
        "\n"
        "void setup() {\n"
        "  pinMode(LED_PIN, OUTPUT);\n"
        "  digitalWrite(LED_PIN, LOW);\n"
        "}\n"
      );

      break;


    // ========================================================
    // 4 - GPIO INTERRUPT
    // ========================================================

    case 4:

      Serial.println("GPIO interrupt template");

      Keyboard.print(
        "volatile bool interruptFlag = false;\n"
        "\n"
        "void IRAM_ATTR handleInterrupt() {\n"
        "  interruptFlag = true;\n"
        "}\n"
        "\n"
        "void setup() {\n"
        "  pinMode(4, INPUT_PULLUP);\n"
        "  attachInterrupt(digitalPinToInterrupt(4), handleInterrupt, FALLING);\n"
        "}\n"
      );

      break;


    // ========================================================
    // 5 - ADC
    // ========================================================

    case 5:

      Serial.println("ADC template");

      Keyboard.print(
        "// ADC\n"
        "const int ADC_PIN = 1;\n"
        "\n"
        "void setup() {\n"
        "  analogReadResolution(12);\n"
        "}\n"
        "\n"
        "void loop() {\n"
        "  int raw = analogRead(ADC_PIN);\n"
        "  float voltage = (raw / 4095.0) * 3.3;\n"
        "}\n"
      );

      break;


    // ========================================================
    // 6 - PWM
    // ========================================================

    case 6:

      Serial.println("PWM template");

      Keyboard.print(
        "// PWM\n"
        "const int PWM_PIN = 2;\n"
        "const int PWM_FREQ = 5000;\n"
        "const int PWM_RESOLUTION = 8;\n"
        "\n"
        "void setup() {\n"
        "  ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);\n"
        "  ledcWrite(PWM_PIN, 128);\n"
        "}\n"
      );

      break;


    // ========================================================
    // 7 - HARDWARE UART
    // ========================================================

    case 7:

      Serial.println("HardwareSerial UART");

      Keyboard.print(
        "// Hardware UART\n"
        "HardwareSerial MySerial(1);\n"
        "\n"
        "void setup() {\n"
        "  MySerial.begin(115200, SERIAL_8N1, 16, 17);\n"
        "}\n"
        "\n"
        "void loop() {\n"
        "  if (MySerial.available()) {\n"
        "    uint8_t data = MySerial.read();\n"
        "  }\n"
        "}\n"
      );

      break;


    // ========================================================
    // 8 - I2C
    // ========================================================

    case 8:

      Serial.println("I2C template");

      Keyboard.print(
        "#include <Wire.h>\n"
        "\n"
        "void setup() {\n"
        "  Wire.begin(8, 9);\n"
        "}\n"
        "\n"
        "void loop() {\n"
        "}\n"
      );

      break;


    // ========================================================
    // 9 - SPI
    // ========================================================

    case 9:

      Serial.println("SPI template");

      Keyboard.print(
        "#include <SPI.h>\n"
        "\n"
        "SPIClass MySPI(FSPI);\n"
        "\n"
        "void setup() {\n"
        "  MySPI.begin();\n"
        "}\n"
        "\n"
        "void loop() {\n"
        "}\n"
      );

      break;


    // ========================================================
    // 10 - FREERTOS TASK
    // ========================================================

    case 10:

      Serial.println("FreeRTOS task");

      Keyboard.print(
        "void taskFunction(void *parameter) {\n"
        "  while (true) {\n"
        "    \n"
        "    vTaskDelay(pdMS_TO_TICKS(1000));\n"
        "  }\n"
        "}\n"
        "\n"
        "void setup() {\n"
        "  xTaskCreate(\n"
        "    taskFunction,\n"
        "    \"MyTask\",\n"
        "    4096,\n"
        "    NULL,\n"
        "    1,\n"
        "    NULL\n"
        "  );\n"
        "}\n"
      );

      break;


    // ========================================================
    // 11 - FREERTOS QUEUE
    // ========================================================

    case 11:

      Serial.println("FreeRTOS queue");

      Keyboard.print(
        "#include <Arduino.h>\n"
        "\n"
        "QueueHandle_t dataQueue;\n"
        "\n"
        "void producerTask(void *parameter) {\n"
        "  int value;\n"
        "\n"
        "  while (true) {\n"
        "    value++;\n"
        "    xQueueSend(dataQueue, &value, portMAX_DELAY);\n"
        "    vTaskDelay(pdMS_TO_TICKS(1000));\n"
        "  }\n"
        "}\n"
        "\n"
        "void consumerTask(void *parameter) {\n"
        "  int value;\n"
        "\n"
        "  while (true) {\n"
        "    if (xQueueReceive(dataQueue, &value, portMAX_DELAY)) {\n"
        "      Serial.println(value);\n"
        "    }\n"
        "  }\n"
        "}\n"
      );

      break;


    // ========================================================
    // 12 - TIMER CALLBACK
    // ========================================================

    case 12:

      Serial.println("Timer callback");

      Keyboard.print(
        "hw_timer_t *timer = NULL;\n"
        "volatile bool timerFlag = false;\n"
        "\n"
        "void IRAM_ATTR onTimer() {\n"
        "  timerFlag = true;\n"
        "}\n"
        "\n"
        "void setup() {\n"
        "  timer = timerBegin(1000000);\n"
        "  timerAttachInterrupt(timer, &onTimer);\n"
        "  timerAlarm(timer, 1000000, true, 0);\n"
        "}\n"
      );

      break;


    // ========================================================
    // 13 - STRUCT + OBJECT
    // ========================================================

    case 13:

      Serial.println("Struct/object template");

      Keyboard.print(
        "struct SensorData {\n"
        "  float voltage;\n"
        "  float current;\n"
        "  float temperature;\n"
        "};\n"
        "\n"
        "SensorData sensor;\n"
        "\n"
        "void readSensor() {\n"
        "  sensor.voltage = 0.0;\n"
        "  sensor.current = 0.0;\n"
        "  sensor.temperature = 0.0;\n"
        "}\n"
      );

      break;


    // ========================================================
    // 14 - C++ CLASS
    // ========================================================

    case 14:

      Serial.println("C++ class template");

      Keyboard.print(
        "class Device {\n"
        "private:\n"
        "  int pin;\n"
        "\n"
        "public:\n"
        "  Device(int p) : pin(p) {}\n"
        "\n"
        "  void begin() {\n"
        "    pinMode(pin, OUTPUT);\n"
        "  }\n"
        "\n"
        "  void on() {\n"
        "    digitalWrite(pin, HIGH);\n"
        "  }\n"
        "\n"
        "  void off() {\n"
        "    digitalWrite(pin, LOW);\n"
        "  }\n"
        "};\n"
        "\n"
        "Device device(2);\n"
      );

      break;


    // ========================================================
    // 15 - DEBUG BLOCK
    // ========================================================

    case 15:

      Serial.println("Debug serial block");

      Keyboard.print(
        "Serial.println(\"--------------------\");\n"
        "Serial.print(\"Value: \");\n"
        "Serial.println(value);\n"
        "Serial.print(\"Time: \");\n"
        "Serial.println(millis());\n"
        "Serial.println(\"--------------------\");\n"
      );

      break;


    // ========================================================
    // 16 - COMPLETE ESP32 PROJECT
    // ========================================================

    case 16:

      Serial.println("Complete ESP32 project skeleton");

      Keyboard.print(
        "#include <Arduino.h>\n"
        "\n"
        "// ======================================================\n"
        "// PIN DEFINITIONS\n"
        "// ======================================================\n"
        "\n"
        "const int LED_PIN = 2;\n"
        "\n"
        "// ======================================================\n"
        "// GLOBAL VARIABLES\n"
        "// ======================================================\n"
        "\n"
        "unsigned long lastTime = 0;\n"
        "const unsigned long INTERVAL = 1000;\n"
        "\n"
        "// ======================================================\n"
        "// SETUP\n"
        "// ======================================================\n"
        "\n"
        "void setup() {\n"
        "\n"
        "  Serial.begin(115200);\n"
        "\n"
        "  pinMode(LED_PIN, OUTPUT);\n"
        "\n"
        "  Serial.println(\"System started\");\n"
        "}\n"
        "\n"
        "// ======================================================\n"
        "// LOOP\n"
        "// ======================================================\n"
        "\n"
        "void loop() {\n"
        "\n"
        "  unsigned long now = millis();\n"
        "\n"
        "  if (now - lastTime >= INTERVAL) {\n"
        "\n"
        "    lastTime = now;\n"
        "\n"
        "    digitalWrite(\n"
        "      LED_PIN,\n"
        "      !digitalRead(LED_PIN)\n"
        "    );\n"
        "\n"
        "    Serial.println(\"Running...\");\n"
        "  }\n"
        "}\n"
      );

      break;


    default:

      Serial.println("Invalid VS Code key");

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

  }

  else if (currentProfile == PROFILE_WORD) {

    runWordAction(key);

  }

  else if (currentProfile == PROFILE_VSCODE) {

    runVSCodeAction(key);
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
    // VS CODE PROFILE
    // ========================================================

    if (command == 0x50) {

      Serial.println(
        "NEXTION -> VS CODE PROFILE"
      );

      setProfile(
        PROFILE_VSCODE
      );

      continue;
    }

    // ========================================================
    // GENERAL BUTTONS
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
    // VS CODE BUTTONS
    // ========================================================

    if (
      command >= 0x51 &&
      command <= 0x60
    ) {

      int key =
        command - 0x50;

      Serial.print(
        "NEXTION -> VS CODE KEY "
      );

      Serial.println(key);

      runVSCodeAction(key);

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
  // VS CODE
  // ----------------------------------------------------------

  if (
    input.equalsIgnoreCase(
      "vscode"
    )
  ) {

    setProfile(
      PROFILE_VSCODE
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

    // VS Code profile
    if (value == 0x50) {

      setProfile(
        PROFILE_VSCODE
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

    // VS Code buttons
    if (
      value >= 0x51 &&
      value <= 0x60
    ) {

      runVSCodeAction(
        (int)(value - 0x50)
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

  // VS Code profile
  if (value == 50) {

    setProfile(
      PROFILE_VSCODE
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

  // VS Code buttons 51-66
  if (
    value >= 51 &&
    value <= 66
  ) {

    runVSCodeAction(
      value - 50
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
    "50 = VS CODE PROFILE"
  );

  Serial.println(
    "51-60 = VS CODE BUTTONS"
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
