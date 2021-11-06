#pragma once
#include <Arduino.h>

#define TO_GND true          // кнопка подключена к земле
#define TO_VCC false         // кнопка подключена к питанию

class dfButton
{
public:
    // параметры - пин, к чему подключена кнопка, время дребезга мс
    dfButton(byte pin, bool toGND = TO_GND, uint32_t bounceTime = 100); 
    void ScanState();        // метод проверки состояние сигнала
    bool ButtonPressed();    // возвращает состояние кнопки
    bool IsStateChanged();   // однократно возвращает true при изменении состояния кнопки
    void OnPressed(void(*fn)());    //callback functions
    void OnReleased(void(*fn)());   //callback functions
    void OnChanged(void(*fn)());    //callback functions
private:
    uint32_t _timer = 0;         // счетчик подтверждений стабильного состояния
    uint16_t _bounceTime;        // время дребезга кнопки
    byte _pin;               // номер вывода
    bool _toGND;             // к чему подключена кнопка (земля или питание)
    bool _pressFlag;         // признак кнопка сейчас нажата
    bool _clickFlag = false; // признак изменения состояния кнопки
    bool _notRead = false;   // признак прочтения изменения состояния кнопки
    void (*_pOnPrs)() = NULL;   // указатель на обработчик события нажатия кнопки
    void (*_pOnRel)() = NULL;   // указатель на обработчик события отпускания кнопки
    void (*_pOnCng)() = NULL;   // указатель на обработчик события отпускания кнопки
};

void dfButton::OnPressed(void(*fn)()){
    _pOnPrs = fn;
}

void dfButton::OnReleased(void(*fn)()){
    _pOnRel = fn;
}

void dfButton::OnChanged(void(*fn)()){
    _pOnCng = fn;
}

bool dfButton::ButtonPressed(){
    return _toGND ? !_pressFlag : _pressFlag;
}

bool dfButton::IsStateChanged(){
    if(!_notRead){
        return false; 
    } 
    _notRead = false;                                // сбр. флаг одноразового чтения
    return true;
}

void dfButton::ScanState()
{
    if (_clickFlag){  
// Serial.printf("_timer=%d, _bounceTime=%d, millis()=%d", _timer, _bounceTime, millis());
        if((_timer + _bounceTime) > millis()) {       // ожидаем окончание дребезга
// Serial.println("bounce");
            return;
        }
        else{                                       // ожидание закончили
// Serial.println("EndOfBounce");
            _clickFlag = false;
            _pressFlag = digitalRead(_pin);
            _notRead = true;     
                               // уст. флаг одноразового чтения
            if(_pOnCng!=NULL) _pOnCng();                //вызываем колбэк изменения состояния
            if((_pressFlag^_toGND)&&(_pOnPrs!=NULL))    //вызываем колбэк нажатия
                _pOnPrs();
            if(!(_pressFlag^_toGND)&&(_pOnRel!=NULL))   //вызываем колбэк отпускания
                _pOnRel();
        } 
    }
    if (_pressFlag != (digitalRead(_pin)))          // кнопка изменила состояние
    {
        _clickFlag = true;                          // флаг ожидания окончание дребезга
        _timer = millis();                          // взводим таймер
        return;
    }
}

dfButton::dfButton(byte pin, bool toGND, uint32_t bounceTime)
{
    _pin = pin;
    _bounceTime = bounceTime;
#ifdef INPUT_PULLDOWN
    _toGND = toGND;
    _toGND ? pinMode(_pin, INPUT_PULLUP) : pinMode(_pin, INPUT_PULLDOWN);
#else
    _toGND = TO_GND;
    pinMode(_pin, INPUT_PULLUP);
#endif
    _pressFlag = digitalRead(_pin);
}