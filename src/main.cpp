#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>

// LCD配置
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C地址0x27，16列2行

// WiFi配置
const char* ssid = "LDQ-AP";
const char* password = "747225581";

// MQTT配置
const char* mqtt_server = "8.153.160.138";
const int mqtt_port = 1883;  // MQTT默认端口
const char* mqtt_topic = "info_box/lvdi/box_001";

// 全局变量
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsgTime = 0;
bool screenActive = true;
const unsigned long screenTimeout = 5000;  // 屏幕亮5秒
String currentLine1 = "";
String currentLine2 = "";

// ========== 函数声明 ==========
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void updateLCD();
void reconnect();
// =============================

void setup() {
  Serial.begin(115200);
  
  // 初始化I2C和LCD
  Wire.begin(3, 4);  // SDA=GPIO3, SCL=GPIO4
  lcd.init();
  lcd.backlight();
  
  // 显示启动信息
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  lcd.setCursor(0, 1);
  lcd.print("System Start");
  delay(1000);
  
  // (1) 连接WiFi
  setup_wifi();
  
  // (2) 设置MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // 初始化显示内容
  currentLine1 = "等待数据...";
  currentLine2 = mqtt_topic;
  updateLCD();
  
  lastMsgTime = millis();
}

void loop() {
  // 检查MQTT连接
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // (4) 屏幕超时控制 - 5秒后关闭背光
  unsigned long currentTime = millis();
  if (screenActive && (currentTime - lastMsgTime > screenTimeout)) {
    screenActive = false;
    lcd.noBacklight();
    Serial.println("屏幕背光已关闭");
  }
  
  // 小延迟避免CPU占用过高
  delay(10);
}

// ========== 函数定义 ==========

// WiFi连接函数
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("正在连接WiFi: ");
  Serial.println(ssid);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    // 显示连接进度
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connect success");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP().toString().substring(0, 12));  // 显示部分IP
  } else {
    Serial.println("WiFi连接失败");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check SSID/PWD");
  }
  delay(2000);
}

// MQTT回调函数 - 当收到消息时调用
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("收到消息 [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // 解析消息格式，假设消息格式为 "行1内容|行2内容"
  int separatorIndex = message.indexOf('|');
  if (separatorIndex != -1) {
    currentLine1 = message.substring(0, separatorIndex);
    currentLine2 = message.substring(separatorIndex + 1);
  } else {
    // 如果没有分隔符，整个消息显示在第一行
    currentLine1 = message;
    currentLine2 = "";
  }
  
  // 更新LCD显示
  updateLCD();
  
  // 激活屏幕并重置计时器
  screenActive = true;
  lastMsgTime = millis();
  lcd.backlight();
  Serial.println("屏幕已激活，将持续亮5秒");
}

// 更新LCD显示内容
void updateLCD() {
  lcd.clear();
  
  // 显示第一行
  lcd.setCursor(0, 0);
  if (currentLine1.length() > 16) {
    lcd.print(currentLine1.substring(0, 16));
  } else {
    lcd.print(currentLine1);
  }
  
  // 显示第二行
  lcd.setCursor(0, 1);
  if (currentLine2.length() > 16) {
    lcd.print(currentLine2.substring(0, 16));
  } else {
    lcd.print(currentLine2);
  }
}

// MQTT重连函数
void reconnect() {
  while (!client.connected()) {
    Serial.print("正在连接MQTT服务器...");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting MQTT");
    
    // 尝试连接，使用唯一的client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT连接成功");
      Serial.print("订阅主题: ");
      Serial.println(mqtt_topic);
      
      // 订阅主题
      client.subscribe(mqtt_topic);
      
      lcd.setCursor(0, 1);
      lcd.print("Subscribed!");
      delay(1000);
      
      // 连接成功后显示初始信息
      currentLine1 = "MQTT Connected";
      currentLine2 = mqtt_topic;
      updateLCD();
    } else {
      Serial.print("连接失败，状态码=");
      Serial.print(client.state());
      Serial.println(" 5秒后重试...");
      
      lcd.setCursor(0, 1);
      lcd.print("Failed: ");
      lcd.print(client.state());
      
      delay(5000);
    }
  }
}