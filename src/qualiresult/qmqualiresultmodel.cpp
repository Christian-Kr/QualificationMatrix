//
// qmqualiresultmodel.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#include "qmqualiresultmodel.h"
#include "qmqualiresultrecord.h"
#include "model/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"

#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include <QDate>
#include <QColor>

QMQualiResultModel::QMQualiResultModel(QObject *parent)
    : QAbstractTableModel(parent), funcModel(nullptr), trainModel(nullptr), trainDataModel(nullptr),
    qualiModel(nullptr), employeeModel(nullptr), employeeFuncModel(nullptr),
    trainExceptionModel(nullptr), resultRecords(new QList<QMQualiResultRecord *>())
{
}

void QMQualiResultModel::updateModels()
{
    auto dm = QMDataManager::getInstance();

    funcModel = dm->getFuncModel();
    trainModel = dm->getTrainModel();
    qualiModel = dm->getQualiModel();
    employeeModel = dm->getEmployeeModel();
    employeeFuncModel = dm->getEmployeeFuncModel();
    trainDataModel = dm->getTrainDataModel();
    trainExceptionModel = dm->getTrainExceptionModel();

    //    emit headerDataChanged(Qt::Orientation::Horizontal, 0, trainModel->rowCount() - 1);
    //    emit headerDataChanged(Qt::Orientation::Vertical, 0, funcModel->rowCount() - 1);
}

void QMQualiResultModel::setEmployeeFilter(const QString &filter)
{
    employeeFilter = filter;
}

void QMQualiResultModel::setTrainFilter(const QString &filter)
{
    trainFilter = filter;
}

void QMQualiResultModel::setFuncFilter(const QString &filter)
{
    funcFilter = filter;
}

void QMQualiResultModel::resetModel()
{
    // Removes all data in the model. Listener should be informed about changes in row number.

    beginRemoveRows(QModelIndex(), 0, resultRecords->size() - 1);

    while (!resultRecords->isEmpty())
    {
        delete resultRecords->takeFirst();
    }

    endRemoveRows();
}

