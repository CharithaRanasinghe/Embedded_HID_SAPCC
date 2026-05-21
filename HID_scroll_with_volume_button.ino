#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDConsumerControl.h"
#include "USBHIDKeyboard.h"

// HID device
USBHIDMouse Mouse;
USBHIDConsumerControl Consumer;
USBHIDKeyboard Keyboard;

// Rotary encoder
#define CLK 4
#define DT 5
#define SW 6

#define POT_PIN 7

int lastStateCLK;
int lastVolume = -1;

bool lastButtonState = HIGH;

void setup() {

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);

  pinMode(SW, INPUT_PULLUP);

  lastStateCLK = digitalRead(CLK);

  analogReadResolution(12);

  USB.begin();

  Mouse.begin();
  Consumer.begin();
  Keyboard.begin();

  delay(2000);
}

void loop() {

  int currentCLK = digitalRead(CLK);

  if (currentCLK != lastStateCLK) {

    if (digitalRead(DT) != currentCLK) {

      Mouse.move(0, 0, 1);

    } else {

      Mouse.move(0, 0, -1);
    }

    delay(2);
  }

  lastStateCLK = currentCLK;

  int raw = analogRead(POT_PIN);

  int volume = map(raw, 0, 4095, 0, 100);

  if (abs(volume - lastVolume) > 2) {

    if (lastVolume == -1) {
      lastVolume = volume;
    }

    if (volume > lastVolume) {

      for (int i = 0; i < (volume - lastVolume); i++) {

        Consumer.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
        Consumer.release();

        delay(2);
      }

    } else if (volume < lastVolume) {

      for (int i = 0; i < (lastVolume - volume); i++) {

        Consumer.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
        Consumer.release();

        delay(2);
      }
    }

    lastVolume = volume;
  }

  bool currentButtonState = digitalRead(SW);

  // Detect button press
  if (lastButtonState == HIGH && currentButtonState == LOW) {

    // Open Run dialog (WIN + R)
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');

    delay(100);

    Keyboard.releaseAll();

    delay(300);

    // Type command
    Keyboard.print("devmgmt.msc");

    delay(100);

    // Press Enter
    Keyboard.write(KEY_RETURN);

    delay(500);
  }

  lastButtonState = currentButtonState;

  delay(5);
}
