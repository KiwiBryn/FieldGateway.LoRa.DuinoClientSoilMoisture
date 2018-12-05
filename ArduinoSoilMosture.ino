/*
  Copyright ® 2018 December devMobile Software, All Rights Reserved

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  You can do what you want with this code, acknowledgment would be nice.

  http://www.devmobile.co.nz

*/
#include <stdlib.h>
#include <LoRa.h>

//#define DEBUG
//#define DEBUG_TELEMETRY
//#define DEBUG_LORA

// LoRa field gateway configuration (these settings must match your field gateway)
const char DeviceAddress[] = {"ArduinoMoist1"};

// Azure IoT Hub FieldGateway
/*
const char FieldGatewayAddress[] = {"LoRaIoT1"};
const float FieldGatewayFrequency =  915000000.0;
const byte FieldGatewaySyncWord = 0x12 ;
*/
// AdaFruit.IO Field Gateway
const char FieldGatewayAddress[] = {"LoRaIoT2"};
const float FieldGatewayFrequency = 433000000.0;
const byte FieldGatewaySyncWord = 0x12 ;

// Dragino LoRa shield radio configuration
const int ChipSelectPin = 10;
const int InterruptPin = 2;
const int ResetPin = 9;

// LoRa radio payload configuration
const byte SensorIdValueSeperator = ' ' ;
const byte SensorReadingSeperator = ',' ;
const byte PayloadSizeMaximum = 64 ;
byte payload[PayloadSizeMaximum];
byte payloadLength = 0 ;

const int LoopDelaySeconds = 30 ;

// Sensor configuration
const int sensorPin = A0;
const int MoistureSensorValueMin = 0;
const int MoistureSensorValueMax = 1023;
const int MoistureValueMin = 0;
const int MoistureValueMax = 100;

void setup() 
{
  Serial.begin(9600);
#ifdef DEBUG
  while (!Serial);
#endif
  Serial.println("Setup called");

  Serial.println("LoRa setup start");
  
  // override the default chip select and reset pins
  LoRa.setPins(ChipSelectPin, ResetPin, InterruptPin); 
  if (!LoRa.begin(FieldGatewayFrequency))
  {
    Serial.println("LoRa begin failed");
    while (true); // Drop into endless loop requiring restart
  }

  // Need to do this so field gateways pays attention to messsages from this device
  LoRa.enableCrc();
  LoRa.setSyncWord(FieldGatewaySyncWord);

#ifdef DEBUG_LORA
  LoRa.dumpRegisters(Serial);
#endif
  Serial.println("LoRa Setup done.");

  PayloadHeader((byte*)FieldGatewayAddress,strlen(FieldGatewayAddress), (byte*)DeviceAddress, strlen(DeviceAddress));

  Serial.println("Setup done");
  Serial.println();
}

void loop() {
  // read the value from the sensor:
  int sensorValue = analogRead(sensorPin);
  Serial.print("Sensor = " );
  Serial.print(sensorValue);

  // Convert it to a percentage
  sensorValue = map(sensorValue, MoistureSensorValueMin, MoistureSensorValueMax, MoistureValueMin, MoistureValueMax);

  Serial.print(" Moisture = " );
  Serial.print(sensorValue);
  Serial.println("%");

  PayloadReset();  

  PayloadAdd("M", sensorValue) ;

  LoRa.beginPacket();
  LoRa.write(payload, payloadLength);
  LoRa.endPacket();

  Serial.println("Loop done");
  Serial.println();
  
  delay(LoopDelaySeconds * 1000);
}


void PayloadHeader( byte *to, byte toAddressLength, byte *from, byte fromAddressLength)
{
  byte addressesLength = toAddressLength + fromAddressLength ;

#ifdef DEBUG_TELEMETRY
  Serial.println("PayloadHeader- ");
  Serial.print( "To Address len:");
  Serial.print( toAddressLength );
  Serial.print( " From Address len:");
  Serial.print( fromAddressLength );
  Serial.print( " Addresses length:");
  Serial.print( addressesLength );
  Serial.println( );
#endif

  payloadLength = 0 ;

  // prepare the payload header with "To" Address length (top nibble) and "From" address length (bottom nibble)
  payload[payloadLength] = (toAddressLength << 4) | fromAddressLength ;
  payloadLength += 1;

  // Copy the "To" address into payload
  memcpy(&payload[payloadLength], to, toAddressLength);
  payloadLength += toAddressLength ;

  // Copy the "From" into payload
  memcpy(&payload[payloadLength], from, fromAddressLength);
  payloadLength += fromAddressLength ;
}


