// qmsqlrelationaltablemodel.cpp is part of QualificationMatrix
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

#ifndef QMSQLRELATIONALTABLEMODEL_H
#define QMSQLRELATIONALTABLEMODEL_H

#include <QSqlRelationalTableModel>

/// Own data class for making it more customize.
/// \author Christian Kr, Copyright 2020
class QMSqlTableModel: public QSqlRelationalTableModel
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    /// \param db The database to work with.
    explicit QMSqlTableModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase(),
            bool doFetchAll = true, bool doFetchAllSub = true);

    /// Override from QSqlRelationalTableModel.
    /// \return True is selection was successful, else false.
    bool select() override;

    /// Override from QSqlRelationalTableModel.
    /// FIXME: Default argument on override
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /// Override from QSqlRelationalTableModel.
    /// FIXME: Default argument on override
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /// Fetch all sub data exceeding 255 limit.
    /// \param value
    void setFetchAll(bool value) { doFetchAll = value; }
    [[nodiscard]] bool getFetchAll() const { return doFetchAll; }

    /// Fetch all sub data (relation tables) exceeding 255 limit.
    /// \param value
    void setFetchAllSub(bool value) { doFetchAllSub = value; }
    [[nodiscard]] bool getFetchAllSub() const { return doFetchAllSub; }

    /// This function musst be implemented for the data to be recreated after clear.
    virtual void initModel() {};

    /// Set the limit of the select rows.
    /// \param value The limit number.
    void setLimit(int value) { limit = value; }

    /// Get the selection limit row number.
    /// \return The limit value.
    [[nodiscard]] int getLimit() const { return limit; }

public slots:
    /// Test for need of new select. Based on the models changed in application.
    /// \param sender The sending data object indicating whether the reciever needs to be updated.
    virtual void otherModelChanged(QObject *sender) {};

protected:
    /// Override from QSqlRelationalTableModel.
    [[nodiscard]] QString selectStatement() const override;

private:
    /// If true, fetch all data exceeding the 255 limit of the tables.
    bool doFetchAll;

    /// If true, fetch all data exceeding the 255 limit for sub table models. This is important
    /// for all relational table models only. The tables of relations will be created with
    /// QTable, which also has the limit of 255.
    bool doFetchAllSub;

    // Limit the number of rows per select statement.
    int limit;
};

#endif // QMSQLRELATIONALTABLEMODEL_H
