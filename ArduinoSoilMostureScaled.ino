int sensorPin = A0;
int sensorValue = 0.0;

#define sensorMin 0
#define sensorMax 1023
#define valueMin 0
#define valueMax 100


void setup() {
    Serial.begin(9600);
}
void loop() {
    // read the value from the sensor:
    sensorValue = analogRead(sensorPin);
    Serial.print("Sensor = " );
    Serial.print(sensorValue);

    // Convert it to a percentage
    sensorValue = map(sensorValue, sensorMin, sensorMax, valueMin, valueMax);

    Serial.print(" Moisture = " );
    Serial.print(sensorValue);
    Serial.println("%");
    delay(1000);
}
