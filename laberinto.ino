#include "index.h"
#include "Adafruit_VL53L0X.h"
#include "Motors.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#define FRONT_LOX_ADDRESS 0x30
#define RIGHT_LOX_ADDRESS 0x31
#define LEFT_LOX_ADDRESS 0x32

#define FRONT_LOX_PIN 5
#define RIGHT_LOX_PIN 18
#define LEFT_LOX_PIN 19

#define TURN_SPEED 350


const char* SSID = "Robot_Vinicios";
const char* PASS = "0123456789";

const char* PARAM_INPUT_KP = "kp";
const char* PARAM_INPUT_KI = "ki";
const char* PARAM_INPUT_KD = "kd";
const char* PARAM_INPUT_MAX_SPEED = "max_speed";
const char* PARAM_INPUT_STATE = "state";

const int SETPOINT = 175;
const int MAX_INTEGRAL = 2000;
const int THRESHOLD_FRONT   = 300;  // mm — sensor frontal perpendicular
const int THRESHOLD_LATERAL = 200;  // mm — sensores laterales a 45°

float kp = 0.0, ki = 0.0, kd = 0.0;
float last_error = 0.0, integral = 0.0;
int error1 = 0, error2 = 0, error3 = 0, error4 = 0, error5 = 0, error6 = 0;

int max_percent_speed, min_speed, max_speed, left_motor_speed, right_motor_speed;

bool state = false;  // variable for state of robot

float frontSensorValue, rightSensorValue, leftSensorValue;


Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Motors motors(12, 14, 13, 26, 27, 25, 1000, 10);
AsyncWebServer server(80);
Preferences preferences;

// objects for the vl53l0x
Adafruit_VL53L0X frontSensor = Adafruit_VL53L0X();
Adafruit_VL53L0X rightSensor = Adafruit_VL53L0X();
Adafruit_VL53L0X leftSensor = Adafruit_VL53L0X();

// kalman struct
struct KalmanFilter {
  float q;
  float r;
  float x;
  float p;
  float k;
};

KalmanFilter filters[3];  // array for kalman filters

String processor(const String& var) {
  if (var == "KP") {
    return String(preferences.getFloat("kp", 0.0));
  } else if (var == "KI") {
    return String(preferences.getFloat("ki", 0.0));
  } else if (var == "KD") {
    return String(preferences.getFloat("kd", 0.0));
  } else if (var == "MAX_SPEED") {
    return String(preferences.getInt("max_speed", 0));
  } else if (var == "STATE") {
    bool aux_state = preferences.getBool("state", 0);
    if (aux_state) {
      return String("Encender");
    } else {
      return String("Apagar");
    }
  }

  return String();
}

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "NOT FOUND");
}

void initSensor(Adafruit_VL53L0X& sensor, int pin, int addr, int num) {
  digitalWrite(pin, HIGH);  // Encender sensor
  delay(10);
  if (!sensor.begin(addr)) {
    Serial.print("Fail sensor: ");
    Serial.println(num);
    while (1)
      ;
  }
}

