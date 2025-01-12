#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <paw3205.h>
#include <BleMouse.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define MOUSE_LEFTPIN  (14)
#define MOUSE_MIDPIN  (12)
#define MOUSE_RIGHTPIN  (13)
#define MOUSE_FORWARDPIN  (4)
#define MOUSE_BACKWARDPIN  (35)
#define MOUSE_RESPIN  (34)
#define ADC_VBAT  (33)
 

static BleMouse bleMouse;
static uint8_t setResolution = (uint8_t)PAW3205_1000DPI;
static uint32_t lastAction = 0;

volatile static int32_t scroll = 0;
static uint16_t adcArray[100];
static uint8_t adcIndex = 0;
static uint16_t vbat = 0;

volatile uint8_t prevB;

static void IRAM_ATTR Scroll1_ISR(void);
static void IRAM_ATTR Scroll2_ISR(void);

void setup() 
{
  
  Serial.begin(115200); // Debug
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // used for AA batteries
  analogSetAttenuation(ADC_2_5db);
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  //Serial.print("Mouse chip ID2: "); // Debug
  switch(wakeup_reason)
  {
      case ESP_SLEEP_WAKEUP_EXT0:
      case ESP_SLEEP_WAKEUP_EXT1:
      // IO wakeup
          //Serial.println("Wake up"); // Debug
          rtc_gpio_deinit(GPIO_NUM_27);
          rtc_gpio_deinit(GPIO_NUM_14);
      break;
      default:
          //Serial.println("Power ON"); // Debug
      // POR
      break;
  }
  Paw3205Drv_Init();
  //Serial.println(Paw3205Drv_GetProductID2());
  //Paw3205Drv_SetResolution(PAW3205_1000DPI);
  bleMouse.begin();
  pinMode(MOUSE_LEFTPIN,INPUT_PULLUP);
  pinMode(MOUSE_RIGHTPIN,INPUT_PULLUP);
  pinMode(MOUSE_MIDPIN,INPUT_PULLUP);
  pinMode(MOUSE_FORWARDPIN,INPUT_PULLUP);
  pinMode(MOUSE_BACKWARDPIN,INPUT_PULLUP);
  pinMode(MOUSE_RESPIN,INPUT_PULLUP);
  pinMode(15,INPUT_PULLUP);
  pinMode(2,INPUT_PULLUP);
  //pinScroll1 = digitalRead(2);
  prevB = digitalRead(15);
  
  attachInterrupt(2, Scroll1_ISR, CHANGE);
  attachInterrupt(15, Scroll2_ISR, CHANGE);
  
  if((ESP_SLEEP_WAKEUP_EXT0 != wakeup_reason) && (ESP_SLEEP_WAKEUP_EXT1 != wakeup_reason))
  {
      Paw3205Drv_SetResolution(PAW3205_1000DPI);
  }
  for(uint8_t tmp = 0; tmp < (sizeof(adcArray)/sizeof(adcArray[0])); tmp++)
  {
    adcArray[tmp] = analogRead(ADC_VBAT);
  }

}

