//
// qualimatrixdelegate.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#ifndef QMQUALIMATRIXDELEGATE_H
#define QMQUALIMATRIXDELEGATE_H

#include <QStyledItemDelegate>

class QMQualiMatrixDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:
    /**
     * @brief constructor
     * @param parent
     */
    explicit QMQualiMatrixDelegate(QWidget *parent = nullptr);

    /**
     * @brief override from QStyleItemDelegate
     */
    void paint(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
    ) const;

    /**
     * @brief override from QStyleItemDelegate
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /**
     * @brief override from QStyleItemDelegate
     */
    QWidget *createEditor(
        QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index
    ) const;

    /**
     * @brief override from QStyleItemDelegate
     */
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    /**
     * @brief override from QStyleItemDelegate
     */
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    /**
     * @Brief override from QStyleItemDelegate
     */
    void updateEditorGeometry(
        QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index
    ) const;

    /**
     * @brief Update the colors from config.
     */
    void updateColors();

private:
    QColor gridColor;
    QColor selectionColor;
};

#endif // QMQUALIMATRIXDELEGATE_H
