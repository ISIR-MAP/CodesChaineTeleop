
//#define pSENSOR         A1
#define pCONTROLE       3

int x;

void setup() {
  
//  Serial.begin(9600);


   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Setup all registers
  pmc_enable_periph_clk(ID_ADC); // To use peripheral, we must enable clock distributon to it
  adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST); // initialize, set maximum posibble speed
  adc_disable_interrupt(ADC, 0xFFFFFFFF);
  adc_set_resolution(ADC, ADC_12_BITS);
  adc_configure_power_save(ADC, 0, 0); // Disable sleep
  adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_3, 1); // Set timings - standard values
  adc_set_bias_current(ADC, 1); // Bias current - maximum performance over current consumption
  adc_stop_sequencer(ADC); // not using it
  adc_disable_tag(ADC); // it has to do with sequencer, not using it
  adc_disable_ts(ADC); // deisable temperature sensor
  adc_disable_channel_differential_input(ADC, ADC_CHANNEL_7);
  adc_configure_trigger(ADC, ADC_TRIG_SW, 1); // triggering from software, freerunning mode
  adc_disable_all_channel(ADC);
  adc_enable_channel(ADC, ADC_CHANNEL_7);

  ////////////////////////////////////////////////

 ////////////////////////////////////////////////// 
  
  //pinMode(pSENSOR, INPUT);
  pinMode(pCONTROLE, OUTPUT);
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


kP = 0.08; //0.08 //0.3  //0.1             //0.09
kI = 4.5; //4 //2.0   //8.0   // 9.0    //4.5
kD = 0.0001;

  //sensor input
  x = ADC->ADC_CDR[7]; //READ VALUE A0 //read position between 0-4096
  double pos = map(x, 0, 4096, 0, 255);

  errorpn = 30 - pos;  //127
  errori = map(errorpn, -128, 128, -128, 128);
  error = abs(errorpn);
    
  //setPoint = 127 //reference
    
  float deltaTime = (millis() - lastProcess) / 100000000.0;
  lastProcess = millis();
    
   //P
   P = error * kP;
    
   //I
   I = I + ((errori) * kI) * deltaTime;
    
    
    //D
    D = (lastSample - pos) * kD / deltaTime;
    lastSample = pos;
    
    pid = P+I;
    
    if(pid> 127){pid = 128;}
    else if(pid< -127){pid = -128;}


 if (errorpn <= 0)  //de 127 a 255
 {
  controlePwm = (pid + 127);  //25  //102
 }
 else if(pid > 0) // de 0 a 127
 {
  controlePwm = (128 - pid);  //229   //153 //103
 }


 /*
  if (myPid.errorpn > 0)
 {
  controlePwm = (myPid.process() + 102);
 }
 else if(myPid.errorpn <= 0)
 {
  controlePwm = (153 - myPid.process());
 }

  */


  //write pwm
  analogWrite(pCONTROLE, controlePwm);


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


