
#include <Servo.h>

// Servo objects
Servo rotaryServo;
Servo armServo;
Servo radarServo;

// Pin definitions
#define TRIG_PIN 3
#define ECHO_PIN 2

// System parameters
#define TARGET_DISTANCE 50

// Initial positions
#define RADAR_INITIAL 0

// Radar scan range
#define RADAR_MIN 0
#define RADAR_MAX 180

// Scanning variables
int angle = RADAR_MIN;
int direction = 1;

// Read filtered distance
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

    readings[i] = duration == 0
                    ? 0
                    : duration * 0.034 / 2;

    delay(20);
  }

  float validSum = 0;
  int validCount = 0;

  for (int i = 0; i < 5; i++)
  {
    if (readings[i] > 0)
    {
      validSum += readings[i];
      validCount++;
    }
  }

  if (validCount == 0)
    return 0;

  return validSum / validCount;
}

void setup()
{
  radarServo.attach(11);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  radarServo.write(RADAR_INITIAL);

  Serial.begin(9600);
  delay(1000);
}

void loop()
{
  // Move radar servo
  radarServo.write(angle);
  delay(60);

  // Measure distance
  float distance = readDistanceFiltered();

  // Display readings
  Serial.print("Angle: ");
  Serial.print(angle);

  Serial.print(" | Distance: ");
  Serial.println(distance);

  // Detect nearby object
  if (distance > 0 && distance <= TARGET_DISTANCE)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("OBJECT DETECTED");
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Update scanning direction
  angle += direction * 3;

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
}
