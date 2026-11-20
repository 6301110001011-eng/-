/*
 * ถังขยะอัตโนมัติ - เซอร์โว 360 องศา (Continuous rotation) ตัวเดียว
 * เซอร์โว 360 สั่งมุมไม่ได้ สั่งได้แค่ "ทิศ + ความเร็ว" จึงใช้วิธีหมุนตามเวลา
 *   write(90)  = หยุด
 *   write(>90) = หมุนทางหนึ่ง (ยิ่งห่าง 90 ยิ่งเร็ว)
 *   write(<90) = หมุนอีกทาง
 *
 * Servo   : ส้ม -> D5, แดง -> 5V, น้ำตาล -> GND
 * HC-SR04 : VCC -> D7, Trig -> D9, Echo -> D10, GND -> GND
 */

#include <Servo.h>

const int SERVO_PIN        = 5;
const int SENSOR_POWER_PIN = 7;
const int TRIG_PIN         = 9;
const int ECHO_PIN         = 10;

// ---------- ค่าที่ปรับได้ ----------
const int  DETECT_DISTANCE_CM    = 25;    // ระยะตรวจจับ (ซม.)
const int  STOP_VALUE            = 90;    // ค่าที่ทำให้เซอร์โวหยุด (บางตัว 88-93)
const int  SPEED                 = 60;    // ความเร็ว 0-90 (มาก = เร็ว)
const unsigned long OPEN_RUN_MS  = 400;   // หมุนนานกี่ ms ตอนเปิด (ปรับให้ฝาเปิดพอดี)
const unsigned long CLOSE_RUN_MS = 400;   // หมุนนานกี่ ms ตอนปิด
const unsigned long OPEN_TIME_MS = 3000;  // เปิดค้าง 3 วินาที
const bool REVERSE               = false; // ถ้าเปิด/ปิดกลับทิศ ให้เปลี่ยนเป็น true

Servo servo1;
bool lidOpen = false;
unsigned long lastDetectTime = 0;

// หมุนตามทิศที่กำหนดเป็นเวลา ms แล้วหยุด
void spin(bool openDir, unsigned long ms) {
  bool dir = REVERSE ? !openDir : openDir;
  servo1.attach(SERVO_PIN);
  servo1.write(dir ? STOP_VALUE + SPEED : STOP_VALUE - SPEED);
  delay(ms);
  servo1.write(STOP_VALUE);
  delay(50);
  servo1.detach();   // ตัดสัญญาณ = หยุดสนิท ไม่ไหลเอง
}

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
  // ไม่ attach ตอนเริ่ม เพื่อให้เซอร์โวอยู่นิ่ง
  Serial.println("Smart Trash Bin (360) ready");
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
      spin(true, OPEN_RUN_MS);
      lidOpen = true;
    }
  } else if (lidOpen && millis() - lastDetectTime >= OPEN_TIME_MS) {
    Serial.println("-> CLOSE");
    spin(false, CLOSE_RUN_MS);
    lidOpen = false;
  }

  delay(60);
}
