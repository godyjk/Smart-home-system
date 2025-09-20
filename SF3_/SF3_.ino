#include "IRremote.h"
#include <Servo.h>        // 舵机库
#include "dht.h"          // 温湿度传感器库
#include <Arduino.h>
#include <LiquidCrystal_I2C.h> // LCD 屏幕库
#include <SPI.h>
#include <Wire.h>
#include <TM1637Display.h>     // 数码管显示库
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// 窗户控制部分
#define servoPin 6
#define angle 170
#define Rain A1
#define CloseAngle 20
#define OpenAngle 170
// 风扇控制
#define DHT11_PIN 7           // 温湿度传感器
#define Fan_Pin 9
#define TEMP_THRESHOLD 29
// 门控制部分
#define servoPin2 31
#define closeDoor 65
#define openDoor 0
// 窗户控制舵机
#define servoPin 6
// 门控制舵机
#define servoPin2 31
// 门铃和蜂鸣器
#define Key_Pin 2
#define beep 8
// 雨滴传感器
#define Rain A1
// LED 灯控制引脚
#define LED_R 43
#define LED_G 3
#define LED_B 5
//WiFi模块
SoftwareSerial MWSerial(15, 16); // RX, TX
SoftwareSerial WMSerial(10, 11); // RX, TX
// 人体感应灯
#define pushButton 25
#define yellowLight 18
#define LTD 10
// 数码管显示屏控制引脚
#define PIN_DIO 3
#define PIN_CLK 4
TM1637Display display(PIN_CLK, PIN_DIO);
// 语音检测模块引脚
#define IVC_O 46  //PA0
#define IVC_C 49  //PC1
#define VC_DO 47  //PA1
#define VC_DC 48  
#define VC_LO 50
#define VC_LC 51  //PA4
#define VC_WO 52
#define VC_WC 53
#define VC_FO 26
#define VC_FC 27
//红外线检测需用常数
#define RECV_PIN A2               // 红外接收数据引脚
IRrecv irrecv(RECV_PIN);         // 初始化一个接收对象
decode_results results;          // 接收数据变量
unsigned long lastCode = 0;      // 上一次接收到的信号值
// 环境数据变量（非常量，将在运行时更新）
float temperature = 0.0, humidity = 0.0;
// 控制循环计数器
int TD = 0;
// 光敏灯光
int val=120;
int sensorValue=0;

//全部布尔值初始化
// 基本设备状态
bool BWin = false;    // 窗户状态
bool BDoor = false;   // 门状态
bool BFan = false;    // 风扇状态
bool BLight = false;  // 灯光状态

// 智能控制状态
bool IBWin = false;   // 窗户的智能控制状态
bool IBDoor = false;  // 门的智能控制状态
bool IBFan = false;   // 风扇的智能控制状态
bool IBLight = false; // 灯光的智能控制状态

// 新建全局舵机对象
Servo myservo;        // 窗户
Servo myservo2;       // 门
dht DHT;              // LCD//此行必须在使用lcd对象之前正确声明
LiquidCrystal_I2C lcd(0x27,16,2);

String sndInfo = "";  // 用于存储要发送的信息
char rcvInfo = "";  // 用于存储接收的信息
//函数声明部分
void SLCD();
void Open_Win();
void Close_Win();
void IWin();
void Open_Door();
void Close_Door();
void Open_Fan();
void Close_Fan();
void IFan();
void Open_Light();
void Close_Light();
void ILight();
bool Man_Scan();
void RC();
void VC();
void Bell();


//开始初始化
void setup() {
  lcd.init();                // 初始化 LCD 屏幕
  lcd.backlight();           // 初始化背光
  pinMode(Key_Pin, INPUT);   // 按键门铃初始化
  pinMode(beep, OUTPUT);     // 蜂鸣器
  myservo.attach(servoPin);  // 窗户舵机
  myservo2.attach(servoPin2);// 门舵机
  pinMode(Fan_Pin, OUTPUT);  // 风扇控制
  pinMode(LED_R, OUTPUT);    // RGB 灯
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(pushButton, INPUT);// 人体感应器
  display.setBrightness(0x40);
  display.clear();
  Serial.begin(9600);        // 启动串口通信
  irrecv.enableIRIn();       // 启动红外接收
  MWSerial.begin(9600);
  MWSerial.flush();
  WMSerial.begin(9600);
  WMSerial.flush();
  digitalWrite(Fan_Pin, LOW);
  myservo.write(OpenAngle);
  Serial.println("程序初始化完毕");
}

