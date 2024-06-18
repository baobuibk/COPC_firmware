/*
 * iou.h
 *
 *  Created on: May 26, 2024
 *      Author: CAO HIEU
 */
/*

Start - CPOC - IOU - Length - Type -    Payload     -    [Crc1] - [Crc2]

 {7E}   {00}  {05}   {03}     {04}     {02}{01}{01}       {2A}     {52} get_temp 1 1
 {7E}   {01}  {05}   {03}     {04}     {02}{01}{01}       {92}     {33}

 {7E}   {00}  {05}   {04}     {04}   {01}{01}{00}{01}     {A7}     {A5} set_temp 1 1

 {7E}   {00}  {05}   {02}     {04}      {03}{01}          {57}     {8D} get_temp_setpoint 1

 {7E}   {00}  {05}   {02}     {01}      {04}{01}          {25}     {EA} tec_ena 1

 {7E}   {00}  {05}   {02}     {01}      {05}{02}          {26}     {B8} tec_dis 2

ACK:
 {7E}   {05}  {01}   {00}     {05}                        {DB}     {D0}

Lưu ý 1: Không dùng CMD được, cmd define trong .h là của nó nên ->
ta dùng data packet để decode cái packet của mình (cmd là 1 packet luôn)
vậy dùng: FSP_PKT_TYPE_DATA_WITH_ACK

    if (ack == FSP_PKT_WITH_ACK) FSP_PKT_WITH_ACK -> Được định nghĩa là  0  hoặc 1 để so sánh thôi, không dùng kiểu đó (ý là truyền tham số vào gen packet)phải dùng kiểu này: FSP_PKT_TYPE_DATA_WITH_ACK
    {
        fsp_gen_pkt(data, data_len, dst_adr, FSP_PKT_TYPE_DATA_WITH_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt(data, data_len, dst_adr, FSP_PKT_TYPE_DATA, fsp);
    }

B A

:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU


 */

#ifndef CMDLINE_CMD_IOU_IOU_CMD_H_
#define CMDLINE_CMD_IOU_IOU_CMD_H_


#define CMD_CODE_SET_TEMP							0x01
#define CMD_CODE_GET_TEMP							0x02
#define CMD_CODE_TEMP_SETPOINT						0x03
#define CMD_CODE_TEC_ENA							0x04
#define CMD_CODE_TEC_DIS							0x05
#define CMD_CODE_TEC_ENA_AUTO						0x06
#define CMD_CODE_TEC_DIS_AUTO						0x07
#define CMD_CODE_TEC_SET_OUTPUT						0x08
#define CMD_CODE_TEC_AUTO_VOL						0x09
#define CMD_CODE_TEC_STATUS							0x0A
#define CMD_CODE_TEC_LOG_ENA						0x0B
#define CMD_CODE_TEC_LOG_DIS						0x0C
#define CMD_CODE_RINGLED_SETRGB	    				0x0D
#define CMD_CODE_RINGLED_GETRGB 					0x0E
#define CMD_CODE_IRLED_SET_BRIGHT					0x0F
#define CMD_CODE_IRLED_GET_BRIGHT					0x10
#define CMD_CODE_GET_ACCEL_GYRO						0x11
#define CMD_CODE_GET_PRESS						 	0x12
#define CMD_CODE_GET_PARAMETERS		   				0x13

int Cmd_iou_set_temp(int argc, char *argv[]);
int Cmd_iou_get_temp(int argc, char *argv[]);
int Cmd_iou_temp_setpoint(int argc, char *argv[]);
int Cmd_iou_tec_ena(int argc, char *argv[]);
int Cmd_iou_tec_dis(int argc, char *argv[]);
int Cmd_iou_tec_ena_auto(int argc, char *argv[]);
int Cmd_iou_tec_dis_auto(int argc, char *argv[]);
int Cmd_iou_tec_set_output(int argc, char *argv[]);
int Cmd_iou_tec_auto_vol(int argc, char *argv[]);
int Cmd_iou_tec_status(int argc, char *argv[]);
int Cmd_iou_tec_log_ena(int argc, char *argv[]);
int Cmd_iou_tec_log_dis(int argc, char *argv[]);
int Cmd_iou_ringled_setRGB(int argc, char *argv[]);
int Cmd_iou_ringled_getRGB(int argc, char *argv[]);
int Cmd_iou_irled_set_bright(int argc, char *argv[]);
int Cmd_iou_irled_get_bright(int argc, char *argv[]);
int Cmd_iou_get_accel(int argc, char *argv[]);
int Cmd_iou_get_press(int argc, char *argv[]);
int Cmd_iou_get_parameters(int argc, char *argv[]);


void IOU_create_task(void);

int send_packet_and_wait_for_ack(uint8_t *encoded_pkt, uint8_t encoded_len, uint32_t timeout_ms);

#endif /* CMDLINE_CMD_IOU_IOU_CMD_H_ */
