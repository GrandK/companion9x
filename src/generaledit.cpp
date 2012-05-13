#include "generaledit.h"
#include "ui_generaledit.h"
#include "helpers.h"
#include <QtGui>

#define BIT_WARN_THR     ( 0x01 )
#define BIT_WARN_SW      ( 0x02 )
#define BIT_WARN_MEM     ( 0x04 )
#define BIT_WARN_BEEP    ( 0x80 )
#define BIT_BEEP_VAL     ( 0x38 ) // >>3
#define BEEP_VAL_SHIFT   3

GeneralEdit::GeneralEdit(RadioData &radioData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralEdit),
    radioData(radioData),
    g_eeGeneral(radioData.generalSettings)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icon.png"));

    QSettings settings("companion9x", "companion9x");
    ui->tabWidget->setCurrentIndex(settings.value("generalEditTab", 0).toInt());
    int profile_id=settings.value("profileId", 0).toInt();
    settings.beginGroup("Profiles");
    QString profile=QString("profile%1").arg(profile_id);
    settings.beginGroup(profile);
    QString name=settings.value("Name","").toString();
    if (name.isEmpty()) {
      ui->calstore_PB->setDisabled(true);
    }
    settings.endGroup();
    settings.endGroup();
    
    QRegExp rx(CHAR_FOR_NAMES_REGEX);
    ui->ownerNameLE->setValidator(new QRegExpValidator(rx, this));
    switchDefPosEditLock=true;
    populateSwitchCB(ui->backlightswCB, g_eeGeneral.lightSw);


    ui->ownerNameLE->setText(g_eeGeneral.ownerName);
    if (!GetEepromInterface()->getCapability(OwnerName)) {
      ui->ownerNameLE->setDisabled(true);
      ui->label_ownerName->hide();
      ui->ownerNameLE->hide();
      ui->hideNameOnSplashChkB->setDisabled(true);
      ui->hideNameOnSplashChkB->hide();
      ui->label_hideOwnerName->hide();      
    }
    if (GetEepromInterface()->getCapability(PerModelThrottleWarning)) {
      ui->thrwarnChkB->setDisabled(true);
      ui->thrwarnChkB->hide();
      ui->thrwarnLabel->hide();
    }
    
    if (!GetEepromInterface()->getCapability(TelemetryTimeshift)) {
      ui->label_timezone->hide();
      ui->timezoneSB->hide();
      ui->timezoneSB->setDisabled(true);
      ui->gpsFormatCB->hide();
      ui->gpsFormatLabel->hide();
    }
    ui->gpsFormatCB->setCurrentIndex(g_eeGeneral.gpsFormat);
    ui->timezoneSB->setValue(g_eeGeneral.timezone);
    
    if (!GetEepromInterface()->getCapability(OptrexDisplay)) {
      ui->label_displayType->hide();
      ui->displayTypeCB->setDisabled(true);
      ui->displayTypeCB->hide();
    }
    if (!GetEepromInterface()->getCapability(HasVolume)) {
      ui->volume_SB->hide();
      ui->volume_SB->setDisabled(true);
      ui->label_volume->hide();
    }
    if (!GetEepromInterface()->getCapability(HasBrightness)) {
      ui->BLBright_SB->hide();
      ui->BLBright_SB->setDisabled(true);
      ui->label_BLBright->hide();
    }
    if (!GetEepromInterface()->getCapability(HasCurrentCalibration)) {
      ui->CurrentCalib_SB->hide();
      ui->CurrentCalib_SB->setDisabled(true);
      ui->label_CurrentCalib->hide();
    }
    
    if (GetEepromInterface()->getCapability(TelemetryRSSIModel) ) {
        ui->tabWidget->removeTab(2);
    }
    ui->tabWidget->setCurrentIndex(0);

    if (!GetEepromInterface()->getCapability(SoundMod)) {
      ui->soundModeCB->setDisabled(true);
      ui->label_soundMode->hide();
      ui->soundModeCB->hide();
    }

    if (!GetEepromInterface()->getCapability(SoundPitch)) {
      ui->speakerPitchSB->setDisabled(true);
      ui->label_speakerPitch->hide();
      ui->speakerPitchSB->hide();
    }
    
    if (!GetEepromInterface()->getCapability(Haptic)) {
      ui->hapticStrengthSB->setDisabled(true);
      ui->hapticStrengthSB->hide();
      ui->label_hapticStrengthSB->hide();
    } 

    if (!GetEepromInterface()->getCapability(HapticMode)) {
      ui->hapticmodeCB->setDisabled(true);
      ui->hapticmodeCB->hide();
      ui->label_hapticmode->hide();
    } 

    if (!GetEepromInterface()->getCapability(BandgapMeasure)) {
      ui->BandGapEnableChkB->setDisabled(true);
      ui->BandGapEnableChkB->hide();
      ui->label_BandGapEnable->hide();
    }

    if (!GetEepromInterface()->getCapability(PotScrolling)) {
      ui->PotScrollEnableChkB->setDisabled(true);
      ui->PotScrollEnableChkB->hide();
      ui->label_PotScrollEnable->hide();
    } 
    
    if (!GetEepromInterface()->getCapability(TrainerSwitch)) {
      ui->label_switch->hide();
      ui->swtchCB_1->setDisabled(true);
      ui->swtchCB_2->setDisabled(true);
      ui->swtchCB_3->setDisabled(true);
      ui->swtchCB_4->setDisabled(true);
      ui->swtchCB_1->hide();
      ui->swtchCB_2->hide();
      ui->swtchCB_3->hide();
      ui->swtchCB_4->hide();
    }
    else {
      populateSwitchCB(ui->swtchCB_1,g_eeGeneral.trainer.mix[0].swtch);
      populateSwitchCB(ui->swtchCB_2,g_eeGeneral.trainer.mix[1].swtch);
      populateSwitchCB(ui->swtchCB_3,g_eeGeneral.trainer.mix[2].swtch);
      populateSwitchCB(ui->swtchCB_4,g_eeGeneral.trainer.mix[3].swtch);
    }
    if (!GetEepromInterface()->getCapability(BLonStickMove)) {
      ui->blOnStickMoveSB->setDisabled(true);
      ui->blOnStickMoveSB->hide();
      ui->label_blOnStickMove->hide();
    }
    else {
      ui->blOnStickMoveSB->setValue(g_eeGeneral.lightOnStickMove*5);
    }
    if (!GetEepromInterface()->getCapability(gsSwitchMask)) {
      ui->swAILChkB->setDisabled(true);
      ui->swELEChkB->setDisabled(true);
      ui->swTHRChkB->setDisabled(true);
      ui->swRUDChkB->setDisabled(true);
      ui->swGEAChkB->setDisabled(true);
      ui->swID0ChkB->setDisabled(true);
      ui->swID1ChkB->setDisabled(true);
      ui->swID2ChkB->setDisabled(true);
      ui->swAILChkB->hide();
      ui->swELEChkB->hide();
      ui->swTHRChkB->hide();
      ui->swRUDChkB->hide();
      ui->swGEAChkB->hide();
      ui->swID0ChkB->hide();
      ui->swID1ChkB->hide();
      ui->swID2ChkB->hide();
      ui->swtchWarnChkB->hide();
    }
    else {
      ui->swtchWarnCB->setDisabled(true);
      ui->swtchWarnCB->hide();
      setSwitchDefPos();
    }
    if (!GetEepromInterface()->getCapability(TelemetryAlarm)) {
      ui->telalarmsChkB->hide();
      ui->label_telalarms->hide();
    }
    if (!GetEepromInterface()->getCapability(HapticLength)) {
      ui->label_HL->hide();
      ui->hapticLengthCB->hide();
    } else {
      ui->hapticLengthCB->setCurrentIndex(g_eeGeneral.hapticLength+2);
    }
    
    ui->telalarmsChkB->setChecked(g_eeGeneral.enableTelemetryAlarm);
    ui->soundModeCB->setCurrentIndex(g_eeGeneral.speakerMode);
    ui->volume_SB->setValue(g_eeGeneral.speakerVolume);
    ui->beeperlenCB->setCurrentIndex(g_eeGeneral.beeperLength+2);
    ui->speakerPitchSB->setValue(g_eeGeneral.speakerPitch);
    ui->hapticStrengthSB->setValue(g_eeGeneral.hapticStrength);
    ui->hapticmodeCB->setCurrentIndex(g_eeGeneral.hapticMode+2);
    ui->PotScrollEnableChkB->setChecked(!g_eeGeneral.disablePotScroll);
    ui->BandGapEnableChkB->setChecked(!g_eeGeneral.disableBG);
    ui->BLBright_SB->setValue(100-g_eeGeneral.backlightBright);
    ui->contrastSB->setValue(g_eeGeneral.contrast);
    ui->battwarningDSB->setValue((double)g_eeGeneral.vBatWarn/10);
    ui->battcalibDSB->setValue((double)g_eeGeneral.vBatCalib/10);
    ui->CurrentCalib_SB->setValue((double)g_eeGeneral.currentCalib);
    ui->battCalib->setValue((double)g_eeGeneral.vBatCalib/10);
    ui->backlightautoSB->setValue(g_eeGeneral.lightAutoOff*5);
    ui->inactimerSB->setValue(g_eeGeneral.inactivityTimer);
    ui->thrrevChkB->setChecked(g_eeGeneral.throttleReversed);
    ui->inputfilterCB->setCurrentIndex(g_eeGeneral.filterInput);
    ui->thrwarnChkB->setChecked(!g_eeGeneral.disableThrottleWarning);   //Default is zero=checked
    ui->swtchWarnChkB->setChecked(g_eeGeneral.switchWarning == -1);
    ui->swtchWarnCB->setCurrentIndex(g_eeGeneral.switchWarning == -1 ? 2 : g_eeGeneral.switchWarning);
    ui->memwarnChkB->setChecked(!g_eeGeneral.disableMemoryWarning);   //Default is zero=checked
    ui->alarmwarnChkB->setChecked(!g_eeGeneral.disableAlarmWarning);//Default is zero=checked
    ui->enableTelemetryAlarmChkB->setChecked(g_eeGeneral.enableTelemetryAlarm);
    ui->beeperCB->setCurrentIndex(g_eeGeneral.beeperMode+2);
    ui->channelorderCB->setCurrentIndex(g_eeGeneral.templateSetup);
    ui->stickmodeCB->setCurrentIndex(g_eeGeneral.stickMode);

    ui->beepMinuteChkB->setChecked(g_eeGeneral.minuteBeep);
    ui->beepCountDownChkB->setChecked(g_eeGeneral.preBeep);
    ui->beepFlashChkB->setChecked(g_eeGeneral.flashBeep);
    ui->splashScreenChkB->setChecked(!g_eeGeneral.disableSplashScreen);
    ui->hideNameOnSplashChkB->setChecked(!g_eeGeneral.hideNameOnSplash);

    ui->trnMode_1->setCurrentIndex(g_eeGeneral.trainer.mix[0].mode);
    ui->trnChn_1->setCurrentIndex(g_eeGeneral.trainer.mix[0].src);
    ui->trnWeight_1->setValue(g_eeGeneral.trainer.mix[0].weight);
    ui->trnMode_2->setCurrentIndex(g_eeGeneral.trainer.mix[1].mode);
    ui->trnChn_2->setCurrentIndex(g_eeGeneral.trainer.mix[1].src);
    ui->trnWeight_2->setValue(g_eeGeneral.trainer.mix[1].weight);
    ui->trnMode_3->setCurrentIndex(g_eeGeneral.trainer.mix[2].mode);
    ui->trnChn_3->setCurrentIndex(g_eeGeneral.trainer.mix[2].src);
    ui->trnWeight_3->setValue(g_eeGeneral.trainer.mix[2].weight);
    ui->trnMode_4->setCurrentIndex(g_eeGeneral.trainer.mix[3].mode);
    ui->trnChn_4->setCurrentIndex(g_eeGeneral.trainer.mix[3].src);
    ui->trnWeight_4->setValue(g_eeGeneral.trainer.mix[3].weight);

    ui->ana1Neg->setValue(g_eeGeneral.calibSpanNeg[0]);
    ui->ana2Neg->setValue(g_eeGeneral.calibSpanNeg[1]);
    ui->ana3Neg->setValue(g_eeGeneral.calibSpanNeg[2]);
    ui->ana4Neg->setValue(g_eeGeneral.calibSpanNeg[3]);
    ui->ana5Neg->setValue(g_eeGeneral.calibSpanNeg[4]);
    ui->ana6Neg->setValue(g_eeGeneral.calibSpanNeg[5]);
    ui->ana7Neg->setValue(g_eeGeneral.calibSpanNeg[6]);

    ui->ana1Mid->setValue(g_eeGeneral.calibMid[0]);
    ui->ana2Mid->setValue(g_eeGeneral.calibMid[1]);
    ui->ana3Mid->setValue(g_eeGeneral.calibMid[2]);
    ui->ana4Mid->setValue(g_eeGeneral.calibMid[3]);
    ui->ana5Mid->setValue(g_eeGeneral.calibMid[4]);
    ui->ana6Mid->setValue(g_eeGeneral.calibMid[5]);
    ui->ana7Mid->setValue(g_eeGeneral.calibMid[6]);

    ui->ana1Pos->setValue(g_eeGeneral.calibSpanPos[0]);
    ui->ana2Pos->setValue(g_eeGeneral.calibSpanPos[1]);
    ui->ana3Pos->setValue(g_eeGeneral.calibSpanPos[2]);
    ui->ana4Pos->setValue(g_eeGeneral.calibSpanPos[3]);
    ui->ana5Pos->setValue(g_eeGeneral.calibSpanPos[4]);
    ui->ana6Pos->setValue(g_eeGeneral.calibSpanPos[5]);
    ui->ana7Pos->setValue(g_eeGeneral.calibSpanPos[6]);

    ui->PPM1->setValue(g_eeGeneral.trainer.calib[0]);
    ui->PPM2->setValue(g_eeGeneral.trainer.calib[1]);
    ui->PPM3->setValue(g_eeGeneral.trainer.calib[2]);
    ui->PPM4->setValue(g_eeGeneral.trainer.calib[3]);
    ui->PPM_MultiplierDSB->setValue((qreal)(g_eeGeneral.PPM_Multiplier+10)/10);
    switchDefPosEditLock=false;
    QTimer::singleShot(0, this, SLOT(shrink()));
}

