#include "DFRobotDFPlayerMini.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


HardwareSerial mySerial(1);  // UART1 を使用（Serial1 でもOK）
DFRobotDFPlayerMini myDFPlayer;

int folder = 1, file = 100;

const char* ssid = "aiiit";
const char* password = "20070625AF";
const char* serverName = "http://10.128.19.104:3001/todos";
// const char* serverName = "http://10.128.21.205:3001/todos";

const int ledPin = 27; // ESP32の内蔵LED（ボードによっては変更）


void setup() {
  Serial.begin(115200);            // モニタ用シリアル
  mySerial.begin(9600, SERIAL_8N1, 16, 17); 
  // 9600bps, デフォルト設定, RX=GPIO16, TX=GPIO17（変更可能）

  if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    //while (true);
  }
  myDFPlayer.volume(10);  // 音量を設定（0〜30）

  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi接続中...");
  }
  Serial.println("WiFi接続完了！");
  Serial.print("EPS32 IP:");
  Serial.println(WiFi.localIP());
  Serial.print("接続先：");
  Serial.println(serverName);
}

void loop() {
  Serial.print("pre");
  Serial.println(file);
  myDFPlayer.playFolder(folder, file);

    if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println(payload); // 受け取ったJSONを出力

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      bool hasTODO = false;

      for (JsonObject todo : doc.as<JsonArray>()) {
        const char* status = todo["status"];
        if (strcmp(status, "TODO") == 0) {
          hasTODO = true;
          break;
        }
      }

      // LED制御：TODOがあれば点灯、なければ消灯
      digitalWrite(ledPin, hasTODO ? HIGH : LOW);
      if(hasTODO){
        file = 1;
      }else{
        file=100;
      }

    } else {
      Serial.println("APIからの取得に失敗");
      Serial.printf("HTTP Responce code: %d\n", httpResponseCode);
    }

    http.end();
  }
  delay(1000); // 再生間隔

  // 入力確認
  if (Serial.available() > 0) {
    char inkey = Serial.read();

    switch (inkey) {
      case 'a':
        file = 1;
        break;
      case 'b':
        file = 6;
        break;
      case 'c':
        file = 1000;
        break;
    }
  }
  Serial.println(file);
}
