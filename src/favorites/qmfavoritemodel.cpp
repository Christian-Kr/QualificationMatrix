// qmfavoritemodel.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmfavoritemodel.h"
#include "settings/qmapplicationsettings.h"

QMFavoriteModel::QMFavoriteModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void QMFavoriteModel::resetModel()
{
    beginRemoveRows(QModelIndex(), 0, (int) m_favorites.size() - 1);

    m_favorites.clear();

    endRemoveRows();
}

QVariant QMFavoriteModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return tr("Name");
            case 1:
                return tr("Pfad zur Datenbank");
            case 2:
                return tr("Zugriff");
            default:
                return {};
        }
    }
    else
    {
        return section + 1;
    }
}

int QMFavoriteModel::rowCount(const QModelIndex &) const
{
    return (int) m_favorites.size();
}

int QMFavoriteModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant QMFavoriteModel::data(const QModelIndex &index, int role) const
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
                return m_favorites.at(row)->name;
            case 1:
                return m_favorites.at(row)->dbFilePath;
            case 2:
                return m_favorites.at(row)->fileAccess;
            default:
                return {};
        }
    }

    return {};
}

bool QMFavoriteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (role == Qt::EditRole)
    {
        int row = index.row();
        switch (index.column())
        {
            // this code look sa little bit weird, just because it has a lot of repeats; in future
            // this might be helpful because types might change per column

            case 0:
                if (!value.canConvert<QString>())
                {
                    return false;
                }
                else
                {
                    m_favorites.at(row)->name = value.toString();
                    break;
                }
            case 1:
                if (!value.canConvert<QString>())
                {
                    return false;
                }
                else
                {
                    m_favorites.at(row)->dbFilePath = value.toString();
                    break;
                }
            case 2:
                if (!value.canConvert<QString>())
                {
                    return false;
                }
                else
                {
                    m_favorites.at(row)->fileAccess = value.toString();
                    break;
                }
            default:
                return false;
        }

        writeFavoritesToSettings();
    }

    return QAbstractTableModel::setData(index, value, role);
}

Qt::ItemFlags QMFavoriteModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    if (index.column() == 1)
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void QMFavoriteModel::readFavoritesFromSettings()
{
    // inform about changes
    beginRemoveRows(QModelIndex(), 0, (int) m_favorites.size() - 1);

    // clear all favorites, cause they will be recreated
    if (!m_favorites.isEmpty())
    {
        m_favorites.clear();
    }

    auto &settings = QMApplicationSettings::getInstance();

    // the data storage is mainly a list of lists. The outer list is separated with ';' and the
    // inner list with ','

    auto data = settings.read("Favorites/Data", QString()).toString();
    auto favListRaw = data.split(";");

    for (const auto &favRaw : favListRaw)
    {
        auto tmpList = favRaw.split(",");
        if (tmpList.size() != 2)
        {
            qWarning() << "parse failure while reading favorites from settings";
            continue;
        }

        const auto &favName = tmpList.at(0);
        const auto &favDbFilePath = tmpList.at(1);

        // create new favorite and add it

        m_favorites.append(std::make_shared<QMFavoriteRecord>());
        auto favorite = m_favorites.last();
        favorite->name = favName;
        favorite->dbFilePath = favDbFilePath;
    }

    // inform about changes
    endRemoveRows();
}

void QMFavoriteModel::writeFavoritesToSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // create the string to safe
    QString favListRaw = "";
    for (const auto &favorite : m_favorites)
    {
        QString favRaw = favorite->name + "," + favorite->dbFilePath;

        if (!favListRaw.isEmpty())
        {
            favListRaw += ";";
        }

        favListRaw += favRaw;
    }

    settings.write("Favorites/Data", favListRaw);
}

void QMFavoriteModel::addDefaultRecord()
{
    // inform about changes
    beginInsertRows(QModelIndex(), m_favorites.size() - 1, m_favorites.size() - 1);

    m_favorites.append(std::make_shared<QMFavoriteRecord>());
    auto favorite = m_favorites.last();
    favorite->name = tr("Favorite");
    favorite->dbFilePath = "";
    favorite->fileAccess = tr("Undefiniert");

    // inform about changes
    endInsertRows();

    writeFavoritesToSettings();
}

bool QMFavoriteModel::removeRecord(int row)
{
    if (row < 0 || row >= m_favorites.size())
    {
        return false;
    }

    // inform about changes
    beginRemoveRows(QModelIndex(), row, row);

    m_favorites.remove(row);

    // inform about changes
    endRemoveRows();

    writeFavoritesToSettings();
}
