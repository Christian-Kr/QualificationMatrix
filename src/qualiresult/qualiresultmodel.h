//
// qualiresultmodel.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef QUALIRESULTMODEL_H
#define QUALIRESULTMODEL_H

#include <QAbstractTableModel>

#include <memory>

// forward declarations
class QSqlRelationalTableModel;

class QualiResultRecord;

/**
 * @brief a model for holding the table view data representation
 * @author Christian Kr, Copyright (c) 2020
 */
class QualiResultModel: public QAbstractTableModel
{
Q_OBJECT

public:
    /**
     * @brief constructor
     * @param parent
     */
    explicit QualiResultModel(QObject *parent = nullptr);

    ~QualiResultModel();

    /**
     * @brief override from QAbstractTableModel
     */
    QVariant headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole
    ) const override;

    /**
     * @brief override from QAbstractTableModel
     */
    bool setHeaderData(
        int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole
    ) override;

    /**
     * @brief override from QAbstractTableModel
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief override from QAbstractTableModel
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief override from QAbstractTableModel
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief override from QAbstractTableModel
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * @brief override from QAbstractTableModel
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief load/reload the models from database object
     */
    void updateModels();

    /**
     * @brief set different filter for the database
     */
    void setEmployeeFilter(const QString &filter);

    void setFuncFilter(const QString &filter);

    void setTrainFilter(const QString &fitler);

    /**
     * @brief update the quali information with filter for all data
     * @return true if succes, else false
     */
    bool updateQualiInfo(
        const QString &filterName, const QString &filterFunc, const QString &filterTrain,
        const QString &filterEmployeeGroup
    );

    /**
     * @brief Search in train model for name and return the intervall
     * @param train The train name to search for
     * @return -1 if train not found
     */
    int getIntervallFromTrain(const QString &train);

    /**
     * @brief Resets the model to be empty.
     */
    void resetModel();

signals:

    /**
     * @brief Emited before the quali info will be updated.
     * @param maxSteps The maximum number of steps for the progress.
     */
    void beforeUpdateQualiInfo(int maxSteps);

    /**
     * @brief Emited after the quali info will be updated.
     */
    void afterUpdateQualiInfo();

    /**
     * @brief Emited when step gets updated.
     * @param currentStep The current number of progress.
     * @param maxSteps The maximum number might get an update while running.
     */
    void updateUpdateQualiInfo(int currentStep);

private:
    /**
     * @brief get the state row in the qualiModel from func and train index
     * @param funcRow the function index
     * @param trainRow the training index
     * @return any number if row found, else < 0
     */
    int qualiStateRowFromFuncTrain(const int &funcRow, const int &trainRow) const;

    std::shared_ptr<QSqlRelationalTableModel> funcModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> trainDataModel;
    std::shared_ptr<QSqlRelationalTableModel> qualiModel;
    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlRelationalTableModel> employeeFuncModel;
    std::shared_ptr<QSqlRelationalTableModel> trainExceptionModel;

    QString employeeFilter;
    QString funcFilter;
    QString trainFilter;

    QList<QualiResultRecord *> *resultRecords;
};

#endif // QUALIRESULTMODEL_H
