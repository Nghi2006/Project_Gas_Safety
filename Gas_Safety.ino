#include <WiFiS3.h>

// 1. WIFI HOTSPOT SETTINGS
char ssid[] = "My_ssid";        
char pass[] = "My_pass"; 

// 2. BLYNK IOT SETTINGS (Using Web API to bypass library errors)
char auth[] = "BT59vqMgiMYLzKFu_40cAzBYPqh-QGlH";
char server[] = "sgp1.blynk.cloud"; // Asia Server for better stability

// 3. PIN DEFINITIONS
const int gasPin = A0;    // MQ-2 Sensor connected to A0
const int ledPin = 13;    // LED connected to Pin 13 (use 220 ohm resistor)
const int buzzerPin = 12; // Buzzer connected to Pin 12

// 4. THRESHOLD SETTINGS
int threshold = 150;      // Alert triggers above this value
int safeLimit = 110;      // Alert stops only when gas drops below this
bool isAlerted = false;   // Flag to prevent spamming notifications

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(gasPin, INPUT);

  // Ensure devices are OFF at startup
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected - System Ready!");
}

// Function to send Push Notification via Blynk Webhook
void sendBlynkAlert() {
  WiFiClient client;
  if (client.connect(server, 80)) {
    // Constructing the Web API URL
    String url = "/external/api/logEvent?token=";
    url += auth;
    url += "&code=gas_alert"; // Must match the Event Code in Blynk Console
    
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
    
    Serial.println(">>> ALERT SENT TO SMARTPHONE!");
    client.stop();
  } else {
    Serial.println(">>> Error: Could not connect to Blynk Server");
  }
}

void loop() {
  int gasValue = analogRead(gasPin);
  
  Serial.print("Current Gas Level: ");
  Serial.println(gasValue);


  // --- ALERT CONDITION ---
  if (gasValue > threshold) {
    digitalWrite(ledPin, HIGH); 
    
    // Buzzer beeping logic (Tít... Tít...)
    digitalWrite(buzzerPin, HIGH);
    delay(200);                   // Beep for 0.2s
    digitalWrite(buzzerPin, LOW);
    delay(200);                   // Silence for 0.2s

    if (!isAlerted) {
      sendBlynkAlert(); 
      isAlerted = true; 
    }
  }
  
  
  // --- SAFE CONDITION (Hysteresis) ---
  else if (gasValue < safeLimit) {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    isAlerted = false; // Reset flag to allow future alerts
  }

  delay(1000); // Check every 1 second
}