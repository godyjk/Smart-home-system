#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "12345"; 
const char* password = "12345678"; 

const char* mqttServer = "192.168.156.91"; // MQTT broker
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

char webInfo;
String megaInfo;

void setup() {
  Serial.begin(9600);

  setupWiFi();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(receiveCallback);
  connectMQTTServer(); // 连接MQTT服务器
}

void loop() {
  getMegaInfo();
  delay(5000);

  if (mqttClient.connected()) {
    mqttClient.loop();
  }
  else {  // 如果开发板未能成功连接服务器
    connectMQTTServer(); // 则尝试连接服务器
  }
}

void setupWiFi() {
  Serial.begin(9600);         // 启动串口通讯
  
  WiFi.begin(ssid, password);                  // 启动网络连接
  Serial.print("Connecting to ");              // 串口监视器输出网络连接信息
  Serial.print(ssid); Serial.println(" ...");  // 告知用户NodeMCU正在尝试WiFi连接
  
  int i = 0;                                   // 这一段程序语句用于检查WiFi是否连接成功
  while (WiFi.status() != WL_CONNECTED) {      // WiFi.status()函数的返回值是由NodeMCU的WiFi连接状态所决定的。 
    delay(1000);                               // 如果WiFi连接成功则返回值为WL_CONNECTED                       
    Serial.print(i++); Serial.print(' ');      // 此处通过While循环让NodeMCU每隔一秒钟检查一次WiFi.status()函数返回值
  }                                            // 同时NodeMCU将通过串口监视器输出连接时长读秒。
                                               
  Serial.println("");                          // WiFi连接成功后
  Serial.println("Connection established!");   // NodeMCU将通过串口监视器输出"连接成功"信息。
  Serial.print("IP address:    ");             // 同时还将输出NodeMCU的IP地址。这一功能是通过调用
  Serial.println(WiFi.localIP());              // WiFi.localIP()函数来实现的。该函数的返回值即NodeMCU的IP地址。
}

void connectMQTTServer(){
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "esp8266-" + WiFi.macAddress();

  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str())) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);
    subscribeTopic();
  } 
  else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }   
}

void publishMessage(){
  static int value; // 客户端发布信息用数字

  String topicString = "hardwareTopic";
  
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());

  String messageString = megaInfo; 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  
  // 实现ESP8266向主题发布信息
  if(mqttClient.publish(publishTopic, publishMsg)){
    Serial.println("Publish Topic:");Serial.println(publishTopic);
    Serial.println("Publish message:");Serial.println(publishMsg);    
  } 
  else {
    Serial.println("Message Publish Failed."); 
  }
}

void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
  webInfo = payload[0];
  getWebInfo();
}

// 订阅指定主题
void subscribeTopic(){
  // 订阅主题，并通过串口监视器输出是否成功、订阅主题、以及订阅的主题名称
  String subTopic = "webTopic";
  if(mqttClient.subscribe(subTopic.c_str())){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic);
  } else {
    Serial.print("Subscribe Fail...");
  }  
}

void getMegaInfo(){
  if (Serial.available()) {
    megaInfo = Serial.readStringUntil('|'); // 读取串口直到换行符
    Serial.print("Data from Mega: "); 
    Serial.println(megaInfo);
    publishMessage();
  }
}

void getWebInfo(){
  Serial.println("Sending to Mega: " + webInfo);
  Serial.write(webInfo);  // 发送数据到 Mega
}