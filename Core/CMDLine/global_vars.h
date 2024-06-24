/*
 * global_vars.h
 *
 *  Created on: Jun 16, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_GLOBAL_VARS_H_
#define CMDLINE_GLOBAL_VARS_H_

extern volatile uint8_t sendFlag;
extern volatile uint8_t retryCount;

extern volatile uint8_t uart_choose_uart5;
extern volatile uint8_t uart_choose_usart2;
extern volatile uint8_t rs422_report_enable;
extern volatile uint8_t gps_report_enable;
extern volatile uint8_t rf_report_enable;


extern volatile uint8_t receive_iouFlag;
extern volatile uint8_t receive_pduFlag;
extern volatile uint8_t receive_pmuFlag;

extern volatile uint8_t disconnect_counter_pdu;
extern volatile uint8_t disconnect_counter_pmu;
extern volatile uint8_t disconnect_counter_iou;


extern volatile uint8_t send_rs422;

extern uint16_t ARRAY_SIZE;
extern uint8_t buffer1[1000];
extern uint8_t buffer2[1000];
extern uint8_t* currentBuffer;
extern uint8_t* nextBuffer;







extern volatile uint8_t auto_report_enabled;
extern volatile uint8_t swap_byte_enable;
//extern uint32_t RS422_PERIOD;

#endif /* CMDLINE_GLOBAL_VARS_H_ */
