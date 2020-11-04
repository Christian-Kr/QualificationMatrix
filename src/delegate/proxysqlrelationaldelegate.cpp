//
// proxysqlrelationaldelegate.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#include "proxysqlrelationaldelegate.h"

#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QSqlRecord>
#include <QComboBox>

ProxySqlRelationalDelegate::ProxySqlRelationalDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{
}

const QAbstractItemModel *
ProxySqlRelationalDelegate::getRelationalModel(const QAbstractItemModel *itemModel) const
{
    const QSqlRelationalTableModel
        *sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(itemModel);

    if (sqlModel == nullptr) {
        const QSortFilterProxyModel
            *proxyModel = dynamic_cast<const QSortFilterProxyModel *>(itemModel);

        if (proxyModel == nullptr) {
            return nullptr;
        }
        else {
            return getRelationalModel(proxyModel->sourceModel());
        }
    }

    return sqlModel;
}

QWidget *ProxySqlRelationalDelegate::createEditor(
    QWidget *aParent, const QStyleOptionViewItem &option, const QModelIndex &index
) const
{
    // Try to cast to relational model. If the cast fails, the model might be an sql proxy model.
    // So go ahead, till a relational table model has been found.
    const QSqlRelationalTableModel
        *sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(getRelationalModel(
        index.model()));

    // Try to get the right child model.
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : nullptr;

    // If no child model could be found, get standard editor.
    if (!childModel) {
        return QItemDelegate::createEditor(aParent, option, index);
    }

    // ... else, create the combo box
    QComboBox *combo = new QComboBox(aParent);

    // The table might have change and the childModel won't be reselected otherwise.
    childModel->select();
    combo->setModel(childModel);
    combo->setEditable(true);
    combo->setModelColumn(
        childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn()));
    combo->installEventFilter(const_cast<ProxySqlRelationalDelegate *>(this));

    return combo;
}

void ProxySqlRelationalDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // set the current value of the model to the current in dex of the combo box

    QString strVal = "";

    // get the model (see createEditor(...)) for more details
    const QSqlRelationalTableModel *sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(index
        .model());

    if (!sqlModel) {
        const QSortFilterProxyModel *proxyModel = qobject_cast<const QSortFilterProxyModel *>(
            index.model());

        if (proxyModel) {
            strVal = proxyModel->data(index).toString();
        }
    }
    else {
        strVal = sqlModel->data(index).toString();
    }

    // get the combo box and set the value
    QComboBox *combo = dynamic_cast<QComboBox *>(editor);

    if (strVal.isEmpty() || !combo) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }

    combo->setCurrentIndex(combo->findText(strVal));
}

void ProxySqlRelationalDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index
) const
{
    if (!index.isValid()) {
        return;
    }

    // Try to cast to relational model. If the cast fails, the model might be an sql proxy model.
    // So go ahead, till a relational table model has been found.
    const QSqlRelationalTableModel
        *sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(getRelationalModel(
        index.model()));

    // Try to get the right child model.
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : nullptr;

    // Get the editor and test for existing objects.
    QComboBox *combo = dynamic_cast<QComboBox *>(editor);
    if (!childModel || !combo) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }

    // current index in the combo box
    int currentItem = combo->currentIndex();
    int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
    int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());

    model->setData(
        index, childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
        Qt::DisplayRole
    );
    model->setData(
        index, childModel->data(childModel->index(currentItem, childEditIndex), Qt::DisplayRole),
        Qt::EditRole
    );
}