void loop() {
  delay(300);
  // 处理门和窗户自动化逻辑
  VC();  // 处理语音命令（请注意，务必先处理语音命令）
  RC();  // 处理红外信号
  Bell(); // 检测门铃
  // 智能设备周期性检测

  TD++;
  if(TD%3==0){
    // WiFi收发
    //rcv();
    //snd();
  }
  if(TD>=10){
    if (IBLight){
      ILight();     // 智能灯光控制
    }
    if(IBFan){
      IFan();       // 智能风扇控制
    }
    if(IBWin){
      IWin();       // 智能窗户控制
    }
    SLCD();        // 更新显示温度和湿度和亮度
    /*if(Man_Scan())// 人体感应器（用于激活摄像头）
      //如果有人，启动摄像头
      Serial.println("发现访客！");
    else
      Serial.println("无人静默");
    TD=0;
    */
    /*
    Serial.println("设备状态:");
    Serial.print("窗户状态 (BWin): "); Serial.println(BWin ? "开启" : "关闭");
    Serial.print("门状态 (BDoor): "); Serial.println(BDoor ? "开启" : "关闭");
    Serial.print("风扇状态 (BFan): "); Serial.println(BFan ? "开启" : "关闭");
    Serial.print("灯光状态 (BLight): "); Serial.println(BLight ? "开启" : "关闭");

    Serial.println("智能控制状态:");
    Serial.print("窗户智能控制状态 (IBWin): "); Serial.println(IBWin ? "开启" : "关闭");
    Serial.print("门智能控制状态 (IBDoor): "); Serial.println(IBDoor ? "开启" : "关闭");
    Serial.print("风扇智能控制状态 (IBFan): "); Serial.println(IBFan ? "开启" : "关闭");
    Serial.print("灯光智能控制状态 (IBLight): "); Serial.println(IBLight ? "开启" : "关闭");
    */
  }
}

void Open_Win() { 
  if(IBWin){
      IBWin=false;//如果窗户关闭，且智能模式开启，则关闭窗户的智能模式
  }
  myservo.write(OpenAngle);
  delay(300);
  }
void Close_Win() { 
  if(IBWin){
      IBWin=false;//如果窗户开启，且智能模式开启，则关闭窗户的智能模式
  }
  myservo.write(CloseAngle);}
void Open_Door() { myservo2.write(openDoor); }
void Close_Door() { myservo2.write(closeDoor); }
void Open_Fan() { 
  if(IBFan){
      IBFan=false;//如果风扇开启，且智能模式开启，则关闭风扇智能模式
  }
  Serial.println("123");
  digitalWrite(Fan_Pin, HIGH);
}
void Close_Fan() { 
  if(IBFan){
      IBFan=false;//如果风扇开启，且智能模式开启，则关闭风扇智能模式
  }
  digitalWrite(Fan_Pin, LOW);
}
void Open_Light() {
  if(IBLight){
      IBLight=false;//如果智能模式开启，则关闭灯光智能模式
  }
  analogWrite(LED_R, val); 
  analogWrite(LED_G, val); 
  analogWrite(LED_B, val); 
}
void Close_Light() {
  if(IBLight){
      IBLight=false;//如果智能模式开启，则关闭灯光智能模式
  }
  analogWrite(LED_R, 0); 
  analogWrite(LED_G, 0); 
  analogWrite(LED_B, 0); 
}
//门铃函数
void Bell() { 
  int buttonState = digitalRead(Key_Pin);  //读取该接口的输出状态
  //判断按键状态，控制蜂鸣器开关
  //Serial.println("F");
  if(buttonState == 1){
    //Serial.println("FG");
    digitalWrite(beep, LOW);}
  else digitalWrite(beep, HIGH);}
void SLCD() {   //LCD显示函数
  DHT.read11(DHT11_PIN);  // DHT11 reads the current temperature and humidity
  temperature = DHT.temperature; // Get temperature value
  humidity = DHT.humidity; // Get humidity value
  // Display temperature and humidity on the OLED
  lcd.setCursor(0, 0);
  lcd.print("Humidity:");
  lcd.setCursor(12, 0);
  lcd.print(humidity, 1);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Temperature:");
  lcd.setCursor(12, 1);
  lcd.print(temperature, 1);
  lcd.print("C"); 
  sensorValue = analogRead(A0);
  display.showNumberDec(sensorValue, false, 4, 0);  //数码管显示A0模拟值
}
void IWin() {   //检测是否下雨
  if(analogRead(Rain) > 800)
  {
   myservo.write(OpenAngle);   //关闭窗户
   //Serial.println("123");
  }
  else
  {
    myservo.write(CloseAngle);   //打开窗户
  }
}
void IFan() { 
  if (temperature > TEMP_THRESHOLD) {
      digitalWrite(Fan_Pin, HIGH); // Temperature exceeds the threshold, turn on the fan
      Serial.println("321");
  } 
  else {
      digitalWrite(Fan_Pin, LOW); // Temperature below the threshold, turn off the fan
  }
}
//智能模式灯光
void ILight() {
  sensorValue = analogRead(A0);
  display.showNumberDec(sensorValue, false, 4, 0);  //数码管显示A0模拟值
  //Serial.println(sensorValue);
  if (val > 225) {val = 225;}
  if (val < 25) {val = 25;}
  while (sensorValue > 500 && val <= 225){
    val = val + 20;
    sensorValue = analogRead(A0);
    display.showNumberDec(sensorValue, false, 4, 0);
    analogWrite(LED_R, val);
    analogWrite(LED_G, val);
    analogWrite(LED_B, val);
    delay(50);
  }
  while (sensorValue < 400 && val >= 45){
    val = val - 20;
    sensorValue = analogRead(A0);
    display.showNumberDec(sensorValue, false, 4, 0);  //数码管显示A0模拟值
    analogWrite(LED_R, val);
    analogWrite(LED_G, val);
    analogWrite(LED_B, val);
    delay(50);
  }
}
 //人体感应灯
