// Legg til servobibliotek 
#include <Wire.h> 
#include <Zumo32U4.h> 
#include <Zumo32U4Buttons.h> 
#include <Zumo32U4Encoders.h>

// Border def
#define QTR_THRESHOLD     1800  // microseconds (cirka 1500 - 1800)

// These might need to be tuned for different motor types.
#define REVERSE_SPEED     200  // 0 is stopped, 400 is full speed
#define TURN_SPEED        200
#define FORWARD_SPEED     200
#define REVERSE_DURATION  200  // ms
#define TURN_DURATION     300  // ms
//
 
// Zumos komponenter som blir brukt 
Zumo32U4Buzzer buzzer; 
Zumo32U4LineSensors lineSensors; 
Zumo32U4Motors motors; 
Zumo32U4ButtonA buttonA; 
Zumo32U4Encoders encoders; 
Zumo32U4LCD display; 

// Linjefolger

// Maksimale hastigheten til motoren, som den får lov til å snu 
uint16_t maxSpeed = 200; 

// Posisjonsverdien 
int value = 0; 

// Error resettes 
int16_t lastError = 0; 
//

// Batteri og distanse

// Teller counts, altså avstand
int countsLeft;
int countsRight;
int counts;
int distanceTraveled;

// Teller tiden per meter
float preTimeTraveled = 0;
float postTimeTraveled = 0;
float timeTraveled = 0;

// Antall meter
float meterCount = 0;

// Batteriprosent(Finne en måte å beholde forrige verdi)
int batteriProsent = 100;


// Kalibrer sensoren 
void calibrateSensors() 
{
  lineSensors.initFiveSensors(); 
  // Vent 1 sekund og begynn deretter automatisk sensorkalibrering
  // ved å rotere på plass for å sveipe sensorene over linjen 
  delay(1000);
  for (uint16_t i = 0; i < 120; i++){
    if (i > 30 && i <= 90){
      motors.setSpeeds(-200, 200);
    }
    else{
      motors.setSpeeds(200, -200);
    }
    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

// Swich Case defualt
int var = 1;

void setup() 
{
  calibrateSensors(); 
  delay(2000); 
  Serial1.begin(9600);
} 

void linjefolger() 
{ 

  Serial1.write(batteriProsent);
  #define NUM_SENSORS 5 
  unsigned int lineSensorValues[NUM_SENSORS]; 
  
  // Få posisjonen til linjen. 
  int16_t position = lineSensors.readLine(lineSensorValues); 
  
  // Vår "error" er hvor langt unna vi er fra sentrum av 
  // linjen, som tilsvarer posisjon 2000 
  int16_t error = position - 2000; 
  // Få motorhastighetsforskjell ved hjelp av proporsjonal og derivert 
  // PID-termer (den integrerte termen er generelt lite nyttig for linjefølge) 
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
} 

void border()
{
  lineSensors.initThreeSensors();
  #define NUM_SENSORS 3
  unsigned int lineSensorValues[NUM_SENSORS]; 

  lineSensors.read(lineSensorValues);
  
  if (lineSensorValues[0] > QTR_THRESHOLD)
  {
    // If leftmost sensor detects line, reverse and turn to the
    // right.
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
  else if (lineSensorValues[NUM_SENSORS - 1] > QTR_THRESHOLD)
  {
    // If rightmost sensor detects line, reverse and turn to the
    // left.
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
  else
  {
    // Otherwise, go straight.
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
}

void loop()
{
  /*
  Kommentar:
  Flytte Batterifunksjonen til ESP32, slik at den ikke påvirker
  linjefølgeren. 
  */
  
  // Teller antall counts
  countsLeft = encoders.getCountsAndResetLeft();
  countsRight = encoders.getCountsAndResetRight();

  // Begynner telleren
  preTimeTraveled = millis();

  // Printer ut batteriprosent
  

  // Skalert til cm for mer presisjon i telling 
  
  // Når Zumo begynner å kjøre, teller den
  do {
    countsLeft = encoders.getCountsLeft();
    countsRight = encoders.getCountsRight();
    counts = ((countsLeft + countsRight)/2);
    distanceTraveled = (counts/8000);
    switch(var){
    case 1:
      linjefolger();

      break;
    case 2:
      border();
      Serial1.write(batteriProsent);
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
  postTimeTraveled = millis();
  timeTraveled = (postTimeTraveled - preTimeTraveled)/10;
  meterCount++;
  batteriProsent = map(meterCount, 0, 2000, 100, 0); 

}
