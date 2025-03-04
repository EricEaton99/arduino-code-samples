// pinout on pi
// for lidar: SDA: 4, SCL: 5, GND: GND, 3.3V: 3.3V
// for servo: PWM: 9, GND: GND, 5V: VBUS

#include <Wire.h>
#include <VL53L1X.h>
#include <Servo.h>

Servo servo;
VL53L1X sensor;

void resetSensor()
{
    Serial.println("Resetting VL53L1X sensor...");
    sensor.stopContinuous(); // Stop continuous mode
    delay(10);
    sensor.init(); // Reinitialize sensor
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(2);
    Serial.println("Sensor reset complete.");
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    while (!Serial)
    {
    }
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000); // use 400 kHz I2C

    sensor.setTimeout(500);
    if (!sensor.init())
    {
        Serial.println("Failed to detect and initialize sensor!");
        while (1)
            ;
    }
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);

    sensor.startContinuous(2);

    servo.attach(9);
}

const int threashhold = 20;
int previouseDistance = 0;

void lidar()
{
    int distance = sensor.read();
    int threashholdProportional = distance / 100;
    Serial.print(distance);
    int absDifference = (distance - previouseDistance < 0) ? -(distance - previouseDistance) : (distance - previouseDistance);
    if (threashhold + threashholdProportional < absDifference)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        int distance = sensor.read();
    }
    previouseDistance = distance;

    if (sensor.timeoutOccurred())
    {
        Serial.println(" TIMEOUT detected! Attempting to reset...");
        resetSensor();
        return; // Skip this iteration
    }

    Serial.println();
}

int servoPos = 0;
bool servoDir = true;

void servoSweep()
{
    if (servoDir)
    {
        servoPos++;
        if (servoPos >= 180)
            servoDir = !servoDir;
    }
    else
    {
        servoPos--;
        if (servoPos <= 0)
            servoDir = !servoDir;
    }
    servo.write(servoPos);
}

void loop()
{
    // Note that lidar() is blocking, so the time between servoSweep() calls is the time for lidar() to be called.
    // If you care about good coding practice, consider changing this. It's functional, but very bad practice
    lidar();
    servoSweep();
}