bool Man_Scan() {   
  int buttonState = digitalRead(pushButton);  //读取接口的输出状态
  if(buttonState == 0){//写入“无人”情况
    return false;
    //if (LTD==0)
    //digitalWrite(yellowLight,LOW);
    //else if (LTD>0)
      //LTD--;
      //Serial.println("Manba out");
  }
  else {
    return true;
    //写入“有人”情况
    //digitalWrite(yellowLight, HIGH);
    //Serial.println("你干嘛，诶呦~");
    //LTD=10;
  } 
}
/* 红外信号处理 */
void RC(){
    if (irrecv.decode(&results)) {  // 如果接收到红外信号
    if (results.value != 0xFFFFFFFF && results.value!=0) { // 检查是否为重复信号
      Serial.print("信号的十六进制值为:");
      Serial.println(results.value, HEX);  // 以十六进制格式打印接收到的信号值
      lastCode=results.value;
      switch(lastCode) {
        case  0x97483BFB:  
        case  0xFF9867:
          Serial.println("按键 0 被按下,切换窗户状态");
          if(BWin) {
            BWin = false;
            Close_Win();      
            break;
          }
          else {
            BWin = true;
            Open_Win();
            break;            
          }
        case  0xE318261B:  
        case  0xFFA25D:
          Serial.println("按键 1 被按下,切换门的状态");
          if (BDoor) {
            BDoor = false;
            Close_Door();
            break;
          }
          else {
            BDoor = true;
            Open_Door();
            break;            
          }

        case  0x511DBB:  
        case  0xFF629D:
          Serial.println("按键 2 被按下,切换风扇状态");
          if (BFan){
            BFan = false;
            Close_Fan();           
          }
          else {
            BFan = true;
            Open_Fan();
          }
            break;            
        case  0xEE886D7F:  
        case  0xFFE21D:
          Serial.println("按键 3 被按下,切换灯状态");
          if (BLight){
            BLight = false;
            Close_Light();                   
            break;
          }
          else {
            BLight = true;
            Open_Light();                    
            break;            
          }
        case  0x52A3D41F:  
        case  0xFF22DD:
          Serial.println("按键 4 被按下,打开窗户智能模式");
          IBWin = true;        
          break;
        case  0xD7E84B1B:  
        case  0xFF02FD:
          Serial.println("按键 5 被按下,关闭窗户智能模式");
          IBWin = false;
          break;
        case  0x20FE4DBB:  
        case  0xFFC23D:
          Serial.println("按键 6 被按下,打开灯光智能模式");
          IBLight = true;
          break;
        case  0xF076C13B:  
        case  0xFFE01F:
          Serial.println("按键 7 被按下，关闭灯光智能模式");
          IBLight = false;
          break;
        case  0xA3C8EDDB:  
        case  0xFFA857:
          Serial.println("按键 8 被按下，打开风扇智能模式");
          IBFan = true;
          break;
        case  0xE5CFBD7F:  
        case  0xFF906F:
          Serial.println("按键 9 被按下，关闭风扇智能模式");
          IBFan = false;
          break;
        case  0x8C22657B:  
        case  0xFF10EF:
          Serial.println("按键 左 被按下");
          //digitalWrite(Beep_Pin, LOW);    
          break;
        case  0x449E79F:  
        case  0xFF5AA5:
          Serial.println("按键 右 被按下");
          //digitalWrite(Beep_Pin, HIGH);   
          break;
      }
    }
    
  // 处理完信号后，清空结果结构中的数据
  results.value = 0;           // 手动清空 results 结构中的值
  delay(150);                   // 延时,单位ms
  irrecv.resume();             // 接收下一个值
  }
  else {
  //Serial.print("未收到信号");
  }
}

