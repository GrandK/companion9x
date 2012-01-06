/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "modelslist.h"
#include "mdichild.h"

class DragDropHeader {
public:
  DragDropHeader():
    general_settings(false),
    models_count(0)
  {
  }
  bool general_settings;
  uint8_t models_count;
  uint8_t models[MAX_MODELS];
};

ModelsListWidget::ModelsListWidget(QWidget *parent):
  QListWidget(parent)
{
    this->setFont(QFont("Courier New",12));
    radioData = &((MdiChild *)parent)->radioData;
    refreshList();

    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(OpenEditWindow()));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
    connect(this, SIGNAL(currentRowChanged(int)), this, SLOT(viableModelSelected(int)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropOverwriteMode(true);
    setDropIndicatorShown(true);

    active_highlight_color = this->palette().color(QPalette::Active, QPalette::Highlight);
}

void ModelsListWidget::ShowContextMenu(const QPoint& pos)
{
    QPoint globalPos = this->mapToGlobal(pos);

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    bool hasData = mimeData->hasFormat("application/x-companion9x");

    QMenu contextMenu;
    contextMenu.addAction(QIcon(":/images/edit.png"), tr("&Edit"),this,SLOT(OpenEditWindow()));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/clear.png"), tr("&Delete"),this,SLOT(confirmDelete()),tr("Delete"));
    contextMenu.addAction(QIcon(":/images/copy.png"), tr("&Copy"),this,SLOT(copy()),tr("Ctrl+C"));
    contextMenu.addAction(QIcon(":/images/cut.png"), tr("&Cut"),this,SLOT(cut()),tr("Ctrl+X"));
    contextMenu.addAction(QIcon(":/images/paste.png"), tr("&Paste"),this,SLOT(paste()),tr("Ctrl+V"))->setEnabled(hasData);
    contextMenu.addAction(QIcon(":/images/duplicate.png"), tr("D&uplicate"),this,SLOT(duplicate()),tr("Ctrl+U"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/currentmodel.png"), tr("&Use as default"),this,SLOT(setdefault()));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/print.png"), tr("P&rint model"),this, SLOT(print()),tr("Alt+R"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/simulate.png"), tr("&Simulate model"),this, SLOT(simulate()),tr("Alt+S"));
    contextMenu.exec(globalPos);
}

void ModelsListWidget::OpenEditWindow()
{
  ((MdiChild *)parent())->OpenEditWindow();
}

void ModelsListWidget::simulate()
{
  ((MdiChild *)parent())->simulate();
}

void ModelsListWidget::print()
{
  ((MdiChild *)parent())->print();
}

void ModelsListWidget::setdefault()
{
  if(currentRow()==0) return;
  if (!radioData->models[currentRow()-1].isempty()) {
    if (radioData->generalSettings.currModel!=(currentRow()-1)) {
      radioData->generalSettings.currModel=currentRow()-1;
      refreshList();
      ((MdiChild *)parent())->setModified();
    }
  }
}


void ModelsListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();

    QListWidget::mousePressEvent(event);
}

void ModelsListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);

    QByteArray gmData;
    doCopy(&gmData);

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-companion9x", gmData);

    drag->setMimeData(mimeData);

    //Qt::DropAction dropAction =
            drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

    //if(dropAction==Qt::MoveAction)

   // QListWidget::mouseMoveEvent(event);
}

void ModelsListWidget::saveSelection()
{
  currentSelection.current_item = currentItem();
  for (int i=0; i<MAX_MODELS+1; ++i)
    currentSelection.selected[i] = item(i)->isSelected();
}

void ModelsListWidget::restoreSelection()
{
  setCurrentItem(currentSelection.current_item);
  for (int i=0; i<MAX_MODELS+1; ++i)
    item(i)->setSelected(currentSelection.selected[i]);
}

void ModelsListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-companion9x"))
    {
         event->acceptProposedAction();
         saveSelection();
    }
}

void ModelsListWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    restoreSelection();
}

void ModelsListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    int row=this->indexAt(event->pos()).row();
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasFormat("application/x-companion9x"))
    {
         QByteArray gmData = mimeData->data("application/x-companion9x");
         event->acceptProposedAction();
         clearSelection();
         DragDropHeader *header = (DragDropHeader *)gmData.data();
         if (row >= 0) {
           if (header->general_settings)
             item(0)->setSelected(true);
           for (int i=row, end=std::min(MAX_MODELS+1, row+header->models_count); i<end; i++)
             item(i)->setSelected(true);
         }
    }
}

void ModelsListWidget::dropEvent(QDropEvent *event)
{
    int row = this->indexAt(event->pos()).row();
    if (row < 0)
      return;

    // QMessageBox::warning(this, tr("companion9x"),tr("Index :%1").arg(row));
    const QMimeData  *mimeData = event->mimeData();
    if(mimeData->hasFormat("application/x-companion9x"))
    {
        QByteArray gmData = mimeData->data("application/x-companion9x");
        if (event->source() && event->dropAction() == Qt::MoveAction)
          ((ModelsListWidget*)event->source())->doCut(&gmData);
        doPaste(&gmData, row);
        clearSelection();
        setCurrentItem(item(row));
        DragDropHeader *header = (DragDropHeader *)gmData.data();
        if (header->general_settings)
          item(0)->setSelected(true);
        for (int i=row, end=std::min(MAX_MODELS+1, row+header->models_count); i<end; i++)
          item(i)->setSelected(true);
    }
    event->acceptProposedAction();
}

#ifndef WIN32
void ModelsListWidget::focusInEvent ( QFocusEvent * event )
{
  QListWidget::focusInEvent(event);
  QPalette palette = this->palette();
  palette.setColor(QPalette::Active, QPalette::Highlight, active_highlight_color);
  palette.setColor(QPalette::Inactive, QPalette::Highlight, active_highlight_color);
  this->setPalette(palette);
}

void ModelsListWidget::focusOutEvent ( QFocusEvent * event )
{
  QListWidget::focusOutEvent(event);
  QPalette palette = this->palette();
  palette.setColor(QPalette::Active, QPalette::Highlight, palette.color(QPalette::Active, QPalette::Midlight));
  palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.color(QPalette::Active, QPalette::Midlight));
  this->setPalette(palette);
}
#endif

void ModelsListWidget::refreshList()
{
    clear();
    int msize;
    QString name = radioData->generalSettings.ownerName;
    if(!name.isEmpty())
        name.prepend(" - ");
    addItem(tr("General Settings") + name);

    EEPROMInterface *eepromInterface = GetEepromInterface();
    int availableEEpromSize = eepromInterface->getEEpromSize();
    availableEEpromSize -= eepromInterface->getSize(radioData->generalSettings);
    
    for(uint8_t i=0; i<MAX_MODELS; i++)
    {
       QString item = QString().sprintf("%02d: ", i+1);
       
       if (!radioData->models[i].isempty()) {
         item += QString().sprintf("%10s", radioData->models[i].name);
         if (eepromInterface) {
           msize = eepromInterface->getSize(radioData->models[i]);
           item += QString().sprintf("%5d", msize);
           availableEEpromSize -= msize;
         }
       }
       addItem(item);
    }
    if (radioData->generalSettings.currModel < MAX_MODELS) {
        QFont f = QFont("Courier New", 12);
        f.setBold(true);
        this->item(radioData->generalSettings.currModel+1)->setFont(f);
    }

    ((MdiChild*)parent())->setEEpromAvail(availableEEpromSize);
}

void ModelsListWidget::cut()
{
    copy();
    deleteSelected(false);
}

void ModelsListWidget::confirmDelete() {
    deleteSelected(true);
}


void ModelsListWidget::deleteSelected(bool ask=true)
{
    QMessageBox::StandardButton ret = QMessageBox::Yes;

    if(ask)
        ret = QMessageBox::warning(this, "companion9x",
                 tr("Delete Selected Models?"),
                 QMessageBox::Yes | QMessageBox::No);


    if (ret == QMessageBox::Yes)
    {
           foreach(QModelIndex index, this->selectionModel()->selectedIndexes())
           {
               if(index.row()>0)
                 radioData->models[index.row()-1].clear();
           }
           ((MdiChild *)parent())->setModified();
    }
}

