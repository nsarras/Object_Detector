aclass pixy_UART_task:public scheduler_task
{
public:
	pixy_UART_task(uint8_t priority):scheduler_task("UART", 2000, priority)
{

}

	bool run(void *p)
	{

		int counter = 0;
		while (counter < 15) {
			uint16_t full_bytes[10];
			uint16_t check = Get_Upper_Lower_Bytes();

			if (check == 0xaa55) {
				full_bytes[0] = check;
				for (int i = 1; i < 10; i++) {
					full_bytes[i] = Get_Upper_Lower_Bytes();
				}
				vTaskDelay(1000);

				printf("Here is the object information \n");
				printf("Reads full bytes and sync matched!!\n");
				for (int k = 0; k < 10; k++) {
					printf("Full Byte %i = %u \n", k, (unsigned int)full_bytes[k]);
				}
				vTaskDelay(1000);
			}
			counter++;
			printf("Counter = %i", counter);
		}


		return true;
	}

	bool init(void)
	{
		LPC_SC->PCONP |= (1 << 24); // powers on UART pin 2
		LPC_SC->PCLKSEL1 &= (3 << 16); // clear the bit in bits 16 and 17
		LPC_SC->PCLKSEL1 |= (1 << 16); // set the speed of the peripheral clock to clk, calculations become easier

		LPC_PINCON->PINSEL4 &= ~(0xF << 16); // Clear values
		LPC_PINCON->PINSEL4 |= (10 << 16);  // Set values for UART2 Rx/Tx

		uint32_t baud = 19200;
		uint16_t dll = sys_get_cpu_clock() / (16 * baud);
		// uint8_t dll= 48 * 1000 * 1000 /(16 *baud);

		LPC_UART2->LCR = (1 << 7); // set the DLAB register to 1

		LPC_UART2->DLM = (dll >> 8);
		LPC_UART2->DLL = (dll >> 0);

		// LPC_UART2->LCR &= ~(1 << 7);
		LPC_UART2->LCR = 3; // selects word length to 8 bits no stop bits or parity bits
		//Stop bit
		LPC_UART2->LCR &= ~(1<<2);
		return true;
	}

	void uart2_putChar(char out) {
		LPC_UART2->THR = out; // starts sending out the data

		// it takes time for the data to be sent across so we will have to wait for this function to complete
		// we can wait till the transmitter register bit is 1 to tell us if there is any data left
		while (1) {
			if (LPC_UART2->LSR & (1 << 6)) {
				break; // if bit 5 is 1 Transmitting register is empty
			}
		}
	}

	uint16_t Get_Upper_Lower_Bytes() {

		uint16_t upperByte, lowerByte;

		upperByte = uart2_GetUint16_t();
		lowerByte = uart2_GetUint16_t();

		uint16_t FullByte = (upperByte << 8) | lowerByte;

		printf("Full Byte = %u\n", (unsigned int)FullByte);
		return FullByte;
	}

	char uart2_getChar(void) {
		while (1) {
			if (LPC_UART2->LSR & (1 << 0)) {
				break; // if bit 0 is a 1 FIFO is not empty
			}
		}

		char out = LPC_UART2->RBR;
		return out;
	}

	uint16_t uart2_GetUint16_t(void) {
		while (1) {
			if (LPC_UART2->LSR & (1 << 0)) {
				break; // if bit 0 is a 1 FIFO is not empty
			}
		}

		uint16_t out = LPC_UART2->RBR;
		return out;
	}
};


