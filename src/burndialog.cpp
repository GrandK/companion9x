#include "burndialog.h"
#include "ui_burndialog.h"

#include <QtGui>
#include "helpers.h"
#include "splashlibrary.h"
#include "flashinterface.h"

burnDialog::burnDialog(QWidget *parent, int Type, QString * fileName, bool * backupEE):
  QDialog(parent),
  ui(new Ui::burnDialog),
  hexfileName(fileName),
  backup(backupEE),
  hexType(Type)
{
  ui->setupUi(this);
  ui->SplashFrame->hide();
  ui->FramFWInfo->hide();
  ui->EEbackupCB->hide();
  ui->EEbackupCB->setCheckState(*backup ? Qt::Checked : Qt::Unchecked);
  if (Type == 2) {
    ui->EEpromCB->hide();
    this->setWindowTitle(tr("Write firmware to TX"));
  }
  else {
    this->setWindowTitle(tr("Write models to TX"));
  }
  if (!hexfileName->isEmpty()) {
    ui->FWFileName->setText(*hexfileName);
    ui->FWFileName->hide();
    ui->FlashLoadButton->hide();   
    hexfileName->clear();
  }
  resize(0, 0);
}

burnDialog::~burnDialog() {
  delete ui;
}

void burnDialog::on_FlashLoadButton_clicked()
{
  QString fileName;
  QSettings settings("companion9x", "companion9x");
  ui->ImageLoadButton->setDisabled(true);
  ui->libraryButton->setDisabled(true);
  ui->InvertColorButton->setDisabled(true);
  ui->BurnFlashButton->setDisabled(true);
  ui->ImageFileName->clear();
  ui->FwImage->clear();
  ui->FWFileName->clear();
  ui->VersionField->clear();
  ui->DateField->clear();
  ui->SVNField->clear();
  ui->ModField->clear();
  ui->FramFWInfo->hide();
  ui->SplashFrame->hide();
  ui->BurnFlashButton->setDisabled(true);
  ui->EEbackupCB->hide();
  QTimer::singleShot(0, this, SLOT(shrink()));
  fileName = QFileDialog::getOpenFileName(this, tr("Open"), settings.value("lastFlashDir").toString(), FLASH_FILES_FILTER);
  if (fileName.isEmpty()) {
    return;
  }
  ui->EEbackupCB->show();
  ui->FWFileName->setText(fileName);
  FlashInterface flash(fileName);
  if (flash.isValid()) {
    ui->FramFWInfo->show();
    ui->VersionField->setText(flash.getVers());
    ui->DateField->setText(flash.getDate() + " " + flash.getTime());
    ui->SVNField->setText(flash.getSvn());
    ui->ModField->setText(flash.getBuild());
    ui->BurnFlashButton->setEnabled(true);
    ui->BurnFlashButton->setText(tr("Burn to TX"));
    if (flash.hasSplash()) {
      ui->SplashFrame->show();
      ui->ImageLoadButton->setEnabled(true);
      ui->libraryButton->setEnabled(true);
      ui->FwImage->show();
      ui->FwImage->setPixmap(QPixmap::fromImage(flash.getSplash()));
      QString ImageStr = settings.value("SplashImage", "").toString();
      if (!ImageStr.isEmpty()) {
        QImage Image = qstring2image(ImageStr);
        ui->imageLabel->setPixmap(QPixmap::fromImage(Image.convertToFormat(QImage::Format_Mono)));
        ui->InvertColorButton->setEnabled(true);
        ui->PreferredImageCB->setChecked(true);
      }
      else {
        QString fileName=ui->ImageFileName->text();
        if (!fileName.isEmpty()) {
          QImage image(fileName);
          if (!image.isNull()) {
            ui->InvertColorButton->setEnabled(true);
            ui->imageLabel->setPixmap(QPixmap::fromImage(image.scaled(SPLASH_WIDTH, SPLASH_HEIGHT).convertToFormat(QImage::Format_Mono)));
            ui->PatchFWCB->setEnabled(true);
          }
          else {
            ui->PatchFWCB->setDisabled(true);
            ui->PatchFWCB->setChecked(false);
            ui->PreferredImageCB->setDisabled(true);         
          }
        }
        else {
          ui->PatchFWCB->setDisabled(true);
          ui->PatchFWCB->setChecked(false);
          ui->PreferredImageCB->setDisabled(true);
        }
      }
    }
    else {
      ui->FwImage->hide();
      ui->ImageFileName->setText("");
      ui->SplashFrame->hide();
    }
  }
  else {
    QMessageBox::warning(this, tr("Warning"), tr("%1 is not a known firmware").arg(fileName));
    ui->BurnFlashButton->setText(tr("Burn anyway !"));
    ui->BurnFlashButton->setEnabled(true);
  }
  QTimer::singleShot(0, this, SLOT(shrink()));
  settings.setValue("lastFlashDir", QFileInfo(fileName).dir().absolutePath());
}

