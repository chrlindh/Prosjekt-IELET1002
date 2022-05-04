//Zumo32u4 Kode
//Denne koden ...
//Start Sektor: .     Slutt Sektor: ..
//    Servobiblioteker.        
#include <Wire.h>                
#include <Zumo32U4.h>             
#include <Zumo32U4Buttons.h>      
#include <Zumo32U4Encoders.h>
//    Slutt Servobiblioteker..      
             
//    Zumos komponenter som blir brukt.     
Zumo32U4Buzzer buzzer;            //Komponent: buzzer
Zumo32U4LineSensors lineSensors;  //Komponent: linjesensorer
Zumo32U4Motors motors;            //Komponent: motorer
Zumo32U4ButtonA buttonA;          //Komponent: Knapp A
Zumo32U4Encoders encoders;        //Komponent: enkodere:
//    Slutt Zumos komponenter..   

//    Definerer Border Konstanter.                              
#define QTR_THRESHOLD     1500    //Definerer underlaget til border, NB! Denne varierer fra underlag til underlag                                    
  //    Border Fart: 0 = stopp, 400 = max hastighet                                 
#define reverseSpeed      200     //Reverseringshastighet: 200, 50% av max
#define turnSpeed         200     //Snuhastighet: 200, 50% av max
#define forwardSpeed      200     //Framhastighet: 200, 50% av max
  //Border Tid 
#define reverseDuration   200     // ms
#define turnDuration      300     // ms  
//    Slutt Border Konstanter..                                

//    Linjesensor Konstanter og variabler.
uint16_t maxSpeed = 200;          //Maksimale snuhastigheten til motoren 
int value = 0;                    //Posisjonsverdien til Linjefølgeren
int16_t lastError = 0;            // Error resettes 
//    Slutt Linjesensor Konstanter og variabler..

//    Counts. 
  //Counts som brukes for avstand Zumoen kjører
int countsLeft;
int countsRight;
int counts;

  //Counts mottakerbeskjeder fra ESP32  
int count_p = 0;                  //Telleren til mottakerbeskjeden p fra ESP32 (p = Lavt batteri)                    
int count_q = 0;                  //Telleren til mottakerbeskjeden q fra ESP32 (q = Kritisk lavt batteri)
//    Counts..                     
                                  
//    Batteri.                        
int batteryPercent = 100;         //Batteriprosent til Zumo
int maxBattery = 100;             //Max batteritilstand
int wearCounter = 0;
//    Batteri Slutt..                               
                                  
//    Teller tiden for avstanden.    
float totalTimeTraveled;            //Antall millisekunder koden har kjørt
float timeTraveled = 0;           //Antall millisekunder bilen har kjørt
//    Slutt tidsteller..          
                                  
//    Antall centimeter.    
int cmCount = 0;                //Centimeter kjørt
int cmDistanceNow = 0;
int cmDistanceNow2 = 0;

//    Slutt antall centimeter kjørt..

float speedNow = 0;               //Speedometer Zumo hvert sekund                                
float speedAverage = 0;           //Speedometer Zumo Gjennomsnitt
int var = 1;                      // Swich Case defualt
                                  
//    Millis som "delayfunksjoner".    
unsigned long previousMillis1 = 0;//Til Millis i funksjon: Linjefølger
unsigned long previousMillis2 = 0;//Til Millis i funksjon: Border
unsigned long previousMillis3 = 0;//Til Millis i funksjon: Speed

//    Slutt Millis..              
                                  
// FUNKSJON: Kalibrerer sensoren.  
void calibrateSensors() {                                 
  lineSensors.initFiveSensors();  //Bruker alle 5 linjesensorene
  // Vent 1 sekund og begynn deretter automatisk sensorkalibrering
  // ved å rotere på plass for å sveipe sensorene over linjen 
  delay(1000);                    //Vent 1 sekund
  for (uint16_t i = 0; i < 120; i++){ //Roterer Zumoen for å sveipe sensorene over linjen
    if (i > 30 && i <= 90){         
      motors.setSpeeds(-200, 200);//Venstrehjul rygger, Høyrehjul kjører framover. Begge med 50% av maxhastighet
    }
    else{
      motors.setSpeeds(200, -200);//Venstrehjul kjører framover, Høyrehjul rygger. Begge med 50% av maxhastighet
    }
    lineSensors.calibrate();      //Kalibrerer Sensorer          
  }
  motors.setSpeeds(0, 0);         //Begge motorer stopper
}                                 
                                  
