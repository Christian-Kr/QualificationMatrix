// qmtraindataconflictmodel.h is part of QualificationMatrix
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

#ifndef QMTRAINDATACONFLICTMODEL_H
#define QMTRAINDATACONFLICTMODEL_H

#include <QAbstractTableModel>
#include <QDate>

#include <memory>

// Data for one train data conflict entry. It holds several properties of an train data entry and the conflict
// which has been choosen by the user.
struct QMTrainDataConflictEntry
{
    // The primary key of the training data entry.
    int trainDataId = -1;

    // The primary key and name of the training entry.
    int trainId = -1;
    QString trainName;

    // The primary key and name of the employee.
    int employeeId = -1;
    QString employeeName;

    // The training date of the train data entry.
    QDate trainDate;

    // The training state id of the training data entry.
    int trainState = -1;
};

/// The model holds some information about every train data row and additionally a column of how to handel the conflict
/// of the entry.
/// \author Christian Kr, Copyright 2022
class QMTrainDataConflictModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMTrainDataConflictModel(QObject *parent = nullptr);

    /// Destructor
    ~QMTrainDataConflictModel() override;

    /// Override from QAbstractTableModel.
    /// \param orientation
    /// \param role
    /// \param section
    /// \return
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /// Override from QAbstractTableModel.
    /// \param section
    /// \param orientation
    /// \param role
    /// \param value
    /// \return
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

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

    /// Update the model with given train data ids. The model will be completly reseted before.
    /// \param ids The ids to be set.
    /// \return True if successful, else false.
    bool updateFromTrainDataIds(const QList<int> &ids);

private:
    QList<QMTrainDataConflictEntry> *m_conflictEntries;
};

#endif // QMTRAINDATACONFLICTMODEL_H
