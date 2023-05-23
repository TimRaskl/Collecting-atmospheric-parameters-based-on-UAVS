#include "PMS.h"
#include <Wire.h>                                  // Стандартная библиотека шины I2C
#include <AHT10.h>                                 // Библиотека датчика
#include <RTClib.h>  // Библиотека для работы с модулем часов DS1307
#include <SoftwareSerial.h>  // Подключение библиотеки SoftwareSerial

AHT10Class AHT10;    // Создаем объект AHT10
                            
RTC_DS1307 rtc;  // Создаем объект RTC_DS1307 для работы с модулем часов
DateTime currentTime;  // Переменная для хранения текущего времени

SoftwareSerial mySerial(D7, D8);  // Создание объекта SoftwareSerial с пинами D7 (RX)!!! и D8 (TX)
unsigned int pm1 = 0;     // Переменная для хранения значения PM1
unsigned int pm2_5 = 0;   // Переменная для хранения значения PM2.5
unsigned int pm10 = 0;    // Переменная для хранения значения PM10

const int sensorPin = D6; // Номер цифрового пина, к которому подключен датчик MQ131
const int dangerThreshold = 0; // Пороговое значение для определения опасности
int count = 0; // Счетчик для отслеживания количества показаний, равных 0

void setup() {
  
  Serial.begin(9600);                              // Инициализируем монитор порта на скорости 9600
  while (!Serial);       // Ожидание, пока монитор порта не станет доступным
  mySerial.begin(9600);  // Инициализация SoftwareSerial на скорости 9600 PM
  rtc.begin();  // Инициализируем модуль часов DS1307
  pinMode(sensorPin, INPUT); // Установка пина датчика как вход MQ131
  Wire.begin();                                   // Инициализируем шину I2C
  AHT10.begin(0x38);
  // if(AHT10.begin(0x38))                            // Инициализируем датчик с адресом 0x38
  //   Serial.println("AHT10 подключен.");            // Если удачно печатаем "AHT10 подключен."
  // else{
  //   Serial.println("AHT10 не подключен.");         // Если не удачно печатаем "AHT10 не подключен."
  //   while(1);                                      // Заканчиваем выполнение
  // }
  Serial.println();                                // Печатаем пустую строку
  delay(2000);                                     // Пауза в 2 секунды

}

void loop() {

  currentTime = rtc.now();  // Получаем текущее время

  Serial.print("Текущее время: ");  // Выводим текущее время
  Serial.print(currentTime.year(), DEC);
  Serial.print("-");
  Serial.print(currentTime.month(), DEC);
  Serial.print("-");
  Serial.print(currentTime.day(), DEC);
  Serial.print(" ");
  Serial.print(currentTime.hour(), DEC);
  Serial.print(":");
  Serial.print(currentTime.minute(), DEC);
  Serial.print(":");
  Serial.print(currentTime.second(), DEC);
  Serial.println();
 
  float T = AHT10.GetTemperature();                // Считываем показание температуры
  float H = AHT10.GetHumidity();                   // Считываем показание влажности
  float D = AHT10.GetDewPoint();                   // Считываем значение точки росы для данной влажности
  /////////////////////////////
  Serial.print("Температура: ");                   // Печатаем "Температура: "
  Serial.print(T);                                 // Печатаем показание температуры
  Serial.println(" *C");                           // Печатаем " *C"

  Serial.print("Влажность: ");                     // Печатаем "Влажность: "
  Serial.print(H);                                 // Печатаем показание влажности
  Serial.println(" %");                            // Печатаем " %"

  Serial.print("Точка росы при этой влажности: "); // Печатаем "Точка росы при этой влажности: "
  Serial.print(D);
  Serial.println("*C");
  
  //Serial.println();
  
  
  int index = 0;         // Индекс для отслеживания текущей позиции в принятых данных
  char value;            // Переменная для хранения текущего значения байта данных
  char previousValue;    // Переменная для хранения предыдущего значения байта данных
  
  int sensorValueMQ5 = analogRead(A0); // Считываем значение с аналогового входа A0 MQ5
  int mappedValueMQ5 = map(sensorValueMQ5, 0, 1023, 0, 7000);// Приводим значение к диапазону 0-7000 (максимально допустимой концентрации)
 
  int sensorValueMQ131 = digitalRead(sensorPin); // Чтение значения с цифрового пина MQ131
  ////////////////////////////////////////////////////////////


  
////////////////////////////////////////
 
 while (mySerial.available()) {  // Цикл, выполняющийся, когда есть доступные данные в SoftwareSerial
    value = mySerial.read();      // Чтение очередного байта данных

    if ((index == 0 && value != 0x42) || (index == 1 && value != 0x4d)) {
      Serial.println("Cannot find the data header.");  // Если не найден заголовок данных, выводим сообщение об ошибке и выходим из цикла
      break;
    }

    if (index == 4 || index == 6 || index == 8 || index == 10 || index == 12 || index == 14) {
      previousValue = value;  // Сохранение текущего значения в previousValue для дальнейшего использования
    } else if (index == 5) {
      pm1 = 256 * previousValue + value;  // Расчет значения PM1
      Serial.print("{ ");
      Serial.print("\"pm1\": ");
      Serial.print(pm1);
      Serial.print(" ug/m3");
      Serial.print(", ");
    } else if (index == 7) {
      pm2_5 = 256 * previousValue + value;  // Расчет значения PM2.5
      Serial.print("\"pm2_5\": ");
      Serial.print(pm2_5);
      Serial.print(" ug/m3");
      Serial.print(", ");
    } else if (index == 9) {
      pm10 = 256 * previousValue + value;  // Расчет значения PM10
      Serial.print("\"pm10\": ");
      Serial.print(pm10);
      Serial.print(" ug/m3");
    } else if (index > 15) {
      break;  // Если индекс превысил 15, выходим из цикла
    }

    index++;  // Увеличение индекса
  }

  while (mySerial.available()) {
    mySerial.read();  // Считываем все доступные данные из буфера SoftwareSerial, чтобы очистить его
  }

  Serial.println(" }");  // Вывод окончания JSON-объекта
  ///////////////////////////
  if (mappedValueMQ5 < 1100) {
    mappedValueMQ5 = 0; // Если значение меньше 1000, заменяем его на 0 MQ131
  }
  Serial.print("MQ5: ");
  Serial.println(mappedValueMQ5);
  /////////////////////////
    if (sensorValueMQ131 == dangerThreshold) {
    count++; // Увеличение счетчика, если показание равно пороговому значению

    if (count >= 4) {// Защита от случайного срабатывание 
      Serial.println("Опасность Озон!"); // Вывод сообщения о опасности
      // Здесь можно добавить дополнительные действия при обнаружении опасности
      count = 0; // Сброс счетчика после срабатывания
    }
  } else {
    count = 0; // Сброс счетчика, если показание не равно пороговому значению
    Serial.println("MQ131:0"); // Вывод значения "MQ131:0"
  }
  ////////////////////////
  delay(3000);

}
