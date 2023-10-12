#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <DHT.h> 
#include <WiFiClientSecure.h>


/**** DHT11 sensor Settings *******/
#define DHTPIN 14   //Set DHT pin as GPIO2
#define DHTTYPE DHT11

/**** LED + FAN Settings *******/
#define LED_PIN 5 //Set LED pin as GPIO5
#define FAN_PIN 4 //Set FAN pin as GPIO4

/**** PHOTON Settings *******/
#define PHOTO_PIN A0 

//WIFI
const char* ssid = "PR HOME 22 TB";
const char* password = "0944811818";


// CLOUD BROKER SERVER
const char* mqttServer = "345580b4244e453588bc751f2d18be9d.s1.eu.hivemq.cloud"; // Địa chỉ IP hoặc tên miền của MQTT broker
const int mqttPort = 8883;
const char* mqttUser = "admin";
const char* mqttPassword = "12345678";

// LOCAL BROKER SERVER
// const char* mqttServer = "192.168.1.12"; // Địa chỉ IP hoặc tên miền của MQTT broker
// const int mqttPort = 1883;
// const char* mqttUser = "admin";
// const char* mqttPassword = "12345678";


/**** Secure WiFi Connectivity Initialisation *****/
WiFiClientSecure espClient;
/**** MQTT Client Initialisation Using WiFi Connection *****/
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

/****** root certificate *********/

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

/************* Connect to WiFi ***********/
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

/************* Connect to MQTT Broker ***********/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe("control");   // subscribe the topics here
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//handle received message from topic control
void callback(char* topic, byte* payload, unsigned int length) {
  String strPayload = "";
  for (int i = 0; i < length; i++) {
    strPayload += (char)payload[i];
  }

  if (String(topic) == "control") {
    Serial.print("Nhận thông điệp trên kênh control: ");
    Serial.println(strPayload);

    // Compare payload with "true" and "false" after on/off to the light
    int spaceIndex = strPayload.indexOf(' ');
    if (spaceIndex >= 0) {
      String part1 = strPayload.substring(0, spaceIndex);
      String part2 = strPayload.substring(spaceIndex + 1);

      // Compare payload with "true" and "false" after on/off to the light and fan
      if (part1 == "true") {
        digitalWrite(LED_PIN, HIGH); // Light on
      } else if (part1 == "false") {
        digitalWrite(LED_PIN, LOW); // Light off
      }

      if (part2 == "true") {
        digitalWrite(FAN_PIN, HIGH); // Fan on 
      } else if (part2 == "false") {
        digitalWrite(FAN_PIN, LOW); // Fan off
      }
    }
  }
}


void setup() {
  dht.begin(); 
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  Serial.begin(9600);
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else
    espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  #endif

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

/******** Main Function *************/
void loop() {
  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int brightness = analogRead(PHOTO_PIN);
  if (isnan(temperature) || isnan(humidity) || isnan(brightness)) {
    Serial.println("error");
  } else {
    String message = "{\"temp\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"brightness\": " + String(brightness) + "}";
    client.publish("data", message.c_str());
  }
  delay(5000);
}
