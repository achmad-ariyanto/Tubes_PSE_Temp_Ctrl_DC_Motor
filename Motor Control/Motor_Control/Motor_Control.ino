#include <Arduino_FreeRTOS.h>
#include <Keypad.h>
#include <dht.h>
#include <math.h>



// Constant
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
const int feedback_min = 0;
const int feedback_max = 500;
const int temperature_min = 18;
const int temperature_max = 30;

// Keypad Mapping
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'#','0','*','D'}
};

// Pin Assignment
int E1 = 5; // PWM Control 
int M1 = 4; // Direction Control
int feedback = A0; // Motor Feedback
int temp = 2; // Temperature sensor
int display[] = {A1, A2, A3, A4, A5, 3};


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

// DHT initialization
dht DHT;

// Global Variable
char key;

int err = 0;
int tmp = 0;
int fedb;
int pwm;
int mode = 0;
/*  mode 0 = get set point from temperature
 *  mode 1 = get input from keypad (set point still from temp)
 *  mode 2 = using set point from keypad
 */

float set_point;

void setup() 
{ 
    // I/O declaration
    pinMode(M1, OUTPUT);   
    pinMode(E1, OUTPUT);
    pinMode(feedback, INPUT);
    pinMode(temp, INPUT);
    
    for (int i = 0; i < 6; i++){
      pinMode(display[i], OUTPUT);
    }
    // Serial Initialization
    Serial.begin(9600);
    
    // Get first temperature reading
    while (tmp == 0){
      tmp = DHT.read11(temp);
    }
    
    // Set Motor Rotation Direction
    digitalWrite(M1, HIGH);
    
    // Task Setup
    xTaskCreate(Task_Set_Speed, (const portCHAR *) "Speed", 100, NULL, 2, NULL);
    //xTaskCreate(Task_Keypad, (const portCHAR *) "Keypad", 100, NULL, 3, NULL);
    xTaskCreate(Task_Read_Temp, (const portCHAR *) "Temperature", 100, NULL, 3, NULL);
    //xTaskCreate(Task_Display, (const portCHAR *) "Display", 100, NULL, 1, NULL);

    // Start Scheduler
    vTaskStartScheduler();
} 

void loop() 
{ 

}

void Task_Read_Temp(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  
  for(;;){
    tmp = DHT.read11(temp);
    vTaskDelayUntil( &xLastWakeTime, ( 5000 / portTICK_PERIOD_MS ) );
  }
}

void Task_Set_Speed(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  // PID constant
  float kp = 1;
  float ki = 0;
  float kd = 0;
  
  float fedb_f;

  float error;
  float error_delay = 0;
  float error_inc = 0;
  float error_diff = 0;
  
  float pwm_f;
  float pwm_f_abs;
  
  for(;;){
    fedb = map(analogRead(A0), feedback_min, feedback_max, 0, 1023);
    fedb_f = (float)fedb;
    
    // Conversion from temperature to set point
    if (mode != 2){
      set_point = map(tmp, temperature_min, temperature_max, 0, 255);
    }
    
    error = set_point - fedb_f;
    error_inc =+ error;
    error_diff = error - error_delay;
    
    pwm_f = kp * error + ki * error_inc + kd * error_diff;

    pwm_f_abs = abs(pwm_f);
    pwm = (int)((pwm_f_abs * 255.0)/5.0);

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
  int count = 0;
  int input[] = {0,0,0};
  int total = 0;
  
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
                set_point = total;
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
            break;
          }
        }
      }
    vTaskDelayUntil( &xLastWakeTime, ( 5000 / portTICK_PERIOD_MS ) );
  }
}
