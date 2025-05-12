#define buzzer 2
#define led 3

#define x A0
#define y A1
#define z A2

#define samples 50
#define magnitudeThreshold 50
#define magnitudeThresholdSq 2500    // 50^2
#define buzTime 5000

float alpha = 0.1;  // Low-pass filter smoothing factor

float xFiltered = 0, yFiltered = 0, zFiltered = 0;
int xBase = 0, yBase = 0, zBase = 0;

unsigned long alertStart = 0;
bool alertActive = false;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("System Initialized...");
  Serial.println("Earthquake Detector");
  delay(2000);
  Serial.println("Calibrating... Please wait...");

  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  digitalWrite(buzzer, LOW);

  // Calibrate baseline
  for (int i = 0; i < samples; i++) {
    xBase += analogRead(x);
    yBase += analogRead(y);
    zBase += analogRead(z);
    delay(10);
  }

  xBase /= samples;
  yBase /= samples;
  zBase /= samples;

  xFiltered = xBase;
  yFiltered = yBase;
  zFiltered = zBase;

  Serial.println("Calibration Complete!");
  Serial.print("X Baseline: "); Serial.println(xBase);
  Serial.print("Y Baseline: "); Serial.println(yBase);
  Serial.print("Z Baseline: "); Serial.println(zBase);
  Serial.println("Device Ready");
}

void loop() {
  int xNow = analogRead(x);
  int yNow = analogRead(y);
  int zNow = analogRead(z);

  // Apply low-pass filter
  xFiltered = alpha * xNow + (1 - alpha) * xFiltered;
  yFiltered = alpha * yNow + (1 - alpha) * yFiltered;
  zFiltered = alpha * zNow + (1 - alpha) * zFiltered;

  int xDiff = xFiltered - xBase;
  int yDiff = yFiltered - yBase;
  int zDiff = zFiltered - zBase;

  long magnitudeSq = (long)xDiff * xDiff + (long)yDiff * yDiff + (long)zDiff * zDiff;

  if (magnitudeSq > magnitudeThresholdSq) {
    if (!alertActive) {
      alertActive = true;
      Serial.println("** Earthquake Detected! Alert Activated **");
    }
    alertStart = millis();  // Extend alert duration
  }

  if (alertActive) {
    digitalWrite(led, HIGH);
    digitalWrite(buzzer, (millis() / 500) % 2 == 0 ? HIGH : LOW);

    if (millis() - alertStart >= buzTime) {
      alertActive = false;
      digitalWrite(led, LOW);
      digitalWrite(buzzer, LOW);
      Serial.println("Alert Deactivated: Buzzer and LED OFF");
    }
  }

  // Print filtered difference
  Serial.print("X: "); Serial.print(xDiff);
  Serial.print(" | Y: "); Serial.print(yDiff);
  Serial.print(" | Z: "); Serial.print(zDiff);
  Serial.print(" | MagSq: "); Serial.println(magnitudeSq);

  delay(100);
}
