
void setupSensors ()
{
 
    sensors.begin();

    pinMode(FANS_PIN, OUTPUT); 
    pinMode(AQUA_PIN, OUTPUT);
    digitalWrite(FANS_PIN, HIGH);
    digitalWrite(AQUA_PIN, HIGH);

    // The first requests sensor for measurement
    sensors.request(sensorAddressFans);
    sensors.request(sensorAddressAqua); 
}



void fansControl()
{
   // If the sesor measurement is ready, print the results
    if (sensors.available()) {
        // Reads the temperature from sensor
        temperatureFans = sensors.readTemperature(sensorAddressFans);
        temperatureAqua = sensors.readTemperature(sensorAddressAqua);

        // Prints the temperature on Serial Monitor
        if (temperatureFans != TEMP_ERROR) {
            if (currentTimeSec - previousMillisFans > FANS_INTERVAL) {
                previousMillisFans = currentTimeSec;
                if (temperatureFans > MAX_BULB_TEMP)
                {
                    digitalWrite(FANS_PIN, LOW);
                    ledFansStatus = true;
  
                }
                else
                {
                    digitalWrite(FANS_PIN, HIGH);
                    waterFansStatus = false;
                }
            }

        }

        if (temperatureAqua != TEMP_ERROR) {
            if (currentTimeSec - previousMillisAqua > AQUA_INTERVAL) {
                previousMillisAqua = currentTimeSec;
                if (temperatureAqua > MAX_AQUA_TEMP)
                {
                    digitalWrite(AQUA_PIN, LOW);
                    waterFansStatus = true;
                }
                else
                {
                    digitalWrite(AQUA_PIN, HIGH);
                    waterFansStatus = false;
                }
            }
        }

        // Another requests sensor for measurement
        sensors.request(sensorAddressFans);
        sensors.request(sensorAddressAqua);
    }

}





















