#define BLYNK_TEMPLATE_ID "TMPL6ofIEFl_Q"
#define BLYNK_TEMPLATE_NAME "HienThiCamBien"
#define BLYNK_AUTH_TOKEN "95UA4sFr1G0i-wXqMcOcWvy9bJYLX6Sj"

#define BLYNK_PRINT Serial

unsigned long times=millis();

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
BlynkTimer timer; // Thư viện hẹn giờ của Blynk

// Update these with values suitable for your network.

#define KHIGAS V1 // Biến hiển thị khí gas lên blynk
#define TRANGTHAICANHBAO V4 // Biến hiển thị trạng thái cảnh báo lên blynk

int mq2_value;

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
int FAN = D0; // Kết nối tín hiệu quạt với D0
int buzzer = D4; // Kết nối còi với D4
int led = D6; // Kết nối led bật chế độ cảnh báo D6

int gas; // Biến lưu nồng độ khí gas

boolean runmode = 0; // Biến lưu giá trị khi chạy chế độ cảnh báo

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

  // Kiểm tra topic mqtt controlData/buzzer để bật còi cảnh báo 
  if (strcmp(topic, "controlData/buzzer") == 0) {
    if(payload[0] != '0'){
     digitalWrite(buzzer, LOW);

    }else{
     digitalWrite(buzzer, HIGH);

    }
  }

  // Kiểm tra topic mqtt controlData/led để bật led chế độ cảnh báo 
  if (strcmp(topic, "controlData/led") == 0) {
    if(payload[0] != '0'){
      runmode = 1;
      digitalWrite(led, HIGH);

    }else{
      runmode = 0;
      digitalWrite(led, LOW);
    }
  }

  // Kiểm tra topic mqtt controlData/baodong để bật đèn, còi, quạt hút gas
  if (strcmp(topic, "controlData/baodong") == 0 && runmode) {
    Blynk.virtualWrite(TRANGTHAICANHBAO,0);
    if(payload[0] != '0'){
      digitalWrite(led, HIGH);
      digitalWrite(buzzer, LOW);
      Blynk.virtualWrite(TRANGTHAICANHBAO,1);
      digitalWrite(FAN, HIGH);



    }else{
    }
  }else{
      digitalWrite(FAN, LOW);
      digitalWrite(buzzer, HIGH);

  }

  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // publish topic sensingData/gas đến mqtt để gửi tín hiệu đến gateway đăng ký
      client.publish("sensingData/gas", "hello IoT Gateway...");

      // publish topic subcribe controlData để nhận tín hiệu điều khiển từ gateway
      client.subscribe("controlData/buzzer");
      client.subscribe("controlData/led");
      client.subscribe("controlData/baodong");

     
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // Thiết lập giá trị khởi tạo còi, còi, quạt thoát khí
  tone(buzzer, 1000);
  digitalWrite(buzzer, HIGH);
  pinMode(led, OUTPUT);
  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, LOW);

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssids, pass);
  client.setCallback(callback);

}

void loop() {
  timer.run();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

 
  unsigned long now = millis();
  if (now - lastMsg > 2000) { // Gửi giá trị cảm biến mỗi 2s
    lastMsg = now;
    gas = analogRead(A0); // Đọc giá trị cảm biến khí gas
    Blynk.virtualWrite(KHIGAS,gas); // Gửi giá trị lên Blynk 

    // publish giá trị lên topic
    snprintf (msg, MSG_BUFFER_SIZE, "%d", gas); 
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensingData/gas", msg);


  }
}
