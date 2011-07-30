#include "simulatordialog.h"
#include "ui_simulatordialog.h"
#include "node.h"
#include <QtGui>
#include <inttypes.h>
#include "eeprominterface.h"
#include "helpers.h"

#define GBALL_SIZE  20

#define RESX    1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

#define IS_THROTTLE(x)  (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4))
#define GET_DR_STATE(x) (!getSwitch(g_model.expoData[x].drSw1,0) ?   \
                          DR_HIGH :                                  \
                          !getSwitch(g_model.expoData[x].drSw2,0)?   \
                          DR_MID : DR_LOW);



simulatorDialog::simulatorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::simulatorDialog)
{
    ui->setupUi(this);

    beepVal = 0;
    beepShow = 0;

    bpanaCenter = 0;
    g_tmr10ms = 0;

    memset(&chanOut,0,sizeof(chanOut));
    memset(&calibratedStick,0,sizeof(calibratedStick));
    memset(&g_ppmIns,0,sizeof(g_ppmIns));
    memset(&ex_chans,0,sizeof(ex_chans));
    memset(&trim,0,sizeof(trim));

    memset(&sDelay,0,sizeof(sDelay));
    memset(&act,0,sizeof(act));

    memset(&anas,0,sizeof(anas));
    memset(&chans,0,sizeof(chans));

    memset(&swOn,0,sizeof(swOn));

    setupSticks();
    setupTimer();
}

simulatorDialog::~simulatorDialog()
{
    delete ui;
}

void simulatorDialog::closeEvent (QCloseEvent*)
{
    timer->stop();
    delete timer;
}

void simulatorDialog::setupTimer()
{
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimerEvent()));
    getValues();
    perOut(true);
    timer->start(10);
}

void simulatorDialog::onTimerEvent()
{
    g_tmr10ms++;

    getValues();

    perOut();

    setValues();
    centerSticks();

    timerTick();
//    if(s_timerState != TMR_OFF)
        setWindowTitle(modelName + QString(" - Timer: (%3, %4) %1:%2")
                       .arg(abs(-s_timerVal)/60, 2, 10, QChar('0'))
                       .arg(abs(-s_timerVal)%60, 2, 10, QChar('0'))
                       .arg(getTimerMode(g_model.timers[0].mode)) // TODO why timers[0]
                       .arg(g_model.timers[0].dir ? "Count Up" : "Count Down"));

    if(beepVal)
    {
        beepVal = 0;
        QApplication::beep();
    }


#define CBEEP_ON  "QLabel { background-color: #FF364E }"
#define CBEEP_OFF "QLabel { }"

    ui->label_beep->setStyleSheet(beepShow ? CBEEP_ON : CBEEP_OFF);
    if(beepShow) beepShow--;
}

void simulatorDialog::centerSticks()
{
    if(ui->leftStick->scene()) nodeLeft->stepToCenter();
    if(ui->rightStick->scene()) nodeRight->stepToCenter();
}

void simulatorDialog::loadParams(const GeneralSettings &gg, const ModelData &gm)
{
    g_eeGeneral = gg;
    g_model = gm;
   
    modelName = tr("Simulating ") + gm.name;
    setWindowTitle(modelName);

    if(g_eeGeneral.stickMode & 1)
    {
        nodeLeft->setCenteringY(false);   //mode 1,3 -> THR on left
        ui->holdLeftY->setChecked(true);
    }
    else
    {
        nodeRight->setCenteringY(false);   //mode 1,3 -> THR on right
        ui->holdRightY->setChecked(true);
    }


    ui->trimHLeft->setValue(g_model.phaseData[0].trim[0]);
    ui->trimVLeft->setValue(g_model.phaseData[0].trim[1]);
    ui->trimVRight->setValue(g_model.phaseData[0].trim[2]);
    ui->trimHRight->setValue(g_model.phaseData[0].trim[3]);

    beepVal = 0;
    beepShow = 0;
    bpanaCenter = 0;
    g_tmr10ms = 0;

    s_timeCumTot = 0;
    s_timeCumAbs = 0;
    s_timeCumSw = 0;
    s_timeCumThr = 0;
    s_timeCum16ThrP = 0;
    s_timerState = 0;
    beepAgain = 0;
    g_LightOffCounter = 0;
    s_timerVal = 0;
    s_time = 0;
    s_cnt = 0;
    s_sum = 0;
    sw_toggled = 0;
}

void simulatorDialog::getValues()
{

    calibratedStick[0] = 1024*nodeLeft->getX(); //RUD
    calibratedStick[1] = -1024*nodeLeft->getY(); //ELE
    calibratedStick[2] = -1024*nodeRight->getY(); //THR
    calibratedStick[3] = 1024*nodeRight->getX(); //AIL

    trim[0] = ui->trimHLeft->value();
    trim[1] = ui->trimVLeft->value();
    trim[2] = ui->trimVRight->value();
    trim[3] = ui->trimHRight->value();

    calibratedStick[4] = ui->dialP_1->value();
    calibratedStick[5] = ui->dialP_2->value();
    calibratedStick[6] = ui->dialP_3->value();

    if(g_eeGeneral.throttleReversed)
    {
        calibratedStick[THR_STICK] *= -1;
        trim[THR_STICK] *= -1;
    }
}

