#include "modelconfigdialog.h"
#include "ui_modelconfigdialog.h"
#include "helpers.h"
#include "eeprominterface.h"
#include <QtGui>

modelConfigDialog::modelConfigDialog(RadioData &radioData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::modelConfigDialog),
    radioData(radioData),
    g_eeGeneral(radioData.generalSettings)
{
    ui->setupUi(this);
    ruddercolor << "#ffd100" << "#ffff00";
    throttlecolor << "#e40000";
    aileroncolor << "#0000ff" << "#00ffff";
    elevatorcolor << "#827f00" << "#ffff00";
    flapscolor << "#007f00" << "#00ff00";
    airbrakecolor << "#9b0099" << "#ff00fc";
    helicolor << "#00ff00" << "#0000ff" << "#00ffff" << "#ffff00" << "#e40000" << "#ffa400";
    connect(ui->ailType_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));    
    connect(ui->flapsType_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));
    connect(ui->spoilersType_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));
    connect(ui->swashType_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));
    connect(ui->engine_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));
    connect(ui->rudder_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));
    connect(ui->tailType_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(tailConfigChanged()));
    connect(ui->gyro_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(tailConfigChanged()));
    connect(ui->chStyle_CB,SIGNAL(currentIndexChanged(int)),this,SLOT(ConfigChanged()));
    formSetup();
    QTimer::singleShot(0, this, SLOT(shrink()));
}

modelConfigDialog::~modelConfigDialog()
{
    delete ui;
}

