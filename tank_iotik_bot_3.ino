#define BLYNK_PRINT Serial

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <BH1750FVI.h>
#include <AM2320.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char auth[] = "baf84adf07e647c6b5e4137d6df4e26a";
char ssid[] = "IOTIK";
char pass[] = "Terminator812";
IPAddress blynk_ip(139, 59, 206, 133);

#define PWMA 5
#define DIRA 4
#define PWMB 12
#define DIRB 13

#define SERVO1_PWM 16
Servo servo_1;

AM2320 am2320;
BH1750FVI bh1750;
#define DS18B20_1 14
OneWire oneWire1(DS18B20_1);
DallasTemperature ds_sensor1(&oneWire1);
#define SOIL_SENSOR A0
const float air_value1 = 430.0;
const float water_value1 = 210.0;
const float moisture_0 = 0.0;
const float moisture_100 = 100.0;

// IoT server sensor parameters
#define sensorCount 5
char* sensorNames[] = {"air_temp", "air_hum", "soil_temp", "soil_hum", "sun_light"};

float sensorValues[sensorCount] = {0};
#define AIR_TEMP      0
#define AIR_HUM       1
#define SOIL_TEMP     2
#define SOIL_HUM      3
#define SUN_LIGHT     4

float spd = 0;
float srv = 120;

void setup()
{
  // Инициализация последовательного порта
  Serial.begin(115200);

  // Инициализация выходов для управления моторами
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Инициализация Blynk
  Blynk.begin(auth, ssid, pass, blynk_ip, 8442);

  // Инициализация датчиков
  Wire.begin(0, 2);
  am2320.begin();
  bh1750.begin();
  bh1750.setMode(Continuously_High_Resolution_Mode);
  ds_sensor1.begin();
  pinMode(SOIL_SENSOR, INPUT);

  // Инициализация сервомоторов
  servo_1.attach(SERVO1_PWM);
  servo_1.write(srv);

  // Первый опрос датчиков и вывод измеренных значений
  readSensorAM2320();
  readSensorBH1750();
  readSensorDS18B20();
  readSensorSOIL();
  printAllSensors();
}

void loop()
{
  Blynk.run();
}

// Мощность мотора "A" от -100% до +100% (от знака зависит направление вращения)
void motorA_setpower(float pwr, bool invert)
{
  // Проверка, инвертирован ли мотор
  if (invert)
  {
    pwr = -pwr;
  }
  // Проверка диапазонов
  if (pwr < -100)
  {
    pwr = -100;
  }
  if (pwr > 100)
  {
    pwr = 100;
  }
  // Установка направления
  if (pwr < 0)
  {
    digitalWrite(DIRA, LOW);
  }
  else
  {
    digitalWrite(DIRA, HIGH);
  }
  // Установка мощности
  int pwmvalue = fabs(pwr) * 10.23;
  analogWrite(PWMA, pwmvalue);
}

// Мощность мотора "B" от -100% до +100% (от знака зависит направление вращения)
void motorB_setpower(float pwr, bool invert)
{
  // Проверка, инвертирован ли мотор
  if (invert)
  {
    pwr = -pwr;
  }
  // Проверка диапазонов
  if (pwr < -100)
  {
    pwr = -100;
  }
  if (pwr > 100)
  {
    pwr = 100;
  }
  // Установка направления
  if (pwr < 0)
  {
    digitalWrite(DIRB, LOW);
  }
  else
  {
    digitalWrite(DIRB, HIGH);
  }
  // Установка мощности
  int pwmvalue = fabs(pwr) * 10.23;
  analogWrite(PWMB, pwmvalue);
}

// Read analog soil sensor
void readSensorSOIL()
{
  float adc0 = analogRead(SOIL_SENSOR);
  //Serial.println("ADC0: " + String(adc0));
  sensorValues[SOIL_HUM] = map(adc0, air_value1, water_value1, moisture_0, moisture_100);
}

// Read AM2320 temp and hum sensor
void readSensorAM2320()
{
  if (am2320.measure())
  {
    sensorValues[AIR_TEMP] = am2320.getTemperature();
    sensorValues[AIR_HUM] = am2320.getHumidity();
  }
}

// Чтение датчика BH1750
void readSensorBH1750()
{
  sensorValues[SUN_LIGHT] = bh1750.getAmbientLight();
}

// Чтение датчиков DS18B20
void readSensorDS18B20()
{
  ds_sensor1.requestTemperatures();
  sensorValues[SOIL_TEMP] = ds_sensor1.getTempCByIndex(0);
}

// Print sensors data to terminal
void printAllSensors()
{
  for (int i = 0; i < sensorCount; i++)
  {
    Serial.print(sensorNames[i]);
    Serial.print(" = ");
    Serial.println(sensorValues[i]);
  }
  Serial.println();
}

BLYNK_WRITE(V12)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Backward");
    motorA_setpower(-spd, false);
    motorB_setpower(-spd, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V18)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Forward");
    motorA_setpower(spd, false);
    motorB_setpower(spd, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V14)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Left");
    motorA_setpower(spd, false);
    motorB_setpower(-spd, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V16)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Right");
    motorA_setpower(-spd, false);
    motorB_setpower(spd, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V13)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Right backward");
    motorA_setpower(-spd * 3 / 2, false);
    motorB_setpower(-spd, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V11)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Left backward");
    motorA_setpower(-spd, false);
    motorB_setpower(-spd * 3 / 2, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V17)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Right forward");
    motorA_setpower(spd * 3 / 2, false);
    motorB_setpower(spd, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V19)
{
  int ctl =  param.asInt();
  if (ctl)
  {
    Serial.println("Left forward");
    motorA_setpower(spd, false);
    motorB_setpower(spd * 3 / 2, true);
  }
  else
  {
    Serial.println("Stop");
    motorA_setpower(0, false);
    motorB_setpower(0, true);
  }
}

BLYNK_WRITE(V100)
{
  spd =  param.asInt();
  Serial.print("Robot speed: ");
  Serial.println(spd);
}

BLYNK_WRITE(V101)
{
  srv =  param.asInt();
  Serial.print("Gear angle: ");
  Serial.println(120 - srv);
  servo_1.write(120 - srv);
}

BLYNK_READ(V0)
{
  Serial.println("Requested V0 port");
  readSensorAM2320();
  Blynk.virtualWrite(V0, sensorValues[AIR_TEMP]);
}

BLYNK_READ(V1)
{
  Serial.println("Requested V1 port");
  readSensorAM2320();
  Blynk.virtualWrite(V1, sensorValues[AIR_HUM]);
}

BLYNK_READ(V2)
{
  Serial.println("Requested V2 port");
  readSensorDS18B20();
  Blynk.virtualWrite(V3, sensorValues[SOIL_TEMP]);
}

BLYNK_READ(V3)
{
  Serial.println("Requested V3 port");
  readSensorSOIL();
  Blynk.virtualWrite(V4, sensorValues[SOIL_HUM]);
}

BLYNK_READ(V4)
{
  Serial.println("Requested V4 port");
  readSensorBH1750();
  Blynk.virtualWrite(V2, sensorValues[SUN_LIGHT]);
}