void ModelsListWidget::doCut(QByteArray *gmData)
{
    DragDropHeader *header = (DragDropHeader *)gmData->data();
    for (int i=0; i<header->models_count; i++) {
      radioData->models[header->models[i]-1].clear();
    }
    ((MdiChild *)parent())->setModified();
}

void ModelsListWidget::doCopy(QByteArray *gmData)
{
    DragDropHeader header;

    foreach(QModelIndex index, this->selectionModel()->selectedIndexes())
    {
        char row = index.row();
        if(!row) {
            header.general_settings = true;
            gmData->append('G');
            gmData->append((char*)&radioData->generalSettings, sizeof(GeneralSettings));
        }
        else {
            header.models[header.models_count++] = row;
            gmData->append('M');
            gmData->append((char*)&radioData->models[row-1], sizeof(ModelData));
        }
    }

    gmData->prepend((char *)&header, sizeof(header));
}

void ModelsListWidget::copy()
{
    QByteArray gmData;
    doCopy(&gmData);

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-companion9x", gmData);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData,QClipboard::Clipboard);
}

void ModelsListWidget::doPaste(QByteArray *gmData, int index)
{
    //QByteArray gmData = mimeD->data("application/x-companion9x");
    char *gData = gmData->data()+sizeof(DragDropHeader);//new char[gmData.size() + 1];
    int i = sizeof(DragDropHeader);
    int id = index;
    int ret,modified=0;
    if(!id) id++;

    while((i<gmData->size()) && (id<=MAX_MODELS)) {
        char c = *gData;
        i++;
        gData++;
        if(c=='G')  //general settings
        {
            radioData->generalSettings = *((GeneralSettings *)gData);
            gData += sizeof(GeneralSettings);
            i     += sizeof(GeneralSettings);
            modified=1;
        }
        else //model data
        {
            QString name=(QString)radioData->models[id-1].name;
            if (!name.isEmpty()) {
                ret = QMessageBox::question(this, "companion9x", tr("You are pasting on an not empty model, are you sure?"),
                        QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    radioData->models[id-1] = *((ModelData *)gData);
                    gData += sizeof(ModelData);
                    i += sizeof(ModelData);
                    id++;
                    modified = 1;
                } else {
                    gData += sizeof(ModelData);
                    i += sizeof(ModelData);
                    id++;
                }
            } else {
                radioData->models[id-1] = *((ModelData *)gData);
                gData += sizeof(ModelData);
                i     += sizeof(ModelData);
                id++;
                modified=1;
            }
        }
    }
    if (modified==1) {
        ((MdiChild *)parent())->setModified();
    }
}

bool ModelsListWidget::hasPasteData()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    return mimeData->hasFormat("application/x-companion9x");
}

void ModelsListWidget::paste()
{
    if (hasPasteData()) {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        QByteArray gmData = mimeData->data("application/x-companion9x");
        doPaste(&gmData,this->currentRow());
    }
}

void ModelsListWidget::duplicate()
{
    int i = this->currentRow();
    if(i && i<MAX_MODELS)
    {
        ModelData *model = &radioData->models[i-1];
        while(i<MAX_MODELS) {
          if (radioData->models[i].isempty()) {
            radioData->models[i] = *model;
            ((MdiChild *)parent())->setModified();
            break;
          }
        }
    }
}

bool ModelsListWidget::hasSelection()
{
    return (this->selectionModel()->hasSelection());
}

void ModelsListWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Delete))
    {
        deleteSelected();
        return;
    }

    if(event->matches(QKeySequence::Cut))
    {
        cut();
        return;
    }

    if(event->matches(QKeySequence::Copy))
    {
        copy();
        return;
    }

    if(event->matches(QKeySequence::Paste))
    {
        paste();
        return;
    }

    if(event->matches(QKeySequence::Underline))
    {
        duplicate();
        return;
    }

    QListWidget::keyPressEvent(event);//run the standard event in case we didn't catch an action
}

void ModelsListWidget::viableModelSelected(int idx)
{
  if (!isVisible())
    ((MdiChild*)parent())->viableModelSelected(false);
  else if (idx<1)
    ((MdiChild*)parent())->viableModelSelected(false);
  else
    ((MdiChild*)parent())->viableModelSelected(!radioData->models[currentRow()-1].isempty());
}



