
#include <Servo.h>

// --------------------------------------------------
// SERVO OBJECTS
// --------------------------------------------------

Servo rotaryServo;
Servo armServo;
Servo radarServo;

// --------------------------------------------------
// PIN DEFINITIONS
// --------------------------------------------------

#define TRIG_PIN 3
#define ECHO_PIN 2
#define IGNITION_PIN 4

// --------------------------------------------------
// SYSTEM SETTINGS
// --------------------------------------------------

#define TARGET_DISTANCE 50

#define ROTARY_INITIAL 90
#define ARM_INITIAL 0
#define RADAR_INITIAL 0

#define ARM_DETECT_POS 60

#define RADAR_MIN 0
#define RADAR_MAX 180
#define RADAR_STEP 3

// --------------------------------------------------
// SCANNING VARIABLES
// --------------------------------------------------

int angle = RADAR_MIN;
int direction = 1;

int detectedAngle = 0;

bool objectDetected = false;

// --------------------------------------------------
// SYSTEM STATES
// --------------------------------------------------

enum SystemState
{
  SCANNING,
  DETECTION,
  RESET_SYSTEM
};

SystemState currentState = SCANNING;

// --------------------------------------------------
// ULTRASONIC DISTANCE FUNCTION
// --------------------------------------------------

float readDistanceFiltered()
{
  float readings[5];

  for (int i = 0; i < 5; i++)
  {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0)
      readings[i] = 0;
    else
      readings[i] = duration * 0.034 / 2;

    delay(20);
  }

  float sum = 0;
  int count = 0;

  for (int i = 0; i < 5; i++)
  {
    if (readings[i] > 0)
    {
      sum += readings[i];
      count++;
    }
  }

  if (count == 0)
    return 0;

  return sum / count;
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup()
{
  Serial.begin(9600);

  // Attach servos
  rotaryServo.attach(9);
  armServo.attach(10);
  radarServo.attach(11);

  // Configure pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Safe initial output
  digitalWrite(IGNITION_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  // Initial servo positions
  rotaryServo.write(ROTARY_INITIAL);
  armServo.write(ARM_INITIAL);
  radarServo.write(RADAR_INITIAL);

  delay(2000);

  Serial.println("Radar system started");
}

// --------------------------------------------------
// MAIN LOOP
// --------------------------------------------------

void loop()
{
  switch (currentState)
  {

    // ----------------------------------------------
    // STATE 1: CONTINUOUS RADAR SCANNING
    // ----------------------------------------------

    case SCANNING:
    {
      radarServo.write(angle);

      delay(60);

      float distance = readDistanceFiltered();

      Serial.print("Angle: ");
      Serial.print(angle);

      Serial.print(" | Distance: ");
      Serial.println(distance);

      if (distance > 0 && distance <= TARGET_DISTANCE)
      {
        detectedAngle = angle;
        objectDetected = true;

        Serial.println("OBJECT DETECTED");

        currentState = DETECTION;
        break;
      }

      // Move forward or backward
      angle += direction * RADAR_STEP;

      if (angle >= RADAR_MAX)
      {
        angle = RADAR_MAX;
        direction = -1;
      }

      if (angle <= RADAR_MIN)
      {
        angle = RADAR_MIN;
        direction = 1;
      }

      break;
    }

    // ----------------------------------------------
    // STATE 2: HARMLESS DETECTION INDICATION
    // ----------------------------------------------

    case DETECTION:
    {
      digitalWrite(LED_BUILTIN, HIGH);

      Serial.print("Detected at angle: ");
      Serial.println(detectedAngle);

      // Return to a neutral indicator position
      rotaryServo.write(ROTARY_INITIAL);

      // Raise indicator arm
      armServo.write(ARM_DETECT_POS);

      delay(1000);

      currentState = RESET_SYSTEM;

      break;
    }

    // ----------------------------------------------
    // STATE 3: RESET
    // ----------------------------------------------

    case RESET_SYSTEM:
    {
      armServo.write(ARM_INITIAL);

      rotaryServo.write(ROTARY_INITIAL);

      digitalWrite(LED_BUILTIN, LOW);

      objectDetected = false;

      delay(500);

      currentState = SCANNING;

      Serial.println("Restarting radar scan");

      break;
    }
  }
}
