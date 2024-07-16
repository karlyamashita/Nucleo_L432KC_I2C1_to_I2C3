/*
 * PollingRoutine.c
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template for projects.
 *
 *      The object of this PollingRoutine.c/h files is to not have to write code in main.c which already has a lot of generated code.
 *      It is cumbersome having to scroll through all the generated code for your own code and having to find a USER CODE section so your code is not erased when CubeMX re-generates code.
 *      
 *      Direction: Call PollingInit before the main while loop. Call PollingRoutine from within the main while loop
 * 
 *      Example;
        // USER CODE BEGIN WHILE
        PollingInit();
        while (1)
        {
            PollingRoutine();
            // USER CODE END WHILE

            // USER CODE BEGIN 3
        }
        // USER CODE END 3

 */


#include "main.h"

extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

// Create UART buffer instance for Rx/Tx of messages
UART_DMA_QueueStruct uart2 =
{
	.huart = &huart2,
	.rx.queueSize = UART_DMA_QUEUE_SIZE,
	.tx.queueSize = UART_DMA_QUEUE_SIZE
};

// I2C data buffer
uint8_t i2c1_data[6] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
uint8_t i2c3_data[6] = {0};
// Flag to indicate new I2C data received
volatile bool i2c3_dataRdy = false;

// called before main while loop
void PollingInit(void)
{
	UART_DMA_EnableRxInterrupt(&uart2);

	I2C_SlaveReceive(&hi2c3);
}

// called within main while loop
void PollingRoutine(void)
{
	UART_Parse(&uart2);

	I2C_CheckData();
}

/*
 * Description: Parse UART data and find get or set commands.
 * 				Will notify user of good command or error messages.
 */
void UART_Parse(UART_DMA_QueueStruct *msg)
{
	char *ptr;
	int status = NO_ERROR;
	char retStr[64] = "ok";
	char msgCpy[128] = {0}; // make copy of msgToParse data (ptr) because the original message will be modified.

	if(UART_DMA_MsgRdy(msg))
	{
		ptr = (char*)msg->rx.msgToParse->data;
		memcpy(&msgCpy, ptr, strlen(ptr) - 2); // less CR/LF
		RemoveSpaces(ptr);
		ToLower(ptr);

		if(strncmp(ptr, "set", strlen("set")) == 0)
		{
			ptr += strlen("set");
			status = UART_ParseSet(ptr);
		}
		else if(strncmp(ptr, "get", strlen("get")) == 0)
		{
			ptr += strlen("get");
			status = UART_ParseGet(ptr, retStr);
		}
		else
		{
			status = COMMAND_UNKNOWN;
		}

		if(status == NO_ACK)
		{
			return;
		}
		else if(status != NO_ERROR)
		{
			PrintError(msgCpy, status);
		}
		else
		{
			PrintReply(msgCpy, retStr);
		}
	}
}

/*
 * Description: Parse set commands. For now we're looking for the "i2c:" command.
 */
int UART_ParseSet(char *msg)
{
	int status = NO_ERROR;

	if(strncmp(msg, "i2c:", strlen("i2c:")) == 0)
	{
		msg += strlen("i2c:");
		status = I2C_ParseCommand(msg);
	}
	else
	{
		return COMMAND_UNKNOWN;
	}

	return status;
}

/*
 * Description: Parse get commands.
 */
int UART_ParseGet(char *msg, char *retStr)
{
	int status = NO_ERROR;

	// N/A

	return status;
}

/*
 * Description: Parse the string data. There is no error checking, assuming user sent data correctly.
 */
int I2C_ParseCommand(char *msg)
{
	int status = NO_ERROR;
	char *token;
	char *token2;
	char *token3;
	char *token4;
	char *token5;
	char *token6;
	char *rest = msg;
	char delim[] = ":,\r";
	uint32_t value = 0;

	token = strtok_r(rest, delim, &rest);
	token2 = strtok_r(rest, delim, &rest);
	token3 = strtok_r(rest, delim, &rest);
	token4 = strtok_r(rest, delim, &rest);
	token5 = strtok_r(rest, delim, &rest);
	token6 = strtok_r(rest, delim, &rest);

	IsHex(token, &value);
	i2c1_data[0] = value;

	IsHex(token2, &value);
	i2c1_data[1] = value;

	IsHex(token3, &value);
	i2c1_data[2] = value;

	IsHex(token4, &value);
	i2c1_data[3] = value;

	IsHex(token5, &value);
	i2c1_data[4] = value;

	IsHex(token6, &value);
	i2c1_data[5] = value;

	if(HAL_I2C_Master_Transmit_IT(&hi2c1, SLAVE_ADDRESS, i2c1_data, 6) != HAL_OK)
	{
		status = I2C_ERROR_WRITE;
	}

	return status;
}

/*
 * Description: Init the I2C slave to receive 6 bytes
 */
void I2C_SlaveReceive(I2C_HandleTypeDef *i2c)
{
	HAL_I2C_Slave_Receive_IT(i2c, i2c3_data, 6);
}

/*
 * Description: The I2Cx interrupt callback
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c == &hi2c3)
	{
		i2c3_dataRdy = true;
		I2C_SlaveReceive(&hi2c3); // enable interrupt again.
	}
}

/*
 * Description: Poll to see if there is new I2C3 data received and notify the user of the values.
 * 				There is no error checking. Will assume the user sent UART message correctly.
 */
void I2C_CheckData(void)
{
	char str[64] = {0};

	if(i2c3_dataRdy)
	{
		i2c3_dataRdy = false;

		sprintf(str, "I2C3 received 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X", i2c3_data[0], i2c3_data[1], i2c3_data[2], i2c3_data[3], i2c3_data[4], i2c3_data[5]);

		UART_DMA_NotifyUser(&uart2, str, strlen(str), true);
	}
}

/*
 * Description: Increment the queue pointer and enable UART interrupt again.
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == uart2.huart)
	{
		RingBuff_Ptr_Input(&uart2.rx.ptr, uart2.rx.queueSize);
		UART_DMA_EnableRxInterrupt(&uart2);
	}
}

/*
 * Description: Transmit of queue done. We can test to see if there are more messages to send.
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == uart2.huart)
	{
		UART_DMA_SendMessage(&uart2);
	}
}

/*
 * Description: Print the status error
 */
void PrintError(char *msgCpy, uint32_t error)
{
	char str[64] = {0};

	GetErrorString(error, str);

	strcat(msgCpy, "=");
	strcat(msgCpy, str);

	UART_DMA_NotifyUser(&uart2, msgCpy, strlen(msgCpy), true);
}

/*
 * Description: Reply back with original command with OK
 */
void PrintReply(char *msgCpy, char *msg2)
{
	strcat(msgCpy, "=");
	strcat(msgCpy, msg2);
	UART_DMA_NotifyUser(&uart2, msgCpy, strlen(msgCpy), true);
}

