
// i2c utility functions
bool write_packet(byte device, int address, int data) {
    Wire.beginTransmission(device);
    Wire.write(address);
    Wire.write(data);
    Wire.endTransmission();
    return true;
  }

bool read_packet(byte device, byte address, byte * buffer, byte length) {
    // send phase
    Wire.beginTransmission(device);
    Wire.write(address);
    Wire.endTransmission();
    // recieve phase
    Wire.beginTransmission(device);
    Wire.requestFrom(device, (uint8_t)length);
    // how many did we get?
    byte r = Wire.available();
    if(r <= length) {
      for(byte i = 0; i < r; i++) { 
        buffer[i] = Wire.read(); 
        // Serial.print("."); Serial.print(buffer[i]);
      }
    } else {
      // consume unexpected bytes
      for(byte i = 0; i < length; i++) { Wire.read(); } //  { Serial.print("!"); Serial.print(Wire.read()); }
    }
    Wire.endTransmission();
    // if(r < length) { Serial.print(" packet too short! "); Serial.print(r); }
    // if(r > length) { Serial.print(" packet too long! "); Serial.print(r); }
    return (r==length);
}

/*
 * MPU6040 Accellerometer + Gyro
 * 
 */
static const byte I2C_MPU6050 = 0x68; // i2c device address
// public properties
void MPU6050_start() {
  // reset
  write_packet(I2C_MPU6050, 0x6B, 0x80); //
  delay(50);
  // configuration
  write_packet(I2C_MPU6050, 0x19, 0x01); // Sample Rate
  write_packet(I2C_MPU6050, 0x6B, 0x03); // Z-Axis gyro reference used for improved stability (X or Y is fine too)
  delay(30);
  write_packet(I2C_MPU6050, 0x1B, 0x18); // Gyro Configuration FS_SEL = 3
  delay(30);
  write_packet(I2C_MPU6050, 0x1C, 0b11101000); // Accel Configuration AFS_SEL = 1
  delay(20);
}

void MPU6050_stop() {
}    

bool MPU6050_get_vector(byte reg, int * v) {
  byte buffer[6];
  if(read_packet(I2C_MPU6050, reg, buffer, 6)) {
    // [todo: bit shuffling might not be necessary - try just dumping directly into the vector]
    v[0] = (buffer[0] << 8)| buffer[1];
    v[1] = (buffer[2] << 8)| buffer[3];
    v[2] = (buffer[4] << 8)| buffer[5];
    return true;
  } else {
    return false;
  }
}

bool MPU6050_gyro_vector(int * v) {
  return MPU6050_get_vector(0x43, v);
}

bool MPU6050_accel_vector(int * v) {
  return MPU6050_get_vector(0x3B, v);
}

bool MPU6050_temp_vector(int * v) {
  byte buffer[2];
  if(read_packet(I2C_MPU6050, 0x41, buffer, 2)) {
    // [todo: bit shuffling might not be necessary - try just dumping directly into the vector]
    v[0] = (buffer[0] << 8)| buffer[1];
    return true;
  } else {
    return false;
  }
}

bool MPU6050_get_ident() {
  byte buffer[3];
  // [todo: might not be necessary - try just dumping directly into the vector]
  return read_packet(I2C_MPU6050, 0x75, buffer, 1) && (buffer[0]==0x68);
}


// vector math functions
void vec3_scale(float *v, float n) {
  for(int i=0; i<3; i++) v[i] *= n;
}

void vec3_addint(float *v, int * a) {
  for(int i=0; i<3; i++) v[i] += a[i];
}

float vec3_length(float *v) {
  float r = 0;
  for(int i=0; i<3; i++) r += (v[i]*v[i]);
  return sqrt(r);
}

void int3_add(int * v, int * a) {
  for(int i=0; i<3; i++) v[i] += a[i];  
}

void int3_sub(int * v, int * a) {
  for(int i=0; i<3; i++) v[i] -= a[i];  
}

void int3_print(int * v) {
  for(int i=0; i<3; i++) { Serial.print(v[i]); Serial.print(' '); }
  Serial.println();
}
void vec3_print(float * v) {
  for(int i=0; i<3; i++) { Serial.print(v[i]); Serial.print(' '); }
  Serial.println();
}


