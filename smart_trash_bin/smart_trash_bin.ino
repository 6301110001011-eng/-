/*
 * ถังขยะอัตโนมัติ (Smart Trash Bin)
 * อุปกรณ์: Arduino UNO, Ultrasonic HC-SR04, Servo MG996R x 2
 *
 * การต่อสาย
 *   HC-SR04 : VCC -> D7 (ใช้ขาดิจิทัลจ่ายไฟแทน 5V), GND -> GND, TRIG -> D9, ECHO -> D10
 *   Servo 1 : สีส้ม(สัญญาณ) -> D5, สีแดง -> +5V, สีน้ำตาล -> GND
 *   Servo 2 : สีส้ม(สัญญาณ) -> D6, สีแดง -> +5V, สีน้ำตาล -> GND
 *
 * หมายเหตุ: MG996R กินกระแสสูง (สูงสุดราว 2A ต่อตัว)
 *   ควรใช้แหล่งจ่าย 5-6V แยก (เช่น อะแดปเตอร์ 5V 3A หรือแบตเตอรี่ + บอร์ดลดแรงดัน)
 *   และต่อ GND ของแหล่งจ่ายรวมกับ GND ของ Arduino เสมอ
 */

#include <Servo.h>

// ---------- ขาที่ใช้ ----------
const int TRIG_PIN   = 9;
const int ECHO_PIN   = 10;
const int SENSOR_POWER_PIN = 7; // จ่ายไฟให้ HC-SR04 (กินไฟแค่ ~15mA จึงใช้ขาดิจิทัลได้)
const int SERVO1_PIN = 5;
const int SERVO2_PIN = 6;

// ---------- ค่าที่ปรับได้ ----------
const int  DETECT_DISTANCE_CM = 25;    // ระยะที่ถือว่ามีคนมาทิ้งขยะ (ซม.)
const int  CLOSED_ANGLE       = 0;     // มุมตอนฝาปิด
const int  OPEN_ANGLE         = 90;    // มุมตอนฝาเปิด
const unsigned long OPEN_TIME_MS = 3000; // เปิดค้างไว้กี่มิลลิวินาทีหลังคนออกไป
const int  STEP_DELAY_MS      = 8;     // ความเร็วการหมุน (มาก = ช้าและนุ่มนวล)

// ถ้าติดเซอร์โว 2 ตัวหันหน้าเข้าหากัน (คนละฝั่งของฝา) ให้ตั้งเป็น true
// เพื่อให้ตัวที่ 2 หมุนกลับทิศ ฝาจะได้ยกขึ้นพร้อมกัน
const bool SERVO2_REVERSED = true;

Servo servo1;
Servo servo2;

int  currentAngle = CLOSED_ANGLE;
bool lidOpen = false;
unsigned long lastDetectTime = 0;

// เขียนมุมไปที่เซอร์โวทั้งสองตัว
void writeBoth(int angle) {
  servo1.write(angle);
  servo2.write(SERVO2_REVERSED ? 180 - angle : angle);
}

// ค่อย ๆ หมุนไปยังมุมเป้าหมาย
void moveLid(int target) {
  int step = (target > currentAngle) ? 1 : -1;
  while (currentAngle != target) {
    currentAngle += step;
    writeBoth(currentAngle);
    delay(STEP_DELAY_MS);
  }
}

// อ่านระยะจาก HC-SR04 (ซม.) คืนค่า -1 ถ้าอ่านไม่ได้
long readDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (~5 m)
  if (duration == 0) return -1;
  return duration / 58; // แปลงเป็นเซนติเมตร
}

// อ่าน 3 ครั้งแล้วเอาค่ากลาง เพื่อลดค่าเพี้ยน
long readDistance() {
  long d[3];
  for (int i = 0; i < 3; i++) {
    d[i] = readDistanceOnce();
    delay(10);
  }
  // เรียงลำดับ 3 ค่าแบบง่าย
  if (d[0] > d[1]) { long t = d[0]; d[0] = d[1]; d[1] = t; }
  if (d[1] > d[2]) { long t = d[1]; d[1] = d[2]; d[2] = t; }
  if (d[0] > d[1]) { long t = d[0]; d[0] = d[1]; d[1] = t; }
  return d[1];
}

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH); // ขา D7 = 5V ให้เซนเซอร์
  delay(100);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  writeBoth(CLOSED_ANGLE);   // เริ่มต้นปิดฝา
  delay(500);

  Serial.println("Smart Trash Bin ready");
}

void loop() {
  long distance = readDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  bool someoneNear = (distance > 0 && distance <= DETECT_DISTANCE_CM);

  if (someoneNear) {
    lastDetectTime = millis();
    if (!lidOpen) {
      Serial.println("-> เปิดฝา");
      moveLid(OPEN_ANGLE);
      lidOpen = true;
    }
  } else if (lidOpen && millis() - lastDetectTime >= OPEN_TIME_MS) {
    Serial.println("-> ปิดฝา");
    moveLid(CLOSED_ANGLE);
    lidOpen = false;
  }

  delay(60);
}
