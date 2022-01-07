int count = 2; //Start at 2- 1 doesn't count
int printDelay = 1000; //Minimum prime number delay

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
}

void loop() {
  unsigned long start = millis();
  if(isPrime(count)) {
    Serial.println(count);
    
    if(millis() - start < printDelay) {
      delay(abs(printDelay - (millis() - start)));
    }
  }
  count++;
}

boolean isPrime(int x) {
  boolean prime = true;
  
  for(int i = 2; i <= x/2; i++) { //Loop every number up to half
    if(x % i == 0) { //If it's divisible...
      prime = false; //It isn't prime!
      break;
    }
  }
  
  return prime;
}
