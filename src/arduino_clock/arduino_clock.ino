#define BUTTON 3
#define MANUAL 4
#define CLK 5
#define DELAY 500

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON, INPUT);
  pinMode(MANUAL, INPUT);
  pinMode(CLK, OUTPUT);
}

void pulse() {
  digitalWrite(CLK, HIGH);
  delay(DELAY);
  digitalWrite(CLK, LOW);
  delay(DELAY);  
}

bool pressed = false;
void loop() {
  bool manual = digitalRead(MANUAL);
  if (not manual) {
    pulse();
    return;
  }

  if (digitalRead(BUTTON)) {
    if (pressed) return;
    pressed = true;
    pulse();
  }
  else
    pressed = false;
}
