#include <Arduino.h>
#include "Button.hpp"
#include <LiquidCrystal_I2C.h>

/*
// @Versin history
// v DP.Aty85.01.05 - заработал дисплей
// v DP.Aty85.01.06 - перевел индикацию на об/мин
// v DP.Aty85.01.07 - сделал начальную заставку
*/

#define PRODUCT " Nozzle Cleaner"
#define MAKER   " dfVenture inc"
#define VERSION "v01.07"

#define SDA_PIN 0
#define SCL_PIN 2

#define CTRL_PIN 1
#define BTN_FR_UP_PIN 3
#define BTN_DU_UP_PIN 4

dfButton BtnFrUp(BTN_FR_UP_PIN);      //кнопка увеличения частоты
dfButton BtnDuUp(BTN_DU_UP_PIN);      //кнопка увеличения длительности
LiquidCrystal_I2C lcd(0x27,16,2);     // set the LCD address to 0x27 for a 16 chars and 2 line display


uint8_t Frequency;                    // частота форсунки Гц (5-50 соответствует 600-6000 об/мин)
uint8_t frDelay;                      // рассчитанная длительность задержки для цикла (200-20)
uint8_t duDelay;                      // длительность впрыска мс 2-50 (для индикации)

uint32_t ft, fd;
bool lvl;

uint8_t CalcDelay(uint8_t fr)
{
  return 1000 / fr;
}

void Display()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Frequency");
  lcd.setCursor(12, 0);
  lcd.print(Frequency * 120);

  lcd.setCursor(0, 1);
  lcd.print("Duration");
  lcd.setCursor(12, 1);
  lcd.print(duDelay);
}

void OnBtnFrUp()
{
  Frequency < 10 ? Frequency++ : Frequency += 5;
  if(Frequency > 50) Frequency = 5;
  frDelay = CalcDelay(Frequency);
  if(duDelay > frDelay >> 2 ) duDelay = frDelay >> 2;
  Display();
};

void OnBtnDuUp()
{
  duDelay < 10 ? duDelay++ : duDelay += 5;
  if(duDelay > 40) duDelay = 2;
  if(duDelay > frDelay >> 2 ) duDelay = 2;
  Display();
};

void setup()
{
  pinMode(CTRL_PIN, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(PRODUCT);
  lcd.setCursor(0, 1);
  lcd.print(MAKER);
  delay(4000);

  Frequency = 5;
  frDelay = CalcDelay(Frequency);
  duDelay = 2;
  BtnFrUp.OnPressed(OnBtnFrUp);
  BtnDuUp.OnPressed(OnBtnDuUp);
  Display();
}

void loop()
{
  BtnFrUp.ScanState();
  BtnDuUp.ScanState();
  if (ft + frDelay <= millis())
  {
    digitalWrite(CTRL_PIN, HIGH);
    lvl = 1;
    ft = millis();
    fd = millis();
  }
  else
  {
    if (fd + duDelay <= millis())
    {
      if (lvl)
      { 
        digitalWrite(CTRL_PIN, LOW);
        lvl = 0;
      }
    }
  }
}