#ifndef __PWR_BUTTON_H__
#define __PWR_BUTTON_H__

#define BOARD_SYS_EN_PIN   GPIO_NUM_41
#define BOARD_SYS_OUT_PIN  GPIO_NUM_40

void board_power_latch(void);
void board_power_off(void);

#endif // __PWR_BUTTON_H__