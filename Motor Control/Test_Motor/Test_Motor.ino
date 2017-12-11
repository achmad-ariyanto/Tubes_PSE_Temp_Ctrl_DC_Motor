#include <Arduino_FreeRTOS.h>

//Arduino PWM Speed Control：
int E1 = 5;  
int M1 = 4; 
int E2 = 6;                      
int M2 = 7;                        
int data = 2;

void Task_Read(void *pvParameters);

void setup() 
{ 
    pinMode(M1, OUTPUT);   
    pinMode(M2, OUTPUT);
    pinMode(data, INPUT);   
    Serial.begin(9600);

    digitalWrite(M1, HIGH);
    analogWrite(E1, 255); 

    xTaskCreate(Task_Read, (const portCHAR *) "Read Data", 100, NULL, 1, NULL);

    vTaskStartScheduler();
} 

void loop() 
{ 
  /*
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
  */
}

void Task_Read(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  int count = 0;
  
  for(;;){
    if (digitalRead(data) == HIGH) count++;
    //count++;
    Serial.println(count);
    vTaskDelayUntil( &xLastWakeTime, ( 50 / portTICK_PERIOD_MS ) );
  }
}