/* 语音命令处理 */ 
void VC(){
  if (digitalRead(IVC_O) == HIGH){
    Serial.println("智能模式已开启");
    IBLight = true;
    IBWin = true;
    IBFan = true;
  }
  if (digitalRead(IVC_C) == HIGH){
    Serial.println("智能模式已关闭");
    IBLight = false;
    IBWin = false;
    IBFan = false;
  }


  if (digitalRead(VC_DO) == HIGH){
    Serial.println("语音控制门已开启");
    BDoor = true;
    Open_Door();    
  }
  if (digitalRead(VC_LO) == HIGH){
    Serial.println("语音控制灯已开启");
    BLight = true;
    Open_Light();    
  }
  if (digitalRead(VC_WO) == HIGH){
    Serial.println("语音控制窗户已开启");
    BWin = true;
    Open_Win();     
  }
  if (digitalRead(VC_FO) == HIGH){
    Serial.println("语音控制风扇已开启");
    BFan = true;
    Open_Fan();   
  }


  if (digitalRead(VC_DC) == HIGH){ 
    Serial.println("语音控制门已关闭"); 
    BDoor = false; 
    Close_Door(); 
  } 
  if (digitalRead(VC_LC) == HIGH){
    Serial.println("语音控制灯已关闭"); 
    BLight = false; 
    Close_Light(); 
  } 
  if (digitalRead(VC_WC) == HIGH){ 
    Serial.println("语音控制窗户已关闭"); 
    BWin = false; 
    Close_Win(); 
  } 
  if (digitalRead(VC_FC) == HIGH){ 
    Serial.println("语音控制风扇已关闭"); 
    BFan = false; 
    Close_Fan(); 
  }
}

void rcv() {
  if (WMSerial.available()) {      // 检查是否有数据等待读取
    String receivedData = WMSerial.readStringUntil('\n'); // 读取直到换行符
    rcvInfo = receivedData[0];
    Serial.print("从web接收: ");
    Serial.println(rcvInfo);

    switch(rcvInfo) {
      case '0': // 智能模式开启
          Serial.println("web控制智能模式已开启");
          IBLight = true;
          IBWin = true;
          IBFan = true;
          break;
      case '1': // 智能模式关闭
          Serial.println("web控制智能模式已关闭");
          IBLight = false;
          IBWin = false;
          IBFan = false;
          break;
      case '2': // 门开启
          Serial.println("web控制门已开启");
          BDoor = true;
          Open_Door();
          break;
      case '3': // 门关闭
          Serial.println("web控制门已关闭");
          BDoor = false;
          Close_Door();
          break;
      case '4': // 灯开启
          Serial.println("web控制灯已开启");
          BLight = true;
          Open_Light();
          break;
      case '5': // 灯关闭
          Serial.println("web控制灯已关闭");
          BLight = false;
          Close_Light();
          break;
      case '6': // 窗户开启
          Serial.println("web控制窗户已开启");
          BWin = true;
          Open_Win();
          break;
      case '7': // 窗户关闭
          Serial.println("web控制窗户已关闭");
          BWin = false;
          Close_Win();
          break;
      case '8': // 风扇开启
          Serial.println("web控制风扇已开启");
          BFan = true;
          Open_Fan();
          break;
      case '9': // 风扇关闭
          Serial.println("web控制风扇已关闭");
          BFan = false;
          Close_Fan();
          break;
      default:
          Serial.println("未知命令");
          break;
    }
  }
}

void snd() {
  String sndInfo = "";  // 初始化发送信息的字符串变量

  // 添加窗户状态
  sndInfo += "Window: ";
  sndInfo += (BWin ? "1" : "0");
  sndInfo += '\n';  // 换行

  // 添加门状态
  sndInfo += "Door: ";
  sndInfo += (BDoor ? "1" : "0");
  sndInfo += '\n';  // 换行

  // 添加温度信息
  sndInfo += "Temperature: ";
  sndInfo += String(temperature, 1);  // 温度值保留一位小数
  sndInfo += '\n';  // 换行

  // 添加湿度信息
  sndInfo += "Humidity: ";
  sndInfo += String(humidity, 1);  // 湿度值保留一位小数
  sndInfo += '\n';  // 换行

  // 发送构建好的信息到串口
  MWSerial.print(sndInfo);
  MWSerial.write('|');  // 发送信息结束的标志
  Serial.print("Sent: ");
  Serial.println(sndInfo);
  sndInfo = "";  // 发送完毕后清空，防止重复发送
}
//蓝牙模块

/* 老协议的红外接受对应表
1   FFA25D
2   FF629D
3   FFE21D
4   FF22DD
5   FF02FD
6   FFC23D
7   FFE01F
8   FFA857
9   FF906F
0   FF9867
*   FF6897
#   FFB04F
k   FF38C7
↑   FF18E7
↓   FF4AB5
←   FF10EF
→   FF5AA5
*/
