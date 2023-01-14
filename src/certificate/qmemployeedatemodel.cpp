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
#include "qmemployeedateentry.h"

#include <QSqlDatabase>

QMEmployeeDateModel::QMEmployeeDateModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_entries(new QList<QMEmployeeDateEntry*>())
{}

QMEmployeeDateModel::~QMEmployeeDateModel()
{
    while (!m_entries->empty())
    {
        delete m_entries->takeFirst();
    }

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
                return tr("id");
            case 1:
                return tr("Mitarbeiter");
            case 2:
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
    return 3;
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
                return m_entries->at(row)->employeeId;
            case 1:
                return m_entries->at(row)->employeeName;
            case 2:
                return m_entries->at(row)->trainDate.toString(Qt::DateFormat::ISODate);
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
    if (role == Qt::EditRole)
    {
        if (index.row() >= m_entries->size() || index.row() < 0)
        {
            qWarning() << "QMEmployeeDateModel: Wrong size of m_entries";
            return false;
        }

        switch (index.column())
        {
            case 1:
                m_entries->at(index.row())->employeeName = value.toString();
                break;
            case 2:
                m_entries->at(index.row())->trainDate = value.toDate();
                break;
            default:
                return false;
        }

        return true;
    }

    return false;
}

Qt::ItemFlags QMEmployeeDateModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags baseFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    // allow edit of date column
    if (index.column() == 2)
    {
        baseFlags = baseFlags | Qt::ItemIsEditable;
    }

    return baseFlags;
}

bool QMEmployeeDateModel::addEntry(const QMEmployeeDateEntry &entry)
{
    // don't add if employee id does not exist at all
    if (entry.employeeId <= -1)
    {
        return false;
    }

    // don't add entry if the employee already exist by its id
    for (const QMEmployeeDateEntry *tmpEntry : *m_entries)
    {
        if (tmpEntry->employeeId == entry.employeeId)
        {
            return false;
        }
    }

    auto *employeeDataEntry = new QMEmployeeDateEntry();
    employeeDataEntry->employeeId = entry.employeeId;
    employeeDataEntry->employeeName = entry.employeeName;
    employeeDataEntry->trainDate = entry.trainDate;

    m_entries->append(employeeDataEntry);

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

    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();

    return true;
}

QMEmployeeDateEntry QMEmployeeDateModel::getEntry(int row) const
{
    Q_ASSERT(row >= 0 && row < rowCount());

    return *m_entries->at(row);
}
