/*
 * Author - Bertrand Songis <bsongis@gmail.com>
 * 
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "open9xSky9xsimulator.h"
#include "open9xinterface.h"

#define SIMU
#define SIMU_EXCEPTIONS
#define PCBSKY9X
#define CPUARM
#define REVB
#define ROTARY_ENCODERS 1
#define HELI
#define TEMPLATES
#define SPLASH
#define FLIGHT_MODES
#define FRSKY
#define FRSKY_HUB
#define WS_HOW_HIGH
#define VARIO
#define PPM_UNIT_PERCENT_PREC1
#define BUZZER
#define AUDIO
#define VOICE
#define HAPTIC
#define PXX
#define DSM2
#define DSM2_PPM
#define DBLKEYS
#define AUTOSWITCH
#define GRAPHICS
#define SDCARD
#define CURVES
#define XCURVES
#define GVARS
#define BOLD_FONT
#define PPM_CENTER_ADJUSTABLE
#define PPM_LIMITS_SYMETRICAL
#define GAUGES
#define GPS
#define FAI_CHOICE

#define EEPROM_VARIANT 3

#undef min
#undef max

#ifndef __GNUC__
#include "../winbuild/winbuild.h"
#endif

#include <exception>

namespace Open9xSky9x {
#include "../opentx/targets/sky9x/board_sky9x.cpp"
#include "../opentx/protocols/ppm_arm.cpp"
#include "../opentx/protocols/pxx_arm.cpp"
#include "../opentx/protocols/dsm2_arm.cpp"  
#include "../opentx/targets/sky9x/pwr_driver.cpp"
#include "../opentx/targets/sky9x/eeprom_driver.cpp"
#include "../opentx/eeprom_common.cpp"
#include "../opentx/eeprom_raw.cpp"
#include "../opentx/eeprom_conversions.cpp"
#include "../opentx/opentx.cpp"
#include "../opentx/targets/sky9x/pulses_driver.cpp"
#include "../opentx/protocols/pulses_arm.cpp"
#include "../opentx/stamp.cpp"
#include "../opentx/maths.cpp"
#include "../opentx/gui/menus.cpp"
#include "../opentx/gui/menu_model.cpp"
#include "../opentx/gui/menu_general.cpp"
#include "../opentx/gui/view_main.cpp"
#include "../opentx/gui/view_statistics.cpp"
#include "../opentx/gui/view_telemetry.cpp"
#include "../opentx/gui/view_about.cpp"
#include "../opentx/lcd.cpp"
#include "../opentx/logs.cpp"
#include "../opentx/targets/sky9x/keys_driver.cpp"
#include "../opentx/keys.cpp"
#include "../opentx/simpgmspace.cpp"
#include "../opentx/templates.cpp"
#include "../opentx/translations.cpp"
#include "../opentx/telemetry/frsky.cpp"
#include "../opentx/targets/sky9x/audio_driver.cpp"
#include "../opentx/audio_arm.cpp"
#include "../opentx/buzzer.cpp"
#include "../opentx/targets/sky9x/sdcard_driver.cpp"
#include "../opentx/targets/sky9x/coproc_driver.cpp"
#include "../opentx/targets/sky9x/haptic_driver.cpp"
#include "../opentx/haptic.cpp"
#include "../opentx/translations/tts_cz.cpp"
#include "../opentx/translations/tts_de.cpp"
#include "../opentx/translations/tts_en.cpp"
#include "../opentx/translations/tts_es.cpp"
#include "../opentx/translations/tts_se.cpp"
#include "../opentx/translations/tts_it.cpp"
#include "../opentx/translations/tts_fr.cpp"
#include "../opentx/translations/tts_pt.cpp"
#include "../opentx/translations/tts_sk.cpp"
#include "../opentx/translations/tts_pl.cpp"

int16_t g_anas[NUM_STICKS+BOARD_9X_NUM_POTS];

uint16_t anaIn(uint8_t chan)
{
  if (chan == 7)
    return 1500;
  else
    return g_anas[chan];
}

bool hasExtendedTrims()
{
  return g_model.extendedTrims;
}

uint8_t getStickMode()
{
  return g_eeGeneral.stickMode;
}

}

using namespace Open9xSky9x;

Open9xSky9xSimulator::Open9xSky9xSimulator(Open9xInterface * open9xInterface):
  open9xInterface(open9xInterface)
{
    QSettings settings("companion9x", "companion9x");
    QString path=settings.value("sdPath", ".").toString()+"/";
    int i=0;
    for (i=0; i< std::min(path.length(),1022); i++) {
      simuSdDirectory[i]=path.at(i).toAscii();
    }
    simuSdDirectory[i]=0;  
}

bool Open9xSky9xSimulator::timer10ms()
{
#define TIMER10MS_IMPORT
#include "simulatorimport.h"
}

uint8_t * Open9xSky9xSimulator::getLcd()
{
#define GETLCD_IMPORT
#include "simulatorimport.h"
}

bool Open9xSky9xSimulator::lcdChanged(bool & lightEnable)
{
#define LCDCHANGED_IMPORT
#include "simulatorimport.h"
}

void Open9xSky9xSimulator::start(RadioData &radioData, bool tests)
{
  g_rotenc[0] = 0;
  open9xInterface->save(&eeprom[0], radioData);
  StartEepromThread(NULL);
  StartMainThread(tests);
}

void Open9xSky9xSimulator::stop()
{
  StopMainThread();
  StopEepromThread();
}

void Open9xSky9xSimulator::getValues(TxOutputs &outputs)
{
#define GETVALUES_IMPORT
#define g_chans512 channelOutputs
#include "simulatorimport.h"
  outputs.beep = g_beepCnt;
  g_beepCnt = 0;
}

void Open9xSky9xSimulator::setValues(TxInputs &inputs)
{
#define SETVALUES_IMPORT
#include "simulatorimport.h"
}

void Open9xSky9xSimulator::setTrim(unsigned int idx, int value)
{
  idx = Open9xSky9x::modn12x3[4*getStickMode() + idx] - 1;
  uint8_t phase = getTrimFlightPhase(getFlightPhase(), idx);
  setTrimValue(phase, idx, value);
}

void Open9xSky9xSimulator::getTrims(Trims & trims)
{
  uint8_t phase = getFlightPhase();
  trims.extended = hasExtendedTrims();
  for (uint8_t idx=0; idx<4; idx++) {
    trims.values[idx] = getTrimValue(getTrimFlightPhase(phase, idx), idx);
  }

  for (int i=0; i<2; i++) {
    uint8_t idx = Open9xSky9x::modn12x3[4*getStickMode() + i] - 1;
    int16_t tmp = trims.values[i];
    trims.values[i] = trims.values[idx];
    trims.values[idx] = tmp;
  }
}

void Open9xSky9xSimulator::wheelEvent(uint8_t steps)
{
  g_rotenc[0] += steps*4;
}

unsigned int Open9xSky9xSimulator::getPhase()
{
  return getFlightPhase();
}

const char * Open9xSky9xSimulator::getError()
{
#define GETERROR_IMPORT
#include "simulatorimport.h"
}
