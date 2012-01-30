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
#include "open9xinterface.h"
#include "open9xeeprom.h"
#include "open9xsimulator.h"
#include "open9xv4simulator.h"
#include "file.h"

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2

/// fileId of general file
#define FILE_GENERAL   0
/// convert model number 0..MAX_MODELS-1  int fileId
#define FILE_MODEL(n) (1+n)
#define FILE_TMP      (1+16)

Open9xInterface::Open9xInterface(int size):
efile(new EFile()),
size(size)
{
}

Open9xInterface::~Open9xInterface()
{
  delete efile;
}

const char * Open9xInterface::getName()
{
  if (size == 2048)
    return "Open9x";
  else
    return "Open9x v4";
}

const int Open9xInterface::getEEpromSize() {
    return size;
}

template <class T>
void Open9xInterface::loadModel(ModelData &model, unsigned int stickMode)
{
  T _model;
  if (efile->readRlc2((uint8_t*)&_model, sizeof(T))) {
    model = _model;
    if (stickMode) {
      applyStickModeToModel(model, stickMode);
    }
  }
  else {
    model.clear();
  }
}

template <class T>
bool Open9xInterface::loadGeneral(GeneralSettings &settings)
{
  T _settings;
  if (efile->readRlc2((uint8_t*)&_settings, sizeof(T))) {
    settings = _settings;
    return true;
  }

  std::cerr << "error when loading general settings";
  return false;
}

bool Open9xInterface::load(RadioData &radioData, uint8_t *eeprom, int size)
{
  std::cout << "trying open9x " << this->size << " import... ";

  if (size != this->size) {
    std::cout << "wrong size\n";
    return false;
  }

  efile->EeFsInit(eeprom, size);
  efile->openRd(FILE_GENERAL);
  
  uint8_t version;
  if (efile->readRlc2(&version, 1) != 1) {
    std::cout << "no\n";
    return false;
  }

  std::cout << "version " << (unsigned int)version << " ";

  switch(version) {
    case 201:
      // first version
      break;
    case 202:
      // channel order is now always RUD - ELE - THR - AIL
      // changes in timers
      // ppmFrameLength added
      // thrTraceSrc added
      break;
    default:
      std::cout << "not open9x\n";
      return false;
  }

  efile->openRd(FILE_GENERAL);
  if (1/*version == 201 || version == 202*/) {
    if (!loadGeneral<Open9xGeneral_v201>(radioData.generalSettings)) {
      std::cout << "ko\n";
      return false;
    }
  }
  
  for (int i=0; i<MAX_MODELS; i++) {
    efile->openRd(FILE_MODEL(i));
    if (version == 201) {
      loadModel<Open9xModelData_v201>(radioData.models[i], radioData.generalSettings.stickMode+1);
    }
    else if (version == 202) {
      loadModel<Open9xModelData_v202>(radioData.models[i], 0 /*no more stick mode messed*/);
    }
    else {
      std::cout << "ko\n";
      return false;
    }
  }

  std::cout << "ok\n";
  return true;
}

int Open9xInterface::save(uint8_t *eeprom, RadioData &radioData)
{
  EEPROMWarnings.clear();

  efile->EeFsInit(eeprom, size, true);

  Open9xGeneral open9xGeneral(radioData.generalSettings);
  int sz = efile->writeRlc2(FILE_TMP, FILE_TYP_GENERAL, (uint8_t*)&open9xGeneral, sizeof(Open9xGeneral));
  if(sz != sizeof(Open9xGeneral)) {
    return 0;
  }
  efile->swap(FILE_GENERAL, FILE_TMP);

  for (int i=0; i<MAX_MODELS; i++) {
    if (!radioData.models[i].isempty()) {
      Open9xModelData open9xModel(radioData.models[i]);
      sz = efile->writeRlc2(FILE_TMP, FILE_TYP_MODEL, (uint8_t*)&open9xModel, sizeof(Open9xModelData));
      if(sz != sizeof(Open9xModelData)) {
        return 0;
      }
      efile->swap(FILE_MODEL(i), FILE_TMP);
    }
  }

  return size;
}

int Open9xInterface::getSize(ModelData &model)
{
  if (model.isempty())
    return 0;

  uint8_t tmp[EESIZE_V4];
  efile->EeFsInit(tmp, EESIZE_V4, true);

  Open9xModelData open9xModel(model);
  int sz = efile->writeRlc2(FILE_TMP, FILE_TYP_MODEL, (uint8_t*)&open9xModel, sizeof(Open9xModelData));
  if(sz != sizeof(Open9xModelData)) {
     return -1;
  }
  return efile->size(FILE_TMP);
}

int Open9xInterface::getSize(GeneralSettings &settings)
{
  uint8_t tmp[EESIZE_V4];
  efile->EeFsInit(tmp, EESIZE_V4, true);

  Open9xGeneral open9xGeneral(settings);
  int sz = efile->writeRlc1(FILE_TMP, FILE_TYP_GENERAL, (uint8_t*)&open9xGeneral, sizeof(Open9xGeneral));
  if(sz != sizeof(open9xGeneral)) {
    return -1;
  }
  return efile->size(FILE_TMP);
}

int Open9xInterface::getCapability(const Capability capability)
{
  switch (capability) {
    case OwnerName:
      return 0;
    case PPMExtCtrl:
      return 1;      
    case Phases:
      return 4;
    case Timers:
      return 2;
    case FuncSwitches:
      return 12;
    case Outputs:
      return 16;
    case ExtraChannels:
      return 8;
    case ExtendedTrims:
      return 500;
    case Simulation:
      return 1;
    case DSM2Indexes:
      return 1;
    case Telemetry:
      return 7;
    default:
      return 0;
  }
}

int Open9xInterface::hasProtocol(Protocol proto)
{
  switch (proto) {
    case PPM:
    case DSM2:
    case PXX:
    case PPM16:
      return 1;
    default:
      return 0;
  }
}


SimulatorInterface * Open9xInterface::getSimulator()
{
  if (size == 2048)
    return new Open9xSimulator(this);
  else
    return new Open9xV4Simulator(this);
}
