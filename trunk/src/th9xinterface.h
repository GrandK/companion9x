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
#ifndef th9x_interface_h
#define th9x_interface_h

#include "eeprominterface.h"

class EFile;

class Th9xInterface : public EEPROMInterface
{
  public:

    Th9xInterface();

    virtual ~Th9xInterface();

    virtual bool load(RadioData &, uint8_t *eeprom, int size);

    virtual int save(uint8_t *eeprom, RadioData &radioData);

    virtual int getSize(ModelData &);

    virtual int getCapability(const Capability);

  protected:

    EFile *efile;

};

#endif