bool QMQualiResultModel::updateQualiInfo(
    const QString &filterName, const QString &filterFunc, const QString &filterTrain,
    const QString &filterEmployeeGroup)
{
    if (funcModel == nullptr || trainModel == nullptr || employeeModel == nullptr ||
        employeeFuncModel == nullptr || qualiModel == nullptr || trainDataModel == nullptr)
    {
        return false;
    }

    resetModel();

    // Informate listener.
    emit beforeUpdateQualiInfo(employeeModel->rowCount());

    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    QStringList ignoreList = settings.read("QualiResult/IgnoreList", QStringList()).toStringList();
    bool doIgnore = settings.read("QualiResult/DoIgnore", true).toBool();

    for (int i = 0; i < employeeModel->rowCount(); i++)
    {
        // Informate listener.
        emit updateUpdateQualiInfo(i);

        // Build new data structure.
        QString name = employeeModel->data(employeeModel->index(i, 1)).toString();
        QString employeeGroup = employeeModel->data(employeeModel->index(i, 2)).toString();
        QString id = employeeModel->data(employeeModel->index(i, 0)).toString();
        bool activated = employeeModel->data(employeeModel->index(i, 3)).toBool();

        if ((!filterName.isEmpty() && !name.contains(filterName)) || !activated)
        {
            continue;
        }

        if (!filterEmployeeGroup.isEmpty() && !employeeGroup.contains(filterEmployeeGroup))
        {
            continue;
        }

        // get all functions as a list from an employee
        employeeFuncModel->setFilter("employee='" + id + "'");
        for (int j = 0; j < employeeFuncModel->rowCount(); j++)
        {
            QString func = employeeFuncModel->data(employeeFuncModel->index(j, 2)).toString();

            // Ignore if filtered or global filtered in settings.
            if (!filterFunc.isEmpty() && !func.contains(filterFunc))
            {
                continue;
            }

            // go through all trainings of a function
            qualiModel->setFilter("relTblAl_1.name='" + func + "'");
            for (int k = 0; k < qualiModel->rowCount(); k++)
            {
                QString train = qualiModel->data(qualiModel->index(k, 2)).toString();
                QString qualiState = qualiModel->data(qualiModel->index(k, 3)).toString();

                // Ignore if filtered or global filtered in settings.
                if ((!filterTrain.isEmpty() && !train.contains(filterTrain)) ||
                    (doIgnore && ignoreList.contains(train)))
                {
                    continue;
                }

                // General filter in settings für ignoring trainings and training groups.
                if (doIgnore)
                {
                    QString trainGroup;
                    for (int l = 0; l < trainModel->rowCount(); l++)
                    {
                        if (trainModel->data(trainModel->index(l, 1)).toString() == train)
                        {
                            trainGroup = trainModel->data(trainModel->index(l, 2)).toString();
                        }
                    }
                    if (!trainGroup.isEmpty() && ignoreList.contains(trainGroup))
                    {
                        continue;
                    }
                }

                // Search for train exception and filter train.
                bool filterException = false;
                for (int l = 0; l < trainExceptionModel->rowCount(); l++)
                {
                    QString temEmployee = trainExceptionModel
                        ->data(trainExceptionModel->index(l, 1)).toString();
                    QString temTrain = trainExceptionModel->data(trainExceptionModel->index(l, 2))
                        .toString();
                    if (temEmployee == name && temTrain == train)
                    {
                        filterException = true;
                    }
                }
                if (filterException)
                {
                    continue;
                }

                // create result record with all properties
                auto record = new QMQualiResultRecord(this);
                record->setFirstName(name);
                record->setFunction(func);
                record->setTraining(train);
                record->setQualiState(qualiState);

                // Find all trainings of this type for the employee and add them to calculate
                // the latest last date
                trainDataModel->setFilter("relTblAl_1.name='" + name + "'");
                QDate date;
                QString trainDataState;
                for (int l = 0; l < trainDataModel->rowCount(); l++)
                {
                    QString trainDataEntry = trainDataModel->data(trainDataModel->index(l, 2))
                        .toString();

                    if (train == trainDataEntry)
                    {
                        // Found an entry that fits the train and employee in train data.

                        // Get and set the date and train data state.
                        QString strDate = trainDataModel->data(trainDataModel->index(l, 3))
                            .toString();
                        QDate tmpDate = QDate::fromString(strDate, Qt::ISODate);
                        if (date.isNull())
                        {
                            date = tmpDate;
                            trainDataState = trainDataModel->data(trainDataModel->index(l, 4))
                                .toString();
                        }
                        else
                        {
                            if (tmpDate > date)
                            {
                                date = tmpDate;
                                trainDataState = trainDataModel->data(trainDataModel->index(l, 4))
                                    .toString();
                            }
                        }
                    }
                }
                if (date.isValid())
                {
                    record->setLastDate(date.toString(Qt::ISODate));
                    record->setTrainingDataState(trainDataState);
                }
                else
                {
                    record->setLastDate("Nicht vorhanden!");
                }

                // Next date
                int intervall = getIntervallFromTrain(train);
                if (intervall == 0 && date.isValid())
                {
                    record->setNextDate("Keine Notwendigkeit!");
                }
                else
                {
                    record->setNextDate(date.addYears(intervall).toString(Qt::ISODate));
                }

                // Training state
                if (qualiState != "Pflicht" || (date.isValid() && intervall == 0))
                {
                    record->setTrainingState("Gut");
                }
                else
                {
                    int yearDiff = QDate::currentDate().year() - date.addYears(intervall).year();
                    if (yearDiff == 0)
                    {
                        record->setTrainingState("Ausreichend");
                    }
                    else if (yearDiff > 0)
                    {
                        record->setTrainingState("Schlecht");
                    }
                    else
                    {
                        record->setTrainingState("Gut");
                    }
                }

                beginInsertRows(
                    index(resultRecords->size(), 0), resultRecords->size(), resultRecords->size());
                resultRecords->append(record);
                endInsertRows();
            }
        }
    }

    employeeFuncModel->setFilter("");
    qualiModel->setFilter("");
    trainDataModel->setFilter("");

    // Informate listener.
    emit afterUpdateQualiInfo();

    return true;
}