GeneralEdit::~GeneralEdit()
{
    delete ui;
}

void GeneralEdit::setSwitchDefPos()
{
    quint8 x = g_eeGeneral.switchWarningStates & 0x38;
    if(x==0x00 || x==0x18 || x== 0x28 || x==0x38 || x==0x30) //illegal states for ID0/1/2
    {
        g_eeGeneral.switchWarningStates &= ~0x38; // turn all off, make sure only one is on
        g_eeGeneral.switchWarningStates |=  0x08;
    }

    switchDefPosEditLock = true;
    ui->swTHRChkB->setChecked(g_eeGeneral.switchWarningStates & 0x01);
    ui->swRUDChkB->setChecked(g_eeGeneral.switchWarningStates & 0x02);
    ui->swELEChkB->setChecked(g_eeGeneral.switchWarningStates & 0x04);
    ui->swID0ChkB->setChecked(g_eeGeneral.switchWarningStates & 0x08);
    ui->swID1ChkB->setChecked(g_eeGeneral.switchWarningStates & 0x10);
    ui->swID2ChkB->setChecked(g_eeGeneral.switchWarningStates & 0x20);
    ui->swAILChkB->setChecked(g_eeGeneral.switchWarningStates & 0x40);
    ui->swGEAChkB->setChecked(g_eeGeneral.switchWarningStates & 0x80);
    switchDefPosEditLock = false;
}

