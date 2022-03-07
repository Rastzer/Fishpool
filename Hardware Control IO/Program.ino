#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire pin_DS18B20(2);
DallasTemperature DS18B20(&pin_DS18B20);

#define DO_PIN A1
#define VREF 5000
#define ADC_RES 1024

#define TWO_POINT_CALIBRATION 0

#define READ_TEMP (25)

#define CAL1_V (2109) //mv
#define CAL1_T (30.38)   //℃
#define CAL2_V (1300) //mv
#define CAL2_T (15)   //℃

String strdetect, strdatatemp, strdata, stroxy;

unsigned int trigPin[12] = {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44};
unsigned int echoPin[12] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45};

const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

int relay = 3;

uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;

long duration[12];
unsigned int distance[12];
bool detect1 = false;
bool detect2 = false;
bool detect3 = false;
bool detect = false;

int pHSense = A0;
int samples = 10;
float adc_resolusi = 1024.0;

float data, datatemp, oxy;

unsigned long TimeA, TimeB, TimeC, TimeD, TimeE, TimeF, TimeG;
unsigned long interval = 50000;

float ph(float volt) {
  return 7 + ((2.5 - volt) / 0.18);
}

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
#if TWO_POINT_CALIBRATION == 0
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200);
  for (int i = 0; i < 10; i++) {
    pinMode(trigPin[i], OUTPUT);
    pinMode(echoPin[i], INPUT);
  }
  DS18B20.begin();
  pinMode(relay, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial2.print('*');
  ultrasonic();
  suhu_sens();
  ph_sens();
  oxygen_sens();
  //  strdetect = String (detect);
  //  strdatatemp = String (datatemp);
  //  strdata = String (data);
  //  stroxy = String (oxy);
  //Serial2.println('*' + strdetect + ',' + strdatatemp + ',' + strdata + ',' + stroxy + '#');
  Serial2.println('#');
  delay(500);
}

//------------- program sensor ultrasonic -----------------//
void ultrasonic() {
  TimeA = millis();
  for (int i = 0; i < 10; i ++) {
    digitalWrite(trigPin[i], LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin[i], LOW);
    duration[i] = pulseIn(echoPin[i], HIGH);
    distance[i] = duration[i] * 0.034 / 2;
    if (distance[i] < 500) {
      Serial.println("Dst" + String(i) + (":\t") + String (distance[i]) + ("\tcm\t"));
    }
  }
  if ((distance[0] < 50) || (distance[6] < 50)) {
    detect1 = true;
  }
  else if ((distance[1] < 30) || (distance[2] < 30) || (distance[3] < 30) || (distance[4] < 30) || (distance[5] < 30)) {
    detect2 = true;
  }
  else if ((distance[7] < 45) || (distance[8] < 45) || (distance[9] < 45)) {
    detect3 = true;
  }
  else {
    detect1 = false;
    detect2 = false;
    detect3 = false;
  }
  if (detect1 == true) {
    TimeB = TimeA;
  }
  else if (detect1 == false) {
    TimeB = TimeE = TimeA;
  }
  if (detect2 == true) {
    TimeC = TimeA;
  }
  else if (detect2 == false) {
    TimeC = TimeF = TimeA;
  }
  if (detect3 == true) {
    TimeD = TimeA;
  }
  else if (detect3 == false) {
    TimeD = TimeG = TimeA;
  }
  if (((TimeB - TimeE) >= (interval)) || ((TimeC - TimeF) >= (interval)) || ((TimeD - TimeG) >= (interval))) {
    Serial.print("Detect = ");
    Serial.println("Detect");
    Serial2.print('1');
    Serial2.print(',');
    detect = true;
  }
  else {
    Serial2.print('0');
    Serial2.print(',');
    detect = false;
  }
}

//--------------- program sensor suhu -------------------//
void suhu_sens() {
  DS18B20.requestTemperatures();
  Serial.println("Tmp:\t" + String (DS18B20.getTempCByIndex(0)) + ("\tC\t"));
  float datatemp = DS18B20.getTempCByIndex(0);
  Serial2.print(String (datatemp));
  Serial2.print(',');
}

//--------------- program sensor ph ---------------------//
void ph_sens() {
  int measuring = 0;
  for (int i = 0; i < samples; i++) {
    measuring += analogRead(pHSense);
    delay(10);
  }
  float volt = 5 / adc_resolusi * measuring / samples;
  float data = ph(volt);
  Serial.println("ph:\t" + String (ph(volt)) + "\t");
  Serial2.print(String (data));
  Serial2.print(",");
}

//--------------- program sensor oxy -------------------//
void oxygen_sens() {
  Temperaturet = (uint8_t)READ_TEMP;
  ADC_Raw = analogRead(DO_PIN);
  ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
  float oxy = readDO(ADC_Voltage, Temperaturet) / 1000.0;
  Serial.println("DO :\t" + String (oxy) + "\tmg/L\t");
  Serial2.print(String (oxy));
  Serial2.print(",");
  if (oxy < 2.5) {
    digitalWrite(relay, HIGH);
  }
  else {
    digitalWrite(relay, LOW);
  }
}

//---------------- calibrasi code -----------------------//
//-------- comment code jika tidak diperlukan -----------//

//#include <Arduino.h>
//
//#define VREF    5000//VREF(mv)
//#define ADC_RES 1024//ADC Resolution
//
//uint32_t raw;
//
//void setup()
//{
//    Serial.begin(115200);
//}
//
//void loop()
//{
//    raw=analogRead(A1);
//    Serial.println("raw:\t"+String(raw)+"\tVoltage(mv)"+String(raw*VREF/ADC_RES));
//    delay(1000);
//}
