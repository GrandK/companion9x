#include "printdialog.h"
#include "ui_printdialog.h"
#include "helpers.h"
#include "eeprominterface.h"
#include <QtGui>
#include <QImage>
#include <QColor>
#include <QPainter>

#if !defined WIN32 && defined __GNUC__
#include <unistd.h>
#endif

#define ISIZE 200 // curve image size
#define ISIZEW 400 // curve image size

printDialog::printDialog(QWidget *parent, GeneralSettings *gg, ModelData *gm) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    ui(new Ui::printDialog)
{
    ui->setupUi(this);
    g_model = gm;
    g_eeGeneral = gg;
    eepromInterface = GetEepromInterface();
    te = ui->textEdit;

    setWindowTitle(tr("Setup for: ") + g_model->name);
    ui->textEdit->clear();
    
    curvefile5=QString("%1/%2-curve5.png").arg(qd->tempPath()).arg(g_model->name);
    curvefile9=QString("%1/%2-curve9.png").arg(qd->tempPath()).arg(g_model->name);
    printSetup();
    if (GetCurrentFirmwareVariant() & GVARS_VARIANT) {
      te->append(printPhases());
      te->append("<br>");
    }
    printExpo();
    printMixes();
    printLimits();
    printCurves();
    printSwitches();
    printSafetySwitches();
    printFSwitches();
    printFrSky();
    
    te->scrollToAnchor("1");
}

void printDialog::closeEvent(QCloseEvent *event) 
{
    QByteArray ba = curvefile5.toLatin1();
    char *name = ba.data(); 
    unlink(name);
    ba = curvefile9.toLatin1();
    name = ba.data(); 
    unlink(name);    
}

printDialog::~printDialog()
{
    delete ui;
}

QString doTC(const QString s, const QString color="", bool bold=false)
{
    QString str = s;
    if(bold) str = "<b>" + str + "</b>";
    if(!color.isEmpty()) str = "<font color=" + color + ">" + str + "</font>";
    return "<td align=center>" + str + "</td>";
}

QString doTR(const QString s, const QString color="", bool bold=false)
{
    QString str = s;
    if(bold) str = "<b>" + str + "</b>";
    if(!color.isEmpty()) str = "<font color=" + color + ">" + str + "</font>";
    return "<td align=right>" + str + "</td>";
}

QString doTL(const QString s, const QString color="", bool bold=false)
{
    QString str = s;
    if(bold) str = "<b>" + str + "</b>";
    if(!color.isEmpty()) str = "<font color=" + color + ">" + str + "</font>";
    return "<td align=left>" + str + "</td>";
}


QString printDialog::fv(const QString name, const QString value)
{
    return "<b>" + name + ": </b><font color=green>" + value + "</font><br>";
}

QString printDialog::getTimer1()
{
  // TODO timer2
    QString str = ", " + (g_model->timers[0].dir ? tr("Count Up") : tr("Count Down"));
    return tr("%1:%2, ").arg(g_model->timers[0].val/60, 2, 10, QChar('0')).arg(g_model->timers[0].val%60, 2, 10, QChar('0')) + getTimerMode(g_model->timers[0].mode) + str;
}

QString printDialog::getTimer2()
{
  // TODO timer2
    QString str = ", " + (g_model->timers[1].dir ? tr("Count Up") : tr("Count Down"));
    return tr("%1:%2, ").arg(g_model->timers[1].val/60, 2, 10, QChar('0')).arg(g_model->timers[1].val%60, 2, 10, QChar('0')) + getTimerMode(g_model->timers[1].mode) + str;
}

QString printDialog::FrSkyAtype(int alarm) {
    switch(alarm) {
        case 1:
            return tr("Yellow");
            break;
        case 2:
            return tr("Orange");
            break;
        case 3:
            return tr("Red");
            break;
        default:
            return "----";
            break;
    }
 }

QString printDialog::FrSkyBlades(int blades) {
    switch(blades) {
        case 1:
            return "3";
            break;
        case 2:
            return "4";
            break;
        default:
            return "2";
            break;
    }
 }


QString printDialog::FrSkyUnits(int units) {
    switch(units) {
        case 1:
            return tr("---");
            break;
        default:
            return "V";
            break;
    }
 }

QString printDialog::FrSkyProtocol(int protocol) {
    switch(protocol) {
        case 2:
            if ((GetEepromInterface()->getCapability(Telemetry)&TM_HASWSHH)) {
                 return tr("Winged Shadow How High");
            } else {
                 return tr("Winged Shadow How High (not supported)");
            }
            break;
        case 1:
            return tr("FrSky Sensor Hub");
            break;
        default:
            return tr("None");
            break;
    }
 }

QString printDialog::FrSkyMeasure(int units) {
    switch(units) {
        case 1:
            return tr("Imperial");
            break;
        default:
            return tr("Metric");;
            break;
    }
 }

QString printDialog::getProtocol()
{
    QString str;
    str = QString("PPM   SILV_ASILV_BSILV_CTRAC09").mid(g_model->protocol*6,6).replace(" ","");

    if(!g_model->protocol) //ppm protocol
        str.append(tr(": %1 Channels, %2msec Delay").arg(g_model->ppmNCH).arg(g_model->ppmDelay));

    return str;
}

QString printDialog::getCenterBeep()
{
    //RETA123
    QStringList strl;

    if(g_model->beepANACenter & 0x01) strl << tr("Rudder");
    if(g_model->beepANACenter & 0x02) strl << tr("Elevator");
    if(g_model->beepANACenter & 0x04) strl << tr("Throttle");
    if(g_model->beepANACenter & 0x08) strl << tr("Aileron");
    if(g_model->beepANACenter & 0x10) strl << "P1";
    if(g_model->beepANACenter & 0x20) strl << "P2";
    if(g_model->beepANACenter & 0x40) strl << "P3";

    return strl.join(", ");

}

QString printDialog::getTrimInc()
{
    switch (g_model->trimInc) {
      case (1): return tr("Extra Fine"); break;
      case (2): return tr("Fine"); break;
      case (3): return tr("Medium"); break;
      case (4): return tr("Coarse"); break;
      default: return tr("Exponential"); break;
    }
}

