#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LoRa.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

#define ss 2
#define rst 13
#define dio0 12

unsigned long previousMillis = 0;  
const long interval = 5000;  

int lcdColumns = 16;
int lcdRows = 2;
String v = "" ;

AsyncWebServer server(80); 
AsyncWebSocket ws("/ws");

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

void onReceive(int packetSize){
  
  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    Serial.print(receivedData);
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
    v = receivedData;
    
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    // Si deseas puedes procesar o ponerle condición al mensaje recibido, en este caso solo imprime el mensaje recibido
    Serial.println((char*)data);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  // Procesamos los mensajes recibidos
  if(type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

void setup() {
  Serial.begin(9600);
  
  // Configura el ESP32 en modo AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_AP");
  
  Serial.println("LoRa Sender");
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(433E6)) {
    Serial.print(".");
    delay(500);
  }
  //LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  LoRa.setSpreadingFactor(12);

  // Define el manejador del Websocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

   // Ruta del servidor para mostrar la interfaz web (HTML)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><head><style>body { background-color: #333; color: #FFFF; }</style>";
    html += "<script>var ws=new WebSocket('ws://'+location.hostname+':80/ws');ws.onmessage=function(evt){var values=evt.data.split(',');document.getElementById('value1').innerHTML=values[0];document.getElementById('value2').innerHTML=values[1];};</script></head>";
    html += "<body><h2>Message Value:</h2><p id='value1'></p><h2>Rssi Value:</h2><p id='value2'></p></body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
    int rss = LoRa.packetRssi();
    //ws.textAll(v + " with Rssi " + String(rss));
    ws.textAll(v + "," +rss);
    //delay(1000);
    unsigned long currentMillis = millis();
    onReceive(LoRa.parsePacket());

    if (currentMillis - previousMillis >= interval) {
   
       previousMillis = currentMillis;
       lcd.clear();
    }

    lcd.setCursor(0, 0);
    lcd.print(v); 
    lcd.setCursor(0, 1);
    lcd.print("RSSI "); 
    lcd.setCursor(6, 1);
    lcd.print(rss);
    
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT) {
    Serial.println("Websocket client connection received");
  } else if(type == WS_EVT_DISCONNECT) {
    Serial.println("Client disconnected");
    } else if(type == WS_EVT_DATA) {
    // Handle the data from the client
  }
}
