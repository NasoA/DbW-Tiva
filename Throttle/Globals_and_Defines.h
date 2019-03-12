#ifndef _GLOBALSANDDEFINES_H_
#define _GLOBALSANDDEFINES_H_

#include <stdint.h>
#include <stdbool.h>

//PORTF LED pins
#define PF2  (*((volatile uint32_t *)0x40025010))
#define PF3  (*((volatile uint32_t *)0x40025020))
#define PF4  (*((volatile uint32_t *)0x40025040))
		
//Drive by wire update rate in Hz
#define Update_Rate 10

//Throttle board CAN addfress
#define throttle_board_address 0x012EEEEE

extern bool g_tick_flag;

extern bool g_new_CAN_data;

extern bool DbW_Activated;

//Global CAN buffer
extern uint32_t g_CAN_throttle_pos;


#endif /* _GLOBALSANDDEFINES_H_ */

