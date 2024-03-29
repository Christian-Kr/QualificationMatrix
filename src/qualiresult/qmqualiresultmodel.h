// qmqualiresultmodel.h is part of QualificationMatrix
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

#ifndef QMQUALIRESULTMODEL_H
#define QMQUALIRESULTMODEL_H

#include <QAbstractTableModel>

#include <memory>

class QMQualiResultRecord;
class QMTrainingViewModel;
class QSqlTableModel;
class QSqlRecord;

/// A data for holding the table view data representation.
/// \author Christian Kr, Copyright 2020
class QMQualiResultModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMQualiResultModel(QObject *parent = nullptr);

    /// Destructor
    ~QMQualiResultModel() override = default;

    /// Override from QAbstractTableModel.
    /// \param orientation
    /// \param role
    /// \param section
    /// \return
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

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
    int rowCount(const QModelIndex &parent) const override;

    /// Override from QAbstractTableModel.
    /// \param parent
    /// \return
    int columnCount(const QModelIndex &parent) const override;

    /// Override from QAbstractTableModel.
    /// \param index
    /// \param role
    /// \return
    QVariant data(const QModelIndex &index, int role) const override;

    /// Override from QAbstractTableModel.
    /// \param value
    /// \param index
    /// \param role
    /// \return
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /// Override from QAbstractTableModel.
    /// \param index
    /// \return
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// Update the quali information with filter for all data.
    /// \param filterEmployeeGroup
    /// \param filterFunc
    /// \param filterName
    /// \param filterTrain
    /// \param showTemporaryDeactivated Show employees that are temporarily deactivated if true.
    /// \param showPersonnelLeasing Show employees that are personnel leasing if true.
    /// \param showTrainee Show employees that are trainees if true.
    /// \param showApprentice Show employees that are apprentice if true.
    /// \return true if succes, else false.
    bool updateQualiInfo(const QString &filterName, const QString &filterFunc, const QString &filterTrain,
        const QString &filterEmployeeGroup, bool showTemporaryDeactivated = false, bool showPersonnelLeasing = false,
        bool showTrainee = false, bool showApprentice = false);

    /// Resets the data to be empty.
    void resetModel();

    /// Get the result record with the given id.
    /// \param num The result record number num.
    /// \return Get the record or null if not availabe.
    QMQualiResultRecord* getQualiResultRecord(const int &num);

signals:
    /// Emited before the quali info will be updated.
    /// \param maxSteps The maximum number of steps for the progress.
    /// \param info Info message about upcoming process.
    void beforeUpdateQualiInfo(QString info, int maxSteps);

    /// Emited after the quali info will be updated.
    void afterUpdateQualiInfo();

    /// Emited when step gets updated.
    /// \param currentStep The current number of progress.
    /// \param maxSteps The maximum number might get an update while running.
    void updateUpdateQualiInfo(int currentStep);

private:
    /// Build an interval cache to speed up search.
    /// \param trainViewModel The training view data.
    void buildIntervalCache(QMTrainingViewModel &trainViewModel);

    /// Build an train group cache to speed up search. This correlates the training name to a group.
    /// \param trainViewModel The training group view data.
    void buildTrainGroupCache(QMTrainingViewModel &trainViewModel);

    /// Read the result record and return a new result record object.
    /// The object needs to be deleted by the reciever.
    /// \return The new object or nullptr if reading fails.
    QMQualiResultRecord* getResultFromRecord(const QSqlRecord &record);

    QList<QMQualiResultRecord *> *resultRecords;
    QHash<QString, int> *intervalCache;
    QHash<QString, QString> *trainGroupCache;
};

#endif // QMQUALIRESULTMODEL_H
