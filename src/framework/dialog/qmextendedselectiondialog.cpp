// qmextendedselectiondialog.cpp is part of QualificationMatrix
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

#include "qmextendedselectiondialog.h"
#include "ui_qmextendedselectiondialog.h"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include <QDebug>

QMExtendedSelectionDialog::QMExtendedSelectionDialog(QWidget *parent, QAbstractItemModel *model, int column)
    : QMDialog(parent)
{
    ui = new Ui::QMExtendedSelectionDialog;
    ui->setupUi(this);

    filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(model);
    filterModel->setFilterKeyColumn(column);

    // Be sure only settings will be called, that are available during this state.
    loadSettings();

    // Set data to table view.
    if (model != nullptr)
    {
        ui->tvSelection->setModel(filterModel);

        for (int i = 0; i < model->columnCount(); i++)
        {
            if (i != column)
            {
                ui->tvSelection->hideColumn(i);
            }
        }
    }
}

QModelIndexList QMExtendedSelectionDialog::getFilterSelected() const
{
    return ui->tvSelection->selectionModel()->selectedRows();
}

void QMExtendedSelectionDialog::updateFilter()
{
    filterModel->setFilterFixedString(ui->leFilter->text());
}

bool QMExtendedSelectionDialog::isReverse() const
{
    return ui->cbReverse->isChecked();
}

QString QMExtendedSelectionDialog::getRegExpText() const
{
    auto modelIndexList = getFilterSelected();

    QString prefix = isReverse() ? "^(?!" : "^(";
    QString finalValue;

    if (modelIndexList.size() == 1)
    {
        QModelIndex modelIndex = modelIndexList.at(0);
        finalValue = prefix + filterModel->data(
            filterModel->index(modelIndex.row(), filterModel->filterKeyColumn())).toString() + ")";
    }
    else
    {
        // Only > 1 possible
        QStringList valueList;

        for (int i = 0; i < modelIndexList.size(); i++)
        {
            QModelIndex modelIndex = modelIndexList.at(i);
            auto value = filterModel->data(
                filterModel->index(modelIndex.row(), filterModel->filterKeyColumn())).toString();

            valueList << value;
        }

        finalValue = prefix + valueList.join("|") + ")";
    }

    return finalValue;
}

QStringList QMExtendedSelectionDialog::getSelectedElements() const
{
    QStringList selection;

    auto modelIndexList = getFilterSelected();

    for (auto modelIndex : modelIndexList)
    {
        auto value = filterModel->data(
                filterModel->index(modelIndex.row(), filterModel->filterKeyColumn())).toString();
        selection << value;
    }

    return selection;
}
