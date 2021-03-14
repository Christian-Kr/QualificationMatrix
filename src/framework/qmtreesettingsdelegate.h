//
// qmtreesettingsdelegate.h is part of QualificationMatrix
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

#ifndef QMTREESETTINGSDELEGATE_H
#define QMTREESETTINGSDELEGATE_H

#include <QItemDelegate>

class QMTreeSettingsDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    /// \param height Height for item.
    explicit QMTreeSettingsDelegate(QObject *parent = nullptr, int height = -1);

    /// Set the height of the item.
    /// \param height
    void setHeight(int height) { this->height = height; }

    /// Override from QItemDelegate.
    /// \param option
    /// \param index
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int height;
};

#endif // QMTREESETTINGSDELEGATE_H
