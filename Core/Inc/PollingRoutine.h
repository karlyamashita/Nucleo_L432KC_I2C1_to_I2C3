/*
 * PollingRoutine.h
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template
 */

#ifndef INC_POLLINGROUTINE_H_
#define INC_POLLINGROUTINE_H_


/*

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#define Nop() asm(" NOP ")

*/
#ifndef __weak
#define __weak __attribute__((weak))
#endif


#define SLAVE_ADDRESS (0x7E << 1)

void PollingInit(void);
void PollingRoutine(void);

void UART_Parse(UART_DMA_QueueStruct *msg);
int UART_ParseSet(char *msg);
int UART_ParseGet(char *msg, char *retStr);

void I2C_SlaveReceive(I2C_HandleTypeDef *i2c);
void I2C_CheckData(void);
int I2C_ParseCommand(char *msg);

void PrintError(char *msgCpy, uint32_t error);
void PrintReply(char *msgCpy, char *msg2);

#endif /* INC_POLLINGROUTINE_H_ */
