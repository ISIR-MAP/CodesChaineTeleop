
/* RGB Analog Example, Teensyduino Tutorial #2
   http://www.pjrc.com/teensy/tutorial2.html
   This example code is in the public domain.
*/

const int motorpin =  6;
const int sensA = 23;
const int sensB = 22;
const int ledpin =  13;
const int resolution = 11;
int incomming;
float force = 0.0;
int pwmmax;

void setup()   {                
  pinMode(motorpin, OUTPUT);
  pinMode(sensA, OUTPUT);
  pinMode(sensB, OUTPUT);
  pinMode(ledpin, OUTPUT);
  analogWriteResolution(resolution);
  analogWriteFrequency(motorpin, 23437.5); // Teensy 3.0 pin 3 also changes to 375 kHz
  pwmmax = pow(2,resolution);
  Serial.begin(9600);
  incomming = 0;
}

void loop()                     
{
  
  digitalWrite(ledpin, 1);
  if(Serial.available() > 0)
  {
    char BUFFER[Serial.available()];
    int index = 0;
    //Serial.println(Serial.available());
    while(Serial.peek() != 10)
    {
    BUFFER[index] = Serial.read();
    index++;
    }
    Serial.read();
    sscanf(BUFFER,"%f",&force);
    //Serial.println(force);
    incomming =(int)( (force-2048)/2048*(float)pwmmax);
    
  }
  Serial.println(incomming);
  if (incomming <0)
  {
  analogWrite(motorpin, (int)-incomming);
  
  digitalWrite(sensA, 1);
  digitalWrite(sensB, 0);
  }
  else
  {
  analogWrite(motorpin, (int)incomming);
  
  digitalWrite(sensA, 0);
  digitalWrite(sensB, 1);
    
  } 
  digitalWrite(ledpin, 0);
  
  
}
