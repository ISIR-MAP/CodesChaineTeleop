
#define pSENSOR         A1
#define pCONTROLE       6
const int ledpin =  13;
const int resolution = 12;
const int readresolution = 12;
String message;
long lasttime;
bool debug;
int x;
int pwmmax;
int ref;
int tempsech;
void setup() {
  
  //pinMode(pSENSOR, INPUT);
  pinMode(pCONTROLE, OUTPUT);
    pinMode(ledpin, OUTPUT);
    analogWriteResolution(resolution);
    analogReadRes(readresolution); 
  analogWriteFrequency(pCONTROLE, 5000);
    pwmmax = pow(2,resolution);
    ref = pow(2,readresolution-1);
     lasttime = micros();
      Serial.begin(9600);
      debug = 0;
      if(debug)
      {
        tempsech = 150;
      }
      else
      {
        tempsech = 100;
      }
      message="";
}

int controlePwm = 0;
  double error;
  double errori;
  double errorpn;
  double sample;
  double lastSample;
  double kP, kI, kD;      
  double P, I, D;
  double pid;


  
  double setPoint;
  long lastProcess;

void loop() {
  digitalWrite(ledpin, 1);
kP = 0.8; //0.08 //0.3  //0.1             //0.09
kI = 50; //4 //2.0   //8.0   // 9.0    //4.5
kD = 0.001;

  //sensor input
  x = analogRead(0); //ADC->ADC_CDR[7]; //READ VALUE A0 //read position between 0-4096
  if(debug)
  {
    Serial.print("Pos:");
    Serial.print(x);  
  }
  //double pos = map(x, 0, 1024, 0, 255);

  errorpn = ref - x;  //127
  //errori = map(errorpn, -2048, 2048, -2048, 2048);
  //error = abs(errorpn);
    
  //setPoint = 127 //reference
    
  float deltaTime = (micros() - lasttime) / 1000000.0;
  //lastProcess = millis();
    
   //P
   P = errorpn * kP;
   if(debug)
  {
    Serial.print(" P: ");
    Serial.print(P);  
  }
    
   //I
   I = I + ((errorpn) * kI) * deltaTime;
   I = constrain(I, -2048, 2048); // saturation de l'integrateur
    if(debug)
  {
    Serial.print("I: ");
    Serial.print(I);  
  }
  
    //D
    D = (lastSample - x) * kD / deltaTime;
    lastSample = x;
     if(debug)
  {
    Serial.print("D: ");
    Serial.print(D);  
  }
    
    pid = -(P+I+D);
    
    if(pid> 2048){pid = 2048;}
    else if(pid< -2048){pid = -2048;}
controlePwm = map(pid,-2048, 2048, 0, pwmmax);
controlePwm = constrain(controlePwm, 10.0/100.0*pwmmax, 90.0/100.0* pwmmax);
 
  //write pwm
  analogWrite(pCONTROLE, controlePwm);
   if(debug)
  {
    //message = message + " pwm: "+ controlePwm;
    Serial.print(" pwm:");
    Serial.println(controlePwm);
  }
  Serial.println(pid);
   while ((micros()-lasttime) <= tempsech) {
      digitalWrite(ledpin, 0);
      }
lasttime = micros();


  /*
  Serial.println("pos:");
  Serial.println(pos);
  Serial.println("/n");
  Serial.println("controlePwm:");
  Serial.println(controlePwm);
  Serial.println("/n");
  Serial.println("myPid.errorpn:");
  Serial.println(myPid.errorpn);
  Serial.println("/n");
  Serial.println("myPid.error:");
  Serial.println(myPid.error);
*/
}


