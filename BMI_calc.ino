String rcvString = "";
int Weight = 0;
String rcvString1 = "";
int Height = 0;
void setup() {
   Serial.begin(115200); //set up serial library baud rate to 115200
   delay(1000);
   Serial.println("What is your weight?");
   Serial.println("What is your height?");
}

void loop() {
   if(Serial.available()) { 
      rcvString = Serial.readString();
      Weight = rcvString.toInt(); 
      rcvString1 = Serial.readString();
      Height = rcvString1.toInt(); 
      int BMI = Weight / (sq(Height));
      if (BMI < 16) {
        Serial.println("Severly Underweight");
      }
      else if (BMI >= 16 && BMI <= 18.4) {
        Serial.println("Underweight");
      }
      else if (BMI >= 18.5 && BMI <= 24.9) {
        Serial.println("Normal");
      }
      else if (BMI >= 25 && BMI <= 29.9) {
        Serial.println("Overweight");
      }
      else if (BMI >= 30 && BMI <= 34.9) {
        Serial.println("Moderately Obese");
      }
      else if (BMI >= 35.0 && BMI <= 39.9) {
        Serial.println("Severly Obese");
      }
      else {
        Serial.println("Morbidly Obese");
      }

      
   }
}
         
