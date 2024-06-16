/*
 * rs422.h
 *
 *  Created on: Jun 16, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_RS422_H_
#define CMDLINE_RS422_H_
#include "../../ThirdParty/libfsp/fsp.h"
void	rs422_create_task(void);
void 	frame_processing_rs422(fsp_packet_t *fsp_pkt);

#endif /* CMDLINE_RS422_H_ */
