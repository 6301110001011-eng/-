/*
 * ถังขยะอัตโนมัติ (Smart Trash Bin)
 * อุปกรณ์: Arduino UNO, Ultrasonic HC-SR04, Servo MG996R x 2
 *
 * การต่อสาย (ไม่ต้องใช้เบรดบอร์ด)
 *   Servo 1 : แดง -> 5V      น้ำตาล -> GND (แถว POWER)   ส้ม -> D5
 *   Servo 2 : แดง -> IOREF   น้ำตาล -> GND (แถว POWER)   ส้ม -> D6
 *   HC-SR04 : VCC -> D7      GND -> GND (ข้างขา 13)      TRIG -> D9   ECHO -> D10
 */

#include <Servo.h>

// ---------- ขาที่ใช้ ----------
const int SERVO1_PIN       = 5;
const int SERVO2_PIN       = 6;
const int SENSOR_POWER_PIN = 7;   // จ่ายไฟให้ HC-SR04 แทนขา 5V
const int TRIG_PIN         = 9;
const int ECHO_PIN         = 10;

// ---------- ค่าที่ปรับได้ ----------
const int  DETECT_DISTANCE_CM    = 25;    // ระยะตรวจจับ (ซม.)
const int  CLOSED_ANGLE          = 0;     // มุมฝาปิด
const int  OPEN_ANGLE            = 90;    // มุมฝาเปิด
const unsigned long OPEN_TIME_MS = 3000;  // เปิดค้างกี่มิลลิวินาที
const int  STEP_DELAY_MS         = 8;     // ความเร็วหมุน (มาก = ช้า)
const bool SERVO2_REVERSED       = true;  // เซอร์โวติดคนละฝั่ง ให้ตัวที่ 2 หมุนกลับทิศ

Servo servo1;
Servo servo2;

int  currentAngle = CLOSED_ANGLE;
bool lidOpen = false;
unsigned long lastDetectTime = 0;

void writeBoth(int angle) {
  servo1.write(angle);
  servo2.write(SERVO2_REVERSED ? 180 - angle : angle);
}

void moveLid(int target) {
  int step = (target > currentAngle) ? 1 : -1;
  while (currentAngle != target) {
    currentAngle += step;
    writeBoth(currentAngle);
    delay(STEP_DELAY_MS);
  }
}

long readDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration / 58;  // เซนติเมตร
}

// อ่าน 3 ครั้ง เอาค่ากลาง ลดค่าเพี้ยน
long readDistance() {
  long d[3];
  for (int i = 0; i < 3; i++) {
    d[i] = readDistanceOnce();
    delay(10);
  }
  if (d[0] > d[1]) { long t = d[0]; d[0] = d[1]; d[1] = t; }
  if (d[1] > d[2]) { long t = d[1]; d[1] = d[2]; d[2] = t; }
  if (d[0] > d[1]) { long t = d[0]; d[0] = d[1]; d[1] = t; }
  return d[1];
}

void setup() {
  Serial.begin(9600);

  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH);  // D7 = 5V ให้เซนเซอร์
  delay(100);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  writeBoth(CLOSED_ANGLE);
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
      moveLid(OPEN_ANGLE);
      lidOpen = true;
    }
  } else if (lidOpen && millis() - lastDetectTime >= OPEN_TIME_MS) {
    Serial.println("-> CLOSE");
    moveLid(CLOSED_ANGLE);
    lidOpen = false;
  }

  delay(60);
}
