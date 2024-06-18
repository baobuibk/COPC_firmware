/*
 * fsp.c
 *
 *  Created on: May 25, 2024
 *      Author: CAO HIEU
 */

#include "fsp.h"
#include "crc.h"
#include "../../BSP/UART/uart.h"
#include "string.h"
#include "stdio.h"
#include "../CMDLine/ACK_packet/ACKsend_packet.h"
#include "stdlib.h"
#include "../CMDLine/global_vars.h"

uint8_t fsp_my_adr;

volatile uint8_t fsp_decode_pos = 0;

void fsp_init(uint8_t module_adr)
{
    fsp_my_adr = module_adr;

    fsp_decode_pos = 0;
}

void fsp_reset(void)
{
    fsp_decode_pos = 0;
}

void fsp_gen_data_pkt(uint8_t *data, uint8_t data_len, uint8_t dst_adr, uint8_t ack, fsp_packet_t *fsp)
{
    if (ack == FSP_PKT_WITH_ACK)
    {
        fsp_gen_pkt((void*)0, data, data_len, dst_adr, FSP_PKT_TYPE_DATA_WITH_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt((void*)0, data, data_len, dst_adr, FSP_PKT_TYPE_DATA, fsp);
    }
}

void fsp_gen_cmd_pkt(uint8_t cmd, uint8_t dst_adr, uint8_t ack, fsp_packet_t *fsp)
{
    if (ack == FSP_PKT_WITH_ACK)
    {
        fsp_gen_pkt(&cmd,(void*)0, 0, dst_adr, FSP_PKT_TYPE_CMD_WITH_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt(&cmd,(void*)0,  0, dst_adr, FSP_PKT_TYPE_CMD, fsp);
    }
}

void fsp_gen_cmd_w_data_pkt(uint8_t cmd, uint8_t *data, uint8_t data_len, uint8_t dst_adr, uint8_t ack, fsp_packet_t *fsp)
{
    if (ack == FSP_PKT_WITH_ACK)
    {
        fsp_gen_pkt(&cmd, data, data_len, dst_adr, FSP_PKT_TYPE_CMD_W_DATA_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt(&cmd, data, data_len, dst_adr, FSP_PKT_TYPE_CMD_W_DATA, fsp);
    }
}


void fsp_gen_ack_pkt(uint8_t dst_adr, fsp_packet_t *fsp)
{
    fsp_gen_pkt((void*)0, (void*)0, 0, dst_adr, FSP_PKT_TYPE_ACK, fsp);
}

void fsp_gen_nack_pkt(uint8_t dst_adr, fsp_packet_t *fsp)
{
    fsp_gen_pkt((void*)0, (void*)0, 0, dst_adr, FSP_PKT_TYPE_NACK, fsp);
}


void fsp_gen_pkt(uint8_t *cmd, uint8_t *payload, uint8_t payload_len, uint8_t dst_adr, uint8_t type, fsp_packet_t *fsp)
{
    fsp->sod        = FSP_PKT_SOD;
    fsp->src_adr    = fsp_my_adr;
    fsp->dst_adr    = dst_adr;
    fsp->length     = payload_len;
    fsp->type       = type;

    uint8_t i = 0;
    uint8_t j = 0;

    // Copy cmd payload
    if (cmd != NULL) {
        fsp->length++; // length + byte cmd
        fsp->payload[j++] = *cmd;
    }

    // Copy payload fsp->payload
    for(i=0; i<payload_len; i++)
    {
        fsp->payload[j++] = payload[i];
    }

    fsp->crc16 = crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &fsp->src_adr, fsp->length + 4);

}


void frame_encode(fsp_packet_t *fsp, uint8_t *frame, uint8_t *frame_len)
{
    //frame
    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t encoded_length = 0;

    encoded_frame[encoded_length++] = fsp->sod;
    encoded_frame[encoded_length++] = fsp->src_adr;
    encoded_frame[encoded_length++] = fsp->dst_adr;
    encoded_frame[encoded_length++] = fsp->length;
    encoded_frame[encoded_length++] = fsp->type;

    for(int i=0; i<fsp->length; i++)
    {
        if (fsp->payload[i] == FSP_PKT_SOD) {
            encoded_frame[encoded_length++] = FSP_PKT_ESC;
            encoded_frame[encoded_length++] = FSP_PKT_TSOD;
        } else if (fsp->payload[i] == FSP_PKT_EOF) {
            encoded_frame[encoded_length++] = FSP_PKT_ESC;
            encoded_frame[encoded_length++] = FSP_PKT_TEOF;
        } else if (fsp->payload[i] == FSP_PKT_ESC) {
            encoded_frame[encoded_length++] = FSP_PKT_ESC;
            encoded_frame[encoded_length++] = FSP_PKT_TESC;
        } else {
            encoded_frame[encoded_length++] = fsp->payload[i];
        }
    }

    // CRC16
    uint8_t crc_msb = (uint8_t)(fsp->crc16 >> 8);
    uint8_t crc_lsb = (uint8_t)(fsp->crc16 & 0xFF);

    if (crc_msb == FSP_PKT_SOD) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TSOD;
    } else if (crc_msb == FSP_PKT_EOF) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TEOF;
    } else if (crc_msb == FSP_PKT_ESC) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TESC;
    } else {
        encoded_frame[encoded_length++] = crc_msb;
    }

    if (crc_lsb == FSP_PKT_SOD) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TSOD;
    } else if (crc_lsb == FSP_PKT_EOF) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TEOF;
    } else if (crc_lsb == FSP_PKT_ESC) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TESC;
    } else {
        encoded_frame[encoded_length++] = crc_lsb;
    }

    encoded_frame[encoded_length++] = FSP_PKT_EOF;

    memcpy(frame, encoded_frame, encoded_length);
    *frame_len = encoded_length;
}