void burnDialog::on_ImageLoadButton_clicked()
{
  QString supportedImageFormats;
  for (int formatIndex = 0; formatIndex < QImageReader::supportedImageFormats().count(); formatIndex++) {
    supportedImageFormats += QLatin1String(" *.") + QImageReader::supportedImageFormats()[formatIndex];
  }

  QSettings settings("companion9x", "companion9x");
  QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image to load"), settings.value("lastImagesDir").toString(), tr("Images (%1)").arg(supportedImageFormats));

  if (!fileName.isEmpty()) {
    settings.setValue("lastImagesDir", QFileInfo(fileName).dir().absolutePath());
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, tr("Error"), tr("Cannot load %1.").arg(fileName));
      ui->PatchFWCB->setDisabled(true);
      ui->PatchFWCB->setChecked(false);
      ui->InvertColorButton->setDisabled(true);
      return;
    }
    ui->ImageFileName->setText(fileName);
    ui->InvertColorButton->setEnabled(true);
    ui->imageLabel->setPixmap(QPixmap::fromImage(image.scaled(SPLASH_WIDTH, SPLASH_HEIGHT).convertToFormat(QImage::Format_Mono)));
    ui->PatchFWCB->setEnabled(true);
  }
}

void burnDialog::on_libraryButton_clicked()
{
  QString fileName;
  splashLibrary *ld = new splashLibrary(this,&fileName);
  ld->exec();
  if (!fileName.isEmpty()) {
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, tr("Error"), tr("Cannot load %1.").arg(fileName));
      ui->PatchFWCB->setDisabled(true);
      ui->PatchFWCB->setChecked(false);
      ui->InvertColorButton->setDisabled(true);
      return;
    }
    ui->ImageFileName->setText(fileName);
    ui->InvertColorButton->setEnabled(true);
    ui->imageLabel->setPixmap(QPixmap::fromImage(image.scaled(SPLASH_WIDTH, SPLASH_HEIGHT).convertToFormat(QImage::Format_Mono)));
    ui->PatchFWCB->setEnabled(true);
  }
}

void burnDialog::on_BurnFlashButton_clicked()
{
  if (hexType==2) {
    QString fileName=ui->FWFileName->text();
    if (!fileName.isEmpty()) {
      if (ui->PatchFWCB->isChecked()) {
        QImage image = ui->imageLabel->pixmap()->toImage().scaled(SPLASH_WIDTH, SPLASH_HEIGHT).convertToFormat(QImage::Format_MonoLSB);
        if (!image.isNull()) {
          QString tempDir    = QDir::tempPath();
          QString tempFile = tempDir + "/flash.hex";
          FlashInterface flash(fileName);
          flash.setSplash(image);
          if (flash.saveFlash(tempFile) > 0) {
            hexfileName->clear();
            hexfileName->append(tempFile);
            QSettings settings("companion9x", "companion9x");
            settings.setValue("lastFlashDir", QFileInfo(fileName).dir().absolutePath());
          }
          else {
            hexfileName->clear();
            QMessageBox::critical(this, tr("Warning"), tr("Cannot save customized firmware"));
          }
        }
        else {
          hexfileName->clear();
          QMessageBox::critical(this, tr("Warning"), tr("Custom image not found"));
        }
      }
      else {
            hexfileName->clear();
            hexfileName->append(fileName);
      }
    }
    else {
      QMessageBox::critical(this, tr("Warning"), tr("No firmware selected"));
      hexfileName->clear();     
    }
  }
  this->close();
}

void burnDialog::on_cancelButton_clicked() {
  hexfileName->clear();     
  this->close();  
}

void burnDialog::on_InvertColorButton_clicked() {
    QImage image = ui->imageLabel->pixmap()->toImage();
    image.invertPixels();
    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
}

void burnDialog::on_PreferredImageCB_toggled(bool checked) {
  QString tmpFileName;
  if (checked) {
    QSettings settings("companion9x", "companion9x");
    QString ImageStr = settings.value("SplashImage", "").toString();
    if (!ImageStr.isEmpty()) {
      QImage Image = qstring2image(ImageStr);
      ui->imageLabel->setPixmap(QPixmap::fromImage(Image.convertToFormat(QImage::Format_Mono)));
      ui->InvertColorButton->setEnabled(true);
      ui->PreferredImageCB->setChecked(true);
      ui->ImageFileName->setDisabled(true);
      ui->ImageLoadButton->setDisabled(true);
      ui->libraryButton->setDisabled(true);
      ui->PatchFWCB->setEnabled(true);
    }
  }
  else {
    ui->imageLabel->clear();
    ui->ImageLoadButton->setEnabled(true);
    ui->libraryButton->setEnabled(true);
    tmpFileName = ui->ImageFileName->text();
    if (!tmpFileName.isEmpty()) {
      QImage image(tmpFileName);
      if (!image.isNull()) {
        ui->imageLabel->setPixmap(QPixmap::fromImage(image.scaled(128, 64).convertToFormat(QImage::Format_Mono)));
        ui->InvertColorButton->setEnabled(true);
        ui->ImageFileName->setEnabled(true);
      }
      else {
        ui->InvertColorButton->setDisabled(true);
        ui->PatchFWCB->setDisabled(true);
        ui->PatchFWCB->setChecked(false);
      }
    }
    else {
      ui->InvertColorButton->setDisabled(true);
      ui->PatchFWCB->setDisabled(true);
      ui->PatchFWCB->setChecked(false);
    }
  }
}

void burnDialog::shrink()
{
    resize(0,0);
}

void burnDialog::on_EEbackupCB_clicked()
{
  if (ui->EEbackupCB->isChecked()) {
    *backup=true;
  }
  else {
    *backup=false;
  }
}
