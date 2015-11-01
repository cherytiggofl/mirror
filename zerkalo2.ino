 
/***
*Проект Зеркало Chery tiggo FL. Модуль отображения, установленный в зеркало



Библиотека VirtualWire
http://www.airspayce.com/mikem/arduino/VirtualWire/VirtualWire-1.20.zip


Приемник
Vcc, GND, DATA - соответственно на "5V", "GND" и любой свободный пин (в данном случае 7).


*/

 //Скачать библиотеку для датчика давления https://github.com/adafruit/Adafruit-BMP085-Library

#include <Wire.h>
#include <VirtualWire.h>
#include <Adafruit_BMP085.h>

//Датчик давления
// Соединить VCC датчика BMP085 с +5в Ардуино
// Соединить GND датчика BMP085 с GND Ардуино
// Соединить SCL датчика BMP085 c Ардуино Analog 5
// Соединить SDA датчика BMP085 c Ардуино Analog 4

Adafruit_BMP085 bmp;
 

/*Освещение вход - A1 - фоторезистор на +5В - коричневый
* Фоторезистор безымянный: нет света 330к, яркий свет 200 ом
 * 
 * Схема подключения
 * +5 -> фоторезистор -> Ардуино.А1
 *                    -> резистор 10К -> GND 
 *                    
  
*/
#define PIN_PHOTOSENSOR  1 //и фоторезистора


//Кнопки вход - A0 - через кнопку и резистор на +5В - оранжевый
//ШИМ яркости выход - 11, управляю импульсом +5в
#define PIN_BRIGHT 11  //9-ногу (Таймер1 нельзя для ШИМ использовать из-за того, что он же используется в VirtualWire)

 
 
//Pin Ардуино, который соединятеся с ST_CP  74HC595.12
int latchPin = 13;
//Pin Ардуино, который соединятеся с SH_CP of 74HC595.11
int clockPin = 12;
//Pin Ардуино, который соединятеся с DS of 74HC595.14
int dataPin = 10;
 
//Массив битовых масок для LED сегментных идикаторов
int LED_SEG_TAB[]={0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6,0x3e,0x1a,0x7a,0x9e,0x8e,0x01,0x02};
                  //0     1    2     3    4    5    6    7    8    9   a    b    c    d    e    f,  -
 
 
void setup() {
  Serial.begin(9600);
  
  
  pinMode(7,OUTPUT);


  digitalWrite(7,LOW);  
  Serial.println("Reciever");
  

  vw_set_rx_pin(7);
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_rx_start();
  
  
  
  
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  



  analogWrite(PIN_BRIGHT, 20);

  
InitSensors();

 
  
}


void InitSensors(){
    if (!bmp.begin()) {
 //Serial.println("Не найден BMP085 sau BMP180");

  }
}


/**
 * Установить яркость свечения идикаторов по яркости помещения.
 * В пемещении темнее, индикация темнее; светлее в помещении - ярче индикация
 * Считать показания фоторезистора, установить яркость
 * 
 */
void UpdateBackLight(){
int PhotoSensorLight = analogRead(PIN_PHOTOSENSOR);
  if( PhotoSensorLight < 200 ) PhotoSensorLight = 200;
  if( PhotoSensorLight > 1000 ) PhotoSensorLight = 1000;
  
  int BackLight = map(PhotoSensorLight, 200, 1000, 1, 255);
  
  //Serial.print("Photoresistor=");Serial.println(PhotoSensorLight); 
  //Serial.print("BackLight=");Serial.println(BackLight); 

  analogWrite(PIN_BRIGHT, BackLight);
  return;
  }

unsigned char PressureAndTemperatureShowMode = 0;
//Получение значений давления и температуры внутри 
void PrintPressureAndTemperatureIn(){
   float temperature = bmp.readTemperature();
   float presiune = bmp.readSealevelPressure()/101.325; presiune = presiune * 0.760;
 
   //Моргать по очереди то значение давления то температуры
   if( PressureAndTemperatureShowMode == 0)   DisplayPrepeareValue(1, int(presiune) );
   else DisplayPrepeareValue(1, int(temperature) );
   PressureAndTemperatureShowMode = 1 - PressureAndTemperatureShowMode;
   
} 
 
 
 
 //Очистить индикатор
void DisplayClear(){
      digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, 0);  
    shiftOut(dataPin, clockPin, LSBFIRST, 0);  
    shiftOut(dataPin, clockPin, LSBFIRST, 0);  
    shiftOut(dataPin, clockPin, LSBFIRST, 0);  
    shiftOut(dataPin, clockPin, LSBFIRST, 0);  
    shiftOut(dataPin, clockPin, LSBFIRST, 0);  
    digitalWrite(latchPin, HIGH);
}


#define DispModuleCnt 2
#define DispDigitCnt  3

