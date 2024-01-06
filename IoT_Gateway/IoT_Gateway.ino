
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <BlynkSimpleEsp8266.h>

#include <string.h>

#define BLYNK_TEMPLATE_ID "TMPL63CBGctdQ"
#define BLYNK_TEMPLATE_NAME "NhaBep"
#define BLYNK_AUTH_TOKEN "AoN7S0BQm9QYXQvOu_jecZCBqX-VGj2z"


unsigned long times=millis();

int mucCanhbao=500; // Biến lưu mức cảnh báo gas
int mucNhietDo = 30; // Biến thiết lập nhiệt độ để tự động chạy quạt
int mucAnhSang = 200; // Biến thiết lập ánh sáng tự động kéo rèm


boolean runMode=1;//Bật/tắt chế độ cảnh báo
boolean cuaState=0; // Lưu trạng thái cửa

boolean quatState=0; // Lưu trạng thái quạt
boolean denState=0; // Lưu trạng thái đèn
boolean maiHienState=0; // Lưu trạng thái mái
boolean remCuaState=0; // Lưu trạng thái rèm

// KHAI BÁO BIẾN BLYNK ĐIỀU KHIỂN
WidgetLED led(V0); // Biến LED Widget của Blynk
#define MUCCANHBAO V2 // Nồng độ khi gas thiết lập
#define RUNMODE V3 // Bật chế độ tự động cho tòa nhà
#define SERVO V5 // Điều khiển cửa 

// KHAI báo blynk nhiệt độ phòng khách
#define MUCNHIETDO V7 // Muc Nhiệt độ thiết lập chạy quạt
#define QUATKHACH V9 // Bật tăt quạt phòng khác
// KHái báo đèn
#define DENKHACH V11 // Bật tắt đèn phòng khách
// Khai báo mưa
#define MAIHIEN V13 // Bật tắt mái sân phơi đồ

// Khai báo biến phòng ngủ
#define MUCANHSANG V15 // Mức ánh sáng thiết lập kéo rèm
#define REMCUA V16 // Kèo rèm


