
#define pCONTROLE       6
#define DIR       20
const int ledpin =  13;
const int ena =  17;   // enable
const int resolution = 12;
const int readresolution = 12;
String message;
long lasttime;
bool debug;
int x;
int pwmmax;
int ref;
int tempsech;
int sousech;
int lastdir;
void setup() {
  
  //pinMode(pSENSOR, INPUT);
  pinMode(pCONTROLE, OUTPUT);
  pinMode(DIR, OUTPUT);
    pinMode(ledpin, OUTPUT);
    pinMode(ena, OUTPUT);
    analogWriteResolution(resolution);
    analogReadRes(readresolution); 
  analogWriteFrequency(pCONTROLE, 5000);
    pwmmax = pow(2,resolution);
    ref = 75.0/100.0 * pow(2,readresolution-1);
     lasttime = micros();
      Serial.begin(9600);
      debug =0;
      if(debug)
      {
        tempsech = 1000;
      }
      else
      {
        tempsech = 100;
      }
      message="";
      sousech=0;
      lastdir=0;
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
  double alpha;
  double pb;
  double force;
  int grg = 0;


  
  double setPoint;
  long lastProcess;

void loop() {
  
  digitalWrite(ena, 1);
  digitalWrite(ledpin, 1);
kP = 2.0*0.855; //0.08 //0.3  //0.1             //0.09
kI = 0*2*100.5; //4 //2.0   //8.0   // 9.0    //4.5
kD = 2.0*0.001;  //0.001
pb = 1.0/4000.0;

  //sensor input
  x = analogRead(0); //ADC->ADC_CDR[7]; //READ VALUE A0 //read position between 0-4096
  if(!debug)
  {
    //Serial.print("Pos:");
    Serial.print(x);  
    Serial.print(" ");
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
   I = constrain(I, -4096, 4096); // saturation de l'integrateur
    if(debug)
  {
    Serial.print("I: ");
    Serial.print(I);  
  }
  
    //D
    //D = (lastSample - x) * kD / deltaTime;
    alpha = deltaTime/(pb + deltaTime);
    D = alpha*(lastSample - x) * kD / deltaTime + (1- alpha)*D;
    //D = constrain(D, -2048, 2048); // saturation de l'integrateur
    lastSample = x;
     if(debug)
  {
    Serial.print("D: ");
    Serial.print(D);  
  }
    
    pid = (P+I+D);//+D
    pid=constrain(pid, -4096, 4096);
    if(pid< 0.0){
      if(lastdir=0)
      {
        I=0.0;
      }
      lastdir=1;
     analogWrite(pCONTROLE, 10.0/100.0*pwmmax);
     digitalWrite(DIR, 1);
      controlePwm = map(-pid,0, 4096, 10.0/100.0*pwmmax, 90.0/100.0*pwmmax);
      analogWrite(pCONTROLE, controlePwm);
       
      //pid+= 2048;
    }
      else
      {
       if(lastdir=1)
      {
        I=0.0;
      }
      lastdir=0;
        analogWrite(pCONTROLE, 10.0/100.0*pwmmax);
        digitalWrite(DIR, 0);
        controlePwm = map(pid,0, 4096, 10.0/100.0*pwmmax, 90.0/100.0*pwmmax);
        analogWrite(pCONTROLE, controlePwm);
        
      //pid-= 2048;
      }
   /* if(pid> 2048){pid = 2048;}
    else if(pid< -2048){pid = -2048;}*/
//controlePwm = map(pid,-2048, 2048, 0.1*pwmmax, 0.9*pwmmax);
//controlePwm = constrain(controlePwm, 10.0/100.0*pwmmax, 90.0/100.0* pwmmax);
 
  //write pwm
  
   if(debug)
  {
    message = message + " pwm: "+ controlePwm;


/////////////////////////////////////////

    Serial.print(" pwm:");
    //Serial.print((String)controlePwm);

    //////////////////////////////////////////////////////
    Serial.print(controlePwm);
  }

   alpha = deltaTime/(1.0/1000.0 + deltaTime);
   force = alpha*pid + (1- alpha)*force;
   sousech++;
   if(sousech%1==0)
   {
      Serial.println((int)force);
  sousech=0;
   }
   
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