void fsp_encode(fsp_packet_t *fsp, uint8_t *pkt, uint8_t *pkt_len)
{
    uint8_t i = 0;

    pkt[i++] = fsp->sod;
    pkt[i++] = fsp->src_adr;
    pkt[i++] = fsp->dst_adr;
    pkt[i++] = fsp->length;
    pkt[i++] = fsp->type;

    uint8_t j = 0;
    for(j=0; j<fsp->length; j++)
    {
        pkt[i++] = fsp->payload[j];
    }

    pkt[i++] = (uint8_t)(fsp->crc16 >> 8);
    pkt[i++] = (uint8_t)(fsp->crc16);

    *pkt_len = i;
}

uint8_t fsp_decode(uint8_t byte, fsp_packet_t *fsp)
{
    switch(fsp_decode_pos)
    {
        case FSP_PKT_POS_SOD:
            if (byte == FSP_PKT_SOD)
            {
                fsp->sod = byte;
                fsp_decode_pos++;
                return FSP_PKT_NOT_READY;
            }
            else
            {
                fsp_decode_pos = FSP_PKT_POS_SOD;
                return FSP_PKT_INVALID;
            }
        case FSP_PKT_POS_SRC_ADR:
            fsp->src_adr = byte;
            fsp_decode_pos++;
            return FSP_PKT_NOT_READY;
        case FSP_PKT_POS_DST_ADR:
            fsp->dst_adr = byte;

            if (byte == fsp_my_adr)
            {
                fsp_decode_pos++;
                return FSP_PKT_NOT_READY;
            }
            else
            {
                fsp_decode_pos = FSP_PKT_POS_SOD;
                return FSP_PKT_WRONG_ADR;
            }
        case FSP_PKT_POS_LEN:
            if (byte > FSP_PAYLOAD_MAX_LENGTH)
            {
                fsp_decode_pos = FSP_PKT_POS_SOD;
                return FSP_PKT_INVALID;
            }
            else
            {
                fsp->length = byte;
                fsp_decode_pos++;
                return FSP_PKT_NOT_READY;
            }
        case FSP_PKT_POS_TYPE:
            fsp->type = byte;
            fsp_decode_pos++;
            return FSP_PKT_NOT_READY;
        default:
            if (fsp_decode_pos < (FSP_PKT_POS_TYPE + fsp->length + 1))          // Payload
            {
                fsp->payload[fsp_decode_pos - FSP_PKT_POS_TYPE - 1] = byte;
                fsp_decode_pos++;
                return FSP_PKT_NOT_READY;
            }
            else if (fsp_decode_pos == (FSP_PKT_POS_TYPE + fsp->length + 1))    // CRC16 MSB
            {
                fsp->crc16 = (uint16_t)(byte << 8);
                fsp_decode_pos++;
                return FSP_PKT_NOT_READY;
            }
            else if (fsp_decode_pos == (FSP_PKT_POS_TYPE + fsp->length + 2))    // CRC16 LSB
            {
                fsp->crc16 |= (uint16_t)(byte);

                if (fsp->crc16 == crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &fsp->src_adr, fsp->length + 4))
                {
                    fsp_decode_pos = FSP_PKT_POS_SOD;
                    return FSP_PKT_READY;
                }
                else
                {
                    fsp_decode_pos = FSP_PKT_POS_SOD;
                    return FSP_PKT_INVALID;
                }
            }
            else
            {
                fsp_decode_pos = FSP_PKT_POS_SOD;
                return FSP_PKT_ERROR;
            }
    }
}

