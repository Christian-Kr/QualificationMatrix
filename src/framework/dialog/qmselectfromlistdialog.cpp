// qmselectfromlistdialog.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmselectfromlistdialog.h"
#include "ui_qmselectfromlistdialog.h"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include <QDebug>

QMSelectFromListDialog::QMSelectFromListDialog(QWidget *parent, const QString info, const SelectionMode mode)
    : QMDialog(parent)
    , ui(new Ui::QMSelectFromListDialog)
    , selMode(mode)
    , selectionList(new QStringList)
{
    ui->setupUi(this);

    ui->label->setText(info);
    
    if (selMode == SelectionMode::MULTPLE)
    {
        ui->lwSelection->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
    }
    else
    {
        ui->lwSelection->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    }
}

QMSelectFromListDialog::~QMSelectFromListDialog()
{
    delete selectionList;
}

void QMSelectFromListDialog::setSelectionList(const QStringList list)
{
    selectionList->clear();
    selectionList->append(list);

    ui->lwSelection->clear();
    ui->lwSelection->addItems(*selectionList);
}

QList<int> QMSelectFromListDialog::getSelected() const
{
    QList<int> selectedList;
    QModelIndexList modelIndexList = ui->lwSelection->selectionModel()->selectedIndexes();
    for (QModelIndex modelIndex : modelIndexList)
    {
        selectedList.append(modelIndex.row());
    }

    return selectedList;
}
