// qmqualiresultmodel.cpp is part of QualificationMatrix
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

#include "qmqualiresultmodel.h"
#include "qmqualiresultrecord.h"
#include "settings/qmapplicationsettings.h"
#include "model/view/qmfunctionviewmodel.h"
#include "model/view/qmtrainingviewmodel.h"
#include "model/view/qmemployeeviewmodel.h"
#include "model/view/qmqualificationmatrixviewmodel.h"
#include "model/view/qmtrainingdataviewmodel.h"
#include "model/view/qmtrainingexceptionviewmodel.h"
#include "model/view/qmemployeefunctionviewmodel.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QList>
#include <QDate>
#include <QColor>
#include <QSortFilterProxyModel>
#include <QHash>

#include <QDebug>

QMQualiResultModel::QMQualiResultModel(QObject *parent)
    : QAbstractTableModel(parent)
    , resultRecords(new QList<QMQualiResultRecord *>())
    , intervalCache(new QHash<QString, int>())
    , trainGroupCache(new QHash<QString, QString>())
{}

void QMQualiResultModel::resetModel()
{
    beginRemoveRows(QModelIndex(), 0, resultRecords->size() - 1);

    while (!resultRecords->isEmpty())
    {
        delete resultRecords->takeFirst();
    }

    endRemoveRows();
}

