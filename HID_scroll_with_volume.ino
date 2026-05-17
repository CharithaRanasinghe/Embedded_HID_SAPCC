#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDConsumerControl.h"

// HID devices
USBHIDMouse Mouse;
USBHIDConsumerControl Consumer;

#define CLK 4
#define DT 5

int lastStateCLK;

#define POT_PIN 7 

int lastVolume = -1;

void setup() {

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);

  lastStateCLK = digitalRead(CLK);

  analogReadResolution(12);

  USB.begin();

  Mouse.begin();
  Consumer.begin();

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

  delay(5);
}