QVariant QMQualiResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Orientation::Horizontal)
    {
        switch (section)
        {
            case 0:
                return tr("Name");
            case 1:
                return tr("Funktion");
            case 2:
                return tr("Schulung");
            case 3:
                return tr("Notwendigkeit");
            case 4:
                return tr("Letzte Schulung");
            case 5:
                return tr("Nächste Schulung");
            case 6:
                return tr("Status");
            case 7:
                return tr("Schulungsstatus");
        }
    }
    else
    {
        return section + 1;
    }

    return QVariant();
}

bool QMQualiResultModel::setHeaderData(
    int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

int QMQualiResultModel::rowCount(const QModelIndex &) const
{
    return this->resultRecords->size();
}

int QMQualiResultModel::columnCount(const QModelIndex &) const
{
    return 8;
}

QVariant QMQualiResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        int row = index.row();
        switch (index.column())
        {
            case 0:
                return resultRecords->at(row)->getFirstName();
            case 1:
                return resultRecords->at(row)->getFunction();
            case 2:
                return resultRecords->at(row)->getTraining();
            case 3:
                return resultRecords->at(row)->getQualiState();
            case 4:
                return resultRecords->at(row)->getLastDate();
            case 5:
                return resultRecords->at(row)->getNextDate();
            case 6:
                return resultRecords->at(row)->getTrainingState();
            case 7:
                return resultRecords->at(row)->getTrainingDataState();
        }

        return resultRecords->at(index.row())->getFirstName();
    }

    if (role == Qt::BackgroundRole)
    {
        if (index.column() == 6 && index.data().toString() == "Schlecht")
        {
            QMApplicationSettings &settings = QMApplicationSettings::getInstance();
            QString badColor = settings.read("QualiResult/BadColor", "#ffffff").toString();
            return QVariant(QColor(badColor));
        }
        if (index.column() == 6 && index.data().toString() == "Gut")
        {
            QMApplicationSettings &settings = QMApplicationSettings::getInstance();
            QString okColor = settings.read("QualiResult/OkColor", "#ffffff").toString();
            return QVariant(QColor(okColor));
        }
        if (index.column() == 6 && index.data().toString() == "Ausreichend")
        {
            QMApplicationSettings &settings = QMApplicationSettings::getInstance();
            QString enoughColor = settings.read("QualiResult/EnoughColor", "#ffffff").toString();
            return QVariant(QColor(enoughColor));
        }
    }

    return QVariant();
}

int QMQualiResultModel::qualiStateRowFromFuncTrain(const int &funcRow, const int &trainRow) const
{
    QString funcName = funcModel->data(funcModel->index(funcRow, 1)).toString();
    QString trainName = trainModel->data(trainModel->index(trainRow, 1)).toString();

    for (int i = 0; i < qualiModel->rowCount(); i++)
    {
        QString qualiDataFunc = qualiModel->data(qualiModel->index(i, 1)).toString();
        QString qualiDataTrain = qualiModel->data(qualiModel->index(i, 2)).toString();

        if (funcName == qualiDataFunc && trainName == qualiDataTrain)
        {
            return i;
        }
    }

    return -1;
}

int QMQualiResultModel::getIntervallFromTrain(const QString &train)
{
    for (int i = 0; i < trainModel->rowCount(); i++)
    {
        QString name = trainModel->data(trainModel->index(i, 1)).toString();
        if (name == train)
        {
            return trainModel->data(trainModel->index(i, 3)).toInt();
        }
    }
    return -1;
}

bool QMQualiResultModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QAbstractTableModel::setData(index, value, role);
}

Qt::ItemFlags QMQualiResultModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
