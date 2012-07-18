#ifndef HELPERS_H
#define HELPERS_H

#include <QtGui>
#include <unistd.h>
#include "eeprominterface.h"

#define TMR_NUM_OPTION  (TMR_VAROFS+2*9+2*GetEepromInterface()->getCapability(CustomSwitches)-1)

//convert from mode 1 to mode g_eeGeneral.stickMode
//NOTICE!  =>  1..4 -> 1..4
#define CONVERT_MODE(x) (((x)<=4) ? modn12x3[g_eeGeneral.stickMode][((x)-1)] : (x))
#define CHANNEL_ORDER(x) (chout_ar[g_eeGeneral.templateSetup*4 + (x)-1])

#define CH(x) (SRC_CH1+(x)-1-(SRC_SWC-SRC_3POS))
#define CV(x) (CURVE_BASE+(x)-1)
#define CC(x) (CHANNEL_ORDER(x)) //need to invert this to work with dest

#define CURVE5(x) ((x)-1)
#define CURVE9(x) (MAX_CURVE5+(x)-1)

#define TELEMETRY_SRC "----""TMR1""TMR2""A1  ""A2  ""Tx  ""Rx  ""Alt ""Rpm ""Fuel""T1  ""T2  ""Spd ""Dist""GAlt""Cell""Volt""Curr""Cnsp""Powr""AccX""AccY""AccZ""HDG ""VSpd""A1- ""A2- ""Alt-""Alt+""Rpm+""T1+ ""T2+ ""Spd+""Dst+""Cur+""ACC ""Time"

#define POPULATE_ONOFF      1
#define POPULATE_MSWITCHES  2
void populateSwitchCB(QComboBox *b, const RawSwitch & value, unsigned long attr=0, UseContext context=DefaultContext);
void populateFuncCB(QComboBox *b, unsigned int value);
QString FuncParam(uint function, unsigned int value);
void populateFuncParamCB(QComboBox *b, uint function, unsigned int value);
void populatePhasesCB(QComboBox *b, int value);
void populateTrimUseCB(QComboBox *b, unsigned int phase);
void populateCurvesCB(QComboBox *b, int value);
void populatecsFieldCB(QComboBox *b, int value, bool last, int hubproto);
void populateExpoCurvesCB(QComboBox *b, int value);
void populateTimerSwitchCB(QComboBox *b, int value);

#define POPULATE_SWITCHES  1
#define POPULATE_TELEMETRY 2
#define POPULATE_TRIMS     4

void populateSourceCB(QComboBox *b, const RawSource &source, unsigned int flags);
void populateCSWCB(QComboBox *b, int value);
QString getTimerMode(int tm);
QString getPhaseName(int val);
QString getStickStr(int index);
QString getCSWFunc(int val);
QString getFuncName(unsigned int val);
QString getSignedStr(int value);
QString getCurveStr(int curve);
QString image2qstring(QImage image);
QImage qstring2image(QString imagestr);
int findmult(float value, float base);
bool checkbit(int value, int bit);
#endif // HELPERS_H