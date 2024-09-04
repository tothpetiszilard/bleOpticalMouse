#include "paw3205.h"
#include "Arduino.h"
#include <paw3205_regs.h>

#ifndef PAW3205DRV_SCLK_PIN
    #define PAW3205DRV_SCLK_PIN 25
#endif

#ifndef PAW3205DRV_SDIO_PIN
    #define PAW3205DRV_SDIO_PIN 32
#endif

#ifndef PAW3205DRV_MOTSWK_PIN
    #define PAW3205DRV_MOTSWK_PIN 27
#endif

static uint8_t Paw3205Drv_ReadReg(uint8_t addr);
static void Paw3205Drv_WriteReg(uint8_t addr, uint8_t val);
void Paw3205Drv_MotionISR(void);

static uint8_t currentRes = PAW3205_REG_MOTIONSTAT_RES600;
static volatile uint8_t motionPending = 0;
static int32_t deltaX = 0;
static int32_t deltaY = 0;

void Paw3205Drv_Init(void)
{
    pinMode(PAW3205DRV_SCLK_PIN, OUTPUT); // The mouse sensor reads SDIO on rising edges of SCLK
    pinMode(PAW3205DRV_SDIO_PIN, INPUT); // The mouse controller changes SDIO on falling edges of SCLK
    pinMode(PAW3205DRV_MOTSWK_PIN, INPUT);
    // Clock is idle high
    digitalWrite(PAW3205DRV_SCLK_PIN,HIGH);
    delay(100); 
    digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
    delayMicroseconds(100);
    digitalWrite(PAW3205DRV_SCLK_PIN,HIGH);
    delay(400);
    attachInterrupt(PAW3205DRV_MOTSWK_PIN,Paw3205Drv_MotionISR,FALLING);
    if (LOW == digitalRead(PAW3205DRV_MOTSWK_PIN))
    {
        motionPending = 1;
    }
    currentRes = (Paw3205Drv_ReadReg(PAW3205_REGADDR_CONFIG) & PAW3205_REG_MOTIONSTAT_RESMASK);
}

void Paw3205Drv_SetResolution(Paw3205_ResType res)
{
    while (currentRes != res)
    {
        Paw3205Drv_WriteReg(PAW3205_REGADDR_CONFIG, (uint8_t)res);
        currentRes = Paw3205Drv_ReadReg(PAW3205_REGADDR_CONFIG);
    }
}

uint8_t Paw3205Drv_GetProductID1(void)
{
    return Paw3205Drv_ReadReg(PAW3205_REGADDR_PRODUCTID1);
}

uint8_t Paw3205Drv_GetProductID2(void)
{
    return Paw3205Drv_ReadReg(PAW3205_REGADDR_PRODUCTID2);
}

int32_t Paw3205Drv_GetXDelta(void)
{
    int32_t X = deltaX;
    deltaX = 0;
    return X;
}

int32_t Paw3205Drv_GetYDelta(void)
{
    int32_t Y = deltaY;
    deltaY = 0;
    return Y;
}

void Paw3205Drv_MotionISR(void)
{
    motionPending = 1;
}

void Paw3205Drv_Main(void)
{
    uint8_t motionStat = 0;
    uint8_t dX = 0;
    uint8_t dY = 0;
    if (0 != motionPending)
    {
        motionStat = Paw3205Drv_ReadReg(PAW3205_REGADDR_MOTIONSTAT);
        if ((motionStat & PAW3205_REG_MOTIONSTAT_MOTION) && (currentRes == (motionStat & PAW3205_REG_MOTIONSTAT_RESMASK)))
        {
            dY = Paw3205Drv_ReadReg(PAW3205_REGADDR_DELTAY);
            dX = Paw3205Drv_ReadReg(PAW3205_REGADDR_DELTAX);
            if (0 != (dX & 0x80u))
            {
                deltaX += (int32_t)-128 + (int32_t)((uint8_t)dX & (uint8_t)0x7Fu);
            }
            else
            {
                deltaX += dX;
            }
            
            if (0 != (dY & 0x80u))
            {
                deltaY += (int32_t)-128 + (int32_t)((uint8_t)dY & (uint8_t)0x7Fu);
            }
            else
            {
                deltaY += dY;
            }
            portDISABLE_INTERRUPTS();
            if (HIGH == digitalRead(PAW3205DRV_MOTSWK_PIN))
            {
                motionPending = 0;
            }
            portENABLE_INTERRUPTS();
        }
        else
        {
            delay(4);
            digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
            delayMicroseconds(100);
            digitalWrite(PAW3205DRV_SCLK_PIN,HIGH);
            delay(400);
        }
    }
}