void printDialog::printSetup()
{
    int gvars=0;
    if (GetCurrentFirmwareVariant() & GVARS_VARIANT)
      gvars=1;
    QString str = "<a name=1></a><table border=1 cellspacing=0 cellpadding=3 width=\"100%\">";
    str.append(QString("<tr><td colspan=%1 ><table border=0 width=\"100%\"><tr><td><h1>").arg((GetEepromInterface()->getCapability(FlightPhases) && gvars==0) ? 2 : 1));
    str.append(g_model->name);
    str.append("&nbsp;(");
    str.append(eepromInterface->getName());
    str.append(")</h1></td><td align=right valign=top NOWRAP><font size=-1>"+tr("printed on: %1").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate))+"</font></td></tr></table></td></tr><tr><td><table border=0 cellspacing=0 cellpadding=3>");
    str.append("<tr><td><h2>"+tr("General Model Settings")+"</h2></td></tr>");
    str.append("<tr><td>");
    str.append(fv(tr("Name"), g_model->name));
    str.append("<b>"+tr("EEprom Size")+QString(": </b><font color=green>%1</font><br>").arg(eepromInterface->getSize(*g_model)));
    str.append(fv(tr("Timer1"), getTimer1()));  //value, mode, count up/down
    str.append(fv(tr("Timer2"), getTimer2()));  //value, mode, count up/down
    str.append(fv(tr("Protocol"), getProtocol())); //proto, numch, delay,
    str.append(fv(tr("Pulse Polarity"), g_model->pulsePol ? "NEG" : "POS"));
    str.append(fv(tr("Throttle Trim"), g_model->thrTrim ? tr("Enabled") : tr("Disabled")));
    str.append(fv(tr("Throttle Expo"), g_model->thrExpo ? tr("Enabled") : tr("Disabled")));
    // TODO    str.append(fv(tr("Trim Switch"), getSWName(g_model->trimSw)));
    str.append(fv(tr("Trim Increment"), getTrimInc()));
    str.append(fv(tr("Center Beep"), getCenterBeep())); // specify which channels beep
    str.append("</td></tr></table></td>");
    if (!gvars) {
      str.append("<td width=\"380\">");
      str.append(printPhases());
      str.append("</td>");
    }
    str.append("</tr></table><br>");
    te->append(str);
}

QString printDialog::printPhases()
{      
    int gvars=0;
    if (GetCurrentFirmwareVariant() & GVARS_VARIANT)
      gvars=1;
    QString str="";
    str.append(QString("<table border=1 cellspacing=0 cellpadding=3 width=\"100%\"><tr><td colspan=%1><h2>").arg(gvars==0 ? 8 : 13));
    str.append(tr("Flight Phases Settings"));
    str.append("</h2></td></tr><tr><td style=\"border-style:none;\">&nbsp;</td><td colspan=2 align=center><b>");
    str.append(tr("Fades")+"</b></td>");
    str.append("<td colspan=4 align=center><b>"+tr("Trims"));
    if (gvars) {
      str.append("<td colspan=5 align=center><b>"+tr("Gvars"));
    }
    str.append("</b></td><td rowspan=2 align=\"center\" valign=\"bottom\"><b>"+tr("Switch")+"</b></td></tr><tr><td align=center width=\"90\"><b>"+tr("Phase name"));
    str.append("</b></td><td align=center width=\"30\"><b>"+tr("IN")+"</b></td><td align=center width=\"30\"><b>"+tr("OUT")+"</b></td>");
    for (int i=0; i<4; i++) {
      str.append(QString("<td width=\"40\" align=\"center\"><b>%1</b></td>").arg(getStickStr(i)));
    }
    if (gvars==1) {
      for (int i=1; i<6; i++) {
        str.append(QString("<td width=\"40\" align=\"center\"><b>GV%1</b><br>%2</td>").arg(i).arg(g_model->gvars_names[i-1]));
      }      
    }
    str.append("</tr>");
    for (int i=0; i<GetEepromInterface()->getCapability(FlightPhases); i++) {
      PhaseData *pd=&g_model->phaseData[i];
      str.append("<tr><td><b>"+tr("FP")+QString("%1</b> <font size=+1 face='Courier New' color=green>%2</font></td><td width=\"30\" align=\"right\"><font size=+1 face='Courier New' color=green>%3</font></td><td width=\"30\" align=\"right\"><font size=+1 face='Courier New' color=green>%4</font></td>").arg(i).arg(pd->name).arg(pd->fadeIn).arg(pd->fadeOut));
      for (int k=0; k<4; k++) {
        if (pd->trimRef[k]==-1) {
          str.append(QString("<td align=\"right\" width=\"30\"><font size=+1 face='Courier New' color=green>%1</font></td>").arg(pd->trim[k]));
        } else {
          str.append("<td align=\"right\" width=\"30\"><font size=+1 face='Courier New' color=green>"+tr("FP")+QString("%1</font></td>").arg(pd->trimRef[k]));
        }
      }
      if (gvars==1) {
        for (int k=0; k<5; k++) {
          if (pd->gvars[k]<=1024) {
            str.append(QString("<td align=\"right\" width=\"30\"><font size=+1 face='Courier New' color=green>%1").arg(pd->gvars[k])+"</font></td>");
          }
          else {
            int num = pd->gvars[k] - 1025;
            if (num>=i) num++;
            str.append("<td align=\"right\" width=\"30\"><font size=+1 face='Courier New' color=green>"+tr("FP")+QString("%1</font></td>").arg(num));
          }
        }
      }
      str.append(QString("<td align=center><font size=+1 face='Courier New' color=green>%1</font></td>").arg(pd->swtch.toString()));
      str.append("</tr>");
    }
    str.append("</table>");
    return(str);
}

