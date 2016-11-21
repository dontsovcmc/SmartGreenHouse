
#define BUTTON_1 4   
#define BUTTON_2 5   

#define RELAY1_PIN 8 //Реле 1
#define RELAY2_PIN 9 //Реле 2

#define OPEN_PIN 6   //Открытие крана
#define CLOSE_PIN 7  //Закрытие крана

#define OPEN_TIME_MSEC 10000  //Время нужное для открытия/закрытия крана
#define LED_KRAN  3      //Горит - движется кран
#define BUTTON_START BUTTON_1   //Начать полив (открыть на N сек), закрыть кран

#define LED_WORK  2      //Моргает - отдых, горит - полив

#define BLINK_DELAY   2000   //Период моргания
#define BLINK_TIME    100   //Период моргания

#define POLIV_MANUAL_LONG 5

#define ONE_WIRE_BUS 10

#define LOGGING  // uncomment to turn on Serial monitor

#ifdef LOGGING
#define LOG(str) Serial.println(str);
#else
#define LOG(str) (str)
#endif
