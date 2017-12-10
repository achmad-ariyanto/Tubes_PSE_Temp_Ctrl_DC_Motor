//Arduino PWM Speed Control：
int E1 = 5;  
int M1 = 4; 
int E2 = 6;                      
int M2 = 7;                        

void setup() 
{ 
    pinMode(M1, OUTPUT);   
    pinMode(M2, OUTPUT);
    Serial.begin(9600);
     
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
