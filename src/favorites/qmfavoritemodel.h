// qmfavoritemodel.h is part of QualificationMatrix
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

#ifndef QMFAVORITEMODEL_H
#define QMFAVORITEMODEL_H

#include <QAbstractTableModel>

#include <memory>

/// Data structure for a favorite.
struct QMFavoriteRecord
{
    QString name;
    QString dbFilePath;
    QString fileAccess;
};

/// Model for manage data of favorites.
/// \author Christian Kr, Copyright 2023
class QMFavoriteModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMFavoriteModel(QObject *parent = nullptr);

    /// Destructor
    ~QMFavoriteModel() override = default;

    /// Override from QAbstractTableModel.
    /// \param orientation
    /// \param role
    /// \param section
    /// \return
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
            int role) const override;

    /// Override from QAbstractTableModel.
    /// \param section
    /// \param orientation
    /// \param role
    /// \param value
    /// \return
    bool setHeaderData(int section, Qt::Orientation orientation,
            const QVariant &value, int role) override { return false; }

    /// Override from QAbstractTableModel.
    /// \param parent
    /// \return
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    /// Override from QAbstractTableModel.
    /// \param parent
    /// \return
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;

    /// Override from QAbstractTableModel.
    /// \param index
    /// \param role
    /// \return
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    /// Override from QAbstractTableModel.
    /// \param value
    /// \param index
    /// \param role
    /// \return
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /// Override from QAbstractTableModel.
    /// \param index
    /// \return
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// Resets the data to be empty.
    void resetModel();

    /// Adds an empty entry with defaults values, which can be changed later.
    void addDefaultRecord();

    /// Read favorites from settings.
    void readFavoritesFromSettings();

    /// Write favorites to settings.
    void writeFavoritesToSettings();

    /// Remove record bei row number.
    /// \param row the row number to remove
    /// \return true if succeed, else false
    bool removeRecord(int row);

private:
    QList<std::shared_ptr<QMFavoriteRecord>> m_favorites;
};

#endif // QMFAVORITEMODEL_H
