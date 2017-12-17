#include <Arduino_FreeRTOS.h>
#include <Keypad.h>
#include <dht.h>

// Constant
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns

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

// Global Variable
int tmp = 0;
int fedb;
int pwm;
int mode = 0;

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
    
    // Get first temperature reading
    while (tmp == 0){
      tmp = DHT.read11(temp)  
    }
    
    // Serial Initialization
    Serial.begin(9600);

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
  float kp, ki, kd; // PID constant
  
  float set_point;
  float fedb_f;

  float error;
  float error_delay;
  float error_delay2;

  float pwm_f;
  float pwm_f_delay;
  float pwm_f_delay2;
  float pwm_f_abs;
  
  for(;;){
    fedb = map(analogRead(A0), feedback_min, feedback_max, 0, 1024);
    fedb_f = fedb * 5.0/1023.0
    
    // Conversion from temperature to set point
    /*
    if (mode == 1){
      set_point =
    }
    */
    error = set_point - fedb_f;
    error_delay2 = error_delay
  }
}