int frame_decode(uint8_t *buffer, uint8_t length, fsp_packet_t *pkt){

    fsp_packet_t fsp_pkt;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t escape = 0;
    uint8_t decoded_payload[FSP_PAYLOAD_MAX_LENGTH];


	if (length < FSP_PKT_MIN_LENGTH - 2) {
	        return FSP_PKT_INVALID;
	}


	while (i < length){
        uint8_t byte = buffer[i++];
        if (escape) {
            if (byte == FSP_PKT_TSOD) {
                decoded_payload[j++] = FSP_PKT_SOD;
            } else if (byte == FSP_PKT_TEOF) {
                decoded_payload[j++] = FSP_PKT_EOF;
            } else if (byte == FSP_PKT_TESC) {
                decoded_payload[j++] = FSP_PKT_ESC;
            } else {
            	return FSP_PKT_INVALID;
            }
            escape = 0;
        } else if (byte == FSP_PKT_ESC) {
        	escape = 1;
        } else {
            decoded_payload[j++] = byte;
        }
	}

    i = 0;
    fsp_pkt.src_adr = decoded_payload[i++];
    fsp_pkt.dst_adr = decoded_payload[i++];
    fsp_pkt.length = decoded_payload[i++];
    fsp_pkt.type = decoded_payload[i++];

    if (fsp_pkt.length > FSP_PAYLOAD_MAX_LENGTH || fsp_pkt.length != j - FSP_PKT_HEADER_LENGTH  - FSP_PKT_CRC_LENGTH) {
        return FSP_PKT_WRONG_LENGTH;
    }

    memcpy(fsp_pkt.payload, &decoded_payload[i], fsp_pkt.length);
    i += fsp_pkt.length;
    //CRC
    uint16_t crc_received = (uint16_t)(decoded_payload[i++] << 8);
    crc_received |= (uint16_t)(decoded_payload[i++]);


    // CAL CRC
    uint16_t crc_calculated = crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &fsp_pkt.src_adr, fsp_pkt.length + 4);


    // CHECK CRC
    if (crc_received != crc_calculated) {
        return FSP_PKT_CRC_FAIL;
    }

    // Address
    if (fsp_pkt.dst_adr != fsp_my_adr) {
        return FSP_PKT_WRONG_ADR;
    }

    *pkt = fsp_pkt;


    frame_processing(&fsp_pkt);


    return 0;
}



int frame_decode_rs422(uint8_t *buffer, uint8_t length, fsp_packet_t *pkt){

    fsp_packet_t fsp_pkt;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t escape = 0;
    uint8_t decoded_payload[FSP_PAYLOAD_MAX_LENGTH];

//
//	if (length < FSP_PKT_MIN_LENGTH - 2) {
//	        return FSP_PKT_INVALID;
//	}


	while (i < length){
        uint8_t byte = buffer[i++];
        if (escape) {
            if (byte == FSP_PKT_TSOD) {
                decoded_payload[j++] = FSP_PKT_SOD;
            } else if (byte == FSP_PKT_TEOF) {
                decoded_payload[j++] = FSP_PKT_EOF;
            } else if (byte == FSP_PKT_TESC) {
                decoded_payload[j++] = FSP_PKT_ESC;
            } else {
            	return FSP_PKT_INVALID;
            }
            escape = 0;
        } else if (byte == FSP_PKT_ESC) {
        	escape = 1;
        } else {
            decoded_payload[j++] = byte;
        }
	}

    i = 0;
    fsp_pkt.src_adr = decoded_payload[i++];
    fsp_pkt.dst_adr = decoded_payload[i++];
    fsp_pkt.length = decoded_payload[i++];
    fsp_pkt.type = decoded_payload[i++];

//    if (fsp_pkt.length > FSP_PAYLOAD_MAX_LENGTH || fsp_pkt.length != j - FSP_PKT_HEADER_LENGTH  - FSP_PKT_CRC_LENGTH) {
//        return FSP_PKT_WRONG_LENGTH;
//    }

    memcpy(fsp_pkt.payload, &decoded_payload[i], fsp_pkt.length);
    i += fsp_pkt.length;
    //CRC
//    uint16_t crc_received = (uint16_t)(decoded_payload[i++] << 8);
//    crc_received |= (uint16_t)(decoded_payload[i++]);
//

    // CAL CRC
 //   uint16_t crc_calculated = crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &fsp_pkt.src_adr, fsp_pkt.length + 4);


//    // CHECK CRC
//    if (crc_received != crc_calculated) {
//        return FSP_PKT_CRC_FAIL;
//    }
//
//    // Address
//    if (fsp_pkt.dst_adr != fsp_my_adr) {
//        return FSP_PKT_WRONG_ADR;
//    }

    *pkt = fsp_pkt;


    return 0;
}


char pos_str2[10];

