const int break_analogRead = 377;

const byte temperaturePin = A1;
const byte outputPin = 0;
const byte testPin = 1;

const byte ON = 0;
const byte OFF = 255;

void setup() {
    pinMode(temperaturePin, INPUT);
    pinMode(outputPin, OUTPUT);
}

int fail_count = 0;
void loop() {
    int in = analogRead(temperaturePin);

    analogWrite(testPin, in / 4);
    if (in < break_analogRead) {
      // system failed, temperature too high
      ++fail_count;
    } else {
      fail_count = 0;
    }

    if (fail_count > 4) {
      analogWrite(outputPin, OFF);
    } else {
      analogWrite(outputPin, ON);
    }

    delay(50);
}
