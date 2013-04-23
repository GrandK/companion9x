#include "avroutputdialog.h"
#include "ui_avroutputdialog.h"
#include <QtGui>
#include "eeprominterface.h"

#if !__GNUC__
#include <Windows.h>
#include <WinBase.h>
#include <tlhelp32.h>
#endif

avrOutputDialog::avrOutputDialog(QWidget *parent, QString prog, QStringList arg, QString wTitle, int closeBehaviour, bool displayDetails) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    ui(new Ui::avrOutputDialog),
    kill_timer(NULL),
    hasErrors(false)
{
    ui->setupUi(this);

#ifdef __APPLE__
    QFont newFont("Courier", 13);
    ui->plainTextEdit->setFont(newFont);
    ui->plainTextEdit->setAttribute(Qt::WA_MacNormalSize);
#endif
#if defined WIN32 || !defined __GNUC__
    QFont newFont("Courier", 9);
    ui->plainTextEdit->setFont(newFont);
#endif

    cmdLine = prog;
    closeOpt = closeBehaviour;
    if (cmdLine.isEmpty()) {
      sourceFile=arg.at(0);
      destFile=arg.at(1);
      ui->plainTextEdit->hide();
      ui->progressBar->setMaximum(127);
      QTimer::singleShot(0, this, SLOT(shrink()));
      QTimer::singleShot(0, this, SLOT(doCopy()));
    } else {
      if(wTitle.isEmpty())
          setWindowTitle(getProgrammer() + tr(" result"));
      else
          setWindowTitle(getProgrammer() + " - " + wTitle);
      QFile exec;
      winTitle=wTitle;

      if (!(exec.exists(prog))) {
        QMessageBox::critical(this, "companion9x", getProgrammer() + tr(" executable not found"));
        closeOpt = AVR_DIALOG_FORCE_CLOSE;
        QTimer::singleShot(0, this, SLOT(forceClose()));
      } else {
        foreach(QString str, arg) cmdLine.append(" " + str);
        lfuse = 0;
        hfuse = 0;
        efuse = 0;
        phase=0;
        currLine.clear();
        prevLine.clear();
        if (!displayDetails) {
            ui->plainTextEdit->hide();
            QTimer::singleShot(0, this, SLOT(shrink()));
        } else {
            ui->checkBox->setChecked(true);
        }
        process = new QProcess(this);
        connect(process,SIGNAL(readyReadStandardError()), this, SLOT(doAddTextStdErr()));
        connect(process,SIGNAL(started()),this,SLOT(doProcessStarted()));
        connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(doAddTextStdOut()));
        connect(process,SIGNAL(finished(int)),this,SLOT(doFinished(int)));

  #if !__GNUC__
        kill_timer = new QTimer(this);
        connect(kill_timer, SIGNAL(timeout()), this, SLOT(killTimerElapsed()));
        kill_timer->start(2000);
  #endif

        process->start(prog,arg);
      }
    }  
}

# if !__GNUC__
BOOL KillProcessByName(char *szProcessToKill){
        HANDLE hProcessSnap;
        HANDLE hProcess;
        PROCESSENTRY32 pe32;
        DWORD dwPriorityClass;

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // Takes a snapshot of all the processes

        if(hProcessSnap == INVALID_HANDLE_VALUE){
                return( FALSE );
        }

        pe32.dwSize = sizeof(PROCESSENTRY32);

        if(!Process32First(hProcessSnap, &pe32)){
                CloseHandle(hProcessSnap);
                return( FALSE );
        }

        do{
                if(!strcmp(pe32.szExeFile,szProcessToKill)){    //  checks if process at current position has the name of to be killed app
                        hProcess = OpenProcess(PROCESS_TERMINATE,0, pe32.th32ProcessID);  // gets handle to process
                        TerminateProcess(hProcess,0);   // Terminate process by handle
                        CloseHandle(hProcess);  // close the handle
                }
        }while(Process32Next(hProcessSnap,&pe32));  // gets next member of snapshot

        CloseHandle(hProcessSnap);  // closes the snapshot handle
        return( TRUE );
}
#endif

