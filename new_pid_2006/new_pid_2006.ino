
#define pSENSOR         A1
#define pCONTROLE       6
const int ledpin =  13;
const int resolution = 12;
const int readresolution = 12;

long lasttime;

int x;
int pwmmax;
int ref;
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
kP = 0.08; //0.08 //0.3  //0.1             //0.09
kI = 10; //4 //2.0   //8.0   // 9.0    //4.5
kD = 0.0001;

  //sensor input
  x = analogRead(0); //ADC->ADC_CDR[7]; //READ VALUE A0 //read position between 0-4096
    Serial.print(x);
  //double pos = map(x, 0, 1024, 0, 255);

  errorpn = ref - x;  //127
  //errori = map(errorpn, -2048, 2048, -2048, 2048);
  //error = abs(errorpn);
    
  //setPoint = 127 //reference
    
  float deltaTime = (micros() - lasttime) / 1000000.0;
  //lastProcess = millis();
    
   //P
   P = errorpn * kP;
   Serial.print(" P: ");
   Serial.print(P);
    
   //I
   I = I + ((errorpn) * kI) * deltaTime;
   I = constrain(I, -2048, 2048); // saturation de l'integrateur
   Serial.print(" I: ");
   Serial.print(I);
    
    //D
    D = (lastSample - x) * kD / deltaTime;
    lastSample = x;
    Serial.print(" D: ");
   Serial.print(D);
    
    pid = -(P+I+D);
    
    if(pid> 2048){pid = 2048;}
    else if(pid< -2048){pid = -2048;}
controlePwm = map(pid,-2048, 2048, 0, pwmmax);
controlePwm = constrain(controlePwm, 10.0/100.0*pwmmax, 90.0/100.0* pwmmax);
 
  //write pwm
  analogWrite(pCONTROLE, controlePwm);
  Serial.print(" ");
  Serial.println(controlePwm);
   while ((micros()-lasttime) <= 125) {
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