void setup()                      
{
  Serial1.begin(115200);          //Stiller inn datahastigheten til 115200 baud 
  Serial.begin(115200);           //FJERN MED ESP!!!!         
  calibrateSensors();             //Kalibrerer linjesensorer
  delay(2000);                    //Vent 2 Sekund
} 

void linjefolger() 
{ 
  
  //For hvert 2,5 sekund, skriv batteriprosenten til ESP32
  unsigned long currentMillis1 = millis();                    //Nåværende millis     
  if(currentMillis1 - previousMillis1 >= 2500) {
    previousMillis1 = currentMillis1;                         //Forrige millis settes som Nåværende, slik at den funker som delay
    Serial1.write(batteryPercent);                            //Skriver batteriprosenten til ESP32
  }
  
  #define NUM_SENSORS 5                                       //Definerer de 5 linjesensorene
  unsigned int lineSensorValues[NUM_SENSORS];                 //Alle 5 linjesensorene brukes
  int16_t position = lineSensors.readLine(lineSensorValues);  // Få posisjonen til linjen 
  
  // Vår "error" er hvor langt unna vi er fra sentrum av 
  // linjen, som tilsvarer posisjon 2000 
  int16_t error = position - 2000; 
  // Få motorhastighetsforskjell ved hjelp av proporsjonal og derivert 
  // Her bruker vi en proporsjonal konstant på 1/4 og en derivert konstant på 6, som burde 
  // fungerer anstendig for mange Zumo motoriske valg 
  int16_t speedDifference = error / 4 + 6 * (error - lastError); 

  // Value settes til å være den nåværende posisjonen,  
  // og oppdateres hele tiden 
  lastError = error; 
  value = error + 2000; 
  
  // Få individuelle motorhastigheter  
  // Tegnet på hastighetsforskjell bestemmer om roboten svinger til venstre eller høyre. 
  int16_t leftSpeed = (int16_t)maxSpeed + speedDifference; 
  int16_t rightSpeed = (int16_t)maxSpeed - speedDifference; 

  // Begrens motorhastighetene våre til å være mellom 0 og maxSpeed. 
  // Den ene motoren vil alltid gå på maxSpeed, og den andre 
  // vil være på maxSpeed-|speedDifference| hvis det er positivt, 
  // eller så blir den stasjonær. 
  leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed); 
  rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);   
  motors.setSpeeds(leftSpeed, rightSpeed); 


  if((lineSensorValues[0] < 2) && (lineSensorValues[4] > 400) && batteryPercent < 90) {
    motors.setSpeeds(0, 0);
    //for (int i; i <= 1; i++){
    Serial1.write(101);
    
    buzzer.playNote(NOTE_A(4), 1000, 10);    // Spill note A i octave 4 på volum 10 (max 15)
    delay(1000);                            //Vent 1 sekund
    buzzer.stopPlaying();
    Serial1.write(101);
    
  }
}







