#ifndef _IR_BL_H_
#define _IR_BL_H_

extern void IrInit();
extern void IrResume();
extern void IrLoop();

#include "Ir_model.h"
extern bool IrConfig_Save(IrConfigModel &cfg);
extern bool IrConfig_Load(IrConfigModel &cfg);

#endif //_IR_BL_H_
