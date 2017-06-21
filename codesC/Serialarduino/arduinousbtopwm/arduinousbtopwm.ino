/* RGB Analog Example, Teensyduino Tutorial #2
   http://www.pjrc.com/teensy/tutorial2.html

   This example code is in the public domain.
*/

const int motorpin =  23;
const int enable = 22;
const int ledpin =  13;
const int resolution = 16;
int incomming;
float force = 0.0;
int pwmmax;

void setup()   {                
  pinMode(motorpin, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(ledpin, OUTPUT);
  analogWriteResolution(resolution);
  pwmmax = pow(2,resolution);
  Serial.begin(9600);
  incomming = 0;
}

void loop()                     
{
  digitalWrite(enable, 1);
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
    incomming =(int)( force/42.9/3.0*80.0/100.0*(float)pwmmax+10.0/100.0*(float)pwmmax);
    
  }
  Serial.println((incomming-10.0/100.0*(float)pwmmax)/(float)pwmmax*100.0/80.0*42.9*3.0);
  if(incomming <= 10.0/100.0*pwmmax)
  {
    incomming = 10.0/100.0*pwmmax;
  }
  if(incomming >= 90.0/100.0* pwmmax)
  {
     incomming = 90.0/100.0*pwmmax;
  }
  analogWrite(motorpin, incomming);
  
  digitalWrite(ledpin, 0);
  
  
}