int frame_processing(fsp_packet_t *fsp_pkt){


	switch (fsp_pkt->src_adr){
		case FSP_ADR_PMU:
			switch (fsp_pkt->type)
			{
				case FSP_PKT_TYPE_ACK:
					clear_send_flag();

                    if (uart_choose_uart5) {
                    	Uart_sendstring(UART5, "\nPMU_ACK\r\n> ");
                    }
					Uart_sendstring(USART6, "\nPMU_ACK\r\n> ");

				case FSP_PKT_TYPE_CMD_W_DATA:
					//reverse
					clear_send_flag();
					switch(fsp_pkt->payload[0])
					{
						case 0x00:
						{
							char buffer_0x00[50];
							sprintf(buffer_0x00, "PMU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
		                    if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x00);
		                    }
							Uart_sendstring(USART6, buffer_0x00);

						}
							break;
						case 0xFF:
						{
							char buffer_0xFF[50];

							sprintf(buffer_0xFF, "PMU_Failed: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0xFF);
							}
							Uart_sendstring(USART6, buffer_0xFF);
						}
							break;

						case 0x01:
						{
						    int16_t ntc0 = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
						    int16_t ntc1 = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
						    int16_t ntc2 = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
						    int16_t ntc3 = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

						    char buffer_0x01[100];
						    sprintf(buffer_0x01, "PMU_Res: CMDcode 0x01 [\nNTC0: %s%d.%02d, \nNTC1: %s%d.%02d, \nNTC2: %s%d.%02d, \nNTC3: %s%d.%02d]\n",
						            ntc0 < 0 ? "-" : "", abs(ntc0) / 100, abs(ntc0) % 100,
						            ntc1 < 0 ? "-" : "", abs(ntc1) / 100, abs(ntc1) % 100,
						            ntc2 < 0 ? "-" : "", abs(ntc2) / 100, abs(ntc2) % 100,
						            ntc3 < 0 ? "-" : "", abs(ntc3) / 100, abs(ntc3) % 100);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x01);
							}
							Uart_sendstring(USART6, buffer_0x01);
						}
						break;

						case 0x02:
						{
					        uint16_t bat0 = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
					        uint16_t bat1 = (uint16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
					        uint16_t bat2 = (uint16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
					        uint16_t bat3 = (uint16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

					        char buffer_0x02[100];
					        sprintf(buffer_0x02, "PMU_Res: CMDcode 0x02 [BAT0: %d.%02d V, BAT1: %d.%02d V, BAT2: %d.%02d V, BAT3: %d.%02d V]\n",
					                bat0/100, bat0%100, bat1/100, bat1%100, bat2/100, bat2%100, bat3/100, bat3%100);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x02);
							}
							Uart_sendstring(USART6, buffer_0x02);
						}
							break;
						case 0x03:
						{
						    uint16_t vin = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
						    uint16_t iin = (uint16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);

						    char buffer_0x03[100];
						    sprintf(buffer_0x03, "PMU_Res: CMDcode 0x03 [VIN: %d.%02d V, IIN: %d.%02d A]\n",
						            vin/100, vin%100, iin/100, iin%100);

							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x03);
							}
							Uart_sendstring(USART6, buffer_0x03);

						}
						    break;


						case 0x04:
						{
						    uint16_t vout = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
						    uint16_t iout = (uint16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);

						    char buffer_0x04_pmu[100];
						    sprintf(buffer_0x04_pmu, "PMU_Res: CMDcode 0x04 [VOUT: %d.%02d V, IOUT: %d.%02d A]\n",
						            vout/100, vout%100, iout/100, iout%100);

							if (uart_choose_uart5) {
							    Uart_sendstring(UART5, buffer_0x04_pmu);
							}
							Uart_sendstring(USART6, buffer_0x04_pmu);
						}

							break;

							case 0x08:
							    {
							    	int16_t ntc0 = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
							    	int16_t ntc1 = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
							    	int16_t ntc2 = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
							    	int16_t ntc3 = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

							    	uint16_t bat0 = (uint16_t)((fsp_pkt->payload[9] << 8) | fsp_pkt->payload[10]);
							    	uint16_t bat1 = (uint16_t)((fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12]);
							    	uint16_t bat2 = (uint16_t)((fsp_pkt->payload[13] << 8) | fsp_pkt->payload[14]);
							    	uint16_t bat3 = (uint16_t)((fsp_pkt->payload[15] << 8) | fsp_pkt->payload[16]);

							    	uint16_t vin = (uint16_t)((fsp_pkt->payload[17] << 8) | fsp_pkt->payload[18]);
							    	uint16_t iin = (uint16_t)((fsp_pkt->payload[19] << 8) | fsp_pkt->payload[20]);

							    	uint16_t vout = (uint16_t)((fsp_pkt->payload[21] << 8) | fsp_pkt->payload[22]);
							    	uint16_t iout = (uint16_t)((fsp_pkt->payload[23] << 8) | fsp_pkt->payload[24]);

							    	char buffer_0x08[500];
							    	sprintf(buffer_0x08, "PMU_Res: CMDcode 0x08 [\nNTC0: %s%d.%02d, \nNTC1: %s%d.%02d, \nNTC2: %s%d.%02d, \nNTC3: %s%d.%02d, \nBAT0: %d.%02d V, \nBAT1: %d.%02d V, \nBAT2: %d.%02d V, \nBAT3: %d.%02d V, \nVIN: %d.%02d V, \nIIN: %d.%02d A, \nVOUT: %d.%02d V, \nIOUT: %d.%02d A]\n",
							    	        ntc0 < 0 ? "-" : "", abs(ntc0) / 100, abs(ntc0) % 100,
							    	        ntc1 < 0 ? "-" : "", abs(ntc1) / 100, abs(ntc1) % 100,
							    	        ntc2 < 0 ? "-" : "", abs(ntc2) / 100, abs(ntc2) % 100,
							    	        ntc3 < 0 ? "-" : "", abs(ntc3) / 100, abs(ntc3) % 100,
							    	        bat0 / 100, bat0 % 100, bat1 / 100, bat1 % 100,
							    	        bat2 / 100, bat2 % 100, bat3 / 100, bat3 % 100,
							    	        vin / 100, vin % 100, iin / 100, iin % 100,
							    	        vout / 100, vout % 100, iout / 100, iout % 100);


									if (uart_choose_uart5) {
								    	Uart_sendstring(UART5, buffer_0x08);
									}
									Uart_sendstring(USART6, buffer_0x08);


							    }
							    break;
					}

					break;
				default:
					clear_send_flag();
					return FSP_PKT_INVALID;
					break;
			}
			clear_send_flag();
			break;
		case FSP_ADR_PDU:
			switch (fsp_pkt->type)
			{
				case FSP_PKT_TYPE_ACK:
					clear_send_flag();
					if (uart_choose_uart5) {
						Uart_sendstring(UART5, "\n> PDU_ACK\r\n> ");
					}
					Uart_sendstring(USART6, "\n> PDU_ACK\r\n> ");

					break;
				case FSP_PKT_TYPE_CMD_W_DATA:
					//reverse
					clear_send_flag();
					switch(fsp_pkt->payload[0])
					{
						case 0x00:
						{
							char buffer_0x00[50];
							sprintf(buffer_0x00, "PDU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x00);
							}
							Uart_sendstring(USART6, buffer_0x00);

						}
							break;
						case 0xFF:
						{
							char buffer_0xFF[50];
							sprintf(buffer_0xFF, "PDU_Failed: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0xFF);
							}
							Uart_sendstring(USART6, buffer_0xFF);

						}
							break;

						case 0x04:
						{
							        uint8_t channel = fsp_pkt->payload[1];
							        uint8_t status_0x04 = fsp_pkt->payload[2];
							        uint16_t voltage_0x04 = (fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4];
							        uint16_t current_0x04 = (fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6];

							        char buffer_0x07[100];
							        sprintf(buffer_0x07, "PDU_Res: CMDcode 0x04 [{Channel %u} Status %u, Voltage: %u, Current: %u]\n", channel, status_0x04, voltage_0x04, current_0x04);
									if (uart_choose_uart5) {
										Uart_sendstring(UART5, buffer_0x07);
									}
									Uart_sendstring(USART6, buffer_0x07);

						}
									break;
						case 0x05:
						{
						            uint8_t buck = fsp_pkt->payload[1];
						            uint8_t status_0x05 = fsp_pkt->payload[2];
						            uint16_t voltage_0x05 = (fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4];
						            uint16_t current_0x05 = (fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6];

						            char buffer_0x05[100];
						            sprintf(buffer_0x05, "PDU_Res: CMDcode 0x05 [{Buck %u} Status %u, Voltage: %u, Current: %u]\n", buck, status_0x05, voltage_0x05, current_0x05);
									if (uart_choose_uart5) {
										Uart_sendstring(UART5, buffer_0x05);
									}
									Uart_sendstring(USART6, buffer_0x05);

						}
									break;
						case 0x06:
						{
							uint8_t tec1buck_status = fsp_pkt->payload[1];
							uint16_t tec1buck_voltage = (fsp_pkt->payload[2] << 8) | fsp_pkt->payload[3];

							uint8_t tec2buck_status = fsp_pkt->payload[4];
							uint16_t tec2buck_voltage = (fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6];

							uint8_t tec3buck_status = fsp_pkt->payload[7];
							uint16_t tec3buck_voltage = (fsp_pkt->payload[8] << 8) | fsp_pkt->payload[9];

							uint8_t tec4buck_status = fsp_pkt->payload[10];
							uint16_t tec4buck_voltage = (fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12];

							uint8_t mcubuck_status = fsp_pkt->payload[13];
							uint16_t mcubuck_voltage = (fsp_pkt->payload[14] << 8) | fsp_pkt->payload[15];

							uint8_t ledbuck_status = fsp_pkt->payload[16];
							uint16_t ledbuck_voltage = (fsp_pkt->payload[17] << 8) | fsp_pkt->payload[18];

							uint8_t cm4buck_status = fsp_pkt->payload[19];
							uint16_t cm4buck_voltage = (fsp_pkt->payload[20] << 8) | fsp_pkt->payload[21];

							uint8_t tec1_status = fsp_pkt->payload[22];
							uint16_t tec1_current = (fsp_pkt->payload[23] << 8) | fsp_pkt->payload[24];

							uint8_t tec2_status = fsp_pkt->payload[25];
							uint16_t tec2_current = (fsp_pkt->payload[26] << 8) | fsp_pkt->payload[27];

							uint8_t tec3_status = fsp_pkt->payload[28];
							uint16_t tec3_current = (fsp_pkt->payload[29] << 8) | fsp_pkt->payload[30];

							uint8_t tec4_status = fsp_pkt->payload[31];
							uint16_t tec4_current = (fsp_pkt->payload[32] << 8) | fsp_pkt->payload[33];

							uint8_t copc_status = fsp_pkt->payload[34];
							uint16_t copc_current = (fsp_pkt->payload[35] << 8) | fsp_pkt->payload[36];

							uint8_t iou_status = fsp_pkt->payload[37];
							uint16_t iou_current = (fsp_pkt->payload[38] << 8) | fsp_pkt->payload[39];

							uint8_t rgb_status = fsp_pkt->payload[40];
							uint16_t rgb_current = (fsp_pkt->payload[41] << 8) | fsp_pkt->payload[42];

							uint8_t ir_status = fsp_pkt->payload[43];
							uint16_t ir_current = (fsp_pkt->payload[44] << 8) | fsp_pkt->payload[45];

							uint8_t cm4_status = fsp_pkt->payload[46];
							uint16_t cm4_current = (fsp_pkt->payload[47] << 8) | fsp_pkt->payload[48];

							uint8_t vin_status = fsp_pkt->payload[49];
							uint16_t vin_voltage = (fsp_pkt->payload[50] << 8) | fsp_pkt->payload[51];

							uint8_t vbus_status = fsp_pkt->payload[52];
							uint16_t vbus_voltage = (fsp_pkt->payload[53] << 8) | fsp_pkt->payload[54];


						            char buffer_0x06[1000];
						            sprintf(buffer_0x06, "PDU_Res: CMDcode 0x06 [TEC1BUCK: Status %u, Voltage: %u\r\nTEC2BUCK: Status %u, Voltage: %u\r\nTEC3BUCK: Status %u, Voltage: %u\r\nTEC4BUCK: Status %u, Voltage: %u\r\nMCUBUCK: Status %u, Voltage: %u\r\nLEDBUCK: Status %u, Voltage: %u\r\nCM4BUCK: Status %u, Voltage: %u\r\nTEC1: Status %u, Current: %u\r\nTEC2: Status %u, Current: %u\r\nTEC3: Status %u, Current: %u\r\nTEC4: Status %u, Current: %u\r\nCOPC: Status %u, Current: %u\r\nIOU: Status %u, Current: %u\r\nRGB: Status %u, Current: %u\r\nIR: Status %u, Current: %u\r\nCM4: Status %u, Current: %u\r\nVIN: Status %u, Voltage: %u\r\nVBUS: Status %u, Voltage: %u\r\n]",
						                tec1buck_status, tec1buck_voltage,
						                tec2buck_status, tec2buck_voltage,
						                tec3buck_status, tec3buck_voltage,
						                tec4buck_status, tec4buck_voltage,
						                mcubuck_status, mcubuck_voltage,
						                ledbuck_status, ledbuck_voltage,
						                cm4buck_status, cm4buck_voltage,
						                tec1_status, tec1_current,
						                tec2_status, tec2_current,
						                tec3_status, tec3_current,
						                tec4_status, tec4_current,
						                copc_status, copc_current,
						                iou_status, iou_current,
						                rgb_status, rgb_current,
						                ir_status, ir_current,
						                cm4_status, cm4_current,
						                vin_status, vin_voltage,
						                vbus_status, vbus_voltage);
									if (uart_choose_uart5) {
										Uart_sendstring(UART5, buffer_0x06);
									}
									Uart_sendstring(USART6, buffer_0x06);

						}

						default:
							clear_send_flag();
							return FSP_PKT_INVALID;
							break;

					}


//					if (fsp_pkt->payload[0] == 0) {
//						char buffer[50];
//						sprintf(buffer, "PDU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
//						Uart_sendstring(UART5, buffer);
//					} else {
//					    for (int i = 0; i < fsp_pkt->length; i++) {
//					        Uart_write(UART5, fsp_pkt->payload[i]);
//					    }
//					}


//										    		char buffer[10];
//													for (int i = 0; i < fsp_pkt->length; i++) {
//													      sprintf(buffer, "\n{%d}", fsp_pkt->payload[i]);
//													      Uart_sendstring(UART5, buffer);
//													}

					clear_send_flag();
					break;
				default:
					clear_send_flag();
					return FSP_PKT_INVALID;
					break;
			}

			break;
		case FSP_ADR_CAM:
			clear_send_flag();

			break;

		case FSP_ADR_IOU:
			switch (fsp_pkt->type)
			{
				case FSP_PKT_TYPE_ACK:
					clear_send_flag();
					if (uart_choose_uart5) {
						Uart_sendstring(UART5, "\nIOU_ACK\r\n> ");
					}
					Uart_sendstring(USART6, "\nIOU_ACK\r\n> ");

					break;
				case FSP_PKT_TYPE_CMD_W_DATA:

					clear_send_flag();
					switch(fsp_pkt->payload[0])
					{
						case 0x00:
						{
							char buffer_0x00[50];
							sprintf(buffer_0x00, "IOU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x00);
							}
							Uart_sendstring(USART6, buffer_0x00);

						}
							break;
						case 0xFF:
						{
							char buffer_0xFF[50];
							sprintf(buffer_0xFF, "IOU_Failed: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0xFF);
							}
							Uart_sendstring(USART6, buffer_0xFF);

						}
							break;

						case 0x02:
						{
						    uint8_t device = fsp_pkt->payload[1];
						    uint8_t channel = fsp_pkt->payload[2];
						    int16_t temp = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);

						    char buffer_0x02[100];
						    sprintf(buffer_0x02, "IOU_Res: CMDcode 0x02 [{Device: %u, Channel: %u} Temp: %d.%d]\n",
						            device, channel, temp / 10, abs(temp % 10));
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x02);
							}
							Uart_sendstring(USART6, buffer_0x02);

						}
						break;

						case 0x03:
						{
						    uint8_t channel = fsp_pkt->payload[1];
						    int16_t temp = (int16_t)((fsp_pkt->payload[2] << 8) | fsp_pkt->payload[3]);

						    char buffer_0x03[100];
						    sprintf(buffer_0x03, "IOU_Res: CMDcode 0x03 [{Channel: %u} Temp: %d.%d]\n",
						            channel, temp / 10, temp % 10);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x03);
							}
							Uart_sendstring(USART6, buffer_0x03);

						}
						break;

						case 0x0E:
						{
						    uint8_t red = fsp_pkt->payload[1];
						    uint8_t blue = fsp_pkt->payload[2];
						    uint8_t green = fsp_pkt->payload[3];
						    uint8_t white = fsp_pkt->payload[4];

						    char buffer_0x0E[100];
						    sprintf(buffer_0x0E, "IOU_Res: CMDcode 0x0E [Red: %u, Blue: %u, Green: %u, White: %u]\n",
						            red, blue, green, white);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x0E);
							}
							Uart_sendstring(USART6, buffer_0x0E);

						}

						break;

						case 0x10:
						{
						    uint8_t duty = fsp_pkt->payload[1];

						    char buffer_0x10[100];
						    sprintf(buffer_0x10, "IOU_Res: CMDcode 0x10 [Duty: %u%%]\n", duty);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x10);
							}
							Uart_sendstring(USART6, buffer_0x10);

						}
						break;

						case 0x11:
						{
						    int16_t accel_x = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
						    int16_t accel_y = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
						    int16_t accel_z = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);

						    int16_t gyro_x = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);
						    int16_t gyro_y = (int16_t)((fsp_pkt->payload[9] << 8) | fsp_pkt->payload[10]);
						    int16_t gyro_z = (int16_t)((fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12]);

						    char buffer_0x11[200];
						    sprintf(buffer_0x11, "IOU_Res: CMDcode 0x11 [Accel: X=%d, Y=%d, Z=%d\nGyro: X=%d, Y=%d, Z=%d]\n",
						            accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x11);
							}
							Uart_sendstring(USART6, buffer_0x11);

						}
						break;

						case 0x12:
						{
						    uint16_t pressure = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);

						    char buffer_0x12[100];
						    sprintf(buffer_0x12, "IOU_Res: CMDcode 0x12 [Press: %u]\n", pressure);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x12);
							}
							Uart_sendstring(USART6, buffer_0x12);

						}
						break;

						case 0x13:
						{
							int16_t temp_ntc_channel0 = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
							int16_t temp_ntc_channel1 = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
							int16_t temp_ntc_channel2 = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
							int16_t temp_ntc_channel3 = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

							int16_t temp_onewire_channel0 = (int16_t)((fsp_pkt->payload[9] << 8) | fsp_pkt->payload[10]);
							int16_t temp_onewire_channel1 = (int16_t)((fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12]);

							int16_t temp_sensor = (int16_t)((fsp_pkt->payload[13] << 8) | fsp_pkt->payload[14]);

							int16_t temp_setpoint_channel0 = (int16_t)((fsp_pkt->payload[15] << 8) | fsp_pkt->payload[16]);
							int16_t temp_setpoint_channel1 = (int16_t)((fsp_pkt->payload[17] << 8) | fsp_pkt->payload[18]);
							int16_t temp_setpoint_channel2 = (int16_t)((fsp_pkt->payload[19] << 8) | fsp_pkt->payload[20]);
							int16_t temp_setpoint_channel3 = (int16_t)((fsp_pkt->payload[21] << 8) | fsp_pkt->payload[22]);

							uint16_t voltage_out_tec_channel0 = (uint16_t)((fsp_pkt->payload[23] << 8) | fsp_pkt->payload[24]);
							uint16_t voltage_out_tec_channel1 = (uint16_t)((fsp_pkt->payload[25] << 8) | fsp_pkt->payload[26]);
							uint16_t voltage_out_tec_channel2 = (uint16_t)((fsp_pkt->payload[27] << 8) | fsp_pkt->payload[28]);
							uint16_t voltage_out_tec_channel3 = (uint16_t)((fsp_pkt->payload[29] << 8) | fsp_pkt->payload[30]);

							uint8_t neo_led_r = fsp_pkt->payload[31];
							uint8_t neo_led_g = fsp_pkt->payload[32];
							uint8_t neo_led_b = fsp_pkt->payload[33];
							uint8_t neo_led_w = fsp_pkt->payload[34];

							uint8_t ir_led_duty = fsp_pkt->payload[35];


							char buffer_0x13[1000];

							sprintf(buffer_0x13, "IOU_Res: CMDcode 0x13 [NTC Temp: Ch0=%s%d.%d, Ch1=%s%d.%d, Ch2=%s%d.%d, Ch3=%s%d.%d\n"
							                     "OneWire Temp: Ch0=%s%d.%d, Ch1=%s%d.%d\n"
							                     "Sensor Temp: %s%d.%d\n"
							                     "Setpoint Temp: Ch0=%s%d.%d, Ch1=%s%d.%d, Ch2=%s%d.%d, Ch3=%s%d.%d\n"
							                     "TEC Voltage: Ch0=%d.%02d, Ch1=%d.%02d, Ch2=%d.%02d, Ch3=%d.%02d\n"
							                     "Neo LED: R=%u, G=%u, B=%u, W=%u\n"
							                     "IR LED Duty: %u%%]\n",
							        temp_ntc_channel0 < 0 ? "-" : "", abs(temp_ntc_channel0)/ 10, abs(temp_ntc_channel0) % 10,
							        temp_ntc_channel1 < 0 ? "-" : "", abs(temp_ntc_channel1)/ 10, abs(temp_ntc_channel1) % 10,
							        temp_ntc_channel2 < 0 ? "-" : "", abs(temp_ntc_channel2)/ 10, abs(temp_ntc_channel2) % 10,
							        temp_ntc_channel3 < 0 ? "-" : "", abs(temp_ntc_channel3)/ 10, abs(temp_ntc_channel3) % 10,
							        temp_onewire_channel0 < 0 ? "-" : "", abs(temp_onewire_channel0)/ 10, abs(temp_onewire_channel0) % 10,
							        temp_onewire_channel1 < 0 ? "-" : "", abs(temp_onewire_channel1)/ 10, abs(temp_onewire_channel1) % 10,
							        temp_sensor < 0 ? "-" : "", abs(temp_sensor)/ 10, abs(temp_sensor) % 10,
							        temp_setpoint_channel0 < 0 ? "-" : "", abs(temp_setpoint_channel0)/ 10, abs(temp_setpoint_channel0) % 10,
							        temp_setpoint_channel1 < 0 ? "-" : "", abs(temp_setpoint_channel1)/ 10, abs(temp_setpoint_channel1) % 10,
							        temp_setpoint_channel2 < 0 ? "-" : "", abs(temp_setpoint_channel2)/ 10, abs(temp_setpoint_channel2) % 10,
							        temp_setpoint_channel3 < 0 ? "-" : "", abs(temp_setpoint_channel3)/ 10, abs(temp_setpoint_channel3) % 10,
							        voltage_out_tec_channel0 / 100, voltage_out_tec_channel0 % 100,
							        voltage_out_tec_channel1 / 100, voltage_out_tec_channel1 % 100,
							        voltage_out_tec_channel2 / 100, voltage_out_tec_channel2 % 100,
							        voltage_out_tec_channel3 / 100, voltage_out_tec_channel3 % 100,
							        neo_led_r, neo_led_g, neo_led_b, neo_led_w,
							        ir_led_duty);
							if (uart_choose_uart5) {
								Uart_sendstring(UART5, buffer_0x13);
							}
							Uart_sendstring(USART6, buffer_0x13);



						}
						break;

						default:
							clear_send_flag();
							break;
					}
				default:
					clear_send_flag();
					return FSP_PKT_INVALID;
					break;
			}
			clear_send_flag();
			break;
		default:
			clear_send_flag();
			return FSP_PKT_WRONG_ADR;
			break;

	}
	return 0;

}
//	switch (fsp_pkt->type)
//	{
//		case FSP_PKT_TYPE_DATA:
//
//			Uart_sendstring(UART5, "\nDATA: ");
//
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(UART5, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(UART5, "\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_DATA_WITH_ACK:
//
//			Uart_sendstring(UART5, "\nDATA ACK:");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(UART5, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(UART5, "\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_CMD:
//
//			Uart_sendstring(UART5, "\nCMD: ");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(UART5, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(UART5, "\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_CMD_WITH_ACK:
//
//			Uart_sendstring(UART5, "\nCMD ACK:");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(UART5, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(UART5, "\r\n>");
//			break;
//		case FSP_PKT_TYPE_ACK:
//
//    		clear_send_flag();
//    		Uart_sendstring(UART5, "\nACK\r\n> ");
//
//
//			break;
//		case FSP_PKT_TYPE_NACK:
//
//    		Uart_sendstring(UART5, "\nNACK\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_CMD_W_DATA:
//
//    		Uart_sendstring(UART5, "\nDATA CMD: ");
//    		char buffer[10];
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      sprintf(buffer, "\n{%d}", fsp_pkt->payload[i]);
//			      Uart_sendstring(UART5, buffer);
//			}
//
//			Uart_sendstring(UART5, "\r\n >");
//			break;
//		case FSP_PKT_TYPE_CMD_W_DATA_ACK:
//
//    		Uart_sendstring(UART5, "\nDATA CMD ACK: ");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(UART5, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(UART5, "\r\n >");
//			break;
//
//		default:
//
//			Uart_sendstring(UART5, "\nDEFAULT");
//			Uart_sendstring(UART5, "\r\n >");
//
//			break;
//
//
//	}
//
//
//
//	return 0;
//}
