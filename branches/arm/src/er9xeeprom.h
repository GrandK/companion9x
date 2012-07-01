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
#ifndef er9xeeprom_h
#define er9xeeprom_h

#include <inttypes.h>
#include "eeprominterface.h"

//eeprom data
#define ER9X_MAX_MIXERS  32
#define MAX_CURVE5  8
#define MAX_CURVE9  8

#define MDVERS      10

#define ER9X_NUM_CHNOUT      16 //number of real outputchannels CH1-CH8
#define ER9X_NUM_CSW         12 //number of custom switches
#define NUM_STICKSnPOTS 7  //number of sticks and pots

PACK(typedef struct t_Er9xTrainerMix {
  uint8_t srcChn:3; //0-7 = ch1-8
  int8_t  swtch:5;
  int8_t  studWeight:6;
  uint8_t mode:2;   // off, add-mode, subst-mode

  operator TrainerMix();
  t_Er9xTrainerMix();
  t_Er9xTrainerMix(TrainerMix&);

}) Er9xTrainerMix; //

PACK(typedef struct t_Er9xTrainerData {
  int16_t        calib[4];
  Er9xTrainerMix mix[4];

  operator TrainerData();
  t_Er9xTrainerData();
  t_Er9xTrainerData(TrainerData&);

}) Er9xTrainerData;

PACK(typedef struct t_Er9xGeneral {
  uint8_t   myVers;
  int16_t   calibMid[NUM_STICKSnPOTS];
  int16_t   calibSpanNeg[NUM_STICKSnPOTS];
  int16_t   calibSpanPos[NUM_STICKSnPOTS];
  uint16_t  chkSum;
  uint8_t   currModel; //0..15
  uint8_t   contrast;
  uint8_t   vBatWarn;
  int8_t    vBatCalib;
  int8_t    lightSw;
  Er9xTrainerData trainer;
  uint8_t   view;
  uint8_t   disableThrottleWarning:1;
  uint8_t   disableSwitchWarning:1;
  uint8_t   disableMemoryWarning:1;
  uint8_t   beeperVal:3;
  uint8_t   reserveWarning:1;
  uint8_t   disableAlarmWarning:1;
  uint8_t   stickMode;
  int8_t    inactivityTimer;
  uint8_t   throttleReversed:1;
  uint8_t   minuteBeep:1;
  uint8_t   preBeep:1;
  uint8_t   flashBeep:1;
  uint8_t   disableSplashScreen:1;
  uint8_t   disablePotScroll:1;
  uint8_t   disableBG:1;
  uint8_t   res1:1;
  uint8_t   filterInput;
  uint8_t   lightAutoOff;
  uint8_t   templateSetup;  //RETA order according to chout_ar array
  int8_t    PPM_Multiplier;
  //uint8_t   respre[2]; //mike please check these are correct
  uint8_t   FRSkyYellow:4;
  uint8_t   FRSkyOrange:4;
  uint8_t   FRSkyRed:4;  //mike please check these are correct
  uint8_t   hideNameOnSplash:1;
  uint8_t   spare:3; 
  uint8_t   speakerPitch;
  uint8_t   hapticStrength;
  uint8_t   speakerMode;
  uint8_t   lightOnStickMove;

  char      ownerName[10];
  uint8_t switchWarningStates;

  operator GeneralSettings();
  t_Er9xGeneral();
  t_Er9xGeneral(GeneralSettings&);

}) Er9xGeneral;

PACK(typedef struct t_Er9xExpoData {
  int8_t  expo[3][2][2];
  int8_t  drSw1;
  int8_t  drSw2;

  t_Er9xExpoData() { memset(this, 0, sizeof(t_Er9xExpoData)); }
}) Er9xExpoData;


PACK(typedef struct t_Er9xLimitData {
  int8_t  min;
  int8_t  max;
  bool    revert;
  int16_t  offset;

  operator LimitData();
  t_Er9xLimitData();
  t_Er9xLimitData(LimitData&);
}) Er9xLimitData;

#define MLTPX_ADD  0
#define MLTPX_MUL  1
#define MLTPX_REP  2

