// qmlistvalidator.cpp is part of QualificationMatrix
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

#include "framework/qmlistvalidator.h"

#include <QAbstractTableModel>
#include <QStringList>

QMListValidator::QMListValidator(const QStringList &list, QObject *parent)
    : QValidator(parent)
    , m_list(std::make_unique<QStringList>())
{
    m_list->append(list);
}

QMListValidator::QMListValidator(const QAbstractTableModel &model, int column, QObject *parent)
    : QValidator(parent)
    , m_list(std::make_unique<QStringList>())
{
    Q_ASSERT(column > -1 && column < model.columnCount());

    for (auto i = 0; i < model.rowCount(); i++)
    {
        m_list->append(model.data(model.index(i, column), Qt::DisplayRole).toString());
    }
}


QValidator::State QMListValidator::validate(QString &input, int &pos) const
{
    for (const QString &item : *m_list)
    {
        if (item.startsWith(input, Qt::CaseSensitivity::CaseInsensitive))
        {
            return QValidator::State::Acceptable;
        }
    }
    return QValidator::State::Invalid;
}