void printDialog::printExpo()
{
    QString str = "<table border=1 cellspacing=0 cellpadding=3 width=\"100%\"><tr><td><h2>";
    str.append(tr("Expo/Dr Settings"));
    str.append("</h2></td></tr><tr><td><table border=0 cellspacing=0 cellpadding=3>");
    int ec=0;
    int lastCHN = -1;
    for(int i=0; i<MAX_EXPOS; i++) {
      ExpoData *ed=&g_model->expoData[i];
      if(ed->mode==0)
        continue;
      ec++;
      str.append("<tr><td><font size=+1 face='Courier New'>");
      if(lastCHN!=ed->chn) {
        lastCHN=ed->chn;
        str.append("<b>"+getStickStr(ed->chn)+"</b>");
      }
      else
        str.append("<b>&nbsp;</b>");
      str.append("</font></td>");
      str.append("<td><font size=+1 face='Courier New' color=green>");

      switch(ed->mode) {
        case (1): 
          str += "&lt;-&nbsp;";
          break;
        case (2): 
          str += "-&gt;&nbsp;";
          break;
        default:
          str += "&nbsp;&nbsp;&nbsp;";
          break;
      };

      str += tr("Weight") + QString("%1").arg(getGVarString(ed->weight,true)).rightJustified(6, ' ');
      if (!GetEepromInterface()->getCapability(ExpoIsCurve)) {
        if (ed->expo!=0) {
          if (ed->expo<126) {
            str += " " + tr("Expo") + QString("%1").arg(getGVarString(ed->expo)).rightJustified(7, ' ');
          } else {
            str += " " + tr("Expo") + QString("%1").arg(getGVarString(ed->expo)).rightJustified(7, ' ');
          }
        }
      } else {
        if (ed->curveMode==0 && ed->curveParam!=0) {
          if (ed->curveParam<126) {
            str += " " + tr("Expo") + QString("%1").arg(getGVarString(ed->curveParam)).rightJustified(7, ' ');
          } else {
            str += " " + tr("Expo") + QString("%1").arg(getGVarString(ed->curveParam)).rightJustified(7, ' ');
          }
        }
      }   
      if (GetEepromInterface()->getCapability(ExpoFlightPhases)) {
        if(ed->phases) {
          if (ed->phases!=(1<<GetEepromInterface()->getCapability(FlightPhases))-1) {
            int mask=1;
            int first=0;
            for (int i=0; i<GetEepromInterface()->getCapability(FlightPhases);i++) {
              if (!(ed->phases & mask)) {
                first++;
              }
              mask <<=1;
            }
            if (first>1) {
              str += " " + tr("Phases") + QString("(");
            } else {
              str += " " + tr("Phase") + QString("(");
            }
            mask=1;
            first=1;
            for (int j=0; j<GetEepromInterface()->getCapability(FlightPhases);j++) {
              if (!(ed->phases & mask)) {
                PhaseData *pd = &g_model->phaseData[j];
                if (!first) {
                  str += QString(", ")+ QString("%1").arg(getPhaseName(j+1, pd->name));
                } else {
                  str += QString("%1").arg(getPhaseName(j+1,pd->name));
                  first=0;
                }
              }
              mask <<=1;
            }
            str += QString(")");
          } else {
            str += tr("DISABLED")+QString(" !!!");
          }
        }
      } else {
        if (ed->phase!=0) {
          PhaseData *pd = &g_model->phaseData[abs(ed->phase)-1];
          if (ed->phase<0) {
            str += " "+tr("Phase")+" !"+tr("FP")+QString("%1 (!%2)").arg(-(ed->phase+1)).arg(pd->name);
          } else {
            str += " "+tr("Phase")+" "+tr("FP")+QString("%1 (%2)").arg(ed->phase-1).arg(pd->name);               
          }
        }
      }
      if (ed->swtch.type) 
        str += " " + tr("Switch") + QString("(%1)").arg(ed->swtch.toString());
      if (ed->curveMode)
        if (ed->curveParam) 
            str += " " + tr("Curve") + QString("(%1)").arg(getCurveStr(ed->curveParam).replace("<", "&lt;").replace(">", "&gt;"));
      if (GetEepromInterface()->getCapability(HasExpoNames)) {
        QString ExpoName;
        ExpoName.append(ed->name);
        if (!ExpoName.isEmpty()) {
          str+=QString(" (%1)").arg(ExpoName);
        }
      }
      str += "</font></td></tr>";
    }
    str += "</table></td></tr></table><br>";
    if (ec>0)
      te->append(str);
}


void printDialog::printMixes()
{
    QString str = "<table border=1 cellspacing=0 cellpadding=3 style=\"page-break-after:always;\" width=\"100%\"><tr><td><h2>";
    str.append(tr("Mixers"));
    str.append("</h2></td></tr><tr><td><table border=0 cellspacing=0 cellpadding=3>");

    int lastCHN = -1;
    for(int i=0; i<GetEepromInterface()->getCapability(Mixes); i++) {
      MixData *md = &g_model->mixData[i];
      if(!md->destCh) break;
      str.append("<tr><td><font size=+1 face='Courier New'>");
      if(lastCHN!=md->destCh) {
        lastCHN=md->destCh;
        str.append("<b>"+tr("CH")+QString("%1</b>").arg(lastCHN,2,10,QChar('0')));
      }
      else
      str.append("&nbsp;");
      str.append("</font></td>");
      str.append("<td><font size=+1 face='Courier New' color=green>");
      switch(md->mltpx) {
        case (1): str += "&nbsp;*"; break;
        case (2): str += "&nbsp;R"; break;
        default:  str += "&nbsp;&nbsp;"; break;
      };
      str += QString(" %1").arg(getGVarString(md->weight, true)).rightJustified(6, ' ');
      str += md->srcRaw.toString();
      if (md->swtch.type) str += " " + tr("Switch") + QString("(%1)").arg(md->swtch.toString());
      if (md->carryTrim) str += " " + tr("noTrim");
      if(GetEepromInterface()->getCapability(MixFmTrim) && md->enableFmTrim==1){ 
        if (md->sOffset)  str += " "+ tr("FMTrim") + QString(" %1").arg(getGVarString(md->sOffset));
      } else {
        if (md->sOffset)  str += " "+ tr("Offset") + QString(" %1").arg(getGVarString(md->sOffset));           
      }
      if (md->differential)  str += " "+ tr("Diff") + QString(" %1").arg(getGVarString(md->differential));
      if (md->curve) str += " " + tr("Curve") + QString("(%1)").arg(getCurveStr(md->curve).replace("<", "&lt;").replace(">", "&gt;"));
      if (md->delayDown || md->delayUp) str += tr(" Delay(u%1:d%2)").arg(md->delayUp).arg(md->delayDown);
      if (md->speedDown || md->speedUp) str += tr(" Slow(u%1:d%2)").arg(md->speedUp).arg(md->speedDown);
      if (md->mixWarn)  str += " "+tr("Warn")+QString("(%1)").arg(md->mixWarn);
      if (GetEepromInterface()->getCapability(MixFlightPhases)) {
        if(md->phases) {
          if (md->phases!=(1<<GetEepromInterface()->getCapability(FlightPhases))-1) {
            int mask=1;
            int first=0;
            for (int i=0; i<GetEepromInterface()->getCapability(FlightPhases);i++) {
              if (!(md->phases & mask)) {
                first++;
              }
              mask <<=1;
            }
            if (first>1) {
              str += " " + tr("Phases") + QString("(");
            } else {
              str += " " + tr("Phase") + QString("(");
            }
            mask=1;
            first=1;
            for (int j=0; j<GetEepromInterface()->getCapability(FlightPhases);j++) {
              if (!(md->phases & mask)) {
                PhaseData *pd = &g_model->phaseData[j];
                if (!first) {
                  str += QString(", ")+ QString("%1").arg(getPhaseName(j+1, pd->name));
                } else {
                  str += QString("%1").arg(getPhaseName(j+1,pd->name));
                  first=0;
                }
              }
              mask <<=1;
            }
            str += QString(")");
          } else {
            str += tr("DISABLED")+QString(" !!!");
          }
        }
      } else {
        if (md->phase!=0) {
          PhaseData *pd = &g_model->phaseData[abs(md->phase)-1];
          if (md->phase<0) {
            str += " "+tr("Phase")+" !"+tr("FP")+QString("%1 (!%2)").arg(-(md->phase+1)).arg(pd->name);
          } else {
            str += " "+tr("Phase")+" "+tr("FP")+QString("%1 (%2)").arg(md->phase-1).arg(pd->name);               
          }
        }
      }
      if (GetEepromInterface()->getCapability(HasMixerNames)) {
        QString MixerName;
        MixerName.append(md->name);
        if (!MixerName.isEmpty()) {
          str+=QString(" (%1)").arg(MixerName);
        }
      }
      str.append("</font></td></tr>");
    }
    str.append("</table></td></tr></table><br>");
    te->append(str);
}