unsigned char DispDigit[DispModuleCnt * DispDigitCnt];
/**
*Подготвить значение для отображения его на модуле отображения номер DispNo (1..DispModuleCnt)
*/
void DisplayPrepeareValue(int DispNo, int Value0){
 /* 
Serial.print("void DisplayPrepeareValue(int ");
Serial.print(DispNo);
Serial.print(", int ");
Serial.print(Value0);
Serial.println(")");
*/
  if( (DispNo < 1) || (DispNo > DispModuleCnt) ) return;
  
  /*
         DispNo=1     DispNo=2 
        _  _  _       _  _  _
       | || || |     | || || |
        -  -  -       -  -  -
       | || || |     | || || |
        _  _  _       _  _  _
        
    n0  5  4  3       2  1  0    
        
  
  */
  int Value = abs(Value0);
  
  //Начальная и конечная позиция в цепочке сегментных индикаторов (самая правая цифра == позиция 0)
  int n0 = (DispModuleCnt - DispNo) * DispDigitCnt;
  //int n1 = n0 + DispDigitCnt - 1;
  
  //Serial.print("n0=");Serial.println(n0);
  //Serial.print("n1=");Serial.println(n1);
  
  //Из входного параметра Value выделим цифры: если количество цифр больше 3, ошибка; если значение меньше 0 и количество цифр == 3, ошибка -- т.к. не влезет в 3-х позиционный индикатор
  if( (Value > 999) || ( (Value0 < 0) && (Value > 99) )  ){
    DispDigit[ n0 + 0 ] = LED_SEG_TAB[13];//Буква E
    Serial.println("Error");
    return;
  }
  
  
  //!!! Количество цифр на индикатор использую константно == 3
  int tmp = Value;
  int d=0;
 // Serial.print("d=");
  d =  int( tmp / 100 );  tmp = tmp - d * 100; DispDigit[ n0+3-1 ] = LED_SEG_TAB[d];  if( d == 0 ) DispDigit[ n0+3-1 ] = 0; //Serial.print(d); 
  d = int( tmp / 10 );    tmp = tmp - d * 10;  DispDigit[ n0+2-1 ] = LED_SEG_TAB[d];  if( d == 0 ) DispDigit[ n0+2-1 ] = 0; //Serial.print(d);
  d =  int( tmp );                             DispDigit[ n0+1-1 ] = LED_SEG_TAB[d];  //Serial.print(d);
 //  Serial.println(";");
   
 
   
  if( Value0 < 0 ){ 
    DispDigit[ n0+3-1 ] = LED_SEG_TAB[16];
  }  
   
  return;
  
}


//Отобразить из буфера значения
void DisplayRefresh(){
  
    digitalWrite(latchPin, LOW);
    for(int i=0; i <  DispModuleCnt * DispDigitCnt ; i++){
        shiftOut(dataPin, clockPin, LSBFIRST, DispDigit[ i]);  
    }
    digitalWrite(latchPin, HIGH);
    
}



//Получение значения внешней температуры
void PrintTemperatureOut( int temperature ){
 DisplayPrepeareValue(2,  temperature );
  return;
   
} 

/**
* Получение данных  из радиоканала
*/
void RadioDataUpdate(){
  
  char res[24];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  uint8_t buf[VW_MAX_MESSAGE_LEN];

  if(vw_get_message(buf, &buflen))
  {
    //Serial.println("msg recieved...");
    digitalWrite(7,!digitalRead(7));


    for(int i = 0;i < buflen;i++)
    {
      res[i] = buf[i];
    }
    digitalWrite(7,!digitalRead(7));
    Serial.print(res);
    Serial.println("");
    
    /**
      Здесь требуется разбор строки:
       * выборка из набора символов пакет данных (по коду начала пакета и конца)
       * проверка пакета, что он пришел от нашего датчика, а не от соседней машины
       * проверка контрольной суммы:  можно шифровать пакет особым кодом - если этим же кодом расшифрован, что пакет 1) валиный, 2) он наш, а не соседский, 3) объединим 2 шага алгоритма в один
       * получить маркер датчика "я температура улицы", "я температура масла", "я давление переднего правого колеса"
       * получить значение
       * вызвать требуюмую функцию отображения параметра
    */
    
    PrintTemperatureOut( atoi( res ) );//пока все примитивно: нет проверки, что получили то и отобразим как температуру улицы
  }
  


  return;
}

unsigned char test = 0;

//Настройка частоты срабатывания - получение данных от радиомодуля
unsigned long RadioDataUpdate_cur = 0;
unsigned long RadioDataUpdate_max = 510;

//Настройка частоты срабатывания - обновление давления и температуры
unsigned long PressureAndTemperatureIn_cur = 0;
unsigned long PressureAndTemperatureIn_max = 500;

void loop() {
  
  unsigned long currentMillis = millis();
  
  
  DisplayClear();
  DisplayRefresh();
  UpdateBackLight();
    
    
   if( currentMillis - RadioDataUpdate_cur >= RadioDataUpdate_max ){  
     RadioDataUpdate(); 
     RadioDataUpdate_cur = currentMillis;
   }  
   
    if( currentMillis - PressureAndTemperatureIn_cur >= PressureAndTemperatureIn_max ){  
     PrintPressureAndTemperatureIn(); 
     PressureAndTemperatureIn_cur = currentMillis;
   } 
    

  



//
//test = test + 1;
//if( test > 250 ) test = 0;  
Serial.println(test);
    delay(500);

}
