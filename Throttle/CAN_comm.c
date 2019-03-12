#define PART_TM4C123GH6PM 1;

#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/can.h"
#include "Pedal_ADC.h"
#include "Globals_and_Defines.h"
	
void CAN_Setup(void)
{
	//CAN message object
	tCANMsgObject sMsgObjectRx;

	//Enable PORTE
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
	{}

	GPIOPinConfigure(GPIO_PE4_CAN0RX);
	GPIOPinConfigure(GPIO_PE5_CAN0TX);

	//Configure the pins for CAN
	GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	// Enable the CAN0 module.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0))
	{}

	CANInit(CAN0_BASE);

		
	// Configure the controller for 250 Kbit operation.
	CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 250000);
	
	
	//Enable interrupts for CAN0
	CANIntEnable(CAN0_BASE, CAN_INT_MASTER);
	IntEnable(INT_CAN0);

	//Set CAN0 to silent test mode
	//CAN0_CTL_R |= CAN_CTL_TEST;
	//CAN0_TST_R |= CAN_TST_SILENT;
	
	//Start CAN0 peripheral
	CANEnable(CAN0_BASE);
	

	// Configure a receive object.
	sMsgObjectRx.ui32MsgID = throttle_board_address;
	sMsgObjectRx.ui32MsgIDMask = 0x1FFFFFFF;
	sMsgObjectRx.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
	sMsgObjectRx.ui32MsgLen = 8;
	CANMessageSet(CAN0_BASE, 2, &sMsgObjectRx, MSG_OBJ_TYPE_RX);
}

void CAN0_Handler(void)
{
	uint32_t int_status, CAN_status;
	uint8_t data_array[8];
	tCANMsgObject sMsgObjectRx;
	sMsgObjectRx.pui8MsgData = data_array;
	
	//get interrupt status
	int_status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);  
	//get CAN status
	CAN_status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
	
	
	if (int_status == 0x2)   //interrupt was generated by receive object
	{
		CANMessageGet(CAN0_BASE, 2, &sMsgObjectRx, true);   //get received data
		g_new_CAN_data = true;
		if (data_array[0] == 0x04)
		{
			g_CAN_throttle_pos = 0x00000000;
			g_CAN_throttle_pos |= data_array[2];
			g_CAN_throttle_pos = g_CAN_throttle_pos<<8;
			g_CAN_throttle_pos = g_CAN_throttle_pos | data_array[1];
		}
	}
}
void Send_Throttle_Voltage(void)
{
	tCANMsgObject sMsgObjectTx;
	uint8_t BufferOut[3] = {0x00, 0x00, 0x00};
	uint32_t throttle_pos;
	throttle_pos = get_throttle_input();
	BufferOut[0] = 0x03;  							//SRC ID
	BufferOut[1] = throttle_pos >> 8;   //MSB BYTE
	BufferOut[1] &= 0x00FF;							
	BufferOut[2] = throttle_pos;				//LSB BYTE
	BufferOut[2] &= 0x00FF;
	
	//Configure transmit of message object.
	sMsgObjectTx.ui32MsgID = 0x1CDBFFFF;
	sMsgObjectTx.ui32Flags = 0;
	sMsgObjectTx.ui32MsgLen = 3;
	sMsgObjectTx.pui8MsgData = BufferOut;
	
	//Send out data on CAN
	CANMessageSet(CAN0_BASE, 3, &sMsgObjectTx, MSG_OBJ_TYPE_TX);
}