inline int chVal(int val)
{
    return qMin(1024, qMax(-1024, val));
}

void simulatorDialog::setValues()
{
    ui->chnout_1->setValue(chVal(chanOut[0]));
    ui->chnout_2->setValue(chVal(chanOut[1]));
    ui->chnout_3->setValue(chVal(chanOut[2]));
    ui->chnout_4->setValue(chVal(chanOut[3]));
    ui->chnout_5->setValue(chVal(chanOut[4]));
    ui->chnout_6->setValue(chVal(chanOut[5]));
    ui->chnout_7->setValue(chVal(chanOut[6]));
    ui->chnout_8->setValue(chVal(chanOut[7]));
    ui->chnout_9->setValue(chVal(chanOut[8]));
    ui->chnout_10->setValue(chVal(chanOut[9]));
    ui->chnout_11->setValue(chVal(chanOut[10]));
    ui->chnout_12->setValue(chVal(chanOut[11]));
    ui->chnout_13->setValue(chVal(chanOut[12]));
    ui->chnout_14->setValue(chVal(chanOut[13]));
    ui->chnout_15->setValue(chVal(chanOut[14]));
    ui->chnout_16->setValue(chVal(chanOut[15]));

    ui->chnoutV_1->setText(QString("%1").arg((qreal)chanOut[0]*100/1024, 0, 'f', 1));
    ui->chnoutV_2->setText(QString("%1").arg((qreal)chanOut[1]*100/1024, 0, 'f', 1));
    ui->chnoutV_3->setText(QString("%1").arg((qreal)chanOut[2]*100/1024, 0, 'f', 1));
    ui->chnoutV_4->setText(QString("%1").arg((qreal)chanOut[3]*100/1024, 0, 'f', 1));
    ui->chnoutV_5->setText(QString("%1").arg((qreal)chanOut[4]*100/1024, 0, 'f', 1));
    ui->chnoutV_6->setText(QString("%1").arg((qreal)chanOut[5]*100/1024, 0, 'f', 1));
    ui->chnoutV_7->setText(QString("%1").arg((qreal)chanOut[6]*100/1024, 0, 'f', 1));
    ui->chnoutV_8->setText(QString("%1").arg((qreal)chanOut[7]*100/1024, 0, 'f', 1));
    ui->chnoutV_9->setText(QString("%1").arg((qreal)chanOut[8]*100/1024, 0, 'f', 1));
    ui->chnoutV_10->setText(QString("%1").arg((qreal)chanOut[9]*100/1024, 0, 'f', 1));
    ui->chnoutV_11->setText(QString("%1").arg((qreal)chanOut[10]*100/1024, 0, 'f', 1));
    ui->chnoutV_12->setText(QString("%1").arg((qreal)chanOut[11]*100/1024, 0, 'f', 1));
    ui->chnoutV_13->setText(QString("%1").arg((qreal)chanOut[12]*100/1024, 0, 'f', 1));
    ui->chnoutV_14->setText(QString("%1").arg((qreal)chanOut[13]*100/1024, 0, 'f', 1));
    ui->chnoutV_15->setText(QString("%1").arg((qreal)chanOut[14]*100/1024, 0, 'f', 1));
    ui->chnoutV_16->setText(QString("%1").arg((qreal)chanOut[15]*100/1024, 0, 'f', 1));

    ui->leftXPerc->setText(QString("X %1\%").arg((qreal)nodeLeft->getX()*100, 2, 'f', 0));
    ui->leftYPerc->setText(QString("Y %1\%").arg((qreal)nodeLeft->getY()*-100, 2, 'f', 0));

    ui->rightXPerc->setText(QString("X %1\%").arg((qreal)nodeRight->getX()*100, 2, 'f', 0));
    ui->rightYPerc->setText(QString("Y %1\%").arg((qreal)nodeRight->getY()*-100, 2, 'f', 0));

#define CSWITCH_ON  "QLabel { background-color: #4CC417 }"
#define CSWITCH_OFF "QLabel { }"

    ui->labelCSW_1->setStyleSheet(getSwitch(DSW_SW1,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_2->setStyleSheet(getSwitch(DSW_SW2,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_3->setStyleSheet(getSwitch(DSW_SW3,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_4->setStyleSheet(getSwitch(DSW_SW4,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_5->setStyleSheet(getSwitch(DSW_SW5,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_6->setStyleSheet(getSwitch(DSW_SW6,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_7->setStyleSheet(getSwitch(DSW_SW7,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_8->setStyleSheet(getSwitch(DSW_SW8,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_9->setStyleSheet(getSwitch(DSW_SW9,0)   ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_10->setStyleSheet(getSwitch(DSW_SWA,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_11->setStyleSheet(getSwitch(DSW_SWB,0)  ? CSWITCH_ON : CSWITCH_OFF);
    ui->labelCSW_12->setStyleSheet(getSwitch(DSW_SWC,0)  ? CSWITCH_ON : CSWITCH_OFF);
}

void simulatorDialog::beepWarn1()
{
    beepVal = 1;
    beepShow = 20;
}

void simulatorDialog::beepWarn2()
{
    beepVal = 1;
    beepShow = 20;
}

void simulatorDialog::beepWarn()
{
    beepVal = 1;
    beepShow = 20;
}

void simulatorDialog::setupSticks()
{
    QGraphicsScene *leftScene = new QGraphicsScene(ui->leftStick);
    leftScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    ui->leftStick->setScene(leftScene);

    // ui->leftStick->scene()->addLine(0,10,20,30);

    QGraphicsScene *rightScene = new QGraphicsScene(ui->rightStick);
    rightScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    ui->rightStick->setScene(rightScene);

    // ui->rightStick->scene()->addLine(0,10,20,30);

    nodeLeft = new Node();
    nodeLeft->setPos(-GBALL_SIZE/2,-GBALL_SIZE/2);
    nodeLeft->setBallSize(GBALL_SIZE);
    leftScene->addItem(nodeLeft);

    nodeRight = new Node();
    nodeRight->setPos(-GBALL_SIZE/2,-GBALL_SIZE/2);
    nodeRight->setBallSize(GBALL_SIZE);
    rightScene->addItem(nodeRight);
}

void simulatorDialog::resizeEvent(QResizeEvent *event)
{

    if(ui->leftStick->scene())
    {
        QRect qr = ui->leftStick->contentsRect();
        qreal w  = (qreal)qr.width()  - GBALL_SIZE;
        qreal h  = (qreal)qr.height() - GBALL_SIZE;
        qreal cx = (qreal)qr.width()/2;
        qreal cy = (qreal)qr.height()/2;
        ui->leftStick->scene()->setSceneRect(-cx,-cy,w,h);

        QPointF p = nodeLeft->pos();
        p.setX(qMin(cx, qMax(p.x(), -cx)));
        p.setY(qMin(cy, qMax(p.y(), -cy)));
        nodeLeft->setPos(p);
    }

    if(ui->rightStick->scene())
    {
        QRect qr = ui->rightStick->contentsRect();
        qreal w  = (qreal)qr.width()  - GBALL_SIZE;
        qreal h  = (qreal)qr.height() - GBALL_SIZE;
        qreal cx = (qreal)qr.width()/2;
        qreal cy = (qreal)qr.height()/2;
        ui->rightStick->scene()->setSceneRect(-cx,-cy,w,h);

        QPointF p = nodeRight->pos();
        p.setX(qMin(cx, qMax(p.x(), -cx)));
        p.setY(qMin(cy, qMax(p.y(), -cy)));
        nodeRight->setPos(p);
    }
    QDialog::resizeEvent(event);
}


inline qint16 calc100toRESX(qint8 x)
{
  return (qint16)x*10 + x/4 - x/64;
}

inline qint16 calc1000toRESX(qint16 x)
{
  return x + x/32 - x/128 + x/512;
}


bool simulatorDialog::keyState(EnumKeys key)
{
    switch (key)
    {
    case (SW_ThrCt):   return ui->switchTHR->isChecked(); break;
    case (SW_RuddDR):  return ui->switchRUD->isChecked(); break;
    case (SW_ElevDR):  return ui->switchELE->isChecked(); break;
    case (SW_ID0):     return ui->switchID0->isChecked(); break;
    case (SW_ID1):     return ui->switchID1->isChecked(); break;
    case (SW_ID2):     return ui->switchID2->isChecked(); break;
    case (SW_AileDR):  return ui->switchAIL->isChecked(); break;
    case (SW_Gear):    return ui->switchGEA->isChecked(); break;
    case (SW_Trainer): return ui->switchTRN->isDown(); break;
    default:
        return false;
        break;
    }
}

int simulatorDialog::getValue(qint8 i)
{
  if(i<NUM_STICKS+NUM_POTS) return calibratedStick[i];//-512..512
  else if(i<MIX_FULL/*srcRaw is shifted +1!*/) return 1024; //FULL/MAX
  else if(i<PPM_BASE+NUM_CAL_PPM) return (g_ppmIns[i-PPM_BASE] - g_eeGeneral.trainer.calib[i-PPM_BASE])*2;
  else if(i<PPM_BASE+NUM_PPM) return g_ppmIns[i-PPM_BASE]*2;
  else if(i<CHOUT_BASE+NUM_CHNOUT) return ex_chans[i-CHOUT_BASE];
// TODO   else if(i<CHOUT_BASE+NUM_CHNOUT+NUM_TELEMETRY) return frskyTelemetry[i-CHOUT_BASE-NUM_CHNOUT].value;
  else return 0;
}

unsigned int simulatorDialog::getFlightPhase()
{
  for (uint8_t i=1; i<MAX_PHASES; i++) {
    PhaseData *phase = &g_model.phaseData[i];
    if (phase->swtch && getSwitch(phase->swtch, 0)) {
      return i;
    }
  }
  return 0;
}

unsigned int simulatorDialog::getTrimFlightPhase(uint8_t idx, int8_t phase)
{
  if (phase == -1) phase = getFlightPhase();
  if (phase == 0) return phase;
  int8_t trim = g_model.phaseData[phase].trim[idx];
  if (trim > 125) return 0;
  if (trim >= -125) return phase;
  uint8_t result = 129 + trim;
  if (result == phase) result++;
  return result;
}

bool simulatorDialog::getSwitch(int swtch, bool nc, qint8 level)
{
    if(level>5) return false; //prevent recursive loop going too deep

    switch(swtch){
       case  0:            return  nc;
       case  MAX_DRSWITCH: return  true;
       case -MAX_DRSWITCH: return  false;
     }

     uint8_t dir = swtch>0;
     if(abs(swtch)<(MAX_DRSWITCH-NUM_CSW)) {
       if(!dir) return ! keyState((EnumKeys)(SW_BASE-swtch-1));
       return            keyState((EnumKeys)(SW_BASE+swtch-1));
     }

     //custom switch, Issue 78
     //use putsChnRaw
     //input -> 1..4 -> sticks,  5..8 pots
     //MAX,FULL - disregard
     //ppm
     CustomSwData &cs = g_model.customSw[abs(swtch)-(MAX_DRSWITCH-NUM_CSW)];
     if(!cs.func) return false;


     int8_t a = cs.v1;
     int8_t b = cs.v2;
     int16_t x = 0;
     int16_t y = 0;

     // init values only if needed
     uint8_t s = CS_STATE(cs.func);
     if(s == CS_VOFS)
     {
         x = getValue(cs.v1-1);
         y = calc100toRESX(cs.v2);
     }
     else if(s == CS_VCOMP)
     {
         x = getValue(cs.v1-1);
         y = getValue(cs.v2-1);
     }

     switch (cs.func) {
     case (CS_VPOS):
         return swtch>0 ? (x>y) : !(x>y);
         break;
     case (CS_VNEG):
         return swtch>0 ? (x<y) : !(x<y);
         break;
     case (CS_APOS):
         return swtch>0 ? (abs(x)>y) : !(abs(x)>y);
         break;
     case (CS_ANEG):
         return swtch>0 ? (abs(x)<y) : !(abs(x)<y);
         break;

     case (CS_AND):
         return (getSwitch(a,0,level+1) && getSwitch(b,0,level+1));
         break;
     case (CS_OR):
         return (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
         break;
     case (CS_XOR):
         return (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
         break;

     case (CS_EQUAL):
         return (x==y);
         break;
     case (CS_NEQUAL):
         return (x!=y);
         break;
     case (CS_GREATER):
         return (x>y);
         break;
     case (CS_LESS):
         return (x<y);
         break;
     case (CS_EGREATER):
         return (x>=y);
         break;
     case (CS_ELESS):
         return (x<=y);
         break;
     default:
         return false;
         break;
     }
}


uint16_t expou(uint16_t x, uint16_t k)
{
    // k*x*x*x + (1-k)*x
    return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;
}
// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]

int16_t expo(int16_t x, int16_t k)
{
    if(k == 0) return x;
    int16_t   y;
    bool    neg =  x < 0;
    if(neg)   x = -x;
    if(k<0){
        y = RESXu-expou(RESXu-x,-k);
    }else{
        y = expou(x,k);
    }
    return neg? -y:y;
}

uint16_t isqrt32(uint32_t n)
{
    uint16_t c = 0x8000;
    uint16_t g = 0x8000;

    for(;;) {
        if((uint32_t)g*g > n)
            g ^= c;
        c >>= 1;
        if(c == 0)
            return g;
        g |= c;
    }
}

int16_t simulatorDialog::intpol(int16_t x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
#define D9 (RESX * 2 / 8)
#define D5 (RESX * 2 / 4)
  bool    cv9 = idx >= MAX_CURVE5;
  int8_t *crv = cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx];
  int16_t erg;

  x+=RESXu;
  if(x < 0) {
    erg = (int16_t)crv[0] * (RESX/4);
  } else if(x >= (RESX*2)) {
    erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
  } else {
    int16_t a,dx;
    if(cv9){
      a   = (uint16_t)x / D9;
      dx  =((uint16_t)x % D9) * 2;
    } else {
      a   = (uint16_t)x / D5;
      dx  = (uint16_t)x % D5;
    }
    erg  = (int16_t)crv[a]*((D5-dx)/2) + (int16_t)crv[a+1]*(dx/2);
  }
  return erg / 25; // 100*D5/RESX;
}

void simulatorDialog::timerTick()
{
    int16_t val = 0;
  if ((abs(g_model.timers[0].mode) > 1) && (abs(g_model.timers[0].mode)
      < TMR_VAROFS)) {
    val = calibratedStick[CONVERT_MODE(abs(g_model.timers[0].mode)/2) - 1];
    val = (g_model.timers[0].mode < 0 ? RESX - val : val + RESX) / (RESX / 16); // only used for %
  }

  int8_t tm = g_model.timers[0].mode;

  if (abs(tm) >= (TMR_VAROFS + MAX_DRSWITCH - 1)) { //toggeled switch//abs(g_model.timers[0].mode)<(10+MAX_DRSWITCH-1)
    static uint8_t lastSwPos;
    if (!(sw_toggled | s_sum | s_cnt | s_time | lastSwPos)) lastSwPos = tm < 0; // if initializing then init the lastSwPos
    uint8_t swPos = getSwitch(tm > 0 ? tm - (TMR_VAROFS + MAX_DRSWITCH - 1 - 1)
        : tm + (TMR_VAROFS + MAX_DRSWITCH - 1 - 1), 0);
    if (swPos && !lastSwPos) sw_toggled = !sw_toggled; //if switcdh is flipped first time -> change counter state
    lastSwPos = swPos;
  }

  s_time++;
  if (s_time < 100) return; //1 sec
  s_time = 0;

  if (abs(tm) < TMR_VAROFS)
    sw_toggled = false; // not switch - sw timer off
  else if (abs(tm) < (TMR_VAROFS + MAX_DRSWITCH - 1)) sw_toggled = getSwitch(
      (tm > 0 ? tm - (TMR_VAROFS - 1) : tm + (TMR_VAROFS - 1)), 0); //normal switch

  s_timeCumTot += 1;
  s_timeCumAbs += 1;
  if (val) s_timeCumThr += 1;
  if (sw_toggled) s_timeCumSw += 1;
  s_timeCum16ThrP += val / 2;

  s_timerVal = g_model.timers[0].val;
  uint8_t tmrM = abs(g_model.timers[0].mode);
  if (tmrM == TMRMODE_NONE)
    s_timerState = TMR_OFF;
  else if (tmrM == TMRMODE_ABS)
    s_timerVal -= s_timeCumAbs;
  else if (tmrM < TMR_VAROFS)
    s_timerVal -= (tmrM & 1) ? s_timeCum16ThrP / 16 : s_timeCumThr;// stick% : stick
  else
    s_timerVal -= s_timeCumSw; //switch

  switch (s_timerState) {
  case TMR_OFF:
    if (g_model.timers[0].mode != TMRMODE_NONE) s_timerState = TMR_RUNNING;
    break;
  case TMR_RUNNING:
    if (s_timerVal <= 0 && g_model.timers[0].val) s_timerState = TMR_BEEPING;
    break;
  case TMR_BEEPING:
    if (s_timerVal <= -MAX_ALERT_TIME) s_timerState = TMR_STOPPED;
    if (g_model.timers[0].val == 0) s_timerState = TMR_RUNNING;
    break;
  case TMR_STOPPED:
    break;
  }

  static int16_t last_tmr;

  if (last_tmr != s_timerVal) //beep only if seconds advance
  {
    if (s_timerState == TMR_RUNNING) {
      if (g_eeGeneral.preBeep && g_model.timers[0].val) // beep when 30, 15, 10, 5,4,3,2,1 seconds remaining
      {
        if (s_timerVal == 30) {
          beepAgain = 2;
          beepWarn2();
        } //beep three times
        if (s_timerVal == 20) {
          beepAgain = 1;
          beepWarn2();
        } //beep two times
        if (s_timerVal == 10) beepWarn2();
        if (s_timerVal <= 3) beepWarn2();

        if (g_eeGeneral.flashBeep && (s_timerVal == 30 || s_timerVal == 20
            || s_timerVal == 10 || s_timerVal <= 3)) g_LightOffCounter
            = FLASH_DURATION;
      }

      if (g_eeGeneral.minuteBeep && (((g_model.timers[0].dir ? g_model.timers[0].val
          - s_timerVal : s_timerVal) % 60) == 0)) //short beep every minute
      {
        beepWarn2();
        if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
      }
    }
    else if (s_timerState == TMR_BEEPING) {
      beepWarn();
      if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
    }
  }
  last_tmr = s_timerVal;
  if (g_model.timers[0].dir) s_timerVal = g_model.timers[0].val - s_timerVal; //if counting backwards - display backwards
}

int simulatorDialog::applyCurve(int16_t x, uint8_t idx, uint8_t srcRaw)
{
  switch(idx) {
  case 0:
    return x;
  case 1:
    if (srcRaw == MIX_FULL) { //FULL
      if (x<0 ) x=-RESX;   //x|x>0
      else x=-RESX+2*x;
    }
    else {
      if (x<0) x=0;   //x|x>0
    }
    return x;
  case 2:
    if (srcRaw == MIX_FULL) { //FULL
      if (x>0) x=RESX;   //x|x<0
      else x=RESX+2*x;
    }
    else {
      if (x>0) x=0;   //x|x<0
    }
    return x;
  case 3:       // x|abs(x)
    return abs(x);
  case 4:       //f|f>0
    return x>0 ? RESX : 0;
  case 5:       //f|f<0
    return x<0 ? -RESX : 0;
  case 6:       //f|abs(f)
    return x>0 ? RESX : -RESX;
  }
  return intpol(x, idx-7);
}

void simulatorDialog::applyExpos(int16_t *anas)
{
  static int16_t anas2[4]; // values before expo, to ensure same expo base when multiple expo lines are used
  memcpy(anas2, anas, sizeof(anas2));

  uint8_t phase = getFlightPhase();

  for (uint8_t i=0; i<MAX_EXPOS; i++) {
    ExpoData &ed = g_model.expoData[i];
    if (ed.mode==0) break; // end of list
    if (ed.phase != 0) {
      if (ed.phase < 0) {
        if (phase+1 == -ed.phase)
          continue;
      }
      else {
        if (phase+1 != ed.phase)
          continue;
      }
    }
    if (getSwitch(ed.swtch, 1)) {
      int16_t v = anas2[ed.chn];
      if((v<0 && ed.mode&1) || (v>=0 && ed.mode&2)) {
        int16_t k = ed.expo;
        if (IS_THROTTLE(i) && g_model.thrExpo)
          v = 2*expo((v+RESX)/2, k);
        else
          v = expo(v, k);
        if (ed.curve) v = applyCurve(v, ed.curve, 0);
        v = ((int32_t)v * ed.weight) / 100;
        if (IS_THROTTLE(i) && g_model.thrExpo) v -= RESX;
        anas[ed.chn] = v;
      }
    }
  }
}

void simulatorDialog::perOut(bool init)
{
  int16_t trimA[4];
  uint8_t  anaCenter = 0;
  uint16_t d = 0;

  //===========Swash Ring================
  if(g_model.swashRingData.value)
  {
      uint32_t v = (calibratedStick[ELE_STICK]*calibratedStick[ELE_STICK] +
                    calibratedStick[AIL_STICK]*calibratedStick[AIL_STICK]);
      uint32_t q = RESX*g_model.swashRingData.value/100;
      q *= q;
      if(v>q)
          d = isqrt32(v);
  }
  //===========Swash Ring================


  for(uint8_t i=0;i<7;i++){        // calc Sticks

    //Normalization  [0..2048] ->   [-1024..1024]

    int16_t v = calibratedStick[i];
//    v -= g_eeGeneral.calibMid[i];
//    v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
//                                     g_eeGeneral.calibSpanPos[i] :
//                                     g_eeGeneral.calibSpanNeg[i])));
//    if(v <= -RESX) v = -RESX;
//    if(v >=  RESX) v =  RESX;
//    calibratedStick[i] = v; //for show in expo

    if(!(v/16)) anaCenter |= 1<<(CONVERT_MODE((i+1))-1);

    //===========Swash Ring================
    if(d && (i==ELE_STICK || i==AIL_STICK))
        v = (int32_t)v*g_model.swashRingData.value*RESX/(d*100);
    //===========Swash Ring================

    anas[i] = v;
  }

  /* EXPOs */
  applyExpos(anas);

  /* TRIMs */
  for(uint8_t i=0; i<4; i++) {
      // do trim -> throttle trim if applicable
      int16_t v = anas[i];
      int32_t vv = 2*RESX;
      int8_t trim = g_model.phaseData[getTrimFlightPhase(i)].trim[i];
      if(IS_THROTTLE(i) && g_model.thrTrim) vv = (g_eeGeneral.throttleReversed) ?
                                 ((int32_t)trim-125)*(RESX+v)/(2*RESX) :
                                 ((int32_t)trim+125)*(RESX-v)/(2*RESX);

      //trim
      trimA[i] = (vv==2*RESX) ? (int16_t)trim*2 : (int16_t)vv*2; //    if throttle trim -> trim low end
  }

  //===========BEEP CENTER================
  anaCenter &= g_model.beepANACenter;
  if(((bpanaCenter ^ anaCenter) & anaCenter)) beepWarn1();
  bpanaCenter = anaCenter;


  calibratedStick[MIX_MAX-1]=calibratedStick[MIX_FULL-1]=1024;
  anas[MIX_MAX-1]  = RESX;     // MAX
  anas[MIX_FULL-1] = RESX;     // FULL
// TODO  for(uint8_t i=0;i<NUM_PPM;i++)    anas[i+PPM_BASE]   = g_ppmIns[i] - g_eeGeneral.ppmInCalib[i]; //add ppm channels
  for(uint8_t i=0;i<NUM_CHNOUT;i++) anas[i+CHOUT_BASE] = chans[i]; //other mixes previous outputs

#if 0 // TODO
  //===========Swash Mix================
#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

  if(g_model.swashType)
  {
      int16_t vp = anas[ELE_STICK]+trimA[ELE_STICK];
      int16_t vr = anas[AIL_STICK]+trimA[AIL_STICK];
      int16_t vc = 0;
      if(g_model.swashCollectiveSource)
          vc = anas[g_model.swashCollectiveSource-1];

      if(g_model.swashInvertELE) vp = -vp;
      if(g_model.swashInvertAIL) vr = -vr;
      if(g_model.swashInvertCOL) vc = -vc;

      switch (g_model.swashType)
      {
      case (SWASH_TYPE_120):
//          vp = REZ_SWASH_Y(vp);
//          vr = REZ_SWASH_X(vr);
          anas[MIX_CYC1-1] = vc - vp;
          anas[MIX_CYC2-1] = vc + vp/2 + vr;
          anas[MIX_CYC3-1] = vc + vp/2 - vr;
          break;
      case (SWASH_TYPE_120X):
//          vp = REZ_SWASH_X(vp);
//          vr = REZ_SWASH_Y(vr);
          anas[MIX_CYC1-1] = vc - vr;
          anas[MIX_CYC2-1] = vc + vr/2 + vp;
          anas[MIX_CYC3-1] = vc + vr/2 - vp;
          break;
      case (SWASH_TYPE_140):
//          vp = REZ_SWASH_Y(vp);
//          vr = REZ_SWASH_Y(vr);
          anas[MIX_CYC1-1] = vc - vp;
          anas[MIX_CYC2-1] = vc + vp + vr;
          anas[MIX_CYC3-1] = vc + vp - vr;
          break;
      case (SWASH_TYPE_90):
//          vp = REZ_SWASH_Y(vp);
//          vr = REZ_SWASH_Y(vr);
          anas[MIX_CYC1-1] = vc - vp;
          anas[MIX_CYC2-1] = vc + vr;
          anas[MIX_CYC3-1] = vc - vr;
          break;
      default:
          break;
      }

      calibratedStick[MIX_CYC1-1]=anas[MIX_CYC1-1];
      calibratedStick[MIX_CYC2-1]=anas[MIX_CYC2-1];
      calibratedStick[MIX_CYC3-1]=anas[MIX_CYC3-1];
  }
#endif

  memset(chans,0,sizeof(chans));        // All outputs to 0

   uint8_t mixWarning = 0;
    //========== MIXER LOOP ===============
    for(uint8_t i=0;i<MAX_MIXERS;i++){
      MixData &md = g_model.mixData[i];

      if((md.destCh==0) || (md.destCh>NUM_CHNOUT)) break;

      //Notice 0 = NC switch means not used -> always on line
      int16_t v  = 0;
      uint8_t swTog;

      //swOn[i]=false;
      if(!getSwitch(md.swtch,1)){ // switch on?  if no switch selected => on
        swTog = swOn[i];
        swOn[i] = false;
        if(md.srcRaw!=MIX_MAX && md.srcRaw!=MIX_FULL) continue;// if not MAX or FULL - next loop
        if(md.mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
        v = (md.srcRaw == MIX_FULL ? -RESX : 0); // switch is off and it is either MAX=0 or FULL=-512

        //if(md.srcRaw==MIX_MAX) continue;
      }
      else {
        swTog = !swOn[i];
        swOn[i] = true;
        uint8_t k = md.srcRaw-1;
        v = anas[k]; //Switch is on. MAX=FULL=512 or value.
        if(k>=CHOUT_BASE && (k<i)) v = chans[k];
        if(md.mixWarn) mixWarning |= 1<<(md.mixWarn-1); // Mix warning
      }

      //========== INPUT OFFSET ===============
      if(md.sOffset) v += calc100toRESX(md.sOffset);

      //========== DELAY and PAUSE ===============
      if (md.speedUp || md.speedDown || md.delayUp || md.delayDown)  // there are delay values
      {

#define DEL_MULT 256

        if(init) {
          act[i]=(int32_t)v*DEL_MULT;
          swTog = false;
        }
        int16_t diff = v-act[i]/DEL_MULT;

        if(swTog) {
            //need to know which "v" will give "anas".
            //curves(v)*weight/100 -> anas
            // v * weight / 100 = anas => anas*100/weight = v
          if(md.mltpx==MLTPX_REP)
          {
              act[i] = (int32_t)anas[md.destCh-1+CHOUT_BASE]*DEL_MULT;
              act[i] *=100;
              if(md.weight) act[i] /= md.weight;
          }
          diff = v-act[i]/DEL_MULT;
          if(diff) sDelay[i] = (diff<0 ? md.delayUp :  md.delayDown) * 100;
        }

        if(sDelay[i]){ // perform delay
            sDelay[i]--;
            v = act[i]/DEL_MULT;
            diff = 0;
        }

        if(diff && (md.speedUp || md.speedDown)){
          //rate = steps/sec => 32*1024/100*md.speedUp/Down
          //act[i] += diff>0 ? (32768)/((int16_t)100*md.speedUp) : -(32768)/((int16_t)100*md.speedDown);
          //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 since it's asymmetrical

            int32_t rate = (int32_t)DEL_MULT*2048*100;
            if(md.weight) rate /= abs(md.weight);
            act[i] = (diff>0) ? ((md.speedUp>0)   ? act[i]+(rate)/((int16_t)100*md.speedUp)   :  (int32_t)v*DEL_MULT) :
                     ((md.speedDown>0) ? act[i]-(rate)/((int16_t)100*md.speedDown) :  (int32_t)v*DEL_MULT) ;


          if(((diff>0) && (v<(act[i]/DEL_MULT))) || ((diff<0) && (v>(act[i]/DEL_MULT)))) act[i]=(int32_t)v*DEL_MULT; //deal with overflow
          v = act[i]/DEL_MULT;
        }
      }


      //========== CURVES ===============
      switch(md.curve){
        case 0:
          break;
        case 1:
          if(md.srcRaw == MIX_FULL) //FUL
          {
            if( v<0 ) v=-RESX;   //x|x>0
            else      v=-RESX+2*v;
          }else{
            if( v<0 ) v=0;   //x|x>0
          }
          break;
        case 2:
          if(md.srcRaw == MIX_FULL) //FUL
          {
            if( v>0 ) v=RESX;   //x|x<0
            else      v=RESX+2*v;
          }else{
            if( v>0 ) v=0;   //x|x<0
          }
          break;
        case 3:       // x|abs(x)
          v = abs(v);
          break;
        case 4:       //f|f>0
          v = v>0 ? RESX : 0;
          break;
        case 5:       //f|f<0
          v = v<0 ? -RESX : 0;
          break;
        case 6:       //f|abs(f)
          v = v>0 ? RESX : -RESX;
          break;
        default: //c1..c16
          v = intpol(v, md.curve - 7);
      }

      //========== TRIM ===============
      if((md.carryTrim==0) && (md.srcRaw>0) && (md.srcRaw<=4)) v += trimA[md.srcRaw-1];  //  0 = Trim ON  =  Default

      //========== MULTIPLEX ===============
      int32_t dv = (int32_t)v*md.weight;
      switch(md.mltpx){
        case MLTPX_REP:
          chans[md.destCh-1] = dv;
          break;
        case MLTPX_MUL:
          chans[md.destCh-1] *= dv/100l;
          chans[md.destCh-1] /= RESXl;
          break;
        default:  // MLTPX_ADD
          chans[md.destCh-1] += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
          break;
        }
    }


    //========== MIXER WARNING ===============
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
    if(mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) beepWarn1();
    if(mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) || ((g_tmr10ms&0xFF)== 72)) beepWarn1();
    if(mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) || ((g_tmr10ms&0xFF)==144)) beepWarn1();


  //========== LIMITS ===============
  for(uint8_t i=0;i<NUM_CHNOUT;i++){
    // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
    // later we multiply by the limit (up to 100) and then we need to normalize
    // at the end chans[i] = chans[i]/100 =>  -1024..1024
    // interpolate value with min/max so we get smooth motion from center to stop
    // this limits based on v original values and min=-1024, max=1024  RESX=1024


      int32_t q = chans[i];// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

      chans[i] /= 100; // chans back to -1024..1024
      ex_chans[i] = chans[i]; //for getswitch

      int16_t ofs = g_model.limitData[i].offset;
      int16_t lim_p = 10*(g_model.limitData[i].max+100);
      int16_t lim_n = 10*(g_model.limitData[i].min-100); //multiply by 10 to get same range as ofs (-1000..1000)
      if(ofs>lim_p) ofs = lim_p;
      if(ofs<lim_n) ofs = lim_n;

      if(q) q = (q>0) ?
                q*((int32_t)lim_p-ofs)/100000 :
               -q*((int32_t)lim_n-ofs)/100000 ; //div by 100000 -> output = -1024..1024

      q += calc1000toRESX(ofs);
      lim_p = calc1000toRESX(lim_p);
      lim_n = calc1000toRESX(lim_n);
      if(q>lim_p) q = lim_p;
      if(q<lim_n) q = lim_n;
      if(g_model.limitData[i].revert) q=-q;// finally do the reverse.

      if(g_model.safetySw[i].swtch)  //if safety sw available for channel check and replace val if needed
          if(getSwitch(g_model.safetySw[i].swtch,0)) q = calc100toRESX(g_model.safetySw[i].val);

    //cli();
    chanOut[i] = q; //copy consistent word to int-level
    //sei();
  }
}


void simulatorDialog::on_holdLeftX_clicked(bool checked)
{
    nodeLeft->setCenteringX(!checked);
}

void simulatorDialog::on_holdLeftY_clicked(bool checked)
{
    nodeLeft->setCenteringY(!checked);
}

void simulatorDialog::on_holdRightX_clicked(bool checked)
{
    nodeRight->setCenteringX(!checked);
}

void simulatorDialog::on_holdRightY_clicked(bool checked)
{
    nodeRight->setCenteringY(!checked);
}


void simulatorDialog::on_FixLeftX_clicked(bool checked)
{
    nodeLeft->setFixedX(checked);
}

void simulatorDialog::on_FixLeftY_clicked(bool checked)
{
    nodeLeft->setFixedY(checked);
}

void simulatorDialog::on_FixRightX_clicked(bool checked)
{
    nodeRight->setFixedX(checked);
}

void simulatorDialog::on_FixRightY_clicked(bool checked)
{
    nodeRight->setFixedY(checked);
}