void PayloadAdd( const char *sensorId, float value, byte decimalPlaces)
{
  byte sensorIdLength = strlen( sensorId ) ;

#ifdef DEBUG_TELEMETRY
  Serial.println("PayloadAdd-float ");
  Serial.print( "SensorId:");
  Serial.print( sensorId );
  Serial.print( " sensorIdLen:");
  Serial.print( sensorIdLength );
  Serial.print( " Value:");
  Serial.print( value, decimalPlaces );
  Serial.print( " payloadLength:");
  Serial.print( payloadLength);
#endif

  memcpy( &payload[payloadLength], sensorId,  sensorIdLength) ;
  payloadLength += sensorIdLength ;
  payload[ payloadLength] = SensorIdValueSeperator;
  payloadLength += 1 ;
  payloadLength += strlen( dtostrf(value, -1, decimalPlaces, (char *)&payload[payloadLength]));
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += 1 ;
  
#ifdef DEBUG_TELEMETRY
  Serial.print( " payloadLength:");
  Serial.print( payloadLength);
  Serial.println( );
#endif
}


void PayloadAdd( const char *sensorId, int value )
{
  byte sensorIdLength = strlen( sensorId ) ;

#ifdef DEBUG_TELEMETRY
  Serial.println("PayloadAdd-int ");
  Serial.print( "SensorId:");
  Serial.print( sensorId );
  Serial.print( " sensorIdLen:");
  Serial.print( sensorIdLength );
  Serial.print( " Value:");
  Serial.print( value );
  Serial.print( " payloadLength:");
  Serial.print( payloadLength);
#endif  

  memcpy( &payload[payloadLength], sensorId,  sensorIdLength) ;
  payloadLength += sensorIdLength ;
  payload[ payloadLength] = SensorIdValueSeperator;
  payloadLength += 1 ;
  payloadLength += strlen( itoa( value,(char *)&payload[payloadLength],10));
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += 1 ;
  
#ifdef DEBUG_TELEMETRY
  Serial.print( " payloadLength:");
  Serial.print( payloadLength);
  Serial.println( );
#endif
}

void PayloadAdd( const char *sensorId, unsigned int value )
{
  byte sensorIdLength = strlen( sensorId ) ;

#ifdef DEBUG_TELEMETRY
  Serial.println("PayloadAdd-unsigned int ");
  Serial.print( "SensorId:");
  Serial.print( sensorId );
  Serial.print( " sensorIdLen:");
  Serial.print( sensorIdLength );
  Serial.print( " Value:");
  Serial.print( value );
  Serial.print( " payloadLength:");
  Serial.print( payloadLength);
#endif  

  memcpy( &payload[payloadLength], sensorId,  sensorIdLength) ;
  payloadLength += sensorIdLength ;
  payload[ payloadLength] = SensorIdValueSeperator;
  payloadLength += 1 ;
  payloadLength += strlen( utoa( value,(char *)&payload[payloadLength],10));
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += 1 ;

#ifdef DEBUG_TELEMETRY
  Serial.print( " payloadLength:");
  Serial.print( payloadLength);
  Serial.println( );
#endif
}


void PayloadReset()
{
  byte fromAddressLength = payload[0] & 0xf ;
  byte toAddressLength = payload[0] >> 4 ;
  byte addressesLength = toAddressLength + fromAddressLength ;

  payloadLength = addressesLength + 1;

#ifdef DEBUG_TELEMETRY
  Serial.println("PayloadReset- ");
  Serial.print( "To Address len:");
  Serial.print( toAddressLength );
  Serial.print( " From Address len:");
  Serial.print( fromAddressLength );
  Serial.print( " Addresses length:");
  Serial.print( addressesLength );
  Serial.println( );
#endif
}
