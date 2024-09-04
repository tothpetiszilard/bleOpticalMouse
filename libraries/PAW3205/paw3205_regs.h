#ifndef PAW3205_MOUSE_DRVREGS_H
#define PAW3205_MOUSE_DRVREGS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PAW3205_REGADDR_PRODUCTID1     (0x00u)
#define PAW3205_REGADDR_PRODUCTID2     (0x01u)
#define PAW3205_REGADDR_MOTIONSTAT     (0x02u) // Reading this register freezes the Delta_X and Delta_Y register values.
// Read this register before reading the Delta_X and Delta_Y registers.
// If Delta_X and Delta_Y are not read before the motion register is read a second time, the data in Delta_X and Delta_Y will be lost.
#define PAW3205_REGADDR_DELTAX         (0x03u) // signed X movement is counted since last report. Absolute value is determined by resolution. Reading clears
#define PAW3205_REGADDR_DELTAY         (0x04u) // signed XYmovement is counted since last report. Absolute value is determined by resolution. Reading clears
#define PAW3205_REGADDR_OPMODE         (0x05u)
#define PAW3205_REGADDR_CONFIG         (0x06u)
#define PAW3205_REGADDR_IMAGEQUAL      (0x07u)
#define PAW3205_REGADDR_OPSTATE        (0x08u)
#define PAW3205_REGADDR_WRITEPROT      (0x09u)
#define PAW3205_REGADDR_SLEEP1SET      (0x0Au) // write protected
#define PAW3205_REGADDR_ENTERTIME      (0x0Bu) // write protected
#define PAW3205_REGADDR_SLEEP2SET      (0x0Cu) // write protected
#define PAW3205_REGADDR_IMGTHRESH      (0x0Du) // write protected
#define PAW3205_REGADDR_IMGRECOGN      (0x0Eu) // write protected

#define PAW320X_PROD_ID1               (0x30u)
#define PAW3205_PROD_ID2               (0xD0u)
#define PAW3204_PROD_ID2               (0x50u)
#define PAW320X_PROD_ID2_MASK          (0xF0u)

#define PAW3205_REG_MOTIONSTAT_MOTION  (0x80u) // Motion occurred, data ready for reading in Delta_X and Delta_Y registers
#define PAW3205_REG_MOTIONSTAT_DYOVF   (0x10u) // Motion Delta Y overflow, ΔY buffer has overflowed since last report
#define PAW3205_REG_MOTIONSTAT_DXOVF   (0x08u) // Motion Delta X overflow, ΔX buffer has overflowed since last report
#define PAW3205_REG_MOTIONSTAT_RES400  (0x00u) // 400DPI
#define PAW3205_REG_MOTIONSTAT_RES500  (0x01u) // 500DPI
#define PAW3205_REG_MOTIONSTAT_RES600  (0x02u) // 600DPI
#define PAW3205_REG_MOTIONSTAT_RES800  (0x03u) // 800DPI
#define PAW3205_REG_MOTIONSTAT_RES1000 (0x04u) // 1000DPI
#define PAW3205_REG_MOTIONSTAT_RES1200 (0x05u) // 1200DPI
#define PAW3205_REG_MOTIONSTAT_RES1600 (0x06u) // 1600DPI
#define PAW3205_REG_MOTIONSTAT_RESMASK (0x07u) // MASK

#define PAW3205_REG_OPMODE_WAKEUP      (0x01u) // Manual wake up from sleep mode, self clear
#define PAW3205_REG_OPMODE_SLP1MU      (0x02u) // Manual enter sleep1 mode, self clear
#define PAW3205_REG_OPMODE_SLP2MU      (0x04u) // Manual enter sleep2 mode, self clear
#define PAW3205_REG_OPMODE_SLP2EN      (0x08u) // Enable sleep2 mode
#define PAW3205_REG_OPMODE_SLP1EN      (0x10u) // Enable sleep1 mode
#define PAW3205_REG_OPMODE_RSVD        (0x40u) // Reserved, 6th and 5th must be 01
#define PAW3205_REG_OPMODE_LEDSHUTTER  (0x80u) // LED shutter enable/disable

#define PAW3205_REG_OPSTATE_NORMAL     (0x00u) // Normal state
#define PAW3205_REG_OPSTATE_SLP1PR     (0x01u) // Sleep1 entry is ongoing
#define PAW3205_REG_OPSTATE_SLP2PR     (0x02u) // Sleep2 entry is ongoing
#define PAW3205_REG_OPSTATE_SLP        (0x04u) // Sleep state1
#define PAW3205_REG_OPSTATE_SLPSTATE   (0x0Cu) // Sleep state2
#define PAW3205_REG_OPSTATE_MASK       (0x0Fu) // Other bits are reserved

#define PAW3205_REG_WRITEPROT_UNLOCK   (0x5Au) // Key to unlock the write protection

#ifdef	__cplusplus
}
#endif

#endif