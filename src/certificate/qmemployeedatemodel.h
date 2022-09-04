// qmemployeedatemodel.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEEDATEMODEL_H
#define QMEMPLOYEEDATEMODEL_H

#include <QAbstractTableModel>
#include <QDate>

#include <memory>

// Data for one entry. This includes the employee naame and the date when the training happen. This should be the exact
// training date as written down on the signing list and not the planned date.
struct QMEmployeeDateEntry
{
    // The primary id of the employee and its name.
    int employeeId = -1;
    QString employeeName;

    // The date when the training happened related to the certificate.
    QDate trainDate;
};

/// The model represents a table structure of dates and names of employees' training.
/// \author Christian Kr, Copyright 2022
class QMEmployeeDateModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMEmployeeDateModel(QObject *parent = nullptr);

    /// Destructor
    ~QMEmployeeDateModel() override;

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

    /// Add new entry.
    /// \param entry Only valid if not already existing by testing the employee id.
    /// \return True if successfully added, else false.
    bool addEntry(const QMEmployeeDateEntry &entry);

    /// Remove an existing entry.
    /// \param index The index number of the entry to remove.
    /// \return True if entry removed, else false. False might also be an index number out of range.
    bool removeEntry(int index);

private:
    QList<QMEmployeeDateEntry> *m_entries;
};

#endif // QMEMPLOYEEDATEMODEL_H