void modelConfigDialog::rxUpdate()
{
    QLabel * channel[] = {ui->CH_1,ui->CH_2,ui->CH_3,ui->CH_4,ui->CH_5,ui->CH_6,ui->CH_7,ui->CH_8,ui->CH_9,NULL};
    for (int i=0; channel[i]; i++)  {
      channel[i]->setStyleSheet("");
    }
    for (int i=0; i<9 ; i++) {
      rx[i]=false;
    }

    #define ICC(x) icc[(x)-1]
    uint8_t icc[4] = {0};
    for(uint8_t i=1; i<=4; i++) //generate inverse array
      for(uint8_t j=1; j<=4; j++) if(CC(i)==j) icc[j-1]=i;
    uint8_t stick;
    uint8_t freeCH=4;
    switch (ModelType) {
      case 0: //AIRPLANE
        stick=ICC(STK_THR);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(throttlecolor.at(0)));
        rx[stick-1]=true;
        stick=ICC(STK_RUD);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(ruddercolor.at(0)));
        if (ailerons>0) {
          stick=ICC(STK_AIL);
          channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(aileroncolor.at(0)));
          rx[stick-1]=true;
        }
        stick=ICC(STK_ELE);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(elevatorcolor.at(0)));
        if (ailerons>1) {
          channel[freeCH]->setStyleSheet(QString("background-color: %1;").arg(aileroncolor.at(1)));
          rx[freeCH]=true;
          freeCH++;
        }
        if (elevators>1) {
          channel[freeCH]->setStyleSheet(QString("background-color: %1;").arg(elevatorcolor.at(1)));
          rx[freeCH]=true;
          freeCH++;
        }
        for (uint8_t i=0; i< flaps; i++) {
          channel[freeCH]->setStyleSheet(QString("background-color: %1;").arg(flapscolor.at(i)));
          rx[freeCH]=true;
          freeCH++;
        }
        break;
      case 1:  //HELI
        switch (ui->chStyle_CB->currentIndex()) {
            case 0:
              channel[0]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(0))); //ELE CYC1
              channel[1]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(1))); //AIL  CYC2
              channel[2]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(2))); //PITCH CYC3
              channel[3]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(3))); //RUD
              channel[4]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(4))); //THR
              if (ui->gyro_CB->currentIndex()>0) {
                channel[5]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(5))); //GYRO
              }
              break;
            case 1:
              channel[0]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(1))); //AIL CYC2
              channel[1]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(0))); //ELE CYC1
              channel[2]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(4))); //THR 
              channel[3]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(3))); //RUD
              if (ui->gyro_CB->currentIndex()>0) {
                channel[4]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(5))); //GYRO
              }
              channel[5]->setStyleSheet(QString("background-color: %1;").arg(helicolor.at(2))); //PIT CYC3
              break;
        }
        break;     
      case 2: //GLIDER
        if (ailerons>0) {
          stick=ICC(STK_AIL);
          channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(aileroncolor.at(0)));
          rx[stick-1]=true;
        }
        stick=ICC(STK_ELE);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(elevatorcolor.at(0)));
        rx[stick-1]=true;
        stick=ICC(STK_RUD);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(ruddercolor.at(0)));
        rx[stick-1]=true;
        if (ailerons>1) {
          for (int j=0; j<9 ; j++) {
            if (!rx[j]) {
              channel[j]->setStyleSheet(QString("background-color: %1;").arg(aileroncolor.at(1)));
              rx[j]=true;
              break;
            }
          }
        }
        if (elevators>1) {
          for (int j=0; j<9 ; j++) {
            if (!rx[j]) {
              channel[j]->setStyleSheet(QString("background-color: %1;").arg(elevatorcolor.at(1)));
              rx[j]=true;
              break;
            }
          }
        }
        if (rudders>1) {
          for (int j=0; j<9 ; j++) {
            if (!rx[j]) {
              channel[j]->setStyleSheet(QString("background-color: %1;").arg(ruddercolor.at(1)));
              rx[j]=true;
              break;
            }
          }
        }      
        for (uint8_t i=0; i< flaps; i++) {
          for (int j=0; j<9 ; j++) {
            if (!rx[j]) {
              channel[j]->setStyleSheet(QString("background-color: %1;").arg(flapscolor.at(i)));
              rx[j]=true;
              break;
            }
          }
        }
        for (uint8_t i=0; i< spoilers; i++) {
          for (int j=0; j<9 ; j++) {
            if (!rx[j]) {
              channel[j]->setStyleSheet(QString("background-color: %1;").arg(airbrakecolor.at(i)));
              rx[j]=true;
              break;
            }
          }
        }
        break;
      case 3: //DELTA WING
        if (throttle>0) {
          stick=ICC(STK_THR);
          channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(throttlecolor.at(0)));
          rx[stick-1]=true;
        }
        stick=ICC(STK_AIL);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(aileroncolor.at(0)));
        rx[stick-1]=true;
        stick=ICC(STK_ELE);
        channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(elevatorcolor.at(0)));
        rx[stick-1]=true;
        if (rudders>0) {
          stick=ICC(STK_RUD);
          channel[stick-1]->setStyleSheet(QString("background-color: %1;").arg(ruddercolor.at(0)));
          rx[stick-1]=true;
          if (rudders>1) {
            for (int j=0; j<9 ; j++) {
              if (!rx[j]) {
                channel[j]->setStyleSheet(QString("background-color: %1;").arg(ruddercolor.at(1)));
                rx[j]=true;
                break;
              }
            }
          }
        }
        for (uint8_t i=0; i< flaps; i++) {
          for (int j=0; j<9 ; j++) {
            if (!rx[j]) {
              channel[j]->setStyleSheet(QString("background-color: %1;").arg(flapscolor.at(i)));
              rx[j]=true;
              break;
            }
          }
        }
        break;

    }
}

void modelConfigDialog::tailConfigChanged()
{
    if (tailLock)
        return;
    tailLock=true;
    QImage image;
    int index;
    switch (ModelType) {
      case 0:
        index=ui->tailType_CB->currentIndex();
        switch (index) {
            case 0:
                image.load(":/images/mcw/atailv.png");
                rudders=1;
                elevators=1;
                break;
            case 1:
                rudders=1;
                elevators=1;
                image.load(":/images/mcw/at1e1r.png");
                break;
            case 2:
                rudders=1;
                elevators=2;
                image.load(":/images/mcw/at2e1r.png");
                break;
        }
        break;
      case 1:
        elevators=1;
        rudders=1;
        index=ui->gyro_CB->currentIndex();
        switch (index) {
            case 0:
                image.load(":/images/mcw/htailng.png");
                break;
            case 1:
                image.load(":/images/mcw/htailg.png");
                break;
            case 2:
                image.load(":/images/mcw/htailg.png");
                break;
        }
        break;
      case 2:
        index=ui->tailType_CB->currentIndex();
        switch (index) {
            case 0:
                elevators=1;
                rudders=1;
                image.load(":/images/mcw/gtailv.png");
                break;
            case 1:
                elevators=1;
                rudders=1;
                image.load(":/images/mcw/gt1e1r.png");
                break;
            case 2:
                elevators=2;
                rudders=1;
                image.load(":/images/mcw/gt2e1r.png");
                break;
        }
        break;
    }
    ui->tailImg->setPixmap(QPixmap::fromImage(image));
    tailLock=false;
    rxUpdate();
}

