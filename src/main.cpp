#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <BH1750.h>


// Replace with your network credentials
const char* ssid = "WIR";
const char* password = "WIRgroup123";


const char* webhooksKey = "bo_CAS2d4lLs8RSwMExc8VhO2AaFUbP6KclvebP_hWm"; 
const char* eventName = "BH1750_data"; 
BH1750 lightMeter;


// Initialize Telegram BOT
#define BOTtoken "6575095194:AAHLwyT678fB0lMhjISzhbIscJj2EBKKzHY"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "6694474908"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

String myLink = "https://docs.google.com/spreadsheets/d/1ot3f8Qk4MEIRKPFFWXoPq9TGfq7vGHb0yJaJXHWXrLw/edit?usp=sharing"; // Replace with your desired link


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

unsigned long previousMillis = 0; 


const long interval = 1000; 

bool isNetworkConnected() {
  return WiFi.status() == WL_CONNECTED;
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  lightMeter.begin();

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

   if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/led_on to turn GPIO ON \n";
      welcome += "/led_off to turn GPIO OFF \n";
      welcome += "/state to request current GPIO state \n";
      welcome += "/wifi to get Wi-Fi information \n";
      welcome += "/light to get light intensity \n";
      welcome += "/mylink to get the link \n"; // Added link command
      bot.sendMessage(chat_id, welcome, "");
    }

    else if (text == "/led_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    else if (text == "/led_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    else if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }

    else if (text == "/wifi") {
      // Retrieve Wi-Fi SSID, password, and local IP address
      String response = "Wi-Fi SSID: " + String(WiFi.SSID()) +
                        "\nPassword: " + String(WiFi.psk()) +
                        "\nIP Address: " + WiFi.localIP().toString();
      
      bot.sendMessage(chat_id, response, "");
    }
    else if (text == "/light") {
  float lux = lightMeter.readLightLevel();
  Serial.print("Lux: ");
  Serial.println(lux);

      String response = "Light Intensity: " + String(lux) + " lux";
      
      bot.sendMessage(chat_id, response, "");
    }

    else if (text == "/mylink") {
      // Respond with the link
      String response = "Here is the link: " + myLink;
      bot.sendMessage(chat_id, response, "");
    }
     if (text == "/network") {
      // Check the network connectivity status
      String response = isNetworkConnected() ? "Network is connected." : "Network is not connected.";
      bot.sendMessage(chat_id, response, "");
    }

  }
}

void updatespreadsheet() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Lux: ");
  Serial.println(lux);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("maker.ifttt.com", 80)) {
      String url = "/trigger/" + String(eventName) + "/with/key/" + String(webhooksKey) + "?value1=" + String(lux);
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: maker.ifttt.com\r\n" +
                   "Connection: close\r\n\r\n");
      delay(10); // Give some time for the request to be sent
    }
    client.stop();
  }

}


void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
    float lux = lightMeter.readLightLevel();
  Serial.print("Lux: ");
  Serial.println(lux);



  unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    updatespreadsheet();
  }
}