void avrOutputDialog::doCopy() 
{
  hasErrors=false;
  char buf[65536];
  char * pointer=buf;
  QFile source(sourceFile);
  if (!source.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr("Error"),tr("Cannot open source file"));
    hasErrors=true;
  } else {
    source.read(buf,sizeof(buf));
    source.close();
    QFile dest(destFile);
    if (!dest.open(QIODevice::WriteOnly)) {
      QMessageBox::warning(this, tr("Error"),tr("Cannot write destination"));
      hasErrors=true;
    } else {
      for (int i=0;i<128;i++) {
        if (dest.write(pointer,512)!=512) {
          hasErrors=true;
          break;
        };
        dest.flush();
        dest.flush();
        pointer+=512;
        ui->progressBar->setValue(i);
      }
      dest.close();
    }
  }
  doFinished(0);
}

void avrOutputDialog::killTimerElapsed()
{
  delete kill_timer;
  kill_timer = NULL;
# if !__GNUC__
  KillProcessByName("tasklist.exe");
#endif
}

avrOutputDialog::~avrOutputDialog()
{
    delete ui;
    delete kill_timer;
}

void avrOutputDialog::runAgain(QString prog, QStringList arg, int closeBehaviour)
{
    cmdLine = prog;
    foreach(QString str, arg) cmdLine.append(" " + str);
    closeOpt = closeBehaviour;
    currLine.clear();
    prevLine.clear();
    process->start(prog,arg);
}

void avrOutputDialog::waitForFinish()
{
    process->waitForFinished();
}

void avrOutputDialog::addText(const QString &text)
{
    int val = ui->plainTextEdit->verticalScrollBar()->maximum();
    ui->plainTextEdit->insertPlainText(text);
    if(val!=ui->plainTextEdit->verticalScrollBar()->maximum())
      ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum());
}


void avrOutputDialog::doAddTextStdOut()
{
    QByteArray data = process->readAllStandardOutput();
    QString text = QString(data);
    QString temp;
    int nlPos, pos, size;

    addText(text);
    currStdLine.append(text);
    if (currStdLine.contains("size = ")) {
      pos=currStdLine.lastIndexOf("size = ");
      temp=currStdLine.mid(pos+7);
      pos=temp.lastIndexOf("\n");
      size=temp.left(pos).toInt();
      ui->progressBar->setMaximum(size/2048);
    }
    if (currStdLine.contains("\n")) {
        nlPos=currStdLine.lastIndexOf("\n");
        prevStdLine=currStdLine.left(nlPos).trimmed();
        currStdLine=currStdLine.mid(nlPos+1);
    }
    if (!currStdLine.isEmpty()) {
      if (currStdLine.at(0)==QChar('.')) {
        pos=currStdLine.lastIndexOf(".");
        ui->progressBar->setValue(pos);
      }
    }
    if (text.contains("Complete ")) {
#if !__GNUC__
      if (kill_timer) {
        delete kill_timer;
        kill_timer = NULL;
      }
#endif
      int start = text.indexOf("Complete ");
      int end = text.indexOf("%");
      if (start > 0) {
        start += 9;
        int value = text.mid(start, end-start).toInt();
        ui->progressBar->setValue(value);
      }
    }

    //addText("\n=====\n" + text + "\n=====\n");

    if(text.contains(":010000")) //contains fuse info
    {
        QStringList stl = text.split(":01000000");

        foreach (QString t, stl)
        {
            bool ok = false;
            if(!lfuse)        lfuse = t.left(2).toInt(&ok,16);
            if(!hfuse && !ok) hfuse = t.left(2).toInt(&ok,16);
            if(!efuse && !ok) efuse = t.left(2).toInt(&ok,16);
        }
    }

    if (text.contains("-E-")) {
      hasErrors = true;
    }

}

QString avrOutputDialog::getProgrammer()
{
  EEPROMInterface *eepromInterface = GetEepromInterface();
  if (eepromInterface->getBoard()==BOARD_TARANIS) {
    return "DFU Util";
  } else if (eepromInterface->getBoard()==BOARD_SKY9X) {
    return "SAM-BA";
  } else {
    return "AVRDUDE";
  }
}