PACK(typedef struct t_Er9xMixData {
  uint8_t destCh;            //        1..NUM_CHNOUT
  uint8_t srcRaw;            //
  int8_t  weight;
  int8_t  swtch;
  uint8_t curve;             //0=symmetrisch 1=no neg 2=no pos
  uint8_t delayUp:4;
  uint8_t delayDown:4;
  uint8_t speedUp:4;         // Servogeschwindigkeit aus Tabelle (10ms Cycle)
  uint8_t speedDown:4;       // 0 nichts
  uint8_t carryTrim:1;
  uint8_t mltpx:3;           // multiplex method 0=+ 1=* 2=replace
  uint8_t mixWarn:2;         // mixer warning
  uint8_t enableFmTrim:1;
  uint8_t mixres:1;
  int8_t  sOffset;
  int8_t  res;

  operator MixData();
  t_Er9xMixData();
  t_Er9xMixData(MixData&);
}) Er9xMixData;


PACK(typedef struct t_Er9xCustomSwData { // Custom Switches data
  int8_t  v1; //input
  int8_t  v2; //offset
  uint8_t func;

  operator CustomSwData();
  t_Er9xCustomSwData() { memset(this, 0, sizeof(t_Er9xCustomSwData)); }
  t_Er9xCustomSwData(CustomSwData&);
}) Er9xCustomSwData;

PACK(typedef struct t_Er9xSafetySwData { // Custom Switches data
  int8_t  swtch;
  int8_t  val;

  operator SafetySwData();
  t_Er9xSafetySwData();
  t_Er9xSafetySwData(SafetySwData&);
}) Er9xSafetySwData;

PACK(typedef struct t_Er9xFrSkyChannelData {
  uint8_t   ratio;                // 0.0 means not used, 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_value[2];      // 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_level:4;
  uint8_t   alarms_greater:2;     // 0=LT(<), 1=GT(>)
  uint8_t   type:2;               // future use: 0=volts, ...

  operator FrSkyChannelData();
  t_Er9xFrSkyChannelData();
  t_Er9xFrSkyChannelData(FrSkyChannelData&);
}) Er9xFrSkyChannelData;

PACK(typedef struct t_Er9xFrSkyData {
  Er9xFrSkyChannelData channels[2];

  operator FrSkyData();
  t_Er9xFrSkyData();
  t_Er9xFrSkyData(FrSkyData&);
}) Er9xFrSkyData;

PACK(typedef struct t_Er9xModelData {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   mdVers;
  int8_t    tmrMode;              // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint8_t   tmrDir:1;    //0=>Count Down, 1=>Count Up
  uint8_t   traineron:1;  // 0 disable trainer, 1 allow trainer
  uint8_t   t2throttle:1 ;  // Start timer2 using throttle
  uint8_t   FrSkyUsrProto:2 ;  // Protocol in FrSky User Data, 0=FrSky Hub, 1=WS HowHigh
  uint8_t   FrSkyImperial:1 ;  // Convert FrSky values to imperial units
  uint8_t   FrSkyAltAlarm:2;
  uint16_t  tmrVal;
  uint8_t   protocol;
  int8_t    ppmNCH;
  int8_t    thrTrim:4;            // Enable Throttle Trim
  int8_t    thrExpo:4;            // Enable Throttle Expo
  int8_t    trimInc;              // Trim Increments
  int8_t    ppmDelay;
  int8_t    trimSw;
  uint8_t   beepANACenter;        // 1<<0->A1.. 1<<6->A7
  uint8_t   pulsePol:1;
  uint8_t   extendedLimits:1;
  uint8_t   swashInvertELE:1;
  uint8_t   swashInvertAIL:1;
  uint8_t   swashInvertCOL:1;
  uint8_t   swashType:3;
  uint8_t   swashCollectiveSource;
  uint8_t   swashRingValue;
  int8_t    ppmFrameLength;
  Er9xMixData   mixData[ER9X_MAX_MIXERS];
  Er9xLimitData limitData[ER9X_NUM_CHNOUT];
  Er9xExpoData  expoData[4];
  int8_t    trim[4];
  int8_t    curves5[MAX_CURVE5][5];
  int8_t    curves9[MAX_CURVE9][9];
  Er9xCustomSwData   customSw[ER9X_NUM_CSW];
  uint8_t   frSkyVoltThreshold ;
  uint8_t   res3[2];
  Er9xSafetySwData  safetySw[ER9X_NUM_CHNOUT];
  Er9xFrSkyData frsky;
  operator ModelData();
  t_Er9xModelData() { memset(this, 0, sizeof(t_Er9xModelData)); }
  t_Er9xModelData(ModelData&);
}) Er9xModelData;


#endif
/*eof*/