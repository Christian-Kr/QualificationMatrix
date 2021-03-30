// qmproxysqlrelationaldelegate.cpp is part of QualificationMatrix
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

#include "qmproxysqlrelationaldelegate.h"

#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QSqlRecord>
#include <QComboBox>

QMProxySqlRelationalDelegate::QMProxySqlRelationalDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{
}

const QAbstractItemModel *QMProxySqlRelationalDelegate::getRelationalModel(
    const QAbstractItemModel *itemModel) const
{
    auto sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(itemModel);

    if (sqlModel == nullptr)
    {
        auto proxyModel = dynamic_cast<const QSortFilterProxyModel *>(itemModel);

        if (proxyModel == nullptr)
        {
            return nullptr;
        }
        else
        {
            return getRelationalModel(proxyModel->sourceModel());
        }
    }

    return sqlModel;
}

QWidget *QMProxySqlRelationalDelegate::createEditor(
    QWidget *aParent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Try to cast to relational model. If the cast fails, the model might be an sql proxy model.
    // So go ahead, till a relational table model has been found.
    auto sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(
        getRelationalModel(index.model()));

    // Try to get the right child model.
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : nullptr;

    // If no child model could be found, get standard editor.
    if (!childModel)
    {
        return QItemDelegate::createEditor(aParent, option, index);
    }

    // ... else, create the combo box
    auto combo = new QComboBox(aParent);

    // The table might have change and the childModel won't be reselected otherwise.
    combo->setModel(childModel);
    combo->setEditable(true);
    combo->setModelColumn(childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn()));
    combo->installEventFilter(const_cast<QMProxySqlRelationalDelegate *>(this));

    return combo;
}

void QMProxySqlRelationalDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // set the current value of the model to the current in dex of the combo box

    QString strVal = "";

    // get the model (see createEditor(...)) for more details
    auto sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(index.model());

    if (!sqlModel)
    {
        auto proxyModel = qobject_cast<const QSortFilterProxyModel *>(
            index.model());

        if (proxyModel)
        {
            strVal = proxyModel->data(index).toString();
        }
    }
    else
    {
        strVal = sqlModel->data(index).toString();
    }

    // get the combo box and set the value
    auto combo = dynamic_cast<QComboBox *>(editor);

    if (strVal.isEmpty() || !combo)
    {
        QItemDelegate::setEditorData(editor, index);
        return;
    }

    combo->setCurrentIndex(combo->findText(strVal));
}

void QMProxySqlRelationalDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return;
    }

    // Try to cast to relational model. If the cast fails, the model might be an sql proxy model.
    // So go ahead, till a relational table model has been found.
    auto sqlModel = dynamic_cast<const QSqlRelationalTableModel *>(
        getRelationalModel(index.model()));

    // Try to get the right child model.
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : nullptr;

    // Get the editor and test for existing objects.
     auto combo = dynamic_cast<QComboBox *>(editor);
    if (!childModel || !combo)
    {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }

    // current index in the combo box
    int currentItem = combo->currentIndex();
    int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());

    qDebug() << childModel->data(childModel->index(currentItem, childEditIndex)).toInt();

    model->setData(index, childModel->data(childModel->index(currentItem, childEditIndex), Qt::DisplayRole),
        Qt::EditRole);
}
