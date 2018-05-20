#ifndef _IR_MODEL_H_
#define _IR_MODEL_H_

#ifndef uint64_t
#include <stdint.h>
#endif //uint64_t

struct IrCommandsSingle
{
  uint64_t SetOff;
  uint64_t SetOn;
  uint64_t ToggleOnOff;
};

struct IrConfigModel
{
  IrCommandsSingle fan;
  IrCommandsSingle light;
  uint64_t WiFiForceOn;
  bool irOn : 1;
  unsigned char dummy : 7;
};

#endif //_IR_MODEL_H_
