#include "fsm.h"
#include "parser.h"
#include <stm32f411xe.h>
#include <stm32f4xx.h>
//state definition
enum uart_state{
	UART_IDLE,
	UART_PARSE
};
extern uint8_t uart_rx[20];
extern UART_HandleTypeDef huart2;
rb_struct rb_uart;
uint32_t uart_count;
uint8_t uart_flag;

//inputs
static int uart_timer(fsm_t* this){
	return uart_flag;
}
static int uart_parse_finished(fsm_t* this){
	return !uart_flag;
}

//output
static void uart_parse(fsm_t* this){
	parseMsg(&rb_uart);
	uart_flag = 0;
	uart_count = 0;
}

//transitions rules
static fsm_trans_t uart_tt[] = {

  { UART_IDLE, uart_timer, UART_PARSE,     uart_parse },
  { UART_PARSE,     uart_parse_finished, UART_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};

fsm_t* fsm_new_uart() {
	return fsm_new(uart_tt);
}

//uart interruption
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	if (huart->Instance == USART2) {
			uint8_t Len = UART_MSG_SIZE;
			uint8_t * Buf = uart_rx;
			while(Len--){
				rb_write(&rb_uart, *Buf++);
			}
			HAL_UART_Receive_IT(&huart2,(uint8_t *)uart_rx,UART_MSG_SIZE);

		}
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){

	if (huart->Instance == USART2) {
	}
}
