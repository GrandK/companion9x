#define SIMU
#define SIMU_EXCEPTIONS
#define PCBSKY9X
#define PCBSKY
#define STAMP
#define FRSKY
#define FRSKY_HUB
#define WS_HOW_HIGH
#define TEMPLATES
#define HELI
#define HAPTIC
#define AUDIO
#define REVB

#undef min
#undef max

#ifndef __GNUC__
#include "../winbuild/winbuild.h"
#endif

#include <exception>

namespace Ersky9x {

//#include "../ersky9x/simpgmspace.h"
#include "../ersky9x/timers.h"
#include "../ersky9x/ersky9x.cpp"
#include "../ersky9x/stamp.cpp"
#include "../ersky9x/menus.cpp"
#include "../ersky9x/pers.cpp"
#include "../ersky9x/file.cpp"
#include "../ersky9x/lcd.cpp"
#include "../ersky9x/drivers.cpp"
#include "../ersky9x/logicio.cpp"
#include "../ersky9x/timers.cpp"
#include "../ersky9x/simpgmspace.cpp"
#include "../ersky9x/templates.cpp"
#include "../ersky9x/frsky.cpp"
#include "../ersky9x/audio.cpp"
#include "../ersky9x/sound.cpp"
#include "../ersky9x/diskio.cpp"

//uint8_t pxxFlag = 0;
int16_t g_anas[7];

uint16_t anaIn(uint8_t chan)
{
  if (chan == 7)
    return 150;
  else
    return g_anas[chan];
}

bool hasExtendedTrims()
{
  return false;
}

bool getSwitch(int8_t swtch, bool nc)
{
  return getSwitch(swtch, nc, 0);
}

void setTrim(uint8_t idx, int8_t value)
{
  g_model.trim[idx] = value;
}

void getTrims(int16_t values[4])
{
  for (int i=0; i<4; i++)
    values[i] = g_model.trim[i];
}

}

