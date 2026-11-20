/*
 * โค้ดหาเวลาที่เซอร์โว 360 หมุนได้ 90 องศา
 * เปิด Serial Monitor (9600, เลือก "Newline") แล้วพิมพ์:
 *   ตัวเลข เช่น 300  -> หมุนไปข้างหน้า 300 ms
 *   -300             -> หมุนถอยหลัง 300 ms
 * ลองจนได้ 90 องศาพอดี แล้วเอาตัวเลขไปใส่ OPEN_RUN_MS / CLOSE_RUN_MS
 */

#include <Servo.h>

const int SERVO_PIN  = 5;
const int STOP_VALUE = 90;
const int SPEED      = 60;   // ต้องตรงกับในโค้ดถังขยะ

Servo servo1;

void setup() {
  Serial.begin(9600);
  Serial.println("พิมพ์เวลา ms (เช่น 300 หรือ -300) แล้วกด Enter");
}

void loop() {
  if (Serial.available()) {
    long ms = Serial.parseInt();
    while (Serial.available()) Serial.read();
    if (ms == 0) return;

    servo1.attach(SERVO_PIN);
    servo1.write(ms > 0 ? STOP_VALUE + SPEED : STOP_VALUE - SPEED);
    delay(abs(ms));
    servo1.write(STOP_VALUE);
    delay(50);
    servo1.detach();

    Serial.print("หมุน ");
    Serial.print(ms > 0 ? "ไปข้างหน้า " : "ถอยหลัง ");
    Serial.print(abs(ms));
    Serial.println(" ms");
  }
}
