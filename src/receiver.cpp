#include <Wire.h>

#define I2C_SLAVE_ADDR 0x60

     // I2c
     int8_t I2C_SCL = GPIO_NUM_22;      // This is the poart that connects to all other slaves
     int8_t I2C_SDA = GPIO_NUM_21;

typedef struct __attribute__((packed))
{
    int32_t pos_x;
    int32_t pos_y;
    int32_t pos_z;
    int32_t pos_a;
} DialData;

DialData dialData;

void setup()
{
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL); //, 100000);
}

void loop()
{
    // Request data from the slave
    Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(DialData));

    // Read the received data into dialData struct
    if (Wire.available() == sizeof(DialData))
    {
        Wire.readBytes((char*)&dialData, sizeof(DialData));

        // Print the received data
        Serial.printf("X=%d, Y=%d, Z=%d, A=%d\n", dialData.pos_x, dialData.pos_y, dialData.pos_z, dialData.pos_a);
    }

    delay(500); // Polling delay
}