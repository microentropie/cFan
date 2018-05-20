#ifndef _IR_RECEIVE_H_
#define _IR_RECEIVE_H_

#ifndef uint64_t
#include <stdint.h>
#endif //uint64_t

void IrRxStart();
uint64_t IrRxLoop();
void IrRxStop();

#endif //_IR_RECEIVE_H_
