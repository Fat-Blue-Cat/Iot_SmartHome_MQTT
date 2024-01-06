#define BLYNK_TEMPLATE_ID "TMPL6ofIEFl_Q"
#define BLYNK_TEMPLATE_NAME "HienThiCamBien"
#define BLYNK_AUTH_TOKEN "95UA4sFr1G0i-wXqMcOcWvy9bJYLX6Sj"

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
BlynkTimer timer; 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Servo.h>

// Câu hình Blynk
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


//Khai báo chân của cảm biến nhiệt độ bật quạt(Phòng khách)
#define DHTPIN D5     //Chân kỹ thuật số được kết nối với cảm biến DHT
#define DHTTYPE DHT22   // DHT 22
#define FAN_PIN D2   // FAN RELAY

//khai báo chân của cảm biến hồng ngoại bật đèn(Phòng khách)
#define SENSOR_PIN D0     // Chân kết nối Sensor Module
#define LED_PIN D1        // Chân kết nối đèn

//Khao báo chân cảm biến mưa thu quần áo (Sân nhà)
#define RAIN_SENSOR_PIN D3      // Chân kết nối cảm biến mưa
#define SERVO_PIN D4     // Chân kết nối với servo là mái

// KHAI báo biến blynk
#define NHIETDO V6 
#define DOAM V8
#define HONGNGOAI V10
#define CAMBIENMUA V12



float humDHT = 0; //Biến lưu giá trị độ ẩm
float tempDHT = 0; // Biến lưu giá trị nhiệt độ
DHT dht(DHTPIN, DHTTYPE);
Servo myServo;
int sensorValue; // Biến lưu giá trị hồng ngoại
int rainState = 1;  // Biến lưu giá trị cảm biến mưa

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

  // Kiểm tra tín hiệu gửi đến từ controlData/maiHien để điều khiển mái hiên
  if (strcmp(topic, "controlData/maiHien") == 0) {
    if(payload[0] != '0'){
      myServo.write(180);

    }else{
      myServo.write(0);
    }
  }

  // Kiểm tra tín hiệu gửi đến từ controlData/quatKhach để điều khiển quạt phòng khách
  if (strcmp(topic, "controlData/quatKhach") == 0) {
    if(payload[0] != '0'){
     digitalWrite(FAN_PIN , HIGH);

    }else{
     digitalWrite(FAN_PIN, LOW);

    }
  }

  // Kiểm tra tín hiệu gửi đến từ controlData/denKhach để điều khiển đèn phòng khách
  if (strcmp(topic, "controlData/denKhach") == 0) {
    if(payload[0] != '0'){
      digitalWrite(LED_PIN, HIGH);

    }else{
      digitalWrite(LED_PIN, LOW);
    }
  }

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // publish thông số nhiệt độ, độ ẩm, hồng ngoại phòng khách và thông số cảm biến mưa đến IoT gate way 
      client.publish("sensingData/humi", "hello IoT Gateway...");
      client.publish("sensingData/temp", "hello IoT Gateway...");
       client.publish("sensingData/hongNgoai", "hello IoT Gateway...");
      client.publish("sensingData/rain", "hello IoT Gateway...");

      // Nhận giá trị từ các topic để điều khiển cơ cấu chấp hành 
      client.subscribe("controlData/maiHien");
      client.subscribe("controlData/quatKhach");
      client.subscribe("controlData/denKhach");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {

  //_________________________Phòng Khách__________________________________//
   //cảm biến nhiệt độ
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  Serial.println(F("DHTxx test!"));
  dht.begin();

  //Cảm biến hồng ngoại
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  //_________________________Sân nhà__________________________________//
  //cảm biến mưa
  pinMode(RAIN_SENSOR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);  

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssids, pass);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    // Đọc giá trị cảm biến phòng khác
    humDHT = dht.readHumidity();
    Blynk.virtualWrite(DOAM,humDHT);
    // Đọc nhiệt độ theo độ C (mặc định)
    tempDHT = dht.readTemperature();
    Blynk.virtualWrite(NHIETDO,tempDHT); // Gửi dữ liệu cảm biến lên Blynk
    sensorValue = digitalRead(SENSOR_PIN); //Đọc giá trị cảm biến hồng ngoại
    Blynk.virtualWrite(HONGNGOAI,sensorValue);


    // ----- Đọc giá trị cảm biến mưa----------
    rainState = digitalRead(RAIN_SENSOR_PIN);
    Blynk.virtualWrite(CAMBIENMUA,rainState); // Gửi dữ liệu cảm biến mưa lên Blynk

    // Kiểm tra xem có lỗi đọc nào không và thoát sớm (để thử lại).
    if (isnan(humDHT) || isnan(tempDHT))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    // publish cảm biến phòng khách nhiệt độ, độ ẩm
    snprintf (msg, MSG_BUFFER_SIZE, "%.2f", humDHT);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/humi", msg);

    snprintf (msg, MSG_BUFFER_SIZE, "%.2f", tempDHT);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/temp", msg);


    // publish cảm biến mưa
    snprintf (msg, MSG_BUFFER_SIZE, "%d", rainState);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/rain", msg);

    // publish cảm biến hồng ngoại
    snprintf (msg, MSG_BUFFER_SIZE, "%d", sensorValue);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/hongNgoai", msg);
    
  }
}
