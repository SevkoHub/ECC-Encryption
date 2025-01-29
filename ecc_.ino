#include <WiFi.h>
#include <WebServer.h>
#include <tinyECC.h>

// WiFi bilgileri
const char* ssid = "Cj";          // WiFi ağınızın adı
const char* password = "40102516"; // WiFi şifreniz

// WebServer
WebServer server(80);

// Sensör pinleri
#define echoPin 2
#define trigPin 4

// Mesafe ve şifreleme değişkenleri
long duration;
float distance;
tinyECC ecc;

void setup() {
  Serial.begin(115200);

  // HC-SR04 pin ayarları
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // WiFi'ye bağlan
  Serial.println("WiFi'ye bağlanılıyor...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi bağlantısı başarılı!");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());

  // Web sunucusu endpoint'ini tanımla
  server.on("/data", []() {
    // Mesafe ölçümü
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;

    // Mesafeyi şifrele
    String distanceStr = String(distance, 2); // 2 basamaklı float
    ecc.plaintext = distanceStr;
    ecc.encrypt();

    // Şifreleme kontrolü
    if (ecc.ciphertext.length() == 0) {
      Serial.println("Şifreleme başarısız!");
      server.send(500, "text/plain", "Şifreleme hatası!");
      return;
    }

    // JSON formatında cevap oluştur
    String response = "{";
    response += "\"encrypted_data\": \"" + ecc.ciphertext + "\"";
    response += "}";

    // Yanıt gönder
    server.send(200, "application/json", response);
  });

  // Sunucuyu başlat
  server.begin();
  Serial.println("Web sunucusu başlatıldı!");
}

void loop() {
  // Web sunucusunu dinle
  server.handleClient();
}