void modelConfigDialog::ConfigChanged()
{
    if (wingsLock)
        return;
    wingsLock=true;
    QImage image;
    QString imgname=":/images/mcw/";  
    QStringList wimages;
    int index;
    switch (ModelType) {
      case 0:
        if (ui->ailType_CB->currentIndex()==0) {
            ui->flapsType_CB->setCurrentIndex(0);
            ui->flapsType_CB->setDisabled(true);
        } else {
            ui->flapsType_CB->setEnabled(true);
        }
        ailerons=ui->ailType_CB->currentIndex();
        flaps=ui->flapsType_CB->currentIndex();
        throttle=1;
        imgname.append(QString("aw%1a%2f.png").arg(ui->ailType_CB->currentIndex()).arg(ui->flapsType_CB->currentIndex()));
        image.load(imgname);
        ui->wingImg->setPixmap(QPixmap::fromImage(image));
        break;
      case 1:
        wimages.clear();
        wimages << "h90.png" << "h120.png" << "h120x.png" << "h140.png";
        index=ui->swashType_CB->currentIndex();
        ailerons=2;
        throttle=1;
        imgname.append(wimages.at(index));
        image.load(imgname);
        ui->wingImg->setPixmap(QPixmap::fromImage(image));
        break;
      case 2:
        if (ui->ailType_CB->currentIndex()==0) {
            ui->flapsType_CB->setCurrentIndex(0);
            ui->flapsType_CB->setDisabled(true);
        } else {
            ui->flapsType_CB->setEnabled(true);
        }
        if (ui->flapsType_CB->currentIndex()==0) {
            ui->spoilersType_CB->setCurrentIndex(0);
            ui->spoilersType_CB->setDisabled(true);
        } else {
            ui->spoilersType_CB->setEnabled(true);
        }
        ailerons=ui->ailType_CB->currentIndex();
        flaps=ui->flapsType_CB->currentIndex();
        spoilers=ui->spoilersType_CB->currentIndex();
        throttle=0;
        imgname.append(QString("gw%1a%2f%3s.png").arg(ui->ailType_CB->currentIndex()).arg(ui->flapsType_CB->currentIndex()).arg(ui->spoilersType_CB->currentIndex()));
        image.load(imgname);
        ui->wingImg->setPixmap(QPixmap::fromImage(image));
        break;
      case 3:
        ailerons=ui->ailType_CB->currentIndex();
        flaps=ui->flapsType_CB->currentIndex();
        rudders=ui->rudder_CB->currentIndex();
        throttle=ui->engine_CB->currentIndex();
        imgname.append(QString("dt%1t2e%2f%3r.png").arg(ui->engine_CB->currentIndex()).arg(ui->flapsType_CB->currentIndex()).arg(ui->rudder_CB->currentIndex()));
        image.load(imgname);
        ui->wingImg->setPixmap(QPixmap::fromImage(image));
        imgname.clear();
        imgname.append(QString(":/images/mcw/db%1t2e%2f%3r.png").arg(ui->engine_CB->currentIndex()).arg(ui->flapsType_CB->currentIndex()).arg(ui->rudder_CB->currentIndex()));
        image.load(imgname);
        ui->tailImg->setPixmap(QPixmap::fromImage(image));
        break;
      default:
        ui->wingImg->clear();
        break;
    }
    wingsLock=false;
    rxUpdate();
}