void loop() 
{
    static uint8_t lastPinf = 1;
    static uint8_t lastPinb = 1;
    static uint8_t lastPinres = 1;
    static uint8_t lastPinl = 1;
    static uint8_t lastPinr = 1;
    static uint8_t lastPinm = 1;
    //static uint8_t lastPinScroll1 = 0;
    //static uint8_t lastPinScroll2 = 0; // Debug
    uint8_t pinf = 1;
    uint8_t pinb = 1;
    uint8_t pinres = 1;
    uint8_t pinr = 1;
    uint8_t pinl = 1;
    uint8_t pinm = 1;
    //uint8_t pinScroll1 = 0;
    //uint8_t pinScroll2 = 0; // Debug

    if(bleMouse.isConnected()) 
    {
        int32_t movX = Paw3205Drv_GetXDelta();
        int32_t movY = Paw3205Drv_GetYDelta();
        if ((0 != movX) || (0 != movY))
        {
            bleMouse.move(-1*movY,-1*movX);
        }
        pinf = digitalRead(MOUSE_FORWARDPIN);
        pinb = digitalRead(MOUSE_BACKWARDPIN);
        pinres = digitalRead(MOUSE_RESPIN);
        pinl = digitalRead(MOUSE_LEFTPIN);
        pinr = digitalRead(MOUSE_RIGHTPIN);
        pinm = digitalRead(MOUSE_MIDPIN);
        /*pinScroll1 = digitalRead(2);
        pinScroll2 = digitalRead(15);
        if ((pinScroll1 != lastPinScroll1) || (pinScroll2 != lastPinScroll2))
        {
            Serial.print(pinScroll1);
            Serial.println(pinScroll2);
            lastPinScroll1 = pinScroll1;
            lastPinScroll2 = pinScroll2;
        }*/ // Debug
        if (pinres != lastPinres)
        {
            if (0u != pinres)
            {
                setResolution++;
                if (setResolution > (uint8_t)PAW3205_1600DPI)
                {
                    setResolution = (uint8_t)PAW3205_400DPI;
                }
                Paw3205Drv_SetResolution((Paw3205_ResType)setResolution);
                lastAction = millis();
            }
            lastPinres = pinres;
        }
        if (pinr != lastPinr)
        {
            if (0u != pinr)
            {
                bleMouse.release(MOUSE_RIGHT);
                lastAction = millis();
            }
            else
            {
                bleMouse.press(MOUSE_RIGHT);
                lastAction = millis();
            }
            lastPinr = pinr;
        }
        if (pinl != lastPinl)
        {
            if (0u != pinl)
            {
                bleMouse.release(MOUSE_LEFT);
                lastAction = millis();
            }
            else
            {
                bleMouse.press(MOUSE_LEFT);
                lastAction = millis();
            }
            lastPinl = pinl;
        }
        if (pinm != lastPinm)
        {
            if (0u != pinm)
            {
                bleMouse.release(MOUSE_MIDDLE);
                lastAction = millis();
            }
            else
            {
                bleMouse.press(MOUSE_MIDDLE);
                lastAction = millis();
            }
            lastPinm = pinm;
        }
        if (0 != scroll)
        {
            bleMouse.move(0,0,scroll);
            portDISABLE_INTERRUPTS();
            scroll = 0;
            portENABLE_INTERRUPTS();
            lastAction = millis();
        }
        bleMouse.setBatteryLevel(map(vbat,3500u,4000u,0u,100u));
        /*
        if ((0 != pin2detected) && (0 != pin15detected))
        {
            //portDISABLE_INTERRUPTS();
            if (pin2detected < pin15detected)
            {
                // Scroll down
                if(bleMouse.isConnected()) 
                {
                    bleMouse.move(0,0,-1);
                    lastAction = millis();
                }
            }
            else if (pin2detected != pin15detected)
            {
                // Scroll up
                if(bleMouse.isConnected()) 
                {
                    bleMouse.move(0,0,1);
                    lastAction = millis();
                }
            }
            else
            {
                // ERROR, skip
            }
            pin2detected = 0;
            pin15detected = 0;
            if (1 == falling)
            {
                //detachInterrupt(2);
                //detachInterrupt(15);
                attachInterrupt(2, Scroll1_ISR, RISING);
                attachInterrupt(15, Scroll2_ISR, RISING);
                falling = 0;
            }
            else
            {
                //detachInterrupt(2);
                //detachInterrupt(15);
                attachInterrupt(2, Scroll1_ISR, FALLING);
                attachInterrupt(15, Scroll2_ISR, FALLING);
                falling = 1;
            }
            //portENABLE_INTERRUPTS();
        }
        */
    }
    if ((lastAction + 120000UL) < millis())
    {
        //Serial.println("Sleep");
        bleMouse.end();
        delay(100);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON); // Enable pullups
        rtc_gpio_pullup_en(GPIO_NUM_27);
        rtc_gpio_pullup_en(GPIO_NUM_14);
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0);
        esp_sleep_enable_ext1_wakeup(0x8000000UL,ESP_EXT1_WAKEUP_ALL_LOW); // PIN 27 is a wakeup source
        esp_deep_sleep_start();
    }
    Paw3205Drv_Main();
    adcArray[adcIndex++] = analogRead(ADC_VBAT);
    if (adcIndex >= (sizeof(adcArray)/sizeof(adcArray[0]))) adcIndex = 0;
    uint32_t adcSum = 0;
    for(uint8_t tmp = 0; tmp < (sizeof(adcArray)/sizeof(adcArray[0])); tmp++)
    {
      adcSum += adcArray[tmp];
    }
    vbat = map((adcSum / (sizeof(adcArray)/sizeof(adcArray[0]))),0u,4095u,0u,7700u);
    Serial.println(vbat);
    delay(10);
}

static void IRAM_ATTR Scroll1_ISR(void)
{
    uint8_t pin2, pin15;
    if(bleMouse.isConnected()) 
    {
        pin2 = digitalRead(2);
        pin15 = digitalRead(15);
        if (0 == pin2^prevB)
        {
            scroll++;
        }
        else
        {
            scroll--;
        }
        prevB = pin15;
    }
}
static void IRAM_ATTR Scroll2_ISR(void)
{
    uint8_t pin2, pin15;
    if(bleMouse.isConnected()) 
    {
        pin2 = digitalRead(2);
        pin15 = digitalRead(15);
        if (0 == pin2^prevB)
        {
            scroll++;
        }
        else
        {
            scroll--;
        }
        prevB = pin15;
    }
}
