#ifndef __OTHER_H
#define __OTHER_H

#include "including.h"

void TCPS_Config(void);

void TCP_KeepAlive(void);
void TCP_DealData(u8 *str,u16 len);
void TCP_SendFire(void);

void DealCAN(CanRxMsg* RxMessage);
void DealActuator(void);

#endif