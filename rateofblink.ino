#define ONBOARD_LED 2

void setup() {
   Serial.begin(115200); 
   pinMode(ONBOARD_LED, OUTPUT);
   delay(1000);
   Serial.print("\nEnter a blinking rate:");
}

void loop() {
   if(Serial.available() > 0) { 
      String rate_read = Serial.readString();
      float rate = rate_read.toFloat(); 
      Serial.print("\nThe rate is ");
      Serial.println(rate);
      Serial.print("Press restart to enter a new rate.");
      while (1 == 1) {
        digitalWrite(ONBOARD_LED, HIGH);
        delay(rate);
        digitalWrite(ONBOARD_LED, LOW);
        delay(rate);
      }
   }
}
