#include <Arduino_FreeRTOS.h>

// Pin Assignment
int E1 = 5; // PWM Control 
int M1 = 4; // Direction Control
int feedback = A0; // Motor Feedback
int temp = 3; // Temperature sensor

// Task Initialization
void Task_Set_Speed(void *pvParameters);
//void Task_Keypad(void *pvParameters);
//void Task_Read_Temp(void *pvParameters);
//void Task_Display(void *pvParameters);

void setup() 
{ 
    // I/O declaration
    pinMode(M1, OUTPUT);   
    pinMode(E1, OUTPUT);
    
    pinMode(feedback, INPUT);
    //pinMode(temp, INPUT);
    
    // Serial Initialization
    Serial.begin(9600);

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
  int value;
  for(value = 0 ; value <= 255; value+=5) 
  { 
    digitalWrite(M1,LOW);   
    digitalWrite(M2, LOW);       
    analogWrite(E1, 255);   //PWM Speed Control
    analogWrite(E2, 255);   //PWM Speed Control
    delay(30); 
    Serial.println(255);
  }  
}
