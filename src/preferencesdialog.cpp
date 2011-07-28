#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QtGui>

preferencesDialog::preferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::preferencesDialog)
{
    ui->setupUi(this);

    populateLocale();
    initSettings();

    connect(this,SIGNAL(accepted()),this,SLOT(write_values()));
}

preferencesDialog::~preferencesDialog()
{
    delete ui;
}

void preferencesDialog::write_values()
{
    QSettings settings("er9x-eePe", "eePe");
    if (ui->locale_QB->currentIndex() > 0)
      settings.setValue("locale", ui->locale_QB->itemData(ui->locale_QB->currentIndex()));
    else
      settings.remove("locale");
    settings.setValue("default_channel_order", ui->channelorderCB->currentIndex());
    settings.setValue("default_mode", ui->stickmodeCB->currentIndex());
    settings.setValue("startup_check_er9x", ui->startupCheck_er9x->isChecked());
    settings.setValue("startup_check_eepe", ui->startupCheck_eepe->isChecked());
    settings.setValue("show_splash", ui->showSplash->isChecked());
    settings.setValue("download-version", ui->downloadVerCB->currentIndex());
}


void preferencesDialog::initSettings()
{
    QSettings settings("er9x-eePe", "eePe");
    int i=ui->locale_QB->findData(settings.value("locale"));
    if(i<0) i=0;
    ui->locale_QB->setCurrentIndex(i);

    ui->channelorderCB->setCurrentIndex(settings.value("default_channel_order", 0).toInt());
    ui->stickmodeCB->setCurrentIndex(settings.value("default_mode", 1).toInt());
    ui->downloadVerCB->setCurrentIndex(settings.value("download-version", 0).toInt());

    ui->startupCheck_er9x->setChecked(settings.value("startup_check_er9x", true).toBool());
    ui->startupCheck_eepe->setChecked(settings.value("startup_check_eepe", true).toBool());

    ui->showSplash->setChecked(settings.value("show_splash", true).toBool());
}

void preferencesDialog::populateLocale()
{
    ui->locale_QB->clear();
    ui->locale_QB->addItem("System default language", "");
    ui->locale_QB->addItem("English", "en");

    QStringList strl = QApplication::arguments();
    if(!strl.count()) return;

    QDir directory = QDir(":/");
    QStringList files = directory.entryList(QStringList("eepe_*.qm"), QDir::Files | QDir::NoSymLinks);

    foreach(QString file, files)
    {
        QLocale loc(file.mid(5,2));
        ui->locale_QB->addItem(QLocale::languageToString(loc.language()), loc.name());
    }


    //ui->locale_QB->addItems(files);


}

