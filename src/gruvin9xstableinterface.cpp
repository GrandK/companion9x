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

#include <iostream>
#include "gruvin9xstableinterface.h"
#include "gruvin9xeeprom.h"
#include "gruvin9xstablesimulator.h"
#include "file.h"

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2

/// fileId of general file
#define FILE_GENERAL   0
/// convert model number 0..MAX_MODELS-1  int fileId
#define FILE_MODEL(n) (1+n)
#define FILE_TMP      (1+16)

Gruvin9xStableInterface::Gruvin9xStableInterface():
efile(new EFile())
{
}

Gruvin9xStableInterface::~Gruvin9xStableInterface()
{
  delete efile;
}

const char * Gruvin9xStableInterface::getName()
{
  return "Gruvin9x stable";
}

const int Gruvin9xStableInterface::getEEpromSize()
{
  QSettings settings("companion9x", "companion9x");
  QString avrMCU = settings.value("mcu", QString("m64")).toString();
  if (avrMCU==QString("m128")) {
    return EESIZE_STOCK*2;
  }
  return EESIZE_STOCK;
}

const int Gruvin9xStableInterface::getMaxModels()
{
  return 16;
}

bool Gruvin9xStableInterface::loadxml(RadioData &radioData, QDomDocument &doc)
{
  return false;
}

bool Gruvin9xStableInterface::load(RadioData &radioData, uint8_t *eeprom, int size)
{
  return false;
}

int Gruvin9xStableInterface::save(uint8_t *eeprom, RadioData &radioData)
{
  EEPROMWarnings.clear();

  efile->EeFsCreate(eeprom, getEEpromSize(), 4);

  Gruvin9xGeneral gruvin9xGeneral(radioData.generalSettings);
  int sz = efile->writeRlc2(FILE_TMP, FILE_TYP_GENERAL, (uint8_t*)&gruvin9xGeneral, sizeof(Gruvin9xGeneral));
  if(sz != sizeof(Gruvin9xGeneral)) {
    return 0;
  }
  efile->swap(FILE_GENERAL, FILE_TMP);

  for (int i=0; i<getMaxModels(); i++) {
    if (!radioData.models[i].isempty()) {
      Gruvin9xModelData gruvin9xModel(radioData.models[i]);
      sz = efile->writeRlc2(FILE_TMP, FILE_TYP_MODEL, (uint8_t*)&gruvin9xModel, sizeof(Gruvin9xModelData));
      if(sz != sizeof(Gruvin9xModelData)) {
        return 0;
      }
      efile->swap(FILE_MODEL(i), FILE_TMP);
    }
  }

  return getEEpromSize();
}

int Gruvin9xStableInterface::getSize(ModelData &model)
{
  if (model.isempty())
    return 0;

  uint8_t tmp[EESIZE_GRUVIN9X];
  efile->EeFsCreate(tmp, EESIZE_GRUVIN9X, 4);

  Gruvin9xModelData gruvin9xModel(model);
  int sz = efile->writeRlc2(FILE_TMP, FILE_TYP_MODEL, (uint8_t*)&gruvin9xModel, sizeof(Gruvin9xModelData));
  if(sz != sizeof(Gruvin9xModelData)) {
     return -1;
  }
  return efile->size(FILE_TMP);
}

int Gruvin9xStableInterface::getSize(GeneralSettings &settings)
{
  uint8_t tmp[EESIZE_GRUVIN9X];
  efile->EeFsCreate(tmp, EESIZE_GRUVIN9X, 4);

  Gruvin9xGeneral gruvin9xGeneral(settings);
  int sz = efile->writeRlc1(FILE_TMP, FILE_TYP_GENERAL, (uint8_t*)&gruvin9xGeneral, sizeof(gruvin9xGeneral));
  if(sz != sizeof(gruvin9xGeneral)) {
    return -1;
  }
  return efile->size(FILE_TMP);
}

int Gruvin9xStableInterface::getCapability(const Capability capability)
{
  switch (capability) {
    case OwnerName:
      return 0;
    case FlightPhases:
      return G9X_MAX_PHASES;
    case Mixes:
      return G9X_MAX_MIXERS;
    case Timers:
      return 2;
    case FuncSwitches:
      return 12;
    case CustomSwitches:
      return 12;
    case Outputs:
      return 16;
    case ExtraChannels:
      return 8;
    case ExtendedTrims:
      return 500;
    case Simulation:
      return true;
    case HasExpoCurves:
      return true;
    case Telemetry:
      return TM_HASTELEMETRY;
    case TelemetryMaxMultiplier:
      return 1;
    default:
      return false;
  }
}

int Gruvin9xStableInterface::isAvailable(Protocol proto)
{
  switch (proto) {
    case PPM:
    case SILV_A:
    case SILV_B:
    case SILV_C:
    case CTP1009:
      return 1;
    default:
      return 0;
  }
}

SimulatorInterface * Gruvin9xStableInterface::getSimulator()
{
  return new Gruvin9xStableSimulator(this);
}