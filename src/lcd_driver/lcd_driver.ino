#include "lcdbuffer.h"
#include "settings.h"

LCDBuffer lcdBuffer;

void setup() {
  pinMode(CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(WRITE_ENABLE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onClock, RISING);

  for (int i = 0; i != 8; ++i) {
    pinMode(DATA_PINS[i], INPUT);
  }
  
  pinMode(DISPLAY_MODE_PIN, INPUT);
  pinMode(SCROLL_UP_PIN, INPUT);
  pinMode(SCROLL_DOWN_PIN, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  //getInput(); // will be replaced by the interrupt handler
  handleButtons();
  update();
}

void update() {
  static unsigned long previous = millis();
  unsigned long current = millis();
  if (current - previous > REFRESH_DELAY) {
    previous = current;
    if (digitalRead(DISPLAY_MODE_PIN) == 0)
      lcdBuffer.setMode(HEXADECIMAL);
    else
      lcdBuffer.setMode(ASCII);

    lcdBuffer.send();
  }
}

void getInput() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n')
      return;

    lcdBuffer.push(c);
  }
}

void onClock() {
  if (digitalRead(WRITE_ENABLE_PIN)) {
    byte c = 0;
    for (int i = 0; i != 8; ++i) {
      c |= (digitalRead(DATA_PINS[i]) << i); 
    }
    lcdBuffer.push(c);
  }
}

void handleButtons() {
  if (digitalRead(SCROLL_UP_PIN)) {
    lcdBuffer.scrollUp();
    delay(BUTTON_DEBOUNCE_DELAY);
  }
  else if (digitalRead(SCROLL_DOWN_PIN)) {
    lcdBuffer.scrollDown();
    delay(BUTTON_DEBOUNCE_DELAY);
  }
}

void handleButtons2() {
  enum State {
    IDLE,
    SCROLL_UP_DETECTED,
    SCROLL_UP_EXECUTED,
    SCROLL_DOWN_DETECTED,
    SCROLL_DOWN_EXECUTED,
    WAIT_FOR_CLEAR
  };

  static State state = IDLE;
  static unsigned long timeStamp = 0;

  bool upPushed = digitalRead(SCROLL_UP_PIN);
  bool downPushed = digitalRead(SCROLL_DOWN_PIN);

  switch (state) {
    case IDLE: {
      if (upPushed && !downPushed) {
        state = SCROLL_UP_DETECTED;
        timeStamp = millis();
        return;
      }
      else if (downPushed && !upPushed) {
        state = SCROLL_DOWN_DETECTED;
        timeStamp = millis();
        return;
      }
      else if (upPushed && downPushed) {
        state = WAIT_FOR_CLEAR;
        timeStamp = millis();
        return;
      }
      else return;
    }
    case SCROLL_UP_DETECTED: {
      if (upPushed && !downPushed) {
        if (millis() - timeStamp > REFRESH_DELAY) {
          state = SCROLL_UP_EXECUTED;
          lcdBuffer.scrollUp();
          return;
        }
        else return;
      }
      else if (!upPushed) {
        state = IDLE;
        return;
      }
      else if (upPushed && downPushed) {
        state = WAIT_FOR_CLEAR;
        return;
      }
      else {
        state = IDLE;
        return;
      }
    }
    case SCROLL_DOWN_DETECTED: {
      if (!downPushed) {
        state = IDLE;
        return;
      }
      else if (downPushed && !upPushed) {
        if (millis() - timeStamp > REFRESH_DELAY) {
          state = SCROLL_DOWN_EXECUTED;
          lcdBuffer.scrollDown();
          return;
        }
        else return;        
      }
      else if (upPushed && downPushed) {
        state = WAIT_FOR_CLEAR;
        return;
      }
      else {
        state = IDLE;
        return;
      }
    }
    case SCROLL_UP_EXECUTED: {
      if (!upPushed) {
        state = IDLE;
        return;
      }
    }
    case SCROLL_DOWN_EXECUTED: {
      if (!downPushed) {
        state = IDLE;
        return;
      }
    }
    case WAIT_FOR_CLEAR: {
      if (upPushed && downPushed) {
        if (millis() - timeStamp > CLEAR_HOLD_TIME) {
          state = IDLE;
          lcdBuffer.clear();
          return;
        }
      }
      else {
        state = IDLE;
        return;
      }
    }
  }
}