bool QMQualiResultModel::updateQualiInfo(const QString &filterName, const QString &filterFunc,
    const QString &filterTrain, const QString &filterEmployeeGroup)
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMFunctionViewModel funcViewModel(this, db);
    funcViewModel.select();

    QMTrainingViewModel trainViewModel(this, db);
    trainViewModel.select();

    QMTrainingDataViewModel trainDataViewModel(this, db);
    trainDataViewModel.select();

    QMQualificationMatrixViewModel qualiViewModel(this, db);
    qualiViewModel.select();

    QMEmployeeViewModel employeeViewModel(this, db);
    employeeViewModel.select();

    QMEmployeeFunctionViewModel employeeFuncViewModel(this, db);
    employeeFuncViewModel.select();

    QMTrainingExceptionViewModel trainExceptionViewModel(this, db);
    trainExceptionViewModel.select();

    QSortFilterProxyModel filterEmployeeGroupModel(this);
    filterEmployeeGroupModel.setSourceModel(&employeeViewModel);
    filterEmployeeGroupModel.setFilterKeyColumn(2);
    filterEmployeeGroupModel.setFilterRegularExpression(filterEmployeeGroup);

    QSortFilterProxyModel filterEmployeeModel(this);
    filterEmployeeModel.setSourceModel(&filterEmployeeGroupModel);
    filterEmployeeModel.setFilterKeyColumn(1);
    filterEmployeeModel.setFilterRegularExpression(filterName);

    resetModel();

    // Rebuild caches.
    buildIntervalCache(trainViewModel);
    buildTrainGroupCache(trainViewModel);

    // Informate listener.
    emit beforeUpdateQualiInfo(tr("Berechne Qualifizierungsresultat"), filterEmployeeModel.rowCount());

    auto &settings = QMApplicationSettings::getInstance();
    auto ignoreList = settings.read("QualiResult/IgnoreList", QStringList()).toStringList();
    auto doIgnore = settings.read("QualiResult/DoIgnore", true).toBool();
    auto monthExpire = settings.read("QualiResult/MonthExpire", 6).toInt();

    for (int i = 0; i < filterEmployeeModel.rowCount(); i++)
    {
        // Informate listener.
        emit updateUpdateQualiInfo(i);

        // Build new data structure.
        QString name = filterEmployeeModel.data(filterEmployeeModel.index(i, 1)).toString();

        // get all functions as a list from an employee
        employeeFuncViewModel.setFilter("name='" + name + "'");
        for (int j = 0; j < employeeFuncViewModel.rowCount(); j++)
        {
            QString func = employeeFuncViewModel.data(employeeFuncViewModel.index(j, 2)).toString();

            // Ignore if filtered or global filtered in settings.
            if (!filterFunc.isEmpty() && !func.contains(filterFunc))
            {
                continue;
            }

            // Go through all trainings of a function.
            qualiViewModel.setFilter("name='" + func + "'");
            for (int k = 0; k < qualiViewModel.rowCount(); k++)
            {
                QString train = qualiViewModel.data(qualiViewModel.index(k, 2)).toString();
                QString qualiState = qualiViewModel.data(qualiViewModel.index(k, 3)).toString();

                // Ignore, if there is no qualiState.
                if (qualiState.isEmpty())
                {
                    continue;
                }

                // Ignore if filtered or global filtered in settings.
                if ((!filterTrain.isEmpty() && !train.contains(filterTrain)) ||
                    (doIgnore && ignoreList.contains(train)))
                {
                    continue;
                }

                // General filter in settings für ignoring trainings and training groups.
                // TODO: Implement in frontend (description)
                if (doIgnore)
                {
                    if (trainGroupCache->contains(train))
                    {
                        QString trainGroup = trainGroupCache->value(train);

                        if (!trainGroup.isEmpty() && ignoreList.contains(trainGroup))
                        {
                            continue;
                        }
                    }
                }

                // Search for train exception and filter train.
                bool filterException = false;
                for (int l = 0; l < trainExceptionViewModel.rowCount(); l++)
                {
                    QString temEmployee = trainExceptionViewModel.data(trainExceptionViewModel.index(l, 1)).toString();
                    QString temTrain = trainExceptionViewModel.data(trainExceptionViewModel.index(l, 2)).toString();
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

                // Find all trainings of this type for the employee and add them to calculate the latest last date.
                trainDataViewModel.setFilter("name='" + name + "'");

                // Final date and state objects.
                QDate lastDate;
                QDate nextDate;
                QString trainDataState;

                // Search for a training data entry, that fits the searched train to get a record.
                for (int l = 0; l < trainDataViewModel.rowCount(); l++)
                {
                    QString trainDataEntry = trainDataViewModel.data(trainDataViewModel.index(l, 2)).toString();

                    if (train == trainDataEntry)
                    {
                        // Found an entry that fits the train and employee in train data.

                        // Get and set the date and train data state.
                        QString strDate = trainDataViewModel.data(trainDataViewModel.index(l, 3)).toString();
                        QDate tmpDate = QDate::fromString(strDate, Qt::ISODate);
                        trainDataState = trainDataViewModel.data(trainDataViewModel.index(l, 4)).toString();

                        // Logic: There two different states. The training could have been conducted or registered. For
                        // both states, the newest date are the relevant ones.
                        if (trainDataState == tr("Angemeldet"))
                        {
                            if (nextDate.isNull() || tmpDate > nextDate)
                            {
                                nextDate = tmpDate;
                            }
                        }

                        if (trainDataState == tr("Durchgeführt"))
                        {
                            if (lastDate.isNull() || tmpDate > lastDate)
                            {
                                lastDate = tmpDate;
                            }
                        }
                    }
                }

                if (lastDate.isValid())
                {
                    record->setLastDate(lastDate.toString(Qt::ISODate));
                }

                if (nextDate.isValid())
                {
                    record->setNextDate(nextDate.toString(Qt::ISODate));
                }

                int interval = -1;
                if (intervalCache->contains(train))
                {
                    interval = intervalCache->value(train);
                }

                if (interval < 0)
                {
                    qDebug() << "This should never happen. No training with this name found for interval";
                }

                record->setInterval(interval);

                // Training state: Can only be good or bad. There should nothing exist inbetween.
                if (qualiState == tr("Pflicht"))
                {
                    auto currDate = QDate::currentDate();

                    if (lastDate.isValid())
                    {
                        if (interval == 0)
                        {
                            record->setTrainingState(tr("Gut"));
                        }
                        else
                        {
                            if (lastDate.addYears(interval) < currDate)
                            {
                                record->setTrainingState(tr("Schlecht"));
                                record->setInformation(tr("Schulung planen!"));
                            }
                            else
                            {
                                record->setTrainingState(tr("Gut"));

                                if (lastDate.addYears(interval).addMonths(-1*monthExpire) < currDate &&
                                    !nextDate.isValid())
                                {
                                    record->setInformation(tr("Schulung planen!"));
                                }
                            }
                        }
                    }
                    else
                    {
                        record->setTrainingState(tr("Schlecht"));
                        record->setInformation(tr("Schulung planen!"));
                    }
                }
                else
                {
                    record->setTrainingState(tr("Gut"));
                }

                resultRecords->append(record);
            }
        }
    }

    beginInsertRows(QModelIndex(), 0, resultRecords->size() - 1);
    endInsertRows();

    employeeFuncViewModel.setFilter("");
    qualiViewModel.setFilter("");
    trainDataViewModel.setFilter("");

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
                return tr("Intervall");
            case 4:
                return tr("Notwendigkeit");
            case 5:
                return tr("Letzte Schulung");
            case 6:
                return tr("Nächste Schulung");
            case 7:
                return tr("Schulungsstatus");
            case 8:
                return tr("Information");
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
    return 9;
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
                if (resultRecords->at(row)->getInterval() == 0)
                {
                    return tr("Einmalig");
                }
                else
                {
                    return tr("%1 Jahr(e)").arg(resultRecords->at(row)->getInterval());
                }
            case 4:
                return resultRecords->at(row)->getQualiState();
            case 5:
                return resultRecords->at(row)->getLastDate();
            case 6:
                return resultRecords->at(row)->getNextDate();
            case 7:
                return resultRecords->at(row)->getTrainingState();
            case 8:
                return resultRecords->at(row)->getInformation();
        }

        return resultRecords->at(index.row())->getFirstName();
    }

    if (role == Qt::BackgroundRole)
    {
        if (index.column() == 7 && index.data().toString() == "Schlecht")
        {
            QMApplicationSettings &settings = QMApplicationSettings::getInstance();
            QString badColor = settings.read("QualiResult/BadColor", "#ffffff").toString();
            return QVariant(QColor(badColor));
        }
        if (index.column() == 7 && index.data().toString() == "Gut")
        {
            QMApplicationSettings &settings = QMApplicationSettings::getInstance();
            QString okColor = settings.read("QualiResult/OkColor", "#ffffff").toString();
            return QVariant(QColor(okColor));
        }
        if (index.column() == 7 && index.data().toString() == "Ausreichend")
        {
            QMApplicationSettings &settings = QMApplicationSettings::getInstance();
            QString enoughColor = settings.read("QualiResult/EnoughColor", "#ffffff").toString();
            return QVariant(QColor(enoughColor));
        }
    }

    return QVariant();
}

void QMQualiResultModel::buildIntervalCache(QMTrainingViewModel &trainViewModel)
{
    // Clear all data in cache.
    intervalCache->clear();

    for (int i = 0; i < trainViewModel.rowCount(); i++)
    {
        QString name = trainViewModel.data(trainViewModel.index(i, 1)).toString();
        if (!intervalCache->contains(name))
        {
            intervalCache->insert(name, trainViewModel.data(trainViewModel.index(i, 3)).toInt());
        }
    }
}

void QMQualiResultModel::buildTrainGroupCache(QMTrainingViewModel &trainViewModel)
{
    // Clear all data in cache.
    trainGroupCache->clear();

    for (int i = 0; i < trainViewModel.rowCount(); i++)
    {
        QString name = trainViewModel.data(trainViewModel.index(i, 1)).toString();
        QString group = trainViewModel.data(trainViewModel.index(i, 2)).toString();
        if (!trainGroupCache->contains(name))
        {
            trainGroupCache->insert(name, group);
        }
    }
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
