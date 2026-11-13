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
#define INDICATOR_PIN 4   // LED or buzzer (harmless indicator output)

// --------------------------------------------------
// SYSTEM SETTINGS
// --------------------------------------------------

#define TARGET_DISTANCE 50

#define ROTARY_INITIAL 90
#define ARM_INITIAL 0
#define RADAR_INITIAL 0

#define ARM_TARGET_POS 60      // arm angle when pointing at the object
#define INDICATOR_ON_TIME 1000 // ms the indicator stays on

#define RADAR_MIN 0
#define RADAR_MAX 180
#define RADAR_STEP 3

#define SERVO_STEP_DELAY 15    // ms per degree for smooth movement

// --------------------------------------------------
// SCANNING VARIABLES
// --------------------------------------------------

int angle = RADAR_MIN;
int direction = 1;
int detectedAngle = 0;

int rotaryPos = ROTARY_INITIAL;
int armPos = ARM_INITIAL;

// --------------------------------------------------
// SYSTEM STATES
// --------------------------------------------------

enum SystemState
{
  SCANNING,
  AIM_ROTARY,
  MOVE_ARM,
  INDICATE,
  RETURN_ARM,
  RETURN_ROTARY
};

SystemState currentState = SCANNING;

// --------------------------------------------------
// SMOOTH SERVO MOVE
// --------------------------------------------------

void moveServoSmooth(Servo &servo, int &currentPos, int targetPos)
{
  while (currentPos != targetPos)
  {
    currentPos += (targetPos > currentPos) ? 1 : -1;
    servo.write(currentPos);
    delay(SERVO_STEP_DELAY);
  }
}

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

  rotaryServo.attach(9);
  armServo.attach(10);
  radarServo.attach(11);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(INDICATOR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(INDICATOR_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);

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
    // STATE 1: radar sweep until something is in range
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
        Serial.println("OBJECT DETECTED");
        currentState = AIM_ROTARY;
        break;
      }

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

    // STATE 2: rotary servo turns to the detected direction
    case AIM_ROTARY:
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print("Rotary moving to: ");
      Serial.println(detectedAngle);

      moveServoSmooth(rotaryServo, rotaryPos, detectedAngle);
      delay(300);

      currentState = MOVE_ARM;
      break;
    }

    // STATE 3: arm (up/down) servo moves to the target angle
    case MOVE_ARM:
    {
      Serial.println("Arm moving to target position");

      moveServoSmooth(armServo, armPos, ARM_TARGET_POS);
      delay(300);

      currentState = INDICATE;
      break;
    }

    // STATE 4: harmless indicator (LED / buzzer) turns on
    case INDICATE:
    {
      Serial.println("Indicator ON");

      digitalWrite(INDICATOR_PIN, HIGH);
      delay(INDICATOR_ON_TIME);
      digitalWrite(INDICATOR_PIN, LOW);

      Serial.println("Indicator OFF");

      currentState = RETURN_ARM;
      break;
    }

    // STATE 5: arm returns to its initial position
    case RETURN_ARM:
    {
      Serial.println("Arm returning");

      moveServoSmooth(armServo, armPos, ARM_INITIAL);
      delay(300);

      currentState = RETURN_ROTARY;
      break;
    }

    // STATE 6: rotary returns to centre, radar resumes
    case RETURN_ROTARY:
    {
      moveServoSmooth(rotaryServo, rotaryPos, ROTARY_INITIAL);
      digitalWrite(LED_BUILTIN, LOW);

      delay(300);

      Serial.println("Restarting radar scan");
      currentState = SCANNING;
      break;
    }
  }
}
