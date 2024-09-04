#ifndef PAW3205_MOUSE_DRV_H
#define PAW3205_MOUSE_DRV_H
#include <cstdint>
#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
    PAW3205_400DPI = 0,
    PAW3205_500DPI = 1,
    PAW3205_600DPI = 2,
    PAW3205_800DPI = 3,
    PAW3205_1000DPI = 4,
    PAW3205_1200DPI = 5,
    PAW3205_1600DPI = 6
}Paw3205_ResType;

void Paw3205Drv_Init(void);
void Paw3205Drv_SetResolution(Paw3205_ResType res);
uint8_t Paw3205Drv_GetProductID1(void);
uint8_t Paw3205Drv_GetProductID2(void);
int32_t Paw3205Drv_GetXDelta(void);
int32_t Paw3205Drv_GetYDelta(void);
void Paw3205Drv_Main(void);

#ifdef	__cplusplus
}
#endif

#endif