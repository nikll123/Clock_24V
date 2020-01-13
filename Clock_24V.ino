/* Программа управления настенными часами, предназначенными для работы с часовым сервером на основе модуля DS3231 и подобным*/

#include <EEPROM.h>
#include <iarduino_RTC.h>
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

#define HOUR_ADD 0  //1 byte
#define MIN_ADD 1   //1 byte

#define OUT1 16     //==A2
#define OUT2 17     //==A3
#define BTN_PIN 14  //==A0
#define VOLT_PIN A1

//  iarduino_RTC time(RTC_DS1302, 2, 3, 4);         // Объявляем объект time для работы с RTC модулем на базе чипа DS1302, указывая выводы Arduino подключённые к выводам модуля RST, CLK, DAT.
//  iarduino_RTC time(RTC_DS1307);                  // Объявляем объект time для работы с RTC модулем на базе чипа DS1307, используется шина I2C.
iarduino_RTC time (RTC_DS3231);                     //I2C connection
//LiquidCrystal lcd (8, 9, 4, 5, 6, 7);             //для display shield: RS, E, D4, D5, D6, D7, R/W = GND
LiquidCrystal_I2C lcd(0x3f, 16, 2);                 // устанавливаем адрес 0x3f, дисплей 16 символов в 2 строки (16х2)

byte clock_h = 0;
byte clock_m = 0;

void setup() {
  time.begin();
  I2C_lcdStart();
  lcd.begin (16, 2);
  pinMode (OUT1, OUTPUT);
  pinMode (OUT2, OUTPUT);
  pinMode (BTN_PIN, INPUT);
  pinMode (VOLT_PIN, INPUT);
  pinMode (LED_BUILTIN, OUTPUT);
  if (EEPROM.get (HOUR_ADD, clock_h) == 255) {
    EEPROM.put (HOUR_ADD, 0);
    EEPROM.put (MIN_ADD, 0);
  }
  clock_h = EEPROM.get (HOUR_ADD, clock_h);
  clock_m = EEPROM.get (MIN_ADD, clock_m);
  lcd.setCursor (0, 0);
  lcd.print ("Module");
  lcd.setCursor (0, 1);
  lcd.print ("Clock");
  
  time.gettime();
  lcdPrintTime (7, 0, time.hours, time.minutes);
  lcdPrintTime (6, 1, clock_h, clock_m);
  
  delay (1000);
}

void loop() {
  if (millis() %1000 == 0) {
    time.gettime();
    if (time.hours == 12)
      time.hours = 0;
    
    if (time.hours == 45) {                   //if error in connection with DS1302
      digitalWrite (LED_BUILTIN, HIGH);

      lcdPrintTime (7, 0, time.hours, time.minutes);
      lcdPrintTime (6, 1, clock_h, clock_m);
    }

    while ( (time.hours != clock_h || time.minutes != clock_m) && (map ( (analogRead (VOLT_PIN) / 4), 0, 255, 0, 24) ) >= 22) {
      clockSwitch (clock_m);

      lcdPrintTime (7, 0, time.hours, time.minutes);
      lcdPrintTime (6, 1, clock_h, clock_m);
    }
  }
}

void clockSwitch (byte minutes) {
  if (minutes%2 == 0) {
    digitalWrite (OUT1, HIGH);
    delay (100);
    digitalWrite (OUT1, LOW);
    delay (100);
  }
  else {
    digitalWrite (OUT2, HIGH);
    delay (100);
    digitalWrite (OUT2, LOW);
    delay (100);
  }
  
  clock_m ++;
  if (clock_m == 60) {
  clock_m = 0;
  clock_h++;
  if (clock_h == 12)
    clock_h = 0;
    EEPROM.put (MIN_ADD, clock_m);
    EEPROM.put (HOUR_ADD, clock_h);
  }
}

void lcdClear() {
  lcd.clear();
  lcd.noBlink();
  lcd.setCursor (0, 0);
}

void I2C_lcdStart() {
  lcd.init();                     // инициализация LCD
  lcd.backlight();                // включаем подсветку
}

void lcdPrintTime (byte _symb, byte _line, byte _hour, byte _minute) {
  lcd.setCursor (_symb, _line);
  if (_hour < 10)
    lcd.print ("0");
  lcd.print (_hour);
  lcd.print (":");
  if (_minute < 10)
    lcd.print ("0");
  lcd.print (_minute);
}
