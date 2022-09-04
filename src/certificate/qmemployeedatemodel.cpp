// qmemployeedatemodel.cpp is part of QualificationMatrix
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

#include "qmemployeedatemodel.h"

#include <QSqlDatabase>
#include <QSqlResult>

QMEmployeeDateModel::QMEmployeeDateModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_entries(new QList<QMEmployeeDateEntry>())
{}

QMEmployeeDateModel::~QMEmployeeDateModel()
{
    delete m_entries;
}

QVariant QMEmployeeDateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }

    if (orientation == Qt::Orientation::Horizontal)
    {
        switch (section)
        {
            case 0:
                return tr("Mitarbeiter");
            case 1:
                return tr("Schulungsdatum");
            default:
                return {};
        }
    }
    else
    {
        return section + 1;
    }
}

bool QMEmployeeDateModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

int QMEmployeeDateModel::rowCount(const QModelIndex &) const
{
    return (int) m_entries->size();
}

int QMEmployeeDateModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant QMEmployeeDateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    if (role == Qt::DisplayRole)
    {
        int row = index.row();
        switch (index.column())
        {
            case 0:
                return m_entries->at(row).employeeName;
            case 1:
                return m_entries->at(row).trainDate.toString(Qt::DateFormat::ISODate);
            default:
                return {};
        }
    }

    if (role == Qt::BackgroundRole)
    {
        // TODO:: Return background color.
    }

    return {};
}

bool QMEmployeeDateModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QAbstractTableModel::setData(index, value, role);
}

Qt::ItemFlags QMEmployeeDateModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool QMEmployeeDateModel::addEntry(const QMEmployeeDateEntry &entry)
{
    if (entry.employeeId <= -1)
    {
        return false;
    }

    for (const QMEmployeeDateEntry &tmpEntry : *m_entries)
    {
        if (tmpEntry.employeeId == entry.employeeId)
        {
            return false;
        }
    }

    m_entries->append(entry);

    beginInsertRows(QModelIndex(), (int) m_entries->size(), (int) m_entries->size());
    endInsertRows();

    return true;
}

bool QMEmployeeDateModel::removeEntry(int index)
{
    if (index < 0 || index >= m_entries->size())
    {
        return false;
    }

    m_entries->removeAt(index);
    return true;
}