void modelConfigDialog::on_planeButton_clicked()
{
    ui->planeButton->setStyleSheet("background-color: #007f00; color: white;");
    ui->heliButton->setStyleSheet("");
    ui->gliderButton->setStyleSheet("");
    ui->deltaButton->setStyleSheet("");
    ModelType=0;
    wingsLock=true;
    tailLock=true;
    ui->engine_Label->hide();
    ui->engine_CB->setCurrentIndex(0);
    ui->engine_CB->hide();
    ui->rudder_Label->hide();
    ui->rudder_CB->setCurrentIndex(0);
    ui->rudder_CB->hide();
    ui->spoilersLabel->hide();
    ui->spoilersType_CB->setCurrentIndex(0);
    ui->spoilersType_CB->hide();
    ui->spLabel->hide();
    ui->sp_Label->hide();
    ui->swashType_CB->setCurrentIndex(0);
    ui->swashType_CB->hide();
    ui->gyroType_Label->hide();
    ui->gyro_CB->setCurrentIndex(0);
    ui->gyro_CB->hide();
    ui->chStyle_CB->hide();
    ui->chStyle_CB->setCurrentIndex(0);
    ui->chassign_Label->hide();
    ui->wingLabel->show();
    ui->ailType_Label->show();
    ui->ailType_CB->show();
    ui->ailType_CB->setCurrentIndex(0);
    ui->tailLabel->show();
    ui->tailType_Label->show();
    ui->tailType_CB->show();
    ui->tailType_CB->setCurrentIndex(0);
    ui->flapsType_Label->show();
    ui->flapsType_CB->show();
    ui->flapsType_CB->setCurrentIndex(0);
    ui->flapsType_CB->setDisabled(true);
    ui->planeButton->setDisabled(true);
    ui->heliButton->setEnabled(true);
    ui->gliderButton->setEnabled(true);
    ui->deltaButton->setEnabled(true);
    wingsLock=false;
    tailLock=false;
    resetControls();
    ConfigChanged();
    tailConfigChanged();
}

void modelConfigDialog::on_heliButton_clicked()
{
    ui->planeButton->setStyleSheet("");
    ui->heliButton->setStyleSheet("background-color: #007f00; color: white;");
    ui->gliderButton->setStyleSheet("");
    ui->deltaButton->setStyleSheet("");
    ModelType=1;
    wingsLock=true;
    tailLock=true;
    ui->engine_Label->hide();
    ui->engine_CB->setCurrentIndex(0);
    ui->engine_CB->hide();
    ui->rudder_Label->hide();
    ui->rudder_CB->setCurrentIndex(0);
    ui->rudder_CB->hide();
    ui->wingLabel->hide();
    ui->ailType_Label->hide();
    ui->ailType_CB->hide();
    ui->ailType_CB->setCurrentIndex(0);
    ui->tailType_Label->hide();
    ui->tailType_CB->hide();
    ui->tailType_CB->setCurrentIndex(0);
    ui->flapsType_Label->hide();
    ui->flapsType_CB->hide();
    ui->flapsType_CB->setCurrentIndex(0);
    ui->flapsType_CB->setDisabled(true);
    ui->spoilersLabel->hide();
    ui->spoilersType_CB->setCurrentIndex(0);
    ui->spoilersType_CB->hide();
    ui->tailLabel->show();
    ui->spLabel->show();
    ui->sp_Label->show();
    ui->swashType_CB->setCurrentIndex(0);
    ui->swashType_CB->show();
    ui->gyroType_Label->show();
    ui->gyro_CB->setCurrentIndex(0);
    ui->gyro_CB->show();
    ui->chStyle_CB->show();
    ui->chStyle_CB->setCurrentIndex(0);
    ui->chassign_Label->show();
    ui->planeButton->setEnabled(true);
    ui->heliButton->setDisabled(true);
    ui->gliderButton->setEnabled(true);
    ui->deltaButton->setEnabled(true);
    wingsLock=false;
    tailLock=false;
    resetControls();
    ConfigChanged();
    tailConfigChanged();
}