void setIds() {
  //config outputs
  pinMode(FRONT_LOX_PIN, OUTPUT);
  pinMode(RIGHT_LOX_PIN, OUTPUT);
  pinMode(LEFT_LOX_PIN, OUTPUT);

  // all reset
  digitalWrite(FRONT_LOX_PIN, LOW);
  digitalWrite(RIGHT_LOX_PIN, LOW);
  digitalWrite(LEFT_LOX_PIN, LOW);
  delay(10);
  // all unreset
  digitalWrite(FRONT_LOX_PIN, HIGH);
  digitalWrite(RIGHT_LOX_PIN, HIGH);
  digitalWrite(LEFT_LOX_PIN, HIGH);
  delay(10);

  // activating LOX1 and reseting LOX2
  digitalWrite(FRONT_LOX_PIN, HIGH);
  digitalWrite(RIGHT_LOX_PIN, LOW);
  digitalWrite(LEFT_LOX_PIN, LOW);

  // initing LOX FRONT
  delay(10);
  if (!frontSensor.begin(FRONT_LOX_ADDRESS)) {
    Serial.println(F("Failed to boot front VL53L0X"));
    while (1)
      ;
  }
  delay(10);


  // initing LOX RIGHT
  digitalWrite(RIGHT_LOX_PIN, HIGH);
  delay(10);
  if (!rightSensor.begin(RIGHT_LOX_ADDRESS)) {
    Serial.println(F("Failed to boot right VL53L0X"));
    while (1)
      ;
  }
  delay(10);

  // initing LOX LEFT
  digitalWrite(LEFT_LOX_PIN, HIGH);
  delay(10);
  if (!leftSensor.begin(LEFT_LOX_ADDRESS)) {
    Serial.println(F("Failed to boot left VL53L0X"));
    while (1)
      ;
  }
  delay(10);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println("Initing data storage");
  preferences.begin("data", false);
  Serial.println("Initing data storage done");

  Serial.println("Reading data from storage");
  kp = preferences.getFloat("kp", 0.0);
  ki = preferences.getFloat("ki", 0.0);
  kd = preferences.getFloat("kd", 0.0);
  max_percent_speed = preferences.getInt("max_speed", 0);
  max_speed = map(max_percent_speed, 0, 100, 0, 1020);
  min_speed = (-1) * max_speed;
  state = preferences.getBool("state", 0);
  Serial.println("Reading data from storage done");

  Serial.print("Initing AP: ");
  Serial.println(SSID);
  WiFi.softAP(SSID, PASS);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("Initing sensors and filters");
  pinMode(FRONT_LOX_PIN, OUTPUT);
  pinMode(RIGHT_LOX_PIN, OUTPUT);
  pinMode(LEFT_LOX_PIN, OUTPUT);

  digitalWrite(FRONT_LOX_PIN, LOW);
  digitalWrite(RIGHT_LOX_PIN, LOW);
  digitalWrite(LEFT_LOX_PIN, LOW);

  initSensor(frontSensor, FRONT_LOX_PIN, FRONT_LOX_ADDRESS, 1);
  initSensor(rightSensor, RIGHT_LOX_PIN, RIGHT_LOX_ADDRESS, 2);
  initSensor(leftSensor, LEFT_LOX_PIN, LEFT_LOX_ADDRESS, 3);

  setupKalman(&filters[0], 1.0, 1.0, 1.0, 0.0);
  setupKalman(&filters[1], 1.0, 1.0, 1.0, 0.0);
  setupKalman(&filters[2], 1.0, 1.0, 1.0, 0.0);
  Serial.println("Initing sensors done");

  Serial.println("Initing server");
  server.onNotFound(notFound);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;

    if (request->hasParam(PARAM_INPUT_KP)) {
      inputMessage = request->getParam(PARAM_INPUT_KP)->value();
      kp = inputMessage.toFloat();
      preferences.putFloat("kp", kp);
    }

    if (request->hasParam(PARAM_INPUT_KI)) {
      inputMessage = request->getParam(PARAM_INPUT_KI)->value();
      ki = inputMessage.toFloat();
      preferences.putFloat("ki", ki);
    }

    if (request->hasParam(PARAM_INPUT_KD)) {
      inputMessage = request->getParam(PARAM_INPUT_KD)->value();
      kd = inputMessage.toFloat();
      preferences.putFloat("kd", kd);
    }

    if (request->hasParam(PARAM_INPUT_MAX_SPEED)) {
      inputMessage = request->getParam(PARAM_INPUT_MAX_SPEED)->value();
      max_percent_speed = inputMessage.toInt();
      max_speed = map(max_percent_speed, 0, 100, 0, 1020);
      min_speed = (-1) * max_speed;
      preferences.putInt("max_speed", max_percent_speed);
    }

    request->redirect("/");
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam(PARAM_INPUT_STATE)) {
      String stateMessage = request->getParam(PARAM_INPUT_STATE)->value();
      (stateMessage == "Apagar") ? state = 0 : state = 1;
      preferences.putBool("state", state);
    }

    request->redirect("/");
  });
  server.begin();
  Serial.println("Webserver inting done");

  

}

float readAndFilter(Adafruit_VL53L0X& sensor, int index) {
  VL53L0X_RangingMeasurementData_t measure;
  sensor.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    float filteredMeasure = updateKalman(&filters[index], (float)measure.RangeMilliMeter);
    return filteredMeasure;
  } else {
    return 10000;
  }
}

// setup kalman function
void setupKalman(KalmanFilter* f, float q, float r, float p, float val) {
  f->q = q;
  f->r = r;
  f->p = p;
  f->x = val;
}

// update kalman function
float updateKalman(KalmanFilter* f, float measurement) {
  f->p = f->p + f->q;
  f->k = f->p / (f->p + f->r);
  f->x = f->x + f->k * (measurement - f->x);
  f->p = (1 - f->k) * f->p;
  return f->x;
}