void avrOutputDialog::doAddTextStdErr()
{
    int nlPos;
    int pbvalue;
    QString avrphase;
    QByteArray data = process->readAllStandardError();
    QString text = QString(data);

    currLine.append(text);
    if (currLine.contains("#")) {
        avrphase=currLine.left(1).toLower();
        if (avrphase=="w") {
            ui->progressBar->setStyleSheet("QProgressBar  {text-align: center;} QProgressBar::chunk { background-color: #ff0000; text-align:center;}:");
            phase=1;
            if(winTitle.isEmpty())
                setWindowTitle(getProgrammer() + " - " + tr("Writing"));
            else
                setWindowTitle(getProgrammer() + " - " + winTitle + " - " + tr("Writing"));
            pbvalue=currLine.count("#")*2;
            ui->progressBar->setValue(pbvalue);
        }
        if (avrphase=="r") {
            if (phase==0) {
                ui->progressBar->setStyleSheet("QProgressBar  {text-align: center;} QProgressBar::chunk { background-color: #00ff00; text-align:center;}:");
                if(winTitle.isEmpty())
                    setWindowTitle(getProgrammer() + " - " + tr("Reading"));
                else
                    setWindowTitle(getProgrammer() + " - " + winTitle + " - " + tr("Reading"));
            } else {
                ui->progressBar->setStyleSheet("QProgressBar  {text-align: center;} QProgressBar::chunk { background-color: #0000ff; text-align:center;}:");
                phase=2;
                if(winTitle.isEmpty())
                    setWindowTitle(getProgrammer() + " - " + tr("Verifying"));
                else
                    setWindowTitle(getProgrammer() + " - " + winTitle + " - " + tr("Verifying"));
            }
            pbvalue=currLine.count("#")*2;
            ui->progressBar->setValue(pbvalue);
        }
    }
    if (currLine.contains("\n")) {
        nlPos=currLine.lastIndexOf("\n");
        prevLine=currLine.left(nlPos).trimmed();
        currLine=currLine.mid(nlPos+1);
    }
    if (text.contains("-E-") && !text.contains("-E- No receive file name")) {
      hasErrors = true;
    }

    addText(text);
}

#define HLINE_SEPARATOR "================================================================================="
void avrOutputDialog::doFinished(int code=0)
{
    addText("\n" HLINE_SEPARATOR);
    if (code==1 && getProgrammer()=="SAM-BA")
      code=0;
    
    if (code) {
      ui->checkBox->setChecked(true);
      addText("\n" + getProgrammer() + tr(" done - exit code %1").arg(code));
    } else if (hasErrors) {
      ui->checkBox->setChecked(true);
      addText("\n" + getProgrammer() + tr(" done with errors"));
    } else if (!cmdLine.isEmpty()) {
      addText("\n" + getProgrammer() + tr(" done - SUCCESSFUL"));
    } else {
      addText(tr("done - SUCCESSFUL"));
    }
    addText("\n" HLINE_SEPARATOR "\n");

    if(lfuse || hfuse || efuse) addReadFuses();
    
    switch(closeOpt)
    {
      case AVR_DIALOG_CLOSE_IF_SUCCESSFUL:
        if (!hasErrors && !code) accept();
        break;

      case AVR_DIALOG_FORCE_CLOSE:
        if (hasErrors || code)
          reject();
        else
          accept();
        break;

      case AVR_DIALOG_SHOW_DONE:
        if (hasErrors || code) {
          if (!cmdLine.isEmpty()) {
            QMessageBox::critical(this, "companion9x", getProgrammer() + tr(" did not finish correctly"));
          } else {
            QMessageBox::critical(this, "companion9x",  tr("Copy did not finish correctly"));
          }
            // reject();
        } else {
          if (!cmdLine.isEmpty()) {
            ui->progressBar->setValue(100);
            QMessageBox::information(this, "companion9x", getProgrammer() + tr(" finished correctly"));
            accept();
          } else {
            QMessageBox::information(this, "companion9x", tr("Copy finished correctly"));
            accept();            
          }
        }
        break;

    default: //AVR_DIALOG_KEEP_OPEN
        break;
    }


}

void avrOutputDialog::doProcessStarted()
{
    addText(HLINE_SEPARATOR "\n");
    addText(tr("Started ") + getProgrammer() + "\n");
    addText(cmdLine);
    addText("\n" HLINE_SEPARATOR "\n");
}



void avrOutputDialog::addReadFuses()
{
    addText(HLINE_SEPARATOR "\n");
    addText(tr("FUSES: Low=%1 High=%2 Ext=%3").arg(lfuse,2,16,QChar('0')).arg(hfuse,2,16,QChar('0')).arg(efuse,2,16,QChar('0')));
    addText("\n" HLINE_SEPARATOR "\n");
}

void avrOutputDialog::on_checkBox_toggled(bool checked) {
    if (checked) {
        ui->plainTextEdit->show();
    } else {
        ui->plainTextEdit->hide();
        QTimer::singleShot(0, this, SLOT(shrink()));
    }
}

void avrOutputDialog::shrink() {
    resize(0,0);
}

void avrOutputDialog::forceClose() {
    accept();;
}