void printDialog::printLimits()
{
    QString str = "<table border=1 cellspacing=0 cellpadding=3 width=\"100%\">";
    int numcol;
    numcol=(GetEepromInterface()->getCapability(Outputs)+1)>17 ? 17:GetEepromInterface()->getCapability(Outputs)+1;
    str.append(QString("<tr><td colspan=%1><h2>").arg(numcol)+tr("Limits")+"</h2></td></tr>");
    str.append("<tr><td>&nbsp;</td>");
    if (GetEepromInterface()->getCapability(Outputs)<17) {
      for(int i=0; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTC(tr("CH")+QString(" %1").arg(i+1,2,10,QChar('0')),"",true));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Offset")+"</b></td>");
      for(int i=0; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString::number((qreal)g_model->limitData[i].offset/10, 'f', 1),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Min")+"</b></td>");
      for(int i=0; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString::number(g_model->limitData[i].min),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Max")+"</b></td>");
      for(int i=0; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString::number(g_model->limitData[i].max),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Invert")+"</b></td>");
      for(int i=0; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString(g_model->limitData[i].revert ? tr("INV") : tr("NOR")),"green"));
      }
    } else {
      for(int i=0; i<16; i++) {
        str.append(doTC(tr("CH")+QString(" %1").arg(i+1,2,10,QChar('0')),"",true));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Offset")+"</b></td>");
      for(int i=0; i<16; i++) {
        str.append(doTR(QString::number((qreal)g_model->limitData[i].offset/10, 'f', 1),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Min")+"</b></td>");
      for(int i=0; i<16; i++) {
        str.append(doTR(QString::number(g_model->limitData[i].min),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Max")+"</b></td>");
      for(int i=0; i<16; i++) {
        str.append(doTR(QString::number(g_model->limitData[i].max),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Invert")+"</b></td>");
      for(int i=0; i<16; i++) {
        str.append(doTR(QString(g_model->limitData[i].revert ? tr("INV") : tr("NOR")),"green"));
      }
      str.append("</tr>");
      str.append(QString("<tr><td colspan=%1>&nbsp;").arg(numcol)+"</td></tr>");
      str.append("<tr><td>&nbsp;</td>");
      for(int i=16; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTC(tr("CH")+QString(" %1").arg(i+1,2,10,QChar('0')),"",true));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Offset")+"</b></td>");
      for(int i=16; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString::number((qreal)g_model->limitData[i].offset/10, 'f', 1),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Min")+"</b></td>");
      for(int i=16; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString::number(g_model->limitData[i].min),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Max")+"</b></td>");
      for(int i=16; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString::number(g_model->limitData[i].max),"green"));
      }
      str.append("</tr>");
      str.append("<tr><td><b>"+tr("Invert")+"</b></td>");
      for(int i=16; i<GetEepromInterface()->getCapability(Outputs); i++) {
        str.append(doTR(QString(g_model->limitData[i].revert ? tr("INV") : tr("NOR")),"green"));
      }
    }
    str.append("</tr>");
    str.append("</table>");
    str.append("<br>");
    te->append(str);
}

void printDialog::printCurves()
{
    int i,r,g,b,c,count;
    char buffer[16];
    QPen pen(Qt::black, 2, Qt::SolidLine);
    QColor * qplot_color[16];
    qplot_color[0]=new QColor(0,0,127);
    qplot_color[1]=new QColor(0,127,0);
    qplot_color[2]=new QColor(127,0,0);
    qplot_color[3]=new QColor(0,127,127);
    qplot_color[4]=new QColor(127,0,127);
    qplot_color[5]=new QColor(127,127,0);
    qplot_color[6]=new QColor(127,127,127);
    qplot_color[7]=new QColor(0,0,255);
    qplot_color[8]=new QColor(0,127,255);
    qplot_color[9]=new QColor(127,0,255);
    qplot_color[10]=new QColor(0,255,0);
    qplot_color[11]=new QColor(0,255,127);
    qplot_color[12]=new QColor(127,255,0);
    qplot_color[13]=new QColor(255,0,0);
    qplot_color[14]=new QColor(255,0,127);
    qplot_color[15]=new QColor(255,127,0);
    
    QString str = "<table border=1 cellspacing=0 cellpadding=3 style=\"page-break-before:auto;\" width=\"100%\"><tr><td><h2>";
    str.append(tr("Curves"));
    str.append("</h2></td></tr><tr><td>");
    int numcurves=GetEepromInterface()->getCapability(NumCurves);
    if (numcurves==0) {
      numcurves=16;
    }
    if (!GetEepromInterface()->getCapability(CustomCurves)) {
      QImage qi(ISIZE+1,ISIZE+1,QImage::Format_RGB32);
      QPainter painter(&qi);
      painter.setBrush(QBrush("#FFFFFF"));
      painter.setPen(QColor(0,0,0));
      painter.drawRect(0,0,ISIZE,ISIZE);
      str.append("<table border=1 cellspacing=0 cellpadding=3 width=\"100%\"><tr><td colspan=2><b>"+tr("5 Points Curves")+QString("</b></td></tr><tr><td width=\"200\"><img src=\"%1\" border=0></td><td><table border=1 cellspacing=0 cellpadding=3 width=\"100%\">").arg(curvefile5));
      str.append("<tr>");
      str.append(doTC("&nbsp;"));
      for(i=0; i<5; i++) 
          str.append(doTC(tr("pt %1").arg(i+1), "", true));
      str.append("</tr>");
      for(i=0; i<8; i++) {
        pen.setColor(*qplot_color[i]);
        painter.setPen(pen);
        qplot_color[i]->getRgb(&r,&g,&b);
        c=r;
        c*=256;
        c+=g;
        c*=256;
        c+=b;
        sprintf(buffer,"%06x",c);
        str.append("<tr>");
        str.append(QString("<td width=\"70\"><font color=#%1><b>").arg(buffer)+tr("Curve")+QString(" %1</b></font></td>").arg(i+1));
        count=0;
        for(int j=0; j<5; j++) {
          if (g_model->curves[i].points[j].y!=0)
            count++;
        }
        for(int j=0; j<5; j++) {    
          str.append(doTR(QString::number(g_model->curves[i].points[j].y),"green"));
          if (j>0 && count!=0) {
            painter.drawLine(ISIZE*(j-1)/4,ISIZE/2-(ISIZE*g_model->curves[i].points[j-1].y)/200,ISIZE*(j)/4,ISIZE/2-(ISIZE*g_model->curves[i].points[j].y)/200);
          }
        }
        str.append("</tr>");
      }
      str.append("</table></td></tr></table></td></tr><tr><td>");
      painter.setPen(QColor(0,0,0));
      painter.drawLine(0,ISIZE/2,ISIZE,ISIZE/2);
      painter.drawLine(ISIZE/2,0,ISIZE/2,ISIZE);
      for(i=0; i<5; i++) {
        painter.drawLine(ISIZE/2-2,(ISIZE*i)/4,ISIZE/2+2,(ISIZE*i)/4);
        painter.drawLine((ISIZE*i)/4,ISIZE/2-2,(ISIZE*i)/4,ISIZE/2+2);
      }

      qi.save(curvefile5, "png",100); 
      str.append("<table border=1 cellspacing=0 cellpadding=3 width=\"100%\"><tr><td colspan=2><b>"+tr("9 Points Curves")+QString("</b></td></tr><tr><td width=\"200\"><img src=\"%1\" border=0></td><td><table border=1 cellspacing=0 cellpadding=3 width=\"100%\">").arg(curvefile9));
      str.append("<tr><td width=\"70\">&nbsp;</td>");
      for(i=0; i<9; i++) str.append(doTC(tr("pt %1").arg(i+1), "", true));
      str.append("</tr>");

      painter.setBrush(QBrush("#FFFFFF"));
      painter.setPen(QColor(0,0,0));
      painter.drawRect(0,0,ISIZE,ISIZE);
      for(i=0; i<9; i++) {
        painter.drawLine(ISIZE/2-2,(ISIZE*i)/8,ISIZE/2+2,(ISIZE*i)/8);
        painter.drawLine((ISIZE*i)/8,ISIZE/2-2,(ISIZE*i)/8,ISIZE/2+2);
      }
      for(i=0; i<8; i++) {
        pen.setColor(*qplot_color[i]);
        painter.setPen(pen);
        qplot_color[i]->getRgb(&r,&g,&b);
        c=r;
        c*=256;
        c+=g;
        c*=256;
        c+=b;
        sprintf(buffer,"%06x",c);
        str.append("<tr>");
        str.append(QString("<td width=\"70\"><font color=#%1><b>").arg(buffer)+tr("Curve")+QString(" %1</b></font></td>").arg(i+9));
        count=0;
        for(int j=0; j<9; j++) {
          if (g_model->curves[i].points[j].y!=0)
            count++;
        }
        for(int j=0; j<9; j++) {
          str.append(doTR(QString::number(g_model->curves[i].points[j].y),"green"));
          if (j>0 && count!=0) {
            painter.drawLine(ISIZE*(j-1)/8,ISIZE/2-(ISIZE*g_model->curves[i+8].points[j-1].y)/200,ISIZE*(j)/8,ISIZE/2-(ISIZE*g_model->curves[i+8].points[j].y)/200);
          }
        }
        str.append("</tr>");
      }
      str.append("</table></td></tr></table></td></tr></table>");
      str.append("<br>");
      painter.setPen(QColor(0,0,0));
      painter.drawLine(0,ISIZE/2,ISIZE,ISIZE/2);
      painter.drawLine(ISIZE/2,0,ISIZE/2,ISIZE);
      for(i=0; i<9; i++) {
        painter.drawLine(ISIZE/2-2,(ISIZE*i)/8,ISIZE/2+2,(ISIZE*i)/8);
        painter.drawLine((ISIZE*i)/8,ISIZE/2-2,(ISIZE*i)/8,ISIZE/2+2);
      }
      qi.save(curvefile9, "png",100);     
    } else {
      QImage qi(ISIZEW+1,ISIZEW+1,QImage::Format_RGB32);
      QPainter painter(&qi);
      painter.setBrush(QBrush("#FFFFFF"));
      painter.setPen(QColor(0,0,0));
      painter.drawRect(0,0,ISIZEW,ISIZEW);
      str.append("<table border=0 cellspacing=0 cellpadding=3 width=\"100%\">"+QString("<tr><td width=\"400\"><img src=\"%1\" border=0></td><td><table border=1 cellspacing=0 cellpadding=3 width=\"100%\">").arg(curvefile5));
      for(i=0; i<numcurves; i++) {
        pen.setColor(*qplot_color[i]);
        painter.setPen(pen);
        qplot_color[i]->getRgb(&r,&g,&b);
        c=r;
        c*=256;
        c+=g;
        c*=256;
        c+=b;
        sprintf(buffer,"%06x",c);
        str.append(QString("<tr><td width=\"70\"><font color=#%1><b>").arg(buffer)+tr("Curve")+QString(" %1</b></font></td></tr>").arg(i+1));
      }
      str.append("</table></td></tr><tr><td colspan=2><table border=1 cellspacing=0 cellpadding=3 width=\"100%\">");
      str.append("<tr>");
      str.append(doTC("&nbsp;"));
      str.append(doTC("&nbsp;"));
      int numpoint=0;
      for(i=0; i<numcurves; i++) {
        if (g_model->curves[i].count>numpoint)
          numpoint=g_model->curves[i].count;
      }
      for(i=0; i<numpoint; i++) 
          str.append(doTC(tr("pt %1").arg(i+1), "", true));
      str.append("</tr>");
      for(i=0; i<numcurves; i++) {
        pen.setColor(*qplot_color[i]);
        painter.setPen(pen);
        qplot_color[i]->getRgb(&r,&g,&b);
        c=r;
        c*=256;
        c+=g;
        c*=256;
        c+=b;
        sprintf(buffer,"%06x",c);
        str.append("<tr>");
        int curvepoints=g_model->curves[i].count;
        if (!g_model->curves[i].custom) {
          str.append(QString("<td width=\"70\"><font color=#%1><b>").arg(buffer)+tr("Curve")+QString(" %1</b></font></td><td width=5>Y</td>").arg(i+1));
        } else {
          str.append(QString("<td width=\"70\" rowspan=2 valign=middle><font color=#%1><b>").arg(buffer)+tr("Curve")+QString(" %1</b></font></td><td width=5>Y</td>").arg(i+1));
        }
        count=0;
        for(int j=0; j<curvepoints; j++) {
          if (g_model->curves[i].points[j].y!=0)
            count++;
        }
        for(int j=0; j<curvepoints; j++) {    
          str.append(doTR(QString::number(g_model->curves[i].points[j].y),"green"));
          if (j>0 && count!=0) {
            if (!g_model->curves[i].custom) {
              painter.drawLine(ISIZEW*(j-1)/(curvepoints-1),ISIZEW/2-(ISIZEW*g_model->curves[i].points[j-1].y)/200,ISIZEW*(j)/(curvepoints-1),ISIZEW/2-(ISIZEW*g_model->curves[i].points[j].y)/200);
            } else {
              painter.drawLine(ISIZEW/2+(ISIZEW*g_model->curves[i].points[j-1].x)/200,ISIZEW/2-(ISIZEW*g_model->curves[i].points[j-1].y)/200,ISIZEW/2+(ISIZEW*g_model->curves[i].points[j].x)/200,ISIZEW/2-(ISIZEW*g_model->curves[i].points[j].y)/200);
            }
          }
        }
        for(int j=curvepoints; j<numpoint; j++) {
          str.append(doTR("","green"));
        }     
        str.append("</tr>");
        if (g_model->curves[i].custom) {
          str.append("<tr><td width=5>X</td>");
          for(int j=0; j<curvepoints; j++) {    
            str.append(doTR(QString::number(g_model->curves[i].points[j].x),"green"));
          }
          for(int j=curvepoints; j<numpoint; j++) {
            str.append(doTR("","green"));
          }     
          str.append("</tr>");  
        }
      }
      str.append("</table></td></tr></table></td></tr></table>");
      str.append("<br>");
      painter.setPen(QColor(0,0,0));
      painter.drawLine(0,ISIZEW/2,ISIZEW,ISIZEW/2);
      painter.drawLine(ISIZEW/2,0,ISIZEW/2,ISIZEW);
      for(i=0; i<21; i++) {
        painter.drawLine(ISIZEW/2-5,(ISIZEW*i)/(20),ISIZEW/2+5,(ISIZEW*i)/(20));
        painter.drawLine((ISIZEW*i)/(20),ISIZEW/2-5,(ISIZEW*i)/(20),ISIZEW/2+5);
      }

      qi.save(curvefile5, "png",100); 
      
    }
    te->append(str);
}

void printDialog::printSwitches()
{
    int sc=0;
    QString str = "<table border=1 cellspacing=0 cellpadding=3 width=\"100%\">";
    str.append("<tr><td><h2>"+tr("Custom Switches")+"</h2></td></tr>");
    str.append("<tr><td><table border=0 cellspacing=0 cellpadding=3>");

    for(int i=0; i<GetEepromInterface()->getCapability(CustomSwitches); i++) {
      if(g_model->customSw[i].func) {
        str.append("<tr>");
        str.append("<td width=\"60\" align=\"center\"><b>"+tr("CS")+QString("%1</b></td>").arg(i+1));
        QString tstr;
        switch CS_STATE(g_model->customSw[i].func) {
        case CS_VOFS:
          if (g_model->customSw[i].val1)
            tstr += RawSource(g_model->customSw[i].val1).toString();
          else
            tstr += "0";
          tstr.remove(" ");
          if(g_model->customSw[i].func==CS_APOS || g_model->customSw[i].func==CS_ANEG)
              tstr = "|" + tstr + "|";
          if(g_model->customSw[i].func==CS_APOS || g_model->customSw[i].func==CS_VPOS)
              tstr += " &gt; ";
          if(g_model->customSw[i].func==CS_ANEG || g_model->customSw[i].func==CS_VNEG)
              tstr += " &lt; ";
          tstr += QString::number(g_model->customSw[i].val2);
          break;
        case CS_VBOOL:
          tstr = RawSwitch(g_model->customSw[i].val1).toString();
          switch (g_model->customSw[i].func) {
            case CS_AND:
              tstr += " AND ";
              break;
            case CS_OR:
              tstr += " OR ";
              break;
            case CS_XOR:
              tstr += " XOR ";
              break;
            default:
              break;
          }
          tstr += RawSwitch(g_model->customSw[i].val2).toString();
          break;
        case CS_VCOMP:
          if (g_model->customSw[i].val1)
            tstr += RawSource(g_model->customSw[i].val1).toString();
          else
            tstr += "0";
          switch (g_model->customSw[i].func) {
            case CS_EQUAL:
              tstr += " = ";
              break;
            case CS_NEQUAL:
              tstr += " != ";
              break;
            case CS_GREATER:
              tstr += " &gt; ";
              break;
            case CS_LESS:
              tstr += " &lt; ";
              break;
            case CS_EGREATER:
              tstr += " &gt;= ";
              break;
            case CS_ELESS:
              tstr += " &lt;= ";
              break;
            default:
              break;
          }
          if (g_model->customSw[i].val2)
            tstr += RawSource(g_model->customSw[i].val2).toString();
          else
            tstr += "0";
          break;
        default:
            break;
        }
        str.append(doTC(tstr,"green"));
        str.append("</tr>");
        sc++;
      }
    }
    str.append("</table></td></tr></table>");
    str.append("<br>");
    if (sc!=0)
      te->append(str);
}

void printDialog::printSafetySwitches()
{
    int sc=0;
    QString str = "<table border=1 cellspacing=0 cellpadding=3 width=\"100%\">";
    str.append("<tr><td><h2>"+tr("Safety Switches")+"</h2></td></tr>");
    str.append("<tr><td><table border=0 cellspacing=0 cellpadding=3><tr>");
    str.append("<td width=\"60\">&nbsp;</td>");
    str.append(doTC(tr("Switch"), "", true));
    str.append(doTL(tr("Value"), "", true));
    str.append("</tr>");
    for(int i=0; i<GetEepromInterface()->getCapability(Outputs); i++)
    {
        if (g_model->safetySw[i].swtch.type) {
           str.append("<tr>");
           str.append(doTC(tr("CH")+QString("%1").arg(i+1),"",true));
           str.append(doTC(g_model->safetySw[i].swtch.toString(),"green"));
           str.append(doTC(QString::number(g_model->safetySw[i].val),"green"));
           str.append("</tr>");
           sc++;
        }
    }
    str.append("</table></td></tr></table>");
    str.append("<br>");
    if (sc!=0)
        te->append(str);
}

void printDialog::printFSwitches()
{
    int sc=0;
    QString str = "<table border=1 cellspacing=0 cellpadding=3 width=\"100%\">";
    str.append("<tr><td><h2>"+tr("Function Switches")+"</h2></td></tr>");
    str.append("<tr><td><table border=0 cellspacing=0 cellpadding=3><tr>");
    str.append("<td width=\"60\">&nbsp;</td>");
    str.append(doTC(tr("Switch"), "", true));
    str.append(doTL(tr("Function"), "", true));
    str.append(doTL(tr("Parameter"), "", true));
    str.append(doTL(tr("Enabled"), "", true));
    str.append("</tr>");
    for(int i=0; i<GetEepromInterface()->getCapability(FuncSwitches); i++) {
      if (g_model->funcSw[i].swtch.type!=SWITCH_TYPE_NONE) {
          str.append("<tr>");
          str.append(doTC(tr("FSW")+QString("%1").arg(i+1),"",true));
          str.append(doTC(g_model->funcSw[i].swtch.toString(),"green"));
          str.append(doTC(getFuncName(g_model->funcSw[i].func),"green"));
          str.append(doTC(FuncParam(g_model->funcSw[i].func,g_model->funcSw[i].param),"green"));
          if (g_model->funcSw[i].func<=FuncInstantTrim) {
            str.append(doTC((g_model->funcSw[i].enabled ? "ON" : "OFF"),"green"));
          } else {
            str.append(doTC( "---","green"));
          }
          str.append("</tr>");
          sc++;
      }
    }
    str.append("</table></td></tr></table>");
    str.append("<br>");
    if (sc!=0)
      te->append(str);
}

QString  printDialog::getFrSkyBarSrc(int index) {
  return QString(TELEMETRY_SRC).mid((abs(index))*4, 4);
}

QString  printDialog::getFrSkySrc(int index) {
  return QString(TELEMETRY_SRC).mid((abs(index))*4, 4);
}

float printDialog::getBarValue(int barId, int Value, FrSkyData *fd) 
{
  switch (barId) {
    case TELEM_TM1:
    case TELEM_TM2:
      return (3*Value);
      break;
    case TELEM_RSSI_TX:
    case TELEM_RSSI_RX:
    case TELEM_FUEL:
      if (Value>100) {
        return 100;
      } else {
        return Value;
      }
      break;
    case TELEM_A1:
    case TELEM_MIN_A1:
      if (fd->channels[0].type==0) {
        return ((fd->channels[0].ratio*Value/255.0)+fd->channels[0].offset)/10;
      } else {
        return ((fd->channels[0].ratio*Value/255.0)+fd->channels[0].offset);
      }
      break;
    case TELEM_A2:
    case TELEM_MIN_A2:
      if (fd->channels[1].type==0) {
        return ((fd->channels[1].ratio*Value/255.0)+fd->channels[1].offset)/10;
      } else {
        return ((fd->channels[1].ratio*Value/255.0)+fd->channels[1].offset);
      }
      break;
    case TELEM_ALT:
    case TELEM_GPSALT:
    case TELEM_MAX_ALT:
    case TELEM_MIN_ALT:
      return (8*Value)-510;
      break;
    case TELEM_RPM:
    case TELEM_MAX_RPM:
      return Value * 50;
      break;
    case TELEM_T1:
    case TELEM_T2:
    case TELEM_MAX_T1:
    case TELEM_MAX_T2:
      return  Value - 30.0;
      break;
    case TELEM_CELL:
      return  Value*2.0/100;
      break;
    case TELEM_CELLS_SUM:
    case TELEM_VFAS:
      return  Value/10.0;
      break;      
    case TELEM_HDG:
      if (Value>359) {
        return 359;
      } else {
        return  Value * 2;
      }
      break;
    case TELEM_DIST:
    case TELEM_MAX_DIST:
      return  Value * 8;
      break;
    case TELEM_MAX_CURRENT:
    case TELEM_CURRENT:
      return  Value/2.0;
      break;
    case TELEM_POWER:
      return  Value*5;
      break;
    case TELEM_CONSUMPTION:
      return  Value * 20;
      break;
    case TELEM_SPEED:
    case TELEM_MAX_SPEED:
      if (fd->imperial==1) {
        return Value;
      } else {
        return Value*1.852;
      }
      break;
    default:
      return  Value;
      break;
  }  
}

void printDialog::printFrSky()
{
  int tc=0;
  QString str = "<table border=1 cellspacing=0 cellpadding=3 width=\"100%\">";
  str.append("<tr><td colspan=10><h2>"+tr("Telemetry Settings")+"</h2></td></tr>");
  str.append("<tr><td colspan=4 align=\"center\">&nbsp;</td><td colspan=3 align=\"center\"><b>"+tr("Alarm 1")+"</b></td><td colspan=3 align=\"center\"><b>"+tr("Alarm 2")+"</b></td></tr>");
  str.append("<tr><td align=\"center\"><b>"+tr("Analog")+"</b></td><td align=\"center\"><b>"+tr("Unit")+"</b></td><td align=\"center\"><b>"+tr("Scale")+"</b></td><td align=\"center\"><b>"+tr("Offset")+"</b></td>");
  str.append("<td width=\"40\" align=\"center\"><b>"+tr("Type")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Condition")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Value")+"</b></td>");
  str.append("<td width=\"40\" align=\"center\"><b>"+tr("Type")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Condition")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Value")+"</b></td></tr>");
  FrSkyData *fd=&g_model->frsky;
  for (int i=0; i<2; i++) {
    if (fd->channels[i].ratio!=0) {
      tc++;
      float ratio=(fd->channels[i].ratio/(fd->channels[i].type==0 ?10.0:1));
      str.append("<tr><td align=\"center\"><b>"+tr("A%1").arg(i+1)+"</b></td><td align=\"center\"><font color=green>"+FrSkyUnits(fd->channels[i].type)+"</font></td><td align=\"center\"><font color=green>"+QString::number(ratio,10,(fd->channels[i].type==0 ? 1:0))+"</font></td><td align=\"center\"><font color=green>"+QString::number((fd->channels[i].offset*ratio)/255,10,(fd->channels[i].type==0 ? 1:0))+"</font></td>");
      str.append("<td width=\"40\" align=\"center\"><font color=green>"+FrSkyAtype(fd->channels[i].alarms[0].level)+"</font></td>");
      str.append("<td width=\"40\" align=\"center\"><font color=green>");
      str.append((fd->channels[i].alarms[0].greater==1) ? "&gt;" : "&lt;");
      str.append("</font></td><td width=\"40\" align=\"center\"><font color=green>"+QString::number(ratio*(fd->channels[i].alarms[0].value/255.0+fd->channels[i].offset/255.0),10,(fd->channels[i].type==0 ? 1:0))+"</font></td>");
      str.append("<td width=\"40\" align=\"center\"><font color=green>"+FrSkyAtype(fd->channels[i].alarms[1].level)+"</font></td>");
      str.append("<td width=\"40\" align=\"center\"><font color=green>");
      str.append((fd->channels[i].alarms[1].greater==1) ? "&gt;" : "&lt;");
      str.append("</font></td><td width=\"40\" align=\"center\"><font color=green>"+QString::number(ratio*(fd->channels[i].alarms[1].value/255.0+fd->channels[i].offset/255.0),10,(fd->channels[i].type==0 ? 1:0))+"</font></td></tr>");
    }
  }
  str.append("<tr><td colspan=10 align=\"Left\" height=\"4px\"></td></tr>");
  str.append("<tr><td colspan=4 align=\"center\" rowspan=2>&nbsp;</td><td colspan=3 align=\"center\"><b>"+tr("Alarm 1")+"</b></td><td colspan=3 align=\"center\"><b>"+tr("Alarm 2")+"</b></td></tr>");
  str.append("<tr><td width=\"40\" align=\"center\"><b>"+tr("Type")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Condition")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Value")+"</b></td>");
  str.append("<td width=\"40\" align=\"center\"><b>"+tr("Type")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Condition")+"</b></td><td width=\"40\" align=\"center\"><b>"+tr("Value")+"</b></td></tr>");
  str.append("<tr><td align=\"Left\" colspan=4><b>"+tr("RSSI Alarm")+"</b></td>");
  str.append("<td width=\"40\" align=\"center\"><b>"+FrSkyAtype(fd->rssiAlarms[0].level)+"</b></td><td width=\"40\" align=\"center\"><b>&lt;</b></td><td width=\"40\" align=\"center\"><b>"+QString::number(fd->rssiAlarms[0].value,10)+"</b></td>");
  str.append("<td width=\"40\" align=\"center\"><b>"+FrSkyAtype(fd->rssiAlarms[1].level)+"</b></td><td width=\"40\" align=\"center\"><b>&lt;</b></td><td width=\"40\" align=\"center\"><b>"+QString::number(fd->rssiAlarms[1].value,10)+"</b></td></tr>");
  str.append("<tr><td colspan=10 align=\"Left\" height=\"4px\"></td></tr>");
  str.append("<tr><td colspan=2 align=\"Left\"><b>"+tr("Frsky serial protocol")+"</b></td><td colspan=8 align=\"left\">"+FrSkyProtocol(fd->usrProto)+"</td></tr>");
  str.append("<tr><td colspan=2 align=\"Left\"><b>"+tr("System of units")+"</b></td><td colspan=8 align=\"left\">"+FrSkyMeasure(fd->imperial)+"</td></tr>");
  str.append("<tr><td colspan=2 align=\"Left\"><b>"+tr("Propeller blades")+"</b></td><td colspan=8 align=\"left\">"+FrSkyBlades(fd->blades)+"</td></tr>");
  str.append("<tr><td colspan=10 align=\"Left\" height=\"4px\"></td></tr></table>");
  
  if (GetEepromInterface()->getCapability(TelemetryBars) || (GetEepromInterface()->getCapability(TelemetryCSFields))) {
    for (int j=0; j<GetEepromInterface()->getCapability(TelemetryCSFields)/8; j++ ) {
      if (fd->screens[j].type==0) {
        str.append("<table border=1 cellspacing=0 cellpadding=3 width=\"100%\"><tr><td colspan=3 align=\"Left\"><b>"+tr("Custom Telemetry View")+"</b></td></tr>");
        for (int i=0; i<4; i++) {
          if ((fd->screens[j].body.cells[i*2] !=0) || (fd->screens[j].body.cells[i*2+1]!=0))
            tc++;
          str.append("<tr><td  align=\"Center\" width=\"45%\"><b>"+getFrSkySrc(fd->screens[j].body.cells[i*2])+"</b></td><td  align=\"Center\"width=\"10%\">&nbsp;</td><td  align=\"Center\" width=\"45%\"><b>"+getFrSkySrc(fd->screens[j].body.cells[i*2+1])+"</b></td></tr>");
        }
        str.append("</table>");
      } else {
        str.append("<table border=1 cellspacing=0 cellpadding=3 width=\"100%\"><tr><td colspan=4 align=\"Left\"><b>"+tr("Telemetry Bars")+"</b></td></tr>");
        str.append("<tr><td  align=\"Center\"><b>"+tr("Bar Number")+"</b></td><td  align=\"Center\"><b>"+tr("Source")+"</b></td><td  align=\"Center\"><b>"+tr("Min")+"</b></td><td  align=\"Center\"><b>"+tr("Max")+"</b></td></tr>");
        for (int i=0; i<4; i++) {
          if (fd->screens[j].body.bars[i].source!=0)
            tc++;
          str.append("<tr><td  align=\"Center\"><b>"+QString::number(i+1,10)+"</b></td><td  align=\"Center\"><b>"+getFrSkyBarSrc(fd->screens[j].body.bars[i].source)+"</b></td><td  align=\"Right\"><b>"+QString::number(getBarValue(fd->screens[j].body.bars[i].source, fd->screens[j].body.bars[i].barMin,fd))+"</b></td><td  align=\"Right\"><b>"+QString::number(getBarValue(fd->screens[j].body.bars[i].source,(255-fd->screens[j].body.bars[i].barMax),fd))+"</b></td></tr>");
        }
        str.append("</table><br/>");
      }
    }
  }
  if (tc>0)
      te->append(str);    
}

void printDialog::on_printButton_clicked()
{
    QPrinter printer;
    printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
      return;
    te->print(&printer);
}

void printDialog::on_printFileButton_clicked()
{
    QPrinter printer;
    QString filename = QFileDialog::getSaveFileName(this,tr("Select PDF output file"),QString(),"Pdf File(*.pdf)"); 
    printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    if(!filename.isEmpty()) { 
      if(QFileInfo(filename).suffix().isEmpty()) 
        filename.append(".pdf"); 
      printer.setOutputFileName(filename);
      te->print(&printer);
    }
}
