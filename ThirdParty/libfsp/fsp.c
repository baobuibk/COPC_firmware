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

char pos_str2[10];

int frame_processing(fsp_packet_t *fsp_pkt){


	switch (fsp_pkt->src_adr){
		case FSP_ADR_PMU:
			switch (fsp_pkt->type)
			{
				case FSP_PKT_TYPE_ACK:
					clear_send_flag();
					Uart_sendstring(USART6, "\nPMU_ACK\r\n> ");

				case FSP_PKT_TYPE_CMD_W_DATA:
					//reverse
					clear_send_flag();
					switch(fsp_pkt->payload[0])
					{
						case 0x00:
							char buffer_0x00[50];
							sprintf(buffer_0x00, "PMU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							Uart_sendstring(USART6, buffer_0x00);
							break;
						case 0xFF:
							char buffer_0xFF[50];
							sprintf(buffer_0xFF, "PMU_Failed: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							Uart_sendstring(USART6, buffer_0xFF);
							break;

						case 0x01:
					        int16_t ntc0 = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
					        int16_t ntc1 = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
					        int16_t ntc2 = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
					        int16_t ntc3 = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

					        char buffer_0x01[100];
					        sprintf(buffer_0x01, "PMU_Res: CMDcode 0x01 [NTC0: %d, NTC1: %d, NTC2: %d, NTC3: %d]\n", ntc0, ntc1, ntc2, ntc3);
					        Uart_sendstring(USART6, buffer_0x01);

							break;

						case 0x02:
					        uint16_t bat0 = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
					        uint16_t bat1 = (uint16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
					        uint16_t bat2 = (uint16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
					        uint16_t bat3 = (uint16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

					        char buffer_0x02[100];
					        sprintf(buffer_0x02, "PDU_Res: CMDcode 0x02 [BAT0: %u mV, BAT1: %u mV, BAT2: %u mV, BAT3: %u mV]\n", bat0, bat1, bat2, bat3);
					        Uart_sendstring(USART6, buffer_0x02);

							break;
						case 0x03:
					        uint16_t vin = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
					        uint16_t iin = (uint16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);

					        char buffer_0x03[100];
					        sprintf(buffer_0x03, "PDU_Res: CMDcode 0x03 [VIN: %u mV, IIN: %u mA]\n", vin, iin);
					        Uart_sendstring(USART6, buffer_0x03);

							break;
						case 0x04:
					        uint16_t vout = (uint16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
					        uint16_t iout = (uint16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);

					        char buffer_0x04_pmu[100];
					        sprintf(buffer_0x04_pmu, "PDU_Res: CMDcode 0x04 [VOUT: %u mV, IOUT: %u mA]\n", vout, iout);
					        Uart_sendstring(USART6, buffer_0x04_pmu);

							break;
						case 0x08:


							break;
					}

					break;
				default:
					return FSP_PKT_INVALID;
					break;
			}

			break;
		case FSP_ADR_PDU:
			switch (fsp_pkt->type)
			{
				case FSP_PKT_TYPE_ACK:
					clear_send_flag();
					Uart_sendstring(USART6, "\n> PDU_ACK\r\n> ");
					break;
				case FSP_PKT_TYPE_CMD_W_DATA:
					//reverse
					clear_send_flag();
					switch(fsp_pkt->payload[0])
					{
						case 0x00:
							char buffer_0x00[50];
							sprintf(buffer_0x00, "PDU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							Uart_sendstring(USART6, buffer_0x00);
							break;
						case 0xFF:
							char buffer_0xFF[50];
							sprintf(buffer_0xFF, "PDU_Failed: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
							Uart_sendstring(USART6, buffer_0xFF);
							break;

						case 0x04:

							        uint8_t channel = fsp_pkt->payload[1];
							        uint8_t status_0x04 = fsp_pkt->payload[2];
							        uint16_t voltage_0x04 = (fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4];
							        uint16_t current_0x04 = (fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6];

							        char buffer_0x07[100];
							        sprintf(buffer_0x07, "PDU_Res: CMDcode 0x04 [{Channel %u} Status %u, Voltage: %u, Current: %u]\n", channel, status_0x04, voltage_0x04, current_0x04);
							        Uart_sendstring(USART6, buffer_0x07);
									break;
						case 0x05:
						            uint8_t buck = fsp_pkt->payload[1];
						            uint8_t status_0x05 = fsp_pkt->payload[2];
						            uint16_t voltage_0x05 = (fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4];
						            uint16_t current_0x05 = (fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6];

						            char buffer_0x05[100];
						            sprintf(buffer_0x05, "PDU_Res: CMDcode 0x05 [{Buck %u} Status %u, Voltage: %u, Current: %u]\n", buck, status_0x05, voltage_0x05, current_0x05);
						            Uart_sendstring(USART6, buffer_0x05);

									break;
						case 0x06:
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
						            sprintf(buffer_0x06, "TEC1BUCK: Status %u, Voltage: %u\r\nTEC2BUCK: Status %u, Voltage: %u\r\nTEC3BUCK: Status %u, Voltage: %u\r\nTEC4BUCK: Status %u, Voltage: %u\r\nMCUBUCK: Status %u, Voltage: %u\r\nLEDBUCK: Status %u, Voltage: %u\r\nCM4BUCK: Status %u, Voltage: %u\r\nTEC1: Status %u, Current: %u\r\nTEC2: Status %u, Current: %u\r\nTEC3: Status %u, Current: %u\r\nTEC4: Status %u, Current: %u\r\nCOPC: Status %u, Current: %u\r\nIOU: Status %u, Current: %u\r\nRGB: Status %u, Current: %u\r\nIR: Status %u, Current: %u\r\nCM4: Status %u, Current: %u\r\nVIN: Status %u, Voltage: %u\r\nVBUS: Status %u, Voltage: %u\r\n",
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

						            Uart_sendstring(USART6, buffer_0x06);




						default:
							return FSP_PKT_INVALID;
							break;

					}


//					if (fsp_pkt->payload[0] == 0) {
//						char buffer[50];
//						sprintf(buffer, "PDU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
//						Uart_sendstring(USART6, buffer);
//					} else {
//					    for (int i = 0; i < fsp_pkt->length; i++) {
//					        Uart_write(USART6, fsp_pkt->payload[i]);
//					    }
//					}


//										    		char buffer[10];
//													for (int i = 0; i < fsp_pkt->length; i++) {
//													      sprintf(buffer, "\n{%d}", fsp_pkt->payload[i]);
//													      Uart_sendstring(USART6, buffer);
//													}


					break;
				default:
					return FSP_PKT_INVALID;
					break;
			}

			break;
		case FSP_ADR_CAM:

			break;

		case FSP_ADR_IOU:
			switch (fsp_pkt->type)
			{
				case FSP_PKT_TYPE_ACK:
					clear_send_flag();
					Uart_sendstring(USART6, "\nIOU_ACK\r\n> ");
					break;
				case FSP_PKT_TYPE_CMD_W_DATA:

					if (fsp_pkt->payload[0] == 0) {
					    printf("IOU_Done: CMDcode 0x%02X\n", fsp_pkt->payload[1]);
					} else {
					    for (int i = 0; i < fsp_pkt->length; i++) {
					      //  Uart_write(USART6, fsp_pkt->payload[i]);
					    			char lmao[20];
					 			    sprintf(lmao, "\n{%d}", fsp_pkt->payload[i]);
					     		    Uart_sendstring(USART6, lmao);
					    }
					}

					break;
				default:
					return FSP_PKT_INVALID;
					break;
			}

			break;
		default:
			return FSP_PKT_WRONG_ADR;
			break;

	}
	return 0;

}
//	switch (fsp_pkt->type)
//	{
//		case FSP_PKT_TYPE_DATA:
//
//			Uart_sendstring(USART6, "\nDATA: ");
//
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(USART6, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(USART6, "\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_DATA_WITH_ACK:
//
//			Uart_sendstring(USART6, "\nDATA ACK:");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(USART6, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(USART6, "\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_CMD:
//
//			Uart_sendstring(USART6, "\nCMD: ");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(USART6, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(USART6, "\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_CMD_WITH_ACK:
//
//			Uart_sendstring(USART6, "\nCMD ACK:");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(USART6, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(USART6, "\r\n>");
//			break;
//		case FSP_PKT_TYPE_ACK:
//
//    		clear_send_flag();
//    		Uart_sendstring(USART6, "\nACK\r\n> ");
//
//
//			break;
//		case FSP_PKT_TYPE_NACK:
//
//    		Uart_sendstring(USART6, "\nNACK\r\n> ");
//
//			break;
//		case FSP_PKT_TYPE_CMD_W_DATA:
//
//    		Uart_sendstring(USART6, "\nDATA CMD: ");
//    		char buffer[10];
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      sprintf(buffer, "\n{%d}", fsp_pkt->payload[i]);
//			      Uart_sendstring(USART6, buffer);
//			}
//
//			Uart_sendstring(USART6, "\r\n >");
//			break;
//		case FSP_PKT_TYPE_CMD_W_DATA_ACK:
//
//    		Uart_sendstring(USART6, "\nDATA CMD ACK: ");
//			for (int i = 0; i < fsp_pkt->length; i++) {
//			      Uart_write(USART6, fsp_pkt->payload[i]);
//			}
//
//			Uart_sendstring(USART6, "\r\n >");
//			break;
//
//		default:
//
//			Uart_sendstring(USART6, "\nDEFAULT");
//			Uart_sendstring(USART6, "\r\n >");
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
