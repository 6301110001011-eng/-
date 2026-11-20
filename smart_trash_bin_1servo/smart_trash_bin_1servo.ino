/*
 * ถังขยะอัตโนมัติ - เซอร์โวตัวเดียว (หมุนเร็ว)
 * Servo 1 : ส้ม -> D5, แดง -> 5V, น้ำตาล -> GND
 * HC-SR04 : VCC -> D7, Trig -> D9, Echo -> D10, GND -> GND
 */

#include <Servo.h>

const int SERVO_PIN        = 5;
const int SENSOR_POWER_PIN = 7;
const int TRIG_PIN         = 9;
const int ECHO_PIN         = 10;

const int  DETECT_DISTANCE_CM    = 25;    // ระยะตรวจจับ (ซม.)
const int  CLOSED_ANGLE          = 0;     // มุมฝาปิด
const int  OPEN_ANGLE            = 90;    // มุมฝาเปิด
const unsigned long OPEN_TIME_MS = 3000;  // เปิดค้าง 3 วินาที

Servo servo1;
bool lidOpen = false;
unsigned long lastDetectTime = 0;

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration / 58;
}

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(100);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo1.attach(SERVO_PIN);
  servo1.write(CLOSED_ANGLE);
  delay(500);
  Serial.println("Smart Trash Bin ready");
}

void loop() {
  long distance = readDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance <= DETECT_DISTANCE_CM) {
    lastDetectTime = millis();
    if (!lidOpen) {
      Serial.println("-> OPEN");
      servo1.write(OPEN_ANGLE);    // สั่งไปมุมเปิดทันที = หมุนเร็วสุด
      lidOpen = true;
    }
  } else if (lidOpen && millis() - lastDetectTime >= OPEN_TIME_MS) {
    Serial.println("-> CLOSE");
    servo1.write(CLOSED_ANGLE);
    lidOpen = false;
  }

  delay(60);
}
