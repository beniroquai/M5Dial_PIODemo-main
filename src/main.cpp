#include <Wire.h>
#include <M5Dial.h>
//#include <USBSerial.h> 

#define I2C_SLAVE_ADDR 0x60

typedef struct __attribute__((packed))
{
    int32_t pos_x = 0;
    int32_t pos_y = 0;
    int32_t pos_z = 0;
    int32_t pos_a = 0;
} DialData;



// Array to store X, Y, Z, A positions
long positions[4] = {0, 0, 0, 0};
char axisNames[4] = {'X', 'Y', 'Z', 'A'};
int  currentAxis  = 0; // 0=X, 1=Y, 2=Z, 3=A
int  stepSize     = 1; // Step size starts at 1
long encoderPos   = -999;

// Available touch states
const int TOUCH_BEGIN      = 3; // Represents touch start
const int TOUCH_HOLD_BEGIN = 7; // Represents touch hold start

void requestEvent();

unsigned long       touchStartTime      = 0;
bool                isLongPress         = false;
const unsigned long LONG_PRESS_DURATION = 150; // 1 second for long press

void setup()
{
    USBSerial.begin(115200);
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.drawString("X=" + String(positions[currentAxis]), M5Dial.Display.width() / 2,
                              M5Dial.Display.height() / 2);
    Wire.begin(I2C_SLAVE_ADDR, 13, 15, 400000); // Start I2C communication as slave
    Wire.onRequest(requestEvent); // Register event to send data when requested by the master

}

void requestEvent()
{
    // Send the positions array over I2C when requested
    DialData data;
    data.pos_x = positions[0];
    data.pos_y = positions[1];
    data.pos_z = positions[2];
    data.pos_a = positions[3];

    Wire.write((uint8_t*)&data, sizeof(DialData));
    USBSerial.printf("Sent positions: %d, %d, %d, %d\n", positions[0], positions[1], positions[2], positions[3]);
}

void updateDisplay()
{
    M5Dial.Display.clear();
    M5Dial.Display.drawString(String(axisNames[currentAxis]) + "=" + String(positions[currentAxis]),
                              M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
    M5Dial.Display.drawString("Step: " + String(stepSize), M5Dial.Display.width() / 2,
                              M5Dial.Display.height() / 2 + 30);
}

void loop()
{
    M5Dial.update();

    long newEncoderPos = M5Dial.Encoder.read();
    if (newEncoderPos != encoderPos)
    {
        positions[currentAxis] += (newEncoderPos - encoderPos) * stepSize;
        encoderPos = newEncoderPos;
        updateDisplay();
    }

    auto t = M5Dial.Touch.getDetail();

    // Handle touch begin
    if (t.state == 3)
    { // TOUCH_BEGIN
        touchStartTime = millis();
    }

    // Handle touch end
    if (t.state == 2 or t.state == 7)
    { // TOUCH_END
        long touchDuration = millis() - touchStartTime;
        USBSerial.printf("Touch duration: %d\n", touchDuration);
        if (touchDuration < LONG_PRESS_DURATION)
        {
            // Short press: switch axis
            currentAxis = (currentAxis + 1) % 4;
            updateDisplay();
        }
        else if (touchDuration >= 100)
        {
            // Long press: change step size
            if (stepSize == 1)
            {
                stepSize = 10;
            }
            else if (stepSize == 10)
            {
                stepSize = 100;
            }
            else if (stepSize == 100)
            {
                stepSize = 1000;
            }
            else
            {
                stepSize = 1;
            }
            updateDisplay();
        }
    }
}