//
// proxysqlrelationaldelegate.h is part of QualificationMatrix
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

#ifndef PROXYSQLRELATIONALDELEGATE_H
#define PROXYSQLRELATIONALDELEGATE_H

#include <QSqlRelationalDelegate>

/**
 * @brief gives the possibility to handle relational models in editors for proxy models
 * @author Christian Kr, Copyright (c) 2020
 */
class ProxySqlRelationalDelegate: public QSqlRelationalDelegate
{
Q_OBJECT

public:
    /**
     * @brief constructor
     * @param parent
     */
    explicit ProxySqlRelationalDelegate(QObject *parent = nullptr);

    /**
     * @brief creates a combo box editor and sets all items from the child model
     * @param aParent
     * @param option
     * @param index
     * @return
     */
    QWidget *createEditor(
        QWidget *aParent, const QStyleOptionViewItem &option, const QModelIndex &index
    ) const override;

    /**
     * @brief sets the data from the editor to the model
     * @param editor
     * @param model
     * @param index
     */
    void setModelData(
        QWidget *editor, QAbstractItemModel *model, const QModelIndex &index
    ) const override;

    /**
     * @brief sets the model value to the beginning editor value
     * @param editor
     * @param index
     */
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    /**
     * @brief Search recursively for a relational table model and return it.
     * @param itemModel The starting parent model.
     * @return Relational table model pointer if success, else nullptr.
     */
    const QAbstractItemModel *getRelationalModel(const QAbstractItemModel *itemModel) const;
};

#endif // PROXYSQLRELATIONALDELEGATE_H