void GeneralEdit::updateSettings()
{
    radioData.generalSettings = g_eeGeneral;
    emit modelValuesChanged();
}

void GeneralEdit::on_contrastSB_editingFinished()
{
    g_eeGeneral.contrast = ui->contrastSB->value();
    updateSettings();
}

void GeneralEdit::on_battwarningDSB_editingFinished()
{
    g_eeGeneral.vBatWarn = (int)(ui->battwarningDSB->value()*10);
    updateSettings();
}

void GeneralEdit::on_battcalibDSB_editingFinished()
{
    g_eeGeneral.vBatCalib = ui->battcalibDSB->value()*10;
    ui->battCalib->setValue(ui->battcalibDSB->value());
    updateSettings();
}

void GeneralEdit::on_backlightswCB_currentIndexChanged(int index)
{
  if (switchDefPosEditLock)
    return;
  g_eeGeneral.lightSw = RawSwitch(ui->backlightswCB->itemData(ui->backlightswCB->currentIndex()).toInt());
  updateSettings();
}

void GeneralEdit::on_beeperlenCB_currentIndexChanged(int index)
{
  g_eeGeneral.beeperLength = index-2;
  updateSettings();
}

void GeneralEdit::on_hapticLengthCB_currentIndexChanged(int index)
{
  g_eeGeneral.hapticLength = index-2;
  updateSettings();
}

