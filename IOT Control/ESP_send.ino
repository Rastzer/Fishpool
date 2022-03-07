#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "https://fishpoolv1-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "mO1oZm1BW0Z7lQmwFDmRJ08bSqrR14UIv2yrXdIL"
#define WIFI_SSID "abc"
#define WIFI_PASSWORD "1234567890"

FirebaseData firebaseData;

String dataIn;
String dt[5];
int i;
boolean parsing = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dataIn = "";
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
    if (parsing) {
      parsingData();
      parsing = false;
      dataIn = "";
    }
  }
}

void parsingData() {
  int j = 0;
  Serial.print("data masuk: ");
  Serial.print(dataIn);
  Serial.print("\n");
  dt[j] = "";
  for (i = 1; i < dataIn.length(); i++) {
    if ((dataIn[i] == '#') || (dataIn[i] == ',')) {
      j++;
      dt[j] = "";
    }
    else {
      dt[j] = dt[j] + dataIn[i];
    }
  }
  Serial.print("data1:\t");
  Serial.println(dt[0].toInt());
  Serial.print("data2:\t");
  Serial.println(dt[1].toFloat());
  Serial.print("data3:\t");
  Serial.println(dt[2].toFloat());
  Serial.print("data4:\t");
  Serial.println(dt[3].toFloat());

  int detect = dt[0].toInt();
  float suhu = dt[1].toFloat();
  float ph = dt[2].toFloat();
  float oxy = dt[3].toFloat();

  if (Firebase.setInt(firebaseData, "/Hasil_Pembacaan/ultrasonik", detect)) {
    Serial.println("Ikan Terdeteksi");
  }
  else {
    Serial.println("Ikan Aman");
    Serial.println("Karena: " + firebaseData.errorReason());
  }
  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/suhu", suhu)) {
    Serial.println("Suhu terkirim");
  } else {
    Serial.println("Suhu tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/ph", ph)) {
    Serial.println("PH terkirim");
  } else {
    Serial.println("PH tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/oxygen", oxy)) {
    Serial.println("Oxygen terkirim");
  } else {
    Serial.println("Oxygen tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  delay(1000);

}
