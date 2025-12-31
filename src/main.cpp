
#include <arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// I2C地址通常是0x27
// 参数：地址，列数，行数
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
  Serial.begin(115200);
  
  // 初始化I2C，使用GPIO3(SDA)和GPIO4(SCL)
  Wire.begin(3, 4);  // SDA=GPIO3, SCL=GPIO4
  
  // 初始化LCD
  lcd.init();
  lcd.backlight();  // 打开背光
  
  // 显示Hello World
  lcd.setCursor(0, 0);  // 第一行，第一列
  lcd.print("Hello World!");
  
  lcd.setCursor(0, 1);  // 第二行，第一列
  lcd.print("ESP32 I2C LCD");
  
  Serial.println("LCD initialized successfully!");
}

void loop() {
  // 可以添加滚动效果或其他动态内容
  delay(1000);
  
  // 示例：闪烁效果
  static bool blink = false;
  if (blink) {
    lcd.backlight();  // 打开背光
  } else {
    lcd.noBacklight();  // 关闭背光
  }
  blink = !blink;
}

