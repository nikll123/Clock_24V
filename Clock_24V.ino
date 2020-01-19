/* Программа управления настенными часами, предназначенными для работы с часовым сервером на основе модуля DS1302 и подобным*/
// Теперь не читаем время с модуля каждую секунду. При запуске читаем время, далее работа по внутреннему таймеру
// Присоединён дисплей, сделан вывод времени на дисплей
// в процессе менюшка с переводом времени
#include <EEPROM.h>
#include <iarduino_RTC.h>
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

#define HOUR_ADD 0  //1 byte
#define MIN_ADD 1   //1 byte

#define BTN_LEFT 0
#define BTN_SELECT 1
#define BTN_RIGHT 2


#define MIN_CLOCK_VOLTAGE 19    //Minimum voltage required for stable clock work

//значения клавиш
enum buttons {
  NONE,
  SELECT,
  LEFT,
  RIGHT
};

enum menu {
  MAIN,
  SET_DATE,
  SET_TIME,
  SET_ARROWS,
  ENTER_DATE,
  ENTER_TIME,
  ENTER_ARROWS_TIME,
};

#define BTN1_PIN 14  //A0
#define BTN2_PIN 15  //A1
#define BTN3_PIN 16  //A2

#define VOLT_PIN A1                                            //sensor pin for power voltage control

//iarduino_RTC time (RTC_DS1302, 0, 1, 2);                     //RST, CLK, DAT
//iarduino_RTC time (RTC_DS1307);                              //using I2C
iarduino_RTC time (RTC_DS3231);                                //using I2C
//LiquidCrystal lcd (8, 9, 4, 5, 6, 7);                        //for display shield: RS, E, D4, D5, D6, D7, R/W = GND
LiquidCrystal_I2C lcd(0x3f, 16, 2);                            //Set address 0x3f, display 16 symbols and 2 lines (16х2)

byte out1pin = 16;     //A2
byte out2pin = 17;     //A3
byte clock_h = 0;
byte clock_m = 0;
unsigned long millisPrev = 1000;                               // seconds timer
unsigned long buttonTime = 0;
byte buttonAction = NONE;

void setup() {
  time.begin();
  time.gettime();
  if (time.hours == 12)
    time.hours = 0;

  I2C_lcdStart();

  pinMode (out1pin, OUTPUT);
  pinMode (out2pin, OUTPUT);
  pinMode (BTN1_PIN, INPUT);
  pinMode (BTN2_PIN, INPUT);
  pinMode (BTN3_PIN, INPUT);
  pinMode (VOLT_PIN, INPUT);
  pinMode (LED_BUILTIN, OUTPUT);
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
  if (millis() >= millisPrev) {
    millisPrev += 1000;
    time.gettime();
  }

  if (powerGood(VOLT_PIN) ) {
    if (timeNotMatch() ) {
      digitalWrite (LED_BUILTIN, LOW);
      clock_m ++;
      formatTime();
      
      EEPROM.put (MIN_ADD, clock_m);
      EEPROM.put (HOUR_ADD, clock_h);
      clockSwitch (clock_m);
    }
  }
  else
    digitalWrite (LED_BUILTIN, HIGH);
}

void clockSwitch (byte minutes) {
  byte _pin;
  if (minutes % 2 == 0)
    _pin = out1pin;
  else
    _pin = out2pin;

  digitalWrite (_pin, HIGH);
  digitalWrite (LED_BUILTIN, HIGH);
  delay (100);
  digitalWrite (_pin, LOW);
  digitalWrite (LED_BUILTIN, LOW);
  delay (100);
}

bool timeNotMatch() {
  return (time.hours != clock_h || time.minutes != clock_m);
}

bool powerGood(byte _pin) {
  byte voltage = (map ( (analogRead (_pin) / 4), 0, 255, 0, 24) );
  return (voltage > MIN_CLOCK_VOLTAGE);
}

byte checkButton (bool _btn) {
  bool btn1state = !digitalRead (_btn);
}

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

void formatTime() {
  if (clock_m == 60) {
    clock_m = 0;
    clock_h ++;
    if (clock_h == 12)
      clock_h = 0;
  }
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

void swapOutputs () {
  byte _temp = out1pin;
  out1pin = out2pin;
  out2pin = _temp;
}
