
#include <GyverTM1637.h>
#include <EEPROM.h>

#define CLK 2
#define DIO 3

#define Time_pause 5000 // время паузы после последного счета геркона

//константы для пинов
#define Start_stop 8
#define Menu 9
#define Plus 10
#define Minus 11
#define R_switch 12
#define Relay 13

#define minRevilutions 0 // минимальное количество оборотов
#define maxRevilutions 999 // максимальное количество оборотов

GyverTM1637 disp(CLK, DIO);

// состояния кнопок и геркона
bool Sta_sto_state = false;
bool Menu_state = false;
bool Plus_state = false;
bool Minus_state = false;
bool R_switch_state = false;

int revilutions = 0; // обороты
int extra = 0; // лишние обороты из за инерции, задаются вручную или вычисляются автоматически

void setup(){ 

  pinMode(Start_stop, INPUT);
  pinMode(Menu, INPUT);
  pinMode(Plus, INPUT);
  pinMode(Minus, INPUT);
  pinMode(R_switch, INPUT);
  pinMode(Relay, OUTPUT);

  digitalWrite(Relay, HIGH); // реле выключено


  disp.clear();
  disp.brightness(2); // 0-7
  disp.displayInt(8888);
  disp.point(true);
  delay(1000);
  disp.clear();
  disp.point(false);
  
  EEPROM.get(0, revilutions); // считывание последних заданных оборотов из пзу
  if(revilutions > maxRevilutions || revilutions < minRevilutions)
    revilutions = 0;
  extra = EEPROM.read(8); // считывание лишних оборотов из пзу
  if(extra > 9 || extra < 0)
    extra = 0;
  
  
}

void setRevilutions()
{
    disp.displayInt(revilutions);
    while(!Sta_sto_state)
    {
      
      Sta_sto_state = digitalRead(Start_stop);
      Plus_state = digitalRead(Plus);
      Minus_state = digitalRead(Minus);
      Menu_state = digitalRead(Menu);

      if(Plus_state)
      {
        revilutions++;
        disp.clear();
        disp.displayInt(revilutions);
        delay(200);

        if(revilutions >= maxRevilutions)
          revilutions = minRevilutions;
      }

      if(Minus_state)
      {
        revilutions--;
        disp.clear();
        disp.displayInt(revilutions);
        delay(200);

        if(revilutions <= minRevilutions)
          revilutions = maxRevilutions;
      }

      if(Menu_state)
      {
        menu();
        disp.displayInt(revilutions);
      }
           
    }
    EEPROM.put(0, revilutions); // Если revilutions было изменено, то значение будет перезаписано в пзу
    disp.displayByte(_Y,_E,_S,0x00);
    delay(1000);
    disp.clear();
    disp.displayInt(revilutions);
    Sta_sto_state = false;
}

void next()
{
  
  while(true)
  {
      disp.point(true);
      delay(100);
      disp.point(false);
      delay(100);
      
      Sta_sto_state = digitalRead(Start_stop);
      Menu_state = digitalRead(Menu);
      Plus_state = digitalRead(Plus);
      Minus_state = digitalRead(Minus);

      if(Sta_sto_state)
        work();
              
      if(Plus_state || Minus_state)
        break;

      if(Menu_state)
      {
        menu();
        break;
      }
  }
}

void work()
{
  disp.clear();
  disp.displayInt(0);
  delay(300);
  int count = 0;
  bool last_R_switch = false;
  bool flag = true; // флаг для единоразового входа в проверку (count == revilutions...)
  digitalWrite(Relay, LOW);
  Sta_sto_state = false;
  delay(300);
  long milliseconds = 0;
  while(true)
  {
      Sta_sto_state = digitalRead(Start_stop);
      if(Sta_sto_state)
      {
        disp.displayByte(_B,_r,_E,_A);
        digitalWrite(Relay, HIGH);
        delay(1000);
        //Sta_sto_state = false;
        disp.clear();
        break;
      }

      // чтение состояния геркона
      R_switch_state = digitalRead(R_switch);
          if(R_switch_state != last_R_switch)
          {
            if(R_switch_state == HIGH)
            {
              count++;
              disp.displayInt(count);        
            }
          }
      last_R_switch = R_switch_state;

      if(count == revilutions - extra && flag)
      {
        digitalWrite(Relay, HIGH);
        milliseconds = millis();
        flag = false;
      }

      if(count >= revilutions)
      {
        if(millis() > milliseconds + Time_pause)
        {
          break;
        }
      }
  }
  delay(2000);
  //disp.clear();
}

void menu()
{
  disp.displayByte(_L,_o,0x00,0x00);
  int count = extra;
  delay(1000);
  Menu_state = false;
  disp.clear();
  while(!Menu_state)
  {
      disp.displayInt(count);
      Menu_state = digitalRead(Menu);
      Plus_state = digitalRead(Plus);
      Minus_state = digitalRead(Minus);

      if(Plus_state)
      {
        count++;
        disp.clear();
        disp.displayInt(count);
        delay(200);
      }

      else if(Minus_state)
      {
        count--;
        disp.displayInt(count);
        delay(200);
      }

      if(count < 0)
      {
        count = 9;
      }
      if(count > 9)
        count = 0;
      
  }
  if(count != extra)
  {
    EEPROM.update(8, count);
    extra = count;
  }
  
  disp.displayByte(_G,_o,_o,_d);
  delay(1000);
  Menu_state = false;
  disp.clear();
}

void loop()
{
  setRevilutions();
  next();
}
