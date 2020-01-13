/* Программа управления настенными часами, предназначенными для работы с часовым сервером на основе модуля DS1302 и подобным*/
// V2 Теперь не читаем время с модуля каждую секунду. При запуске читаем время, далее работа по внутреннему таймеру
// V3 Присоединён дисплей, сделан вывод времени на дисплей
// в процессе менюшка с переводом времени
#include <EEPROM.h>
#include <iarduino_RTC.h>
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

#define HOUR_ADD 0  //1 byte
#define MIN_ADD 1   //1 byte


//значения клавиш
#define NONE 0
#define SELECT 1
#define LEFT 2
#define DOWN 3
#define UP 4
#define RIGHT 5

#define OUT1 16     //A2
#define OUT2 17     //A3
#define BTN_PIN 14  //A0
#define LED_PIN 13
#define VOLT_PIN A1

iarduino_RTC time (RTC_DS1302, 0, 1, 2);                       //RST, CLK, DAT
//LiquidCrystal lcd (8, 9, 4, 5, 6, 7);                        //для display shield: RS, E, D4, D5, D6, D7, R/W = GND
LiquidCrystal_I2C lcd(0x3f, 16, 2);                            //устанавливаем адрес 0x3f, дисплей 16 символов в 2 строки (16х2)

byte clock_h = 0;
byte clock_m = 0;
unsigned long millisPrev = 1000;    // таймер секунд
byte buttonPrev = 255;
unsigned long buttonTime = 0;
byte buttonAction = NONE;

void setup() {
  time.begin();
  time.gettime();
  if (time.hours == 12)
    time.hours = 0;

  I2C_lcdStart();
//  lcd.begin (16, 2);

  pinMode (OUT1, OUTPUT);
  pinMode (OUT2, OUTPUT);
  pinMode (BTN_PIN, INPUT);
  pinMode (VOLT_PIN, INPUT);
  pinMode (LED_PIN, OUTPUT);
  if (EEPROM.get (HOUR_ADD, clock_h) == 255) {
    EEPROM.put (HOUR_ADD, 4);
    EEPROM.put (MIN_ADD, 37);
  }
  clock_h = EEPROM.get (HOUR_ADD, clock_h);
  clock_m = EEPROM.get (MIN_ADD, clock_m);

  print_main_screen();                            // "Module, clock"
  print_time (7, 0, time.hours, time.minutes);
  print_time (6, 1, clock_h, clock_m);
}

void loop() {
  if (millis() >= millisPrev) {   //каждую секунду прибавляем по секунде, а так же пересчитываем это в минуты и часы
    millisPrev += 1000;
    time.seconds ++;
    if (time.seconds == 60) {
      time.seconds = 0;
      time.minutes ++;
      if (time.minutes == 60) {
        time.minutes = 0;
        time.hours ++;
        if (time.hours == 12)
          time.hours = 0;
      }
    }
  }
  if ( (time.hours != clock_h || time.minutes != clock_m) && (map ( (analogRead (VOLT_PIN) / 4), 0, 255, 0, 24) ) > 19) {
    digitalWrite (LED_PIN, LOW);
    clock_m ++;
    if (clock_m == 60) {
      clock_m = 0;
      clock_h ++;
      if (clock_h == 12)
        clock_h = 0;
    }
    EEPROM.put (MIN_ADD, clock_m);
    EEPROM.put (HOUR_ADD, clock_h);
    clockSwitch (clock_m);
  }
  else if ( (map ( (analogRead (VOLT_PIN) / 4), 0, 255, 0, 24) ) <= 19)
    digitalWrite (LED_PIN, HIGH);
}

void clockSwitch (byte minutes) {
  if (minutes%2 == 0) {
    digitalWrite (OUT1, HIGH);
    digitalWrite (LED_PIN, HIGH);
    delay (100);
    digitalWrite (OUT1, LOW);
    digitalWrite (LED_PIN, LOW);
    delay (100);
  }
  else {
    digitalWrite (OUT2, HIGH);
    digitalWrite (LED_PIN, HIGH);
    delay (100);
    digitalWrite (OUT2, LOW);
    digitalWrite (LED_PIN, LOW);
    delay (100);
  }
}

/*
void lcdClear() {
  lcd.clear();
  lcd.noBlink();
  lcd.setCursor (0, 0);
}
*/

void I2C_lcdStart() {
  lcd.init();                     // инициализация LCD
  lcd.backlight();                // включаем подсветку
}

void print_main_screen() {
  lcd.setCursor (0, 0);
  lcd.print ("Module");
  lcd.setCursor (0, 1);
  lcd.print ("Clock");
}

void print_time (byte _symb, byte _line, byte _hour, byte _min) {
  lcd.setCursor (_symb, _line);
  if (_hour < 10)
    lcd.print ("0");
  lcd.print (_hour);
  lcd.print (":");
  if (_min < 10)
    lcd.print ("0");
  lcd.print (_min);
}

void checkButton() {
  byte buttonCurrent = analogRead(BTN_PIN) / 4;
  if (buttonCurrent > 250)
    buttonPrev = buttonCurrent;
//NONE = 255, SELECT = 160, LEFT = 103, DOWN = 64, UP = 25, RIGHT = 0
  else if (buttonCurrent < 250 && buttonPrev > 250) {
    buttonTime = millis();
    buttonPrev = buttonCurrent;
    if (buttonCurrent > 155 && buttonCurrent < 165)
      buttonAction = SELECT;
    else if (buttonCurrent > 98 && buttonCurrent < 108)
      buttonAction = LEFT;
    else if (buttonCurrent > 59 && buttonCurrent < 69)
      buttonAction = DOWN;
    else if (buttonCurrent > 20 && buttonCurrent < 30)
      buttonAction = UP;
    else if (buttonCurrent < 5)
      buttonAction = RIGHT;
  }
}
