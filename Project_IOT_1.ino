#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 1    // Sử dụng chân GPIO 1 cho DHT11
#define DHTTYPE DHT11
#define PHOTO_PIN A0 // Sử dụng chân ADC A0 cho quang trở
#define LED_PIN 5    // Sử dụng chân GPIO 5 cho đèn LED
#define FAN_PIN 2    // Sử dụng chân GPIO 2 cho quạt

const char* ssid = "PR HOME 22 TB";
const char* password = "0944811818";
const char* mqttServer = "192.168.1.12"; // Địa chỉ IP hoặc tên miền của MQTT broker
const int mqttPort = 1883;
const char* mqttUser = "admin";
const char* mqttPassword = "12345678";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(10);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Kết nối WiFi...");
    digitalWrite(LED_PIN, LOW);
  }
  Serial.println("Đã kết nối WiFi");

  // Kết nối MQTT và thiết lập hàm callback
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // Đợi kết nối thành công MQTT
  while (!client.connected()) {
    Serial.println("Kết nối MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("Đã kết nối MQTT");
      client.subscribe("control");
    } else {
      Serial.print("Kết nối thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây");
      delay(5000);
    }
  }

  // Khởi động DHT
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
}

void loop() {
  // Kiểm tra kết nối MQTT
  if (!client.connected()) {
    reconnect();
  }

  // Đọc dữ liệu từ DHT
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Đọc giá trị từ quang trở
  int brightness = analogRead(PHOTO_PIN); // Chia cho 1000 và chuyển đổi thành độ sáng

  // Tạo JSON message
  String message = "{\"temp\": " + String(temp) + ", \"humidity\": " + String(humidity) + ", \"brightness\": " + String(brightness) + "}";

  // Publish message vào kênh "data"
  client.publish("data", message.c_str());

  // Duyệt thông điệp và gọi callback khi nhận được
  client.loop();

  delay(5000); // Đợi một khoảng thời gian trước khi gửi dữ liệu tiếp
}

void callback(char* topic, byte* payload, unsigned int length) {
  String strPayload = "";
  for (int i = 0; i < length; i++) {
    strPayload += (char)payload[i];
  }

  if (String(topic) == "control") {
    Serial.print("Nhận thông điệp trên kênh control: ");
    Serial.println(strPayload);

    // So sánh giá trị payload với "true" và "false" và bật/tắt đèn tương ứng
    int spaceIndex = strPayload.indexOf(' ');
    if (spaceIndex >= 0) {
      String part1 = strPayload.substring(0, spaceIndex);
      String part2 = strPayload.substring(spaceIndex + 1);

      // So sánh giá trị của hai phần và bật/tắt đèn và quạt tương ứng
      if (part1 == "true") {
        digitalWrite(LED_PIN, HIGH); // Bật đèn
      } else if (part1 == "false") {
        digitalWrite(LED_PIN, LOW); // Tắt đèn
      }

      if (part2 == "true") {
        digitalWrite(FAN_PIN, HIGH); // Bật quạt
      } else if (part2 == "false") {
        digitalWrite(FAN_PIN, LOW); // Tắt quạt
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("Kết nối thành công");
      client.subscribe("control");
    } else {
      Serial.print("Kết nối thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây");
      delay(5000);
    }
  }
}
