#define BLYNK_TEMPLATE_ID "TMPL6ofIEFl_Q"
#define BLYNK_TEMPLATE_NAME "HienThiCamBien"
#define BLYNK_AUTH_TOKEN "95UA4sFr1G0i-wXqMcOcWvy9bJYLX6Sj"

#define BLYNK_PRINT Serial

unsigned long times=millis();


#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
BlynkTimer timer; 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <Wire.h>
#include <Servo.h>
Servo myservo;
BH1750 lightMeter; // Sử dụng thư viện của Cảm biến ánh sáng

// Cấu hình Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssids[] = "Redmi 9T";
char pass[] = "11111111";

// Cấu hình MQTT
const char* ssid = "Redmi 9T";
const char* password = "11111111";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

// Khai Bao Bien CAMBIEN ANHSANG
#define ANHSANG V14 // Lưu giá trị cảm biến lên Blynk
#define LeftMotorSpeed  4 // Biến lưu tốc độ của motor rèm
#define LeftMotorDir    2 // Biến lưu hướng của motor rèm
int servo = D0; // Cổng D0 làm cổng tín hiệu servo(cửa)
int trangThaiRem=0; // Lưu trạng thái rèm hiện tại đóng hay mở




void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Serial.println(length);
  }
  Serial.println();

 
  // Nhận tín hiệu từ topic controlData/remCua để điều khiển rèm cửa
  if (strcmp(topic, "controlData/remCua" ) == 0 ) {
    // Mở rèm cửa nếu trangthairem = 0 thì rèm chạy
    if(payload[0] != '0' && trangThaiRem==0){
      trangThaiRem = 1;
      analogWrite(LeftMotorSpeed,1020);
      digitalWrite(LeftMotorDir,HIGH);
      delay(4000);
      analogWrite(LeftMotorSpeed,0);
      digitalWrite(LeftMotorDir,LOW);

    }
    // Đóng rèm cửa nếu trangthairem = 1 thì chạy
    if(payload[0] == '0' && trangThaiRem==1){
      trangThaiRem = 0;
      analogWrite(LeftMotorSpeed,1020);
      digitalWrite(LeftMotorDir,LOW);
      delay(4000);
      analogWrite(LeftMotorSpeed,0);
      digitalWrite(LeftMotorDir,LOW);
    }
  }

  // Nhận tín hiệu từ topic controlData/cuaTuDong để mở cửa, đóng cứa tự động sau 5s
  if (strcmp(topic, "controlData/cuaTuDong" ) == 0 ) {
    if(payload[0] != '0' ){
      myservo.write(180);
      delay(5000);
      myservo.write(0);
    }else{

    }
  }

// Nhận tín hiệu từ topic controlData/servo để mở cửa, đóng cứa thủ công
  if (strcmp(topic, "controlData/servo") == 0) {
   if(payload[0] != '0'){
    myservo.write(180);

   }else{
    myservo.write(0);

    }
  }  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // publish chỉ số ánh sáng 
      client.publish("sensingData/anhsang", "hello IoT Gateway...");
     

      // subcribe các topic để nhận tín hiệu điều khiển cơ cấu chấp hành
      client.subscribe("controlData/remCua");
      client.subscribe("controlData/cuaTuDong");
      client.subscribe("controlData/servo");


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {

  pinMode(D1, INPUT); // Khởi tạo cổng D1 là cổng lưu giấ trị cảm biến chuyển động
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssids, pass);
  client.setCallback(callback);
  myservo.write(0); 
  myservo.attach(servo);
  Wire.begin();

  Wire.begin(D7, D6); // Cổng D7, D6 làm giá trị nhận cảm biến của cảm biến ánh sáng
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));

  // Cấu hình rèm
  pinMode(LeftMotorSpeed, OUTPUT);
  pinMode(LeftMotorDir, OUTPUT);

  digitalWrite(LeftMotorSpeed, LOW);
  digitalWrite(LeftMotorDir,HIGH);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000) { // Gửi tín hiệu mỗi 2s theo thời gian thực
    lastMsg = now;
    float lux = lightMeter.readLightLevel(); // Đọc giá trị cảm biến ánh sáng
    int camBienCD = digitalRead(D1); // Đọc giá trị cảm biến cd 
    Blynk.virtualWrite(ANHSANG,lux); // Gửi giá trị cảm biến lên Blynk

    // Publish giá trị từ cảm biến ánh sáng lên topic
    snprintf (msg, MSG_BUFFER_SIZE, "%.2f", lux);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/anhsang", msg);

    // Publish giá trị từ cảm biến chuyển động lên topic
    snprintf (msg, MSG_BUFFER_SIZE, "%d", camBienCD);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/camBienCD", msg);

    
  }
}