void GeneralEdit::on_gpsFormatCB_currentIndexChanged(int index)
{
  g_eeGeneral.gpsFormat = index;
  updateSettings();
}


void GeneralEdit::on_backlightautoSB_editingFinished()
{
  int i = ui->backlightautoSB->value()/5;
  if((i*5)!=ui->backlightautoSB->value())
    ui->backlightautoSB->setValue(i*5);
  else
  {
    g_eeGeneral.lightAutoOff = i;
    updateSettings();
  }
}

void GeneralEdit::on_timezoneSB_editingFinished()
{
  g_eeGeneral.timezone = ui->timezoneSB->value();
  updateSettings();
}


void GeneralEdit::on_blOnStickMoveSB_editingFinished()
{
    int i = ui->blOnStickMoveSB->value()/5;
    if((i*5)!=ui->blOnStickMoveSB->value())
        ui->blOnStickMoveSB->setValue(i*5);

    g_eeGeneral.lightOnStickMove = i;
    updateSettings();
}

void GeneralEdit::on_inactimerSB_editingFinished()
{
    g_eeGeneral.inactivityTimer = ui->inactimerSB->value();
    updateSettings();
}

void GeneralEdit::on_thrrevChkB_stateChanged(int )
{
    g_eeGeneral.throttleReversed = ui->thrrevChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_inputfilterCB_currentIndexChanged(int index)
{
    g_eeGeneral.filterInput = index;
    updateSettings();
}

void GeneralEdit::on_thrwarnChkB_stateChanged(int )
{
    g_eeGeneral.disableThrottleWarning = ui->thrwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_swtchWarnCB_currentIndexChanged(int index)
{
    g_eeGeneral.switchWarning = (index == 2 ? -1 : index);
    updateSettings();
}

void GeneralEdit::on_swtchWarnChkB_stateChanged(int )
{
    g_eeGeneral.switchWarning = ui->swtchWarnChkB->isChecked() ? -1 : 0;
    updateSettings();
}

void GeneralEdit::on_memwarnChkB_stateChanged(int )
{
    g_eeGeneral.disableMemoryWarning = ui->memwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_alarmwarnChkB_stateChanged(int )
{
    g_eeGeneral.disableAlarmWarning = ui->alarmwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_enableTelemetryAlarmChkB_stateChanged(int )
{
    g_eeGeneral.enableTelemetryAlarm = ui->enableTelemetryAlarmChkB->isChecked();
    updateSettings();
}

void GeneralEdit::on_beeperCB_currentIndexChanged(int index)
{
    g_eeGeneral.beeperMode = (BeeperMode)(index-2);
    updateSettings();
}

void GeneralEdit::on_displayTypeCB_currentIndexChanged(int index)
{
    g_eeGeneral.optrexDisplay = index;
    updateSettings();
}

void GeneralEdit::on_hapticmodeCB_currentIndexChanged(int index)
{
    g_eeGeneral.hapticMode = (BeeperMode)(index-2);
    updateSettings();
}


void GeneralEdit::on_channelorderCB_currentIndexChanged(int index)
{
    g_eeGeneral.templateSetup = index;
    updateSettings();
}

void GeneralEdit::on_stickmodeCB_currentIndexChanged(int index)
{
    g_eeGeneral.stickMode = index;
    updateSettings();
}

void GeneralEdit::on_trnMode_1_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[0].mode = index;
    updateSettings();
}

void GeneralEdit::on_trnChn_1_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[0].src = index;
    updateSettings();
}

void GeneralEdit::on_trnWeight_1_editingFinished()
{
    g_eeGeneral.trainer.mix[0].weight = ui->trnWeight_1->value();
    updateSettings();
}

void GeneralEdit::on_trnMode_2_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[1].mode = index;
    updateSettings();
}

void GeneralEdit::on_trnChn_2_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[1].src = index;
    updateSettings();
}