void modelConfigDialog::on_gliderButton_clicked()
{
    ui->planeButton->setStyleSheet("");
    ui->heliButton->setStyleSheet("");
    ui->gliderButton->setStyleSheet("background-color: #007f00; color: white;");
    ui->deltaButton->setStyleSheet("");
    ModelType=2;
    wingsLock=true;
    tailLock=true;
    ui->engine_Label->hide();
    ui->engine_CB->setCurrentIndex(0);
    ui->engine_CB->hide();
    ui->rudder_Label->hide();
    ui->rudder_CB->setCurrentIndex(0);
    ui->rudder_CB->hide();
    ui->spLabel->hide();
    ui->sp_Label->hide();
    ui->swashType_CB->setCurrentIndex(0);
    ui->swashType_CB->hide();
    ui->gyroType_Label->hide();
    ui->gyro_CB->setCurrentIndex(0);
    ui->gyro_CB->hide();
    ui->chStyle_CB->hide();
    ui->chStyle_CB->setCurrentIndex(0);
    ui->chassign_Label->hide();
    ui->wingLabel->show();
    ui->ailType_Label->show();
    ui->ailType_CB->show();
    ui->ailType_CB->setCurrentIndex(0);
    ui->tailLabel->show();
    ui->tailType_Label->show();
    ui->tailType_CB->show();
    ui->tailType_CB->setCurrentIndex(0);
    ui->flapsType_Label->show();
    ui->flapsType_CB->show();
    ui->flapsType_CB->setCurrentIndex(0);
    ui->flapsType_CB->setDisabled(true);
    ui->spoilersLabel->show();
    ui->spoilersType_CB->setCurrentIndex(0);
    ui->spoilersType_CB->setDisabled(true);
    ui->spoilersType_CB->show();
    ui->planeButton->setEnabled(true);
    ui->heliButton->setEnabled(true);
    ui->gliderButton->setDisabled(true);
    ui->deltaButton->setEnabled(true);
    wingsLock=false;
    tailLock=false;
    resetControls();
    ConfigChanged();
    tailConfigChanged();
}

void modelConfigDialog::on_deltaButton_clicked()
{
    ui->planeButton->setStyleSheet("");
    ui->heliButton->setStyleSheet("");
    ui->gliderButton->setStyleSheet("");
    ui->deltaButton->setStyleSheet("background-color: #007f00; color: white;");
    ModelType=3;
    wingsLock=true;
    tailLock=true;
    ui->spLabel->hide();
    ui->sp_Label->hide();
    ui->swashType_CB->setCurrentIndex(0);
    ui->swashType_CB->hide();
    ui->gyroType_Label->hide();
    ui->gyro_CB->setCurrentIndex(0);
    ui->gyro_CB->hide();
    ui->chStyle_CB->hide();
    ui->chStyle_CB->setCurrentIndex(0);
    ui->chassign_Label->hide();
    ui->ailType_Label->hide();
    ui->ailType_CB->hide();
    ui->ailType_CB->setCurrentIndex(0);
    ui->tailLabel->hide();
    ui->tailType_Label->hide();
    ui->tailType_CB->hide();
    ui->tailType_CB->setCurrentIndex(0);
    ui->spoilersLabel->hide();
    ui->spoilersType_CB->setCurrentIndex(0);
    ui->spoilersType_CB->hide();
    ui->engine_Label->show();
    ui->engine_CB->setCurrentIndex(0);
    ui->engine_CB->show();
    ui->rudder_Label->show();
    ui->rudder_CB->setCurrentIndex(0);
    ui->rudder_CB->show();
    ui->wingLabel->show();
    ui->flapsType_Label->show();
    ui->flapsType_CB->show();
    ui->flapsType_CB->setCurrentIndex(0);
    ui->flapsType_CB->setEnabled(true);
    ui->planeButton->setEnabled(true);
    ui->heliButton->setEnabled(true);
    ui->gliderButton->setEnabled(true);
    ui->deltaButton->setDisabled(true);
    wingsLock=false;
    tailLock=false;
    resetControls();
    ConfigChanged();
}


void modelConfigDialog::formSetup()
{
    on_planeButton_clicked();
}

void modelConfigDialog::resetControls() {
  ailerons=0;
  rudders=0;
  throttle=0;
  elevators=0;
  spoilers=0;
  flaps=0;
}


void modelConfigDialog::shrink()
{
    resize(0,0);
}
