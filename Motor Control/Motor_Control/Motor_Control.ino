#include <Arduino_FreeRTOS.h>
#include <math.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

//LCD Variables
const int rs = 3, en = 2, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);

// Constant
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
const int feedback_min = 0;
const int feedback_max = 450;
const int temperature_min = 16;
const int temperature_max = 35;

// Keypad Mapping
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Pin Assignment
#define E1 5 // PWM Control 
#define M1 4 // Direction Control
int feedback = A0; // Motor Feedback
int temp = A1; // temperature sensor

// keypad Row
byte rowPins[ROWS] = { 9, 8, 7, 6 };
// keypad Col
byte colPins[COLS] = { 13, 12, 11, 10 }; 

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Task Initialization
void Task_Set_Speed(void *pvParameters);
void Task_Keypad(void *pvParameters);
void Task_Read_Temp(void *pvParameters);
void Task_Display(void *pvParameters);

// Global Variable
char key;

int Delay = 0;
int input[] = {0,0,0};
int count = 0;
int total = 0;
int err = 0;
int fedb;
int tmp_i;
int pwm;
int mode = 0;
/*  mode 0 = get set point from temperature
 *  mode 1 = get input from keypad (set point still from temp)
 *  mode 2 = using set point from keypad
 */

unsigned long difft;

float set_point;
float tmp;
float fedb_f;
float error;

void setup() 
{ 
    // I/O declaration
    pinMode(M1, OUTPUT);   
    pinMode(E1, OUTPUT);
    pinMode(feedback, INPUT);
    pinMode(temp, INPUT);
    
    // Serial Initialization
    Serial.begin(9600);

    // LCD initialization
    lcd.begin(16,2);
    lcd.clear();
    
    // Set Motor Rotation Direction
    digitalWrite(M1, HIGH);
    
    // Task Setup
    xTaskCreate(Task_Set_Speed, (const portCHAR *) "Speed", 100, NULL, 2, NULL);
    xTaskCreate(Task_Keypad, (const portCHAR *) "Keypad", 100, NULL, 3, NULL);
    xTaskCreate(Task_Read_Temp, (const portCHAR *) "Temperature", 100, NULL, 3, NULL);
    xTaskCreate(Task_Display, (const portCHAR *) "Display", 100, NULL, 1, NULL);

    // Start Scheduler
    vTaskStartScheduler();
} 

void loop() 
{ 
  // empty loop, can be used for debugging
  
}

void Task_Read_Temp(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  
  for(;;){
    
    tmp = ((5.0 * analogRead(temp)) / 1024.0) * 100.0;
    //tmp = analogRead(temp);
    tmp_i = (int)tmp;
    vTaskDelayUntil( &xLastWakeTime, ( 5000 / portTICK_PERIOD_MS ) );
  }
}

void Task_Set_Speed(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  // PID constant
  float kp = 10;
  float ki = 0.2;
  float kd = 0.5;

  
  float error_delay = 0;
  float error_inc = 0;
  float error_diff = 0;
  
  float pwm_f;

  
  for(;;){
    
    
    fedb = map(analogRead(A0), feedback_min, feedback_max, 0, 1024);
    fedb_f = (float)fedb;
    
    // Conversion from temperature to set point
    if (mode == 0){
      set_point = map(tmp_i, temperature_min, temperature_max, 0, 1023);
    }

    
    
    error = set_point - fedb_f;
    
    error_inc += error;
    error_diff = error - error_delay;
    
    pwm_f = kp * error + ki * error_inc + kd * error_diff;

    pwm = map((int)pwm_f,0,1024,0,255);    
    if (pwm > 255) pwm = 255;
    else if (pwm < 0) pwm = 0;
  
    
    analogWrite(E1,pwm);
    
    error_delay = error;

    
    
    vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_PERIOD_MS ) );
  }
}

void Task_Keypad(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  
  for(;;){
    
    key = kpd.getKey();
      if(key){ //check for a valid key
        
        switch (mode){
          case 0:{
            if (key == '#') mode = 1; 
            count = 0;
            break;
          }
          case 1:{
            if (key == '#'){
              switch (count){
                case 0:
                  total = 0;
                  break;
                case 1:
                  total = input[0];
                  break;
                case 2:
                  total = (input[0] * 10) + (input[1]);
                  break;
                case 3:
                  total = (input[0] * 100) + (input[1] * 10) + (input[2]);
                  break;
              }
              if (total > 100){
                err = 1;
                mode = 0;
              }else{
                set_point = map(total, 0, 100, 0, 1024);
                mode = 2;
              }
            }else if (key == '*'){
              mode = 0;
            }else{
              if ((key != 'A') || (key != 'B') || (key != 'C') || (key != 'D')){
                if (count < 3){
                  input[count] = (int)key - 48;
                  count++;
                }else{
                  count = 0;
                  input[count] = (int)key - 48;
                  count++;
                }
              }          
            }
            break;
          }
          case 2:{
            if (key == '#') mode = 0; 
            if (key == '*'){
              mode = 1; 
              count = 0;
            }
            break;
          }
        }
      }
      
    vTaskDelayUntil( &xLastWakeTime, ( 20 / portTICK_PERIOD_MS ) );
  }
}

void Task_Display(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  
  for(;;){
    difft = micros();
    display_LCD(mode);
    difft = micros() - difft;
    Serial.println(difft);
    vTaskDelayUntil( &xLastWakeTime, ( 250 / portTICK_PERIOD_MS ) );
  }
}

void display_LCD(int mode){
  if (err == 1){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR, Max input");
    lcd.setCursor(0,1);
    lcd.print("    is 100%");
    if (Delay == 8){
      err = 0;
      Delay = 0;
    }else{
      Delay++;
    }
  }else{
    switch (mode){
      case 0:{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Speed:");
        lcd.setCursor(6,0);
        float t = fedb_f/1023*100;
        int i = (int)t;
        lcd.print(i);
        lcd.setCursor(9,0);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("Temp:");
        lcd.setCursor(5,1);
        lcd.print(tmp_i);
        lcd.setCursor(7,1);
        lcd.print((char)223);
        lcd.setCursor(8,1);
        lcd.print("C");
        lcd.setCursor(11,0);
        lcd.print("MODE");
        lcd.setCursor(11,1);
        lcd.print("Temp");
        break;
      }
      case 1:{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("input Set Point:");
        switch (count){
          case 3:{
            lcd.setCursor(8,1);    
            lcd.print(input[2]);
          }
          case 2:{
            lcd.setCursor(7,1);    
            lcd.print(input[1]);
          }
          case 1:{
            lcd.setCursor(6,1);    
            lcd.print(input[0]);
          }
        }
        lcd.setCursor(10,1);
        lcd.print("%");
        break;
      }
      case 2:{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Speed:");
        lcd.setCursor(6,0);
        float t = fedb_f/1023*100;
        int i = (int)t;
        lcd.print(i);
        lcd.setCursor(9,0);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("Set :");
        lcd.setCursor(5,1);
        lcd.print(total);
        lcd.setCursor(8,1);
        lcd.print("%");
        lcd.setCursor(11,0);
        lcd.print("MODE");
        lcd.setCursor(10,1);
        lcd.print("Keypad");
        break;
      }
    }
  }
}
