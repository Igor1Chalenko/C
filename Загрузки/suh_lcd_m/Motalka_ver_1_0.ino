#include <EEPROM.h>
#include <LiquidCrystalRus.h>

LiquidCrystalRus lcd(7, 6, 5, 4, 3, 2); // (RS, E, DB4, DB5, DB6, DB7)

//константы для пинов
#define Start_stop 8
#define Plus 9
#define Minus 10
#define R_switch 11
#define Relay 12

#define minRevilutions 0 // минимальное количество оборотов
#define maxRevilutions 999 // максимальное количество оборотов

// состояния кнопок и геркона
bool Sta_sto_state = false;
bool Plus_state = false;
bool Minus_state = false;
bool R_switch_state = false;

int revilutions = 0; // обороты
int extra = 0; // лишние обороты из за инерции, задаются вручную или вычисляются автоматически

void setup(){ 
  
  pinMode(Start_stop, INPUT);
  pinMode(Plus, INPUT);
  pinMode(Minus, INPUT);
  pinMode(R_switch, INPUT);
  pinMode(Relay, OUTPUT);

  digitalWrite(Relay, HIGH); // реле выключено
  
  lcd.begin(8, 2);                  // Задаем размерность экрана

  EEPROM.get(0, revilutions); // считывание последних заданных оборотов из пзу
  extra = EEPROM.read(8); // считывание лишних оборотов из пзу
  
  lcd.setCursor(1, 0);              
  lcd.print("Привет!");              
  lcd.setCursor(0, 1);              
  lcd.print("Трудяга!");                
  delay(4000);
  lcd.clear();
}

void idiot()
{
  for(int i=0; i<3; i++)
  {
    lcd.clear();
    lcd.setCursor(0, 0);              
    lcd.print("Долбоёб!");
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("        ");
    lcd.setCursor(0, 1);              
    lcd.print("Долбоёб!");
    delay(2000);
  }

  lcd.clear();
  
  for(int i=0; i<3; i++)
  {    
    lcd.setCursor(0, 0);              
    lcd.print("Лишние");
    lcd.setCursor(0, 1);              
    lcd.print("обороты ");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);              
    lcd.print("больше");
    lcd.setCursor(0, 1);              
    lcd.print("заданных");
    delay(2000);
  }
  Sta_sto_state = false;
}

void calibration() // Режим калибровки лишних оборотов
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Лишние");
  lcd.setCursor(0, 1);
  lcd.print("обороты");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Авто. +");
  lcd.setCursor(0, 1);
  lcd.print("Ручн. -");

  int count = 0; // начальное состояние счетчика
  int test = 15; // количество оборотов для теста
  long milliseconds; // для таймера ожидания
  bool last_R_switch = false; // состояние геркона
  
  while(!Sta_sto_state)
  {
    Sta_sto_state = digitalRead(Start_stop);
    Plus_state = digitalRead(Plus);
    Minus_state = digitalRead(Minus);

    if(Plus_state) // если нажат + 
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Автосчет");
      lcd.setCursor(0, 1);
      lcd.print("через:");
      
      for(int i = 5; i >= 0; i--) // задержка перед стартом
      {
        delay(1000);
        lcd.setCursor(6, 1);
        lcd.print(i);
      }
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("От винта");
      digitalWrite(Relay, LOW);

      milliseconds = millis();
      while (true)
      {
      
        if(count >= test) // моталка работает пока счетчик не дойдет до 15 оборотов
        {
          digitalWrite(Relay, HIGH); // затем выключение моталки
        }
        
        //чтение состояния геркона с защитой от дребезга
        R_switch_state = digitalRead(R_switch);
        if(R_switch_state != last_R_switch)
        {
          if(R_switch_state == HIGH)
          {
            count++; // увеличиваем счетчик на 1
            lcd.setCursor(0, 1);
            lcd.print(count); // показывает изменение на экран
            milliseconds = millis(); // обновляем показания таймера
           }
         }
         last_R_switch = R_switch_state;

        // Когда питание мотора будет выключено, моталка по инерции сделает несколько оборотов
        // если в течении 5 секунд после выключения мотора, геркон не увидит сигнал, тест будет
        // завершен.
        if(millis() > (milliseconds + 5000) && count >= test) 
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("лишн.");
          lcd.print(count - test);
          
          //Вычисление разницы между тестовыми 15 оборотами и лишними из за инерции
          //если результат меньше 10 то он будет записан в пзу
          if(count - test < 10)
          { 
            extra = count - test;         
            EEPROM.update(8, count - test);                   
            lcd.setCursor(0, 1);
            lcd.print("сохранен");
            delay(2000);
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Ошибка.");
            lcd.setCursor(0, 1);
            lcd.print("записи.");
            delay(3000);            
          }
          break;
        }
      }
      break;
    }
    else if(Minus_state)
    {
      count = extra;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ручн. +-");
      lcd.setCursor(0, 1);
      lcd.print(count);
      lcd.setCursor(3, 1);
      lcd.print("Тек.");
      lcd.print(extra);
      delay(500);
      while(true)
      {
        Sta_sto_state = digitalRead(Start_stop);
        Plus_state = digitalRead(Plus);
        Minus_state = digitalRead(Minus);

        if(Plus_state && count < 9)
        {
          count++;
          lcd.setCursor(0, 1);
          lcd.print(count);
          delay(300);
        }
        else if(Minus_state && count > 0)
        {
          count--;
          lcd.setCursor(0, 1);
          lcd.print(count);
          delay(300);
        }
        else if(Sta_sto_state)
        {
          extra = count;
          EEPROM.update(8, count);                   
          lcd.setCursor(0, 1);
          lcd.print("сохранен");
          delay(2000);
          break;
        }
      }  
    }
  }
}

void setRevolutions() // Установка оборотов моталки
{
  //информация на экран
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Обороты");
  lcd.setCursor(3, 1);
  lcd.print("шт.");
    
  while(!Sta_sto_state) // пока не нажата кнопка старт
  {    
    lcd.setCursor(0, 1);
    lcd.print(revilutions);
   
    Sta_sto_state = digitalRead(Start_stop);
    Plus_state = digitalRead(Plus);
    Minus_state = digitalRead(Minus);
    
    if(Plus_state && revilutions < maxRevilutions) // если количесво оборотов меньше максимального
    {
      revilutions++; // то увеличиваем на 1
      delay(200);
    }
    if(Minus_state && revilutions > minRevilutions) // если количество оборотов больше минимального
    {
      revilutions--; // то уменьшаем на 1
      delay(200);
      lcd.setCursor(0, 1);
      lcd.print("   "); // чистка дисплея при уменьшении с трехзначного на двузначное....
      lcd.setCursor(0, 1);
      lcd.print(revilutions);
    }
  }
  
  EEPROM.put(0, revilutions); // Если revilutions было изменено, то значение будет перезаписано в пзу
  
  lcd.clear();
}

void work() // работа моталки
{
  //информация на экран
  lcd.setCursor(1, 0);
  lcd.print("Готов");
  lcd.setCursor(0, 1);
  lcd.print(revilutions);
  lcd.print(" шт.");
  
  delay(1000); // защита от ложного нажатия
  
  while(true)
  {

    if(revilutions <= extra)
    {
      idiot();
      break;
    }
    
    Sta_sto_state = digitalRead(Start_stop);
    Plus_state = digitalRead(Plus);
    Minus_state = digitalRead(Minus);

    if(Sta_sto_state && revilutions > minRevilutions) // если нажат старт и обороты больше мин количества
    {
        //информация на экран
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Старт");
        lcd.setCursor(3, 1);
        lcd.print(">>");
        lcd.setCursor(5, 1);
        lcd.print(revilutions);
        delay(1000);
        digitalWrite(Relay, LOW);
        
        
        bool last_R_switch = false; // защита от дребезга, пред. состояние геркона
        bool lastSs = true; // защита от дребезга, пред. старт\стоп
        bool start = true; // для переключения пауза\старт
        int count = 0; // счетчик оборотов локальный для сравнения с оборотами глобальными
        
        while(count < revilutions)       
        {
          
        // переключатель старт стоп
        Sta_sto_state = digitalRead(Start_stop);
        Minus_state = digitalRead(Minus);

          // Если счетчик оборотов стал больше чем заданное количество - лишние обороты
          // включается режим паузы без возможности перейти в старт путем нажатия кнопки
          if(count + 1 > revilutions - extra)
            Sta_sto_state = true;

          // переключатель старт стоп
          if(Sta_sto_state != lastSs)
          {
            if(Sta_sto_state == HIGH)
            {
              start = !start;
            }
          }
          lastSs = Sta_sto_state;
                   
          if(start)
          {
            digitalWrite(Relay, LOW);
            lcd.setCursor(0, 0);
            lcd.print("От винта");
            //lcd.print(extra);
          } 
          else
          {
            digitalWrite(Relay, HIGH);
            lcd.setCursor(0, 0);
            lcd.print("Стоп    ");

            // принудительный выход из функции если при паузе нажать минус
            if(Minus_state)
            break;        
          }
          
          //чтение состояния геркона с защитой от дребезга
          R_switch_state = digitalRead(R_switch);
          if(R_switch_state != last_R_switch)
          {
            if(R_switch_state == HIGH)
            {
              count++;
              lcd.setCursor(0, 1);
              lcd.print(count);         
            }
          }
          last_R_switch = R_switch_state;
        }
        
        Sta_sto_state = true; // чтобы не возвращатся в setRevolutions()
        digitalWrite(Relay, HIGH); // выключение моталки      
        break;
        
    }
    else if(Plus_state) // если нажат + то завершаем work и возвращаемся в loop затем setRevolutions()
        break;
    else if(Minus_state)
    {
      calibration();
      break;
    }
       
  }
}

void loop()
{
  setRevolutions();
  work();
}