BlynkTimer timer; // Thư viện hẹn giờ của Blynk

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
  }
  Serial.println();
  // Lấy giá trị cảm biến gas từ topic sensingData/gas
  if (strcmp(topic, "sensingData/gas") == 0) {
    char payloadString[length + 1]; 
    for (int i = 0; i < length; i++) {
      payloadString[i] = (char)payload[i];
    }
    payloadString[length] = '\0'; 
    int gasValue = atoi(payloadString); // Chuyển gas về kiểu int đọc từ topic
    Blynk.virtualWrite(MUCCANHBAO,mucCanhbao); // Đồng bộ mức cảnh báo
    
    snprintf (msg, MSG_BUFFER_SIZE, "%d",runMode);
    // Kiểm tra trạng thái runMode bật hay tắt rồi publish lên topic
    client.publish("controlData/led", msg); 
    // Kiểm tra nồng độ khí gas vượt quá mức thiết lập không
    if(gasValue > mucCanhbao){
      Serial.println("Khí Gas Cao");
      // Trả về cảnh báo
      Blynk.logEvent("canhbao", String("Cảnh báo! Khí gas=" + String(gasValue)+" vượt quá mức cho phép!"));
      if(runMode==HIGH){
        snprintf (msg, MSG_BUFFER_SIZE, "%d",1);
      }
      client.publish("controlData/baodong", msg);
    }else{
      snprintf (msg, MSG_BUFFER_SIZE, "%d",0);
      client.publish("controlData/baodong", msg);

    }
  }

  // Lấy tín hiệu trả về từ topic sensingData/temp
  if (strcmp(topic, "sensingData/temp") == 0) {
    char payloadString[length + 1]; // +1 để chứa ký tự null kết thúc chuỗi
    for (int i = 0; i < length; i++) {
      payloadString[i] = (char)payload[i];
    }
    payloadString[length] = '\0'; // Ký tự null kết thúc chuỗi
    float nhietdo = atoi(payloadString);
    Serial.println(nhietdo);
    // Nếu nhiệt độ cao hơn nhiệt độ thiết lập bật quạt
    if(nhietdo > mucNhietDo){
      Serial.println("Quạt Chạy");
      if(runMode==HIGH){
        Blynk.virtualWrite(QUATKHACH,1);
        snprintf (msg, MSG_BUFFER_SIZE, "%d",1);
        client.publish("controlData/quatKhach", msg);

      }

      
    }else{

    }
  }

    // Lấy tín hiệu trả về từ topic sensingData/hongNgoai
  if (strcmp(topic, "sensingData/hongNgoai") == 0) {
    char payloadString[length + 1]; // +1 để chứa ký tự null kết thúc chuỗi
    for (int i = 0; i < length; i++) {
      payloadString[i] = (char)payload[i];
    }

    // Điều khiển đèn bật nếu tín hiệu trả về là 0 và ngược lại
    if((char)payload[0] ==  '0'){
      Serial.println("Đèn Chạy");
      if(runMode==HIGH){
        Blynk.virtualWrite(DENKHACH,1);
        snprintf (msg, MSG_BUFFER_SIZE, "%d",1);
        client.publish("controlData/denKhach", msg);
      }      
    }else{
      if(runMode==HIGH){
      Blynk.virtualWrite(DENKHACH,0);
      snprintf (msg, MSG_BUFFER_SIZE, "%d",0);
      client.publish("controlData/denKhach", msg);
      }

    }
  }

  // Lấy tín hiệu trả về từ topic sensingData/rain
  if (strcmp(topic, "sensingData/rain") == 0) {
    char payloadString[length + 1]; // +1 để chứa ký tự null kết thúc chuỗi
    for (int i = 0; i < length; i++) {
      payloadString[i] = (char)payload[i];
    }


    // Nếu tín hiệu mưa là 0 thi mái chạy và ngược lại
    if((char)payload[0] ==  '0'){
      if(runMode==HIGH){
        Blynk.virtualWrite(MAIHIEN,1);
        snprintf (msg, MSG_BUFFER_SIZE, "%d",1);
        client.publish("controlData/maiHien", msg);

      }   
    }else{
      if(runMode==HIGH){
        Blynk.virtualWrite(MAIHIEN,0);
        snprintf (msg, MSG_BUFFER_SIZE, "%d",0);
        client.publish("controlData/maiHien", msg);

      }

    }
  }

  // Lấy tín hiệu trả về từ topic sensingData/anhsang
  if (strcmp(topic, "sensingData/anhsang") == 0) {
    char payloadString[length + 1]; // +1 để chứa ký tự null kết thúc chuỗi
    for (int i = 0; i < length; i++) {
      payloadString[i] = (char)payload[i];
    }
    payloadString[length] = '\0'; // Ký tự null kết thúc chuỗi
    float anhsang = atoi(payloadString);
    Serial.println(anhsang);
    // Nếu mức ánh sáng lớn hơn mức ánh sáng thiết lập kéo rèm che và ngược lại
    if(anhsang > mucAnhSang){
      Serial.println("Rèm Chạy");
      if(runMode==HIGH){
        Blynk.virtualWrite(REMCUA,1);
        snprintf (msg, MSG_BUFFER_SIZE, "%d",1);
        client.publish("controlData/remCua", msg);

      }

      
    }else{
      if(runMode==HIGH){
        Blynk.virtualWrite(REMCUA,0);
        snprintf (msg, MSG_BUFFER_SIZE, "%d",0);
        client.publish("controlData/remCua", msg);
      }
    }
  }


  // Lấy tín hiệu trả về từ topic sensingData/camBienCD
  if (strcmp(topic, "sensingData/camBienCD") == 0) {
    char payloadString[length + 1]; // +1 để chứa ký tự null kết thúc chuỗi
    for (int i = 0; i < length; i++) {
      payloadString[i] = (char)payload[i];
    }

    // Nếu tín hiệu bằng 1 thì mở cửa ngược lại đóng
    if((char)payload[0] ==  '1'){
      Serial.println("Mở cửa");
      if(runMode==HIGH){
        snprintf (msg, MSG_BUFFER_SIZE, "%d",1);
        client.publish("controlData/cuaTuDong", msg);
      }     
    }else{
    }
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      // Publish các giá trị để điều khiển các IoTnode 
      Serial.println("connected");
      client.publish("controlData/baodong", "hello IoT Node...");
      client.publish("controlData/runmode", "%d",0);
      client.publish("controlData/led", "%d",0);
      client.publish("controlData/remCua","hello IoT Node...");
      client.publish("controlData/servo","hello IoT Node...");
      // ... and resubscribe
      // Subcribe để lấy giá trị từ IoT để thực hiện logic 
      client.subscribe("sensingData/gas");
      client.subscribe("sensingData/temp");
      client.subscribe("sensingData/hongNgoai");
      client.subscribe("sensingData/rain");
      client.subscribe("sensingData/anhsang");
      client.subscribe("sensingData/camBienCD");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Kiểm tra kết nối
void blinkLedWidget(){
  if (led.getValue()) {
    led.off();
  } else {
    led.on();
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssids, pass);
  delay(100);
  timer.setInterval(1000L, blinkLedWidget); // Tạo hiệu ứng nhấp nháy cho LED Widget trong blynk mỗi 1s kiểm tra kết nốt

}

void loop() {
  timer.run();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

 // Đồng bộ tín hiệu với ứng dụng
BLYNK_CONNECTED() {
  Blynk.syncVirtual(MUCANHSANG,RUNMODE,MUCCANHBAO,SERVO,MUCNHIETDO,DENKHACH,MAIHIEN,REMCUA);
}

// Lấy giá trị từ Blynk để thiết lập mức cảnh báo
BLYNK_WRITE(MUCCANHBAO) {
  mucCanhbao = param.asInt();
  Blynk.virtualWrite(MUCCANHBAO,mucCanhbao);

}
// Lấy giá trị từ Blynk để bật tắt đèn
BLYNK_WRITE(RUNMODE) {
  runMode = param.asInt();
  snprintf (msg, MSG_BUFFER_SIZE, "%d",runMode);
  client.publish("controlData/led", msg);
}

// Lấy giá trị từ Blynk để điều khiển cửa
BLYNK_WRITE(SERVO){
  cuaState = param.asInt();
  snprintf (msg, MSG_BUFFER_SIZE, "%d",cuaState);
  client.publish("controlData/servo", msg);
}

// Lấy giá trị từ Blynk để thiết lập mức nhiệt độ 
BLYNK_WRITE(MUCNHIETDO) {
  mucNhietDo = param.asInt();
  Blynk.virtualWrite(MUCNHIETDO,mucNhietDo);

}
// Lấy giá trị từ Blynk để điều khiển quạt
BLYNK_WRITE(QUATKHACH){
  quatState = param.asInt();
  snprintf (msg, MSG_BUFFER_SIZE, "%d",quatState);
  client.publish("controlData/quatKhach", msg);
}

// Lấy giá trị từ Blynk để điều khiển đèn
BLYNK_WRITE(DENKHACH){
  denState = param.asInt();
  snprintf (msg, MSG_BUFFER_SIZE, "%d",denState);
  client.publish("controlData/denKhach", msg);
}

// Lấy giá trị từ Blynk để điều khiển mái hiên
BLYNK_WRITE(MAIHIEN){
  maiHienState = param.asInt();
  snprintf (msg, MSG_BUFFER_SIZE, "%d",maiHienState);
  client.publish("controlData/maiHien", msg);
}

// Lấy giá trị từ Blynk để thiết lập mức ánh sáng
BLYNK_WRITE(MUCANHSANG) {
  mucAnhSang = param.asInt();
  Blynk.virtualWrite(MUCANHSANG,mucAnhSang);

}
// Lấy giá trị từ Blynk để điều khiển rèm cửa
BLYNK_WRITE(REMCUA) {
  remCuaState = param.asInt();
  snprintf (msg, MSG_BUFFER_SIZE, "%d",remCuaState);
  client.publish("controlData/remCua", msg);

}