static void Paw3205Drv_WriteReg(uint8_t addr, uint8_t val)
{
    uint8_t currBit = 0;
    uint8_t currMask = 0x80;
    pinMode(PAW3205DRV_SDIO_PIN, OUTPUT);
    digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
    for (uint8_t bitCnt = 0; bitCnt < 8; bitCnt++)
    {
        if ((0 != (addr & currMask)) || (0x80 == currMask ))
        {
            currBit = 1;
        }
        else 
        {
            currBit = 0;
        }
        digitalWrite(PAW3205DRV_SDIO_PIN,currBit);
        delayMicroseconds(10);
        digitalWrite(PAW3205DRV_SCLK_PIN,HIGH); // Here mouse reads in
        delayMicroseconds(10);
        currMask = currMask >> 1u;
        digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
    }
    currMask = 0x80;
    for (uint8_t bitCnt = 0; bitCnt < 8; bitCnt++)
    {
        if (0 != (val & currMask))
        {
            currBit = 1;
        }
        else 
        {
            currBit = 0;
        }
        digitalWrite(PAW3205DRV_SDIO_PIN,currBit);
        delayMicroseconds(10);
        digitalWrite(PAW3205DRV_SCLK_PIN,HIGH); // Here mouse reads in
        if (1 == currMask) // last bit was just sent
        {
            pinMode(PAW3205DRV_SDIO_PIN, INPUT); // Let the mouse control the data line
        }
        else
        {
            delayMicroseconds(10);
            currMask = currMask >> 1u;
            digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
        }
    }
}

static uint8_t Paw3205Drv_ReadReg(uint8_t addr)
{
    uint8_t currBit = 0;
    uint8_t currMask = 0x80;
    uint8_t currData = 0;
    pinMode(PAW3205DRV_SDIO_PIN, OUTPUT);
    digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
    for (uint8_t bitCnt = 0; bitCnt < 8; bitCnt++)
    {
        if ((0 == (addr & currMask)) || (0x80 == currMask ))
        {
            currBit = 0;
        }
        else 
        {
            currBit = 1;
        }
        digitalWrite(PAW3205DRV_SDIO_PIN,currBit);
        delayMicroseconds(2);
        digitalWrite(PAW3205DRV_SCLK_PIN,HIGH); // Here mouse reads in
        delayMicroseconds(2);
        if (1 == currMask) // last bit was just sent
        {
            digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
            pinMode(PAW3205DRV_SDIO_PIN, INPUT); // Let the mouse control the data line
        }
        else
        {
            currMask = currMask >> 1u;
            digitalWrite(PAW3205DRV_SCLK_PIN,LOW);
        }
    }
    //delayMicroseconds(5); // inter-byte space delay
    for (uint8_t bitCnt = 0; bitCnt < 8; bitCnt++)
    {
        currData = currData << 1;
        digitalWrite(PAW3205DRV_SCLK_PIN,HIGH); // Here we read
        delayMicroseconds(2);
        currData |= digitalRead(PAW3205DRV_SDIO_PIN);
        if (bitCnt != 7)
        {
            digitalWrite(PAW3205DRV_SCLK_PIN,LOW); // Here mouse writes
            delayMicroseconds(3);
        }
    }
    // SDIO is floating here, CLK is still LOW
    digitalWrite(PAW3205DRV_SCLK_PIN,HIGH); // Clock is idle HIGH
    return currData;
}