void GeneralEdit::on_trnWeight_2_editingFinished()
{
    g_eeGeneral.trainer.mix[1].weight = ui->trnWeight_2->value();
    updateSettings();
}

void GeneralEdit::on_trnMode_3_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[2].mode = index;
    updateSettings();
}

void GeneralEdit::on_trnChn_3_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[2].src = index;
    updateSettings();
}

void GeneralEdit::on_trnWeight_3_editingFinished()
{
    g_eeGeneral.trainer.mix[2].weight = ui->trnWeight_3->value();
    updateSettings();
}

void GeneralEdit::on_trnMode_4_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[3].mode = index;
    updateSettings();
}

void GeneralEdit::on_trnChn_4_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[3].src = index;
    updateSettings();
}

void GeneralEdit::on_trnWeight_4_editingFinished()
{
    g_eeGeneral.trainer.mix[3].weight = ui->trnWeight_4->value();
    updateSettings();
}

void GeneralEdit::on_ana1Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[0] = ui->ana1Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana2Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[1] = ui->ana2Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana3Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[2] = ui->ana3Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana4Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[3] = ui->ana4Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana5Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[4] = ui->ana5Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana6Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[5] = ui->ana6Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana7Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[6] = ui->ana7Neg->value();
    updateSettings();
}



void GeneralEdit::on_ana1Mid_editingFinished()
{
    g_eeGeneral.calibMid[0] = ui->ana1Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana2Mid_editingFinished()
{
    g_eeGeneral.calibMid[1] = ui->ana2Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana3Mid_editingFinished()
{
    g_eeGeneral.calibMid[2] = ui->ana3Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana4Mid_editingFinished()
{
    g_eeGeneral.calibMid[3] = ui->ana4Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana5Mid_editingFinished()
{
    g_eeGeneral.calibMid[4] = ui->ana5Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana6Mid_editingFinished()
{
    g_eeGeneral.calibMid[5] = ui->ana6Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana7Mid_editingFinished()
{
    g_eeGeneral.calibMid[6] = ui->ana7Mid->value();
    updateSettings();
}



void GeneralEdit::on_ana1Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[0] = ui->ana1Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana2Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[1] = ui->ana2Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana3Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[2] = ui->ana3Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana4Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[3] = ui->ana4Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana5Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[4] = ui->ana5Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana6Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[5] = ui->ana6Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana7Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[6] = ui->ana7Pos->value();
    updateSettings();
}

void GeneralEdit::on_battCalib_editingFinished()
{
    g_eeGeneral.vBatCalib = ui->battCalib->value()*10;
    ui->battcalibDSB->setValue(ui->battCalib->value());
    updateSettings();
}

void GeneralEdit::on_volume_SB_editingFinished()
{
    g_eeGeneral.speakerVolume = ui->volume_SB->value();
    updateSettings();
}

void GeneralEdit::on_BLBright_SB_editingFinished()
{
    g_eeGeneral.backlightBright = 100 - ui->BLBright_SB->value();
    updateSettings();
}

void GeneralEdit::on_CurrentCalib_SB_editingFinished()
{
    g_eeGeneral.currentCalib = ui->CurrentCalib_SB->value();
    updateSettings();
}

void GeneralEdit::on_PPM1_editingFinished()
{
    g_eeGeneral.trainer.calib[0] = ui->PPM1->value();
    updateSettings();
}

void GeneralEdit::on_PPM2_editingFinished()
{
    g_eeGeneral.trainer.calib[1] = ui->PPM2->value();
    updateSettings();
}

void GeneralEdit::on_PPM3_editingFinished()
{
    g_eeGeneral.trainer.calib[2] = ui->PPM3->value();
    updateSettings();
}

void GeneralEdit::on_PPM4_editingFinished()
{
    g_eeGeneral.trainer.calib[3] = ui->PPM4->value();
    updateSettings();
}

void GeneralEdit::on_tabWidget_currentChanged(int index)
{
  // TODO why er9x here
    QSettings settings("companion9x", "companion9x");
    settings.setValue("generalEditTab",index);//ui->tabWidget->currentIndex());
}


void GeneralEdit::on_beepMinuteChkB_stateChanged(int )
{
    g_eeGeneral.minuteBeep = ui->beepMinuteChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_beepCountDownChkB_stateChanged(int )
{
    g_eeGeneral.preBeep = ui->beepCountDownChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_beepFlashChkB_stateChanged(int )
{
    g_eeGeneral.flashBeep = ui->beepFlashChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_splashScreenChkB_stateChanged(int )
{
    g_eeGeneral.disableSplashScreen = ui->splashScreenChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_hideNameOnSplashChkB_stateChanged(int )
{
    g_eeGeneral.hideNameOnSplash = ui->hideNameOnSplashChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_PPM_MultiplierDSB_editingFinished()
{
    g_eeGeneral.PPM_Multiplier = (int)(ui->PPM_MultiplierDSB->value()*10)-10;
    updateSettings();
}

void GeneralEdit::on_ownerNameLE_editingFinished()
{
    strncpy(g_eeGeneral.ownerName, ui->ownerNameLE->text().toAscii(), 10);
    updateSettings();
}

void GeneralEdit::on_speakerPitchSB_editingFinished()
{
    g_eeGeneral.speakerPitch = ui->speakerPitchSB->value();
    updateSettings();
}

void GeneralEdit::on_hapticStrengthSB_editingFinished()
{
    g_eeGeneral.hapticStrength = ui->hapticStrengthSB->value();
    updateSettings();
}

void GeneralEdit::on_soundModeCB_currentIndexChanged(int index)
{
    g_eeGeneral.speakerMode = index;
    updateSettings();
}

void GeneralEdit::on_PotScrollEnableChkB_stateChanged(int )
{
    g_eeGeneral.disablePotScroll = ui->PotScrollEnableChkB->isChecked() ? false : true;
    updateSettings();
}

void GeneralEdit::on_BandGapEnableChkB_stateChanged(int )
{
    g_eeGeneral.disableBG = ui->BandGapEnableChkB->isChecked() ? false : true;
    updateSettings();
}

void GeneralEdit::on_swtchCB_1_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[0].swtch = RawSwitch(ui->swtchCB_1->itemData(ui->swtchCB_1->currentIndex()).toInt());
    updateSettings();
}

void GeneralEdit::on_swtchCB_2_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[1].swtch = RawSwitch(ui->swtchCB_2->itemData(ui->swtchCB_2->currentIndex()).toInt());
    updateSettings();
}

void GeneralEdit::on_swtchCB_3_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[2].swtch = RawSwitch(ui->swtchCB_3->itemData(ui->swtchCB_3->currentIndex()).toInt());
    updateSettings();
}

void GeneralEdit::on_swtchCB_4_currentIndexChanged(int index)
{
    g_eeGeneral.trainer.mix[3].swtch = RawSwitch(ui->swtchCB_4->itemData(ui->swtchCB_4->currentIndex()).toInt());
    updateSettings();
}

void GeneralEdit::getGeneralSwitchDefPos(int i, bool val)
{
    if(val)
        g_eeGeneral.switchWarningStates |= (1<<(i-1));
    else
        g_eeGeneral.switchWarningStates &= ~(1<<(i-1));
}

void GeneralEdit::on_swTHRChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(1,ui->swTHRChkB->isChecked());
    updateSettings();
}

void GeneralEdit::on_telalarmsChkB_stateChanged(int )
{
    g_eeGeneral.enableTelemetryAlarm=ui->telalarmsChkB->isChecked();
    updateSettings();
}

void GeneralEdit::on_swRUDChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(2,ui->swRUDChkB->isChecked());
    updateSettings();
}
void GeneralEdit::on_swELEChkB_stateChanged(int )
{
    getGeneralSwitchDefPos(3,ui->swELEChkB->isChecked());
    updateSettings();
}
void GeneralEdit::on_swID0ChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;

    if(ui->swID0ChkB->isChecked())
    {
        switchDefPosEditLock = true;
        ui->swID1ChkB->setChecked(false);
        ui->swID2ChkB->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

    g_eeGeneral.switchWarningStates &= ~0x30; //turn off ID1/2
    getGeneralSwitchDefPos(4,ui->swID0ChkB->isChecked());
    updateSettings();
}
void GeneralEdit::on_swID1ChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;

    if(ui->swID1ChkB->isChecked())
    {
        switchDefPosEditLock = true;
        ui->swID0ChkB->setChecked(false);
        ui->swID2ChkB->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

    g_eeGeneral.switchWarningStates &= ~0x28; //turn off ID0/2
    getGeneralSwitchDefPos(5,ui->swID1ChkB->isChecked());
    updateSettings();
}
void GeneralEdit::on_swID2ChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;

    if(ui->swID2ChkB->isChecked())
    {
        switchDefPosEditLock = true;
        ui->swID0ChkB->setChecked(false);
        ui->swID1ChkB->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

    g_eeGeneral.switchWarningStates &= ~0x18; //turn off ID1/2
    getGeneralSwitchDefPos(6,ui->swID2ChkB->isChecked());
    updateSettings();
}
void GeneralEdit::on_swAILChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(7,ui->swAILChkB->isChecked());
    updateSettings();
}
void GeneralEdit::on_swGEAChkB_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(8,ui->swGEAChkB->isChecked());
    updateSettings();
}

void GeneralEdit::on_calstore_PB_clicked()
{
  QSettings settings("companion9x", "companion9x");
  int profile_id=settings.value("profileId", 0).toInt();
  settings.beginGroup("Profiles");
  QString profile=QString("profile%1").arg(profile_id);
  settings.beginGroup(profile);
  QString name=settings.value("Name","").toString();
  if (name.isEmpty()) {
    ui->calstore_PB->setDisabled(true);
  } else {
    QString calib=settings.value("StickPotCalib","").toString();
    if (!(calib.isEmpty())) {
      int ret = QMessageBox::question(this, "companion9x", 
                      tr("Do you want to store calibration in %1 profile<br>Overwriting existing calibrationt?").arg(name) ,
                      QMessageBox::Yes | QMessageBox::No);
      if (ret == QMessageBox::No) {
        settings.endGroup();
        settings.endGroup();
        return;
      }
    }
    calib.clear();
    for (int i=0; i< (NUM_STICKS+NUM_POTS); i++) {
      calib.append(QString("%1").arg(g_eeGeneral.calibMid[i], 4, 16, QChar('0')));
      calib.append(QString("%1").arg(g_eeGeneral.calibSpanNeg[i], 4, 16, QChar('0')));
      calib.append(QString("%1").arg(g_eeGeneral.calibSpanPos[i], 4, 16, QChar('0')));
    }
    settings.setValue("StickPotCalib",calib);
    calib.clear();
    for (int i=0; i< 4; i++) {
      calib.append(QString("%1").arg(g_eeGeneral.trainer.calib[i], 4, 16, QChar('0')));
    }
    settings.setValue("TrainerCalib",calib);
    settings.setValue("VbatCalib",g_eeGeneral.vBatCalib);
    settings.setValue("currentCalib",g_eeGeneral.currentCalib);
    settings.setValue("PPM_Multiplier",g_eeGeneral.PPM_Multiplier);
    settings.setValue("GSStickMode",g_eeGeneral.stickMode);
/*  TODO: better divided or all together  ?
    settings.setValue("LcdType",(g_eeGeneral.optrexDisplay ? 1:0));
    settings.setValue("Contrast",g_eeGeneral.contrast);
    settings.setValue("Brightness",g_eeGeneral.backlightBright);
    settings.setValue("BeeperMode",g_eeGeneral.beeperMode);
    settings.setValue("BeeperLength",g_eeGeneral.beeperLength);
    settings.setValue("HapticMode",g_eeGeneral.hapticMode);
    settings.setValue("HapticStrength",g_eeGeneral.hapticStrength);
    settings.setValue("HapticLength",g_eeGeneral.hapticLength);
    settings.setValue("speakerMode",g_eeGeneral.speakerMode);
    settings.setValue("speakerPitch",g_eeGeneral.speakerPitch);
    settings.setValue("speakerVolume",g_eeGeneral.speakerVolume);
 */
    settings.setValue("Display",QString("%1%2%3").arg((g_eeGeneral.optrexDisplay ? 1:0), 2, 16, QChar('0')).arg(g_eeGeneral.contrast, 2, 16, QChar('0')).arg(g_eeGeneral.backlightBright, 2, 16, QChar('0')));
    settings.setValue("Beeper",QString("%1%2").arg(g_eeGeneral.beeperMode, 2, 16, QChar('0')).arg(g_eeGeneral.beeperLength, 2, 16, QChar('0')));
    settings.setValue("Haptic",QString("%1%2%3").arg(g_eeGeneral.hapticMode, 2, 16, QChar('0')).arg(g_eeGeneral.hapticStrength, 2, 16, QChar('0')).arg(g_eeGeneral.hapticLength, 2, 16, QChar('0')));
    settings.setValue("Speaker",QString("%1%2%3").arg(g_eeGeneral.speakerMode, 2, 16, QChar('0')).arg(g_eeGeneral.speakerPitch, 2, 16, QChar('0')).arg(g_eeGeneral.speakerVolume, 2, 16, QChar('0')));
  }

  settings.endGroup();
  settings.endGroup();
}

void GeneralEdit::shrink() {
    resize(0,0);
}
