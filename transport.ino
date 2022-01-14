
int setRPM(int motor, float RPM){
    attinySlaveArrayBoard[0] = motor == 0 ? 0x14 : 0x24;  // Command 0x14 or 0x24
    attinySlaveArrayBoard[1] = RPM<<16;  
    attinySlaveArrayBoard[2] = RPM>>16;  
     delay(10);
    Wire.beginTransmission(I2CADDR_B);
    Wire.write(attinySlaveArrayBoard, 3); // Sends 3 bytes i2c to Co-processor.
    if (Wire.endTransmission () == 0) { // Receive 0 = success (ACK response) 
        Serial.println("i2c Write to 0x12 Sucessfull");
        return 0;
    }
    else {
        Serial.println("i2c Write Failed");
        return 1;
    }
}

int rotateMotor(int motor,unsigned int rotations){
  attinySlaveArrayBoard[0] = motor == 0 ? 0x13 : 0x23;  // Command 0x13 or 0x23
  attinySlaveArrayBoard[1] = lowByte(rotations);  
  attinySlaveArrayBoard[2] = highByte(rotations);  
   delay(10);
  Wire.beginTransmission(I2CADDR_B);
  Wire.write(attinySlaveArrayBoard, 3); // Sends 3 bytes i2c to Co-processor.
  if (Wire.endTransmission () == 0) { // Receive 0 = success (ACK response) 
      Serial.println("i2c Write to 0x12 Sucessfull");
      return 0;
  }
  else {
      Serial.println("i2c Write Failed");
      return 1;
  }
}