void border()
{
  //For hvert 2,5 sekund, skriv batteriprosenten til ESP32
  unsigned long currentMillis2 = millis();                //Nåværende millis
    if(currentMillis2 - previousMillis2 >= 2500) {
      previousMillis2 = currentMillis2;                   //Forrige millis settes som Nåværende, slik at den funker som delay
      Serial1.write(batteryPercent);                      //Skriver batteriprosenten til ESP32             
    }
  lineSensors.initThreeSensors();                         //Aktiverer 3 linjesensorer (1,3,5)
  #define NUM_SENSORS 3                                   //Definerer de 3 linjesensorene
  unsigned int lineSensorValues[NUM_SENSORS];             //De 3 linjesensorene brukes
  lineSensors.read(lineSensorValues);                     //Få posisjonen til linjen
  
  if (lineSensorValues[0] > QTR_THRESHOLD)                //Hvis linjesensorverdier er svart linje
  {
    // Hvis sensoren mest til venstre ser den svarte linjen, rygg og snu 90 grader til høyre og kjør videre
    motors.setSpeeds(-reverseSpeed, -reverseSpeed);
    delay(reverseDuration);
    motors.setSpeeds(turnSpeed, -turnSpeed);
    delay(turnDuration);
    motors.setSpeeds(forwardSpeed, forwardSpeed);
  }
  else if (lineSensorValues[NUM_SENSORS - 1] > QTR_THRESHOLD)
  {
    // Hvis sensoren mest til høyre ser den svarte linjen, rygg og snu 90 grader til venstre og kjør videre
    motors.setSpeeds(-reverseSpeed, -reverseSpeed);
    delay(reverseDuration);
    motors.setSpeeds(-turnSpeed, turnSpeed);
    delay(turnDuration);
    motors.setSpeeds(forwardSpeed, forwardSpeed);
  }
  else
  {
    //Dersom den treffer ingen svart linje, kjører den framover
    motors.setSpeeds(forwardSpeed, forwardSpeed);
  }
  if (batteryPercent < 90) {
    delay(reverseDuration);
    motors.setSpeeds(100, 100);
    if (lineSensorValues[2] > 1000) {
      var = 3;
    }
  }
}
//buzzerRecharge funksjonen kjører en buzzer-lyd i 1 sekund
void buzzerRecharge() {   
  buzzer.playNote(NOTE_A(4), 1000, 10);    // Spill note A i octave 4 på volum 10 (max 15)
  delay(1000);                            //Vent 1 sekund
  buzzer.stopPlaying();                   //Stopp buzzer
  }


void loop()
{
  // Teller antall counts
  countsLeft = encoders.getCountsAndResetLeft();
  countsRight = encoders.getCountsAndResetRight();

  // Begynner telleren
  //totalTimeTraveled = millis();
  //timeTraveled = (totalTimeTraveled) - 6095;    //Substraherer på tiden det tar for startup
  int meterCount = ((cmCount/100) + 200);             //Gjør om til meter
  //int speedAverage = ((meterCount*10) / (timeTraveled/1000)) + 150;    //  Gjennomsnittshastighet
  
  
  unsigned long currentMillis3 = millis();                //Nåværende millis
  if(currentMillis3 - previousMillis3 >= 1000) {
    previousMillis3 = currentMillis3;                   //Forrige millis settes som Nåværende, slik at den funker som delay
    //Sender til esp


    Serial1.write(meterCount);

    
    int meterDistanceNow = (cmDistanceNow/100);
    int speedNow = (meterDistanceNow*100) + 180;              //Nåværende hastighet (oppdateres hvert sekund)
    Serial1.write(speedNow);                  
    cmDistanceNow = 0;       
    Serial.print("Current speed: ");
    Serial.println(speedNow);        
  }
  

  // Skalert til cm for mer presisjon i telling 
  
  // Når Zumo begynner å kjøre, teller den
  do {
    countsLeft = encoders.getCountsLeft();
    countsRight = encoders.getCountsRight();
    counts = ((countsLeft + countsRight)/2);
    
    switch(var){
      case 1:
        linjefolger();
        break;
      case 2:
        border();
        break;
      case 3:
        calibrateSensors();
        var = 1;
        break;
    }
  
    if (buttonA.isPressed()){
      if (var == 1) {
        var = 2;
      }
      else if (var == 2) {
        var = 3;
      }
      else  {
        var = 1;
      }
    }   
  }  while(counts < 80); 
    
  String batteryRecharge; 
  while (Serial1.available()){
    char received = (char)Serial1.read();
    batteryRecharge = received;
    if(batteryRecharge == "p") {
      count_p++;
      if (count_p == 1) {
        buzzerRecharge();
        // count_p = 0
      }
//      wearCounter++;
//      if (wearCounter == 10) {
//        maxBattery = maxBattery-1;
//        wearCounter = 0;
//      }
    }
    else if(batteryRecharge == "q") {
      count_q++;
      if (count_q == 6) {
        motors.setSpeeds(0,0);
        delay(1000);
        buzzerRecharge();
        delay(200);
        buzzerRecharge();        
        count_q = 0;
      }
    }
  }
  Serial.println(cmDistanceNow);
  cmDistanceNow++;
  cmDistanceNow2++;
  cmCount++;
  batteryPercent = map(cmCount, 0, 2000, maxBattery, 0);
   

//Ladefunksjon 

}