void readSensors() {
  // this holds the measurement
  // VL53L0X_RangingMeasurementData_t measureFrontSensor;
  // VL53L0X_RangingMeasurementData_t measureRightSensor;
  // VL53L0X_RangingMeasurementData_t measureLeftSensor;

  // frontSensor.rangingTest(&measureFrontSensor, false);  // pass in 'true' to get debug data printout!
  // rightSensor.rangingTest(&measureRightSensor, false);  // pass in 'true' to get debug data printout!
  // leftSensor.rangingTest(&measureLeftSensor, false);    // pass in 'true' to get debug data printout!

  // (measureFrontSensor.RangeStatus != 4) ? frontSensorValue = measureFrontSensor.RangeMilliMeter : frontSensorValue = 10000;
  // (measureRightSensor.RangeStatus != 4) ? rightSensorValue = measureRightSensor.RangeMilliMeter : rightSensorValue = 10000;
  // (measureLeftSensor.RangeStatus != 4) ? leftSensorValue = measureLeftSensor.RangeMilliMeter : leftSensorValue = 10000;

  frontSensorValue = readAndFilter(frontSensor, 0);
  rightSensorValue = readAndFilter(rightSensor, 1);
  leftSensorValue = readAndFilter(leftSensor, 2);
}



void pid() {
  float error = leftSensorValue - SETPOINT;

  error6 = error5;
  error5 = error4;
  error4 = error3;
  error3 = error2;
  error2 = error1;
  error1 = error;
  integral = ki * (error6 + error5 + error4 + error3 + error2 + error1 + error);

  integral = constrain(integral, min_speed, max_speed);

  float derivative = kd * (error - last_error);
  last_error = error;

  float diff = (kp * error) + integral + derivative;

  left_motor_speed = constrain(max_speed - diff, min_speed, max_speed);
  right_motor_speed = constrain(max_speed + diff, min_speed, max_speed);

  motors.driveMotors(left_motor_speed, right_motor_speed);
}
  
void resetPID() {
  last_error = 0.0;
  integral = 0.0;
  error1 = error2 = error3 = error4 = error5 = error6 = 0;
}



void girarIzquierda() {
  unsigned long inicio = millis();
  while (millis()- inicio < 250){
    motors.driveMotors(max_speed * 0.73, max_speed * 1.0);
  }
  
  //delay(900);  // ajusta este valor hasta que gire exactamente 90°
  motors.driveMotors(0, 0);  
}
void girarDerecha() {
  unsigned long inicio = millis();
  while (millis()- inicio < 250){
    motors.driveMotors(max_speed * 1.0, max_speed * 0.73);
  }
  
  //delay(900);  // ajusta este valor hasta que gire exactamente 90°
  motors.driveMotors(0, 0); 
  
}
void girar180(){
      Serial.println("Girando");
      motors.driveMotors(0,0); delay(1000);
      motors.driveMotors(-max_speed, max_speed);
      delay(900);
      motors.driveMotors(0, 0); delay(1000);
}
void girar90() {
      Serial.println("Girando");
      motors.driveMotors(0,0); delay(1000);
      motors.driveMotors(-max_speed, max_speed);
      delay(450);
      motors.driveMotors(0, 0); delay(1000);
}


void printData() {
  Serial.print("error: ");
  Serial.print(leftSensorValue - SETPOINT);
  Serial.print(", Izquierda: ");
  Serial.print(leftSensorValue);
  Serial.print(", Derecha: ");
  Serial.print(rightSensorValue);
  Serial.print(", Frontal: ");
  Serial.print(frontSensorValue);

  // Serial.print(", error: ");
  // Serial.print(error);
  // Serial.print(", diff: ");
  // Serial.print(diff);
  Serial.print(", left speed: ");
  Serial.print(left_motor_speed);
  Serial.print(", right speed: ");
  Serial.print(right_motor_speed);
  // Serial.print(", min speed: ");
  // Serial.print(min_speed);
  // Serial.print(", max speed: ");
  // Serial.print(max_speed);
  // Serial.print(", state: ");
  // Serial.print(state);
  Serial.println("");
  
}
 int j = 0; 
void loop() {
  while (state) {
  
    readSensors();

    pid();
    
    //readSensors();
    //decidirAccion();
    
    //girarDerecha();

    //girarIzquierda(); 

    //printData();
    if(leftSensorValue > 250){
      girarIzquierda();
    }
    else if(rightSensorValue > 250){
      girarDerecha();
    }
    else if(frontSensorValue < 140){
      girar90();
    }
    else if(leftSensorValue < 200 && frontSensorValue < 140 && rightSensorValue < 200 ){
      girar180();
    }
    else{
      pid();
    }
    
  }

  while (!state) {
    motors.driveMotors(0, 0);
  }
}