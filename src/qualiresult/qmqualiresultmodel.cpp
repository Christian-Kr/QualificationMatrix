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
#include "model/training/qmtrainingviewmodel.h"
#include "ams/qmamsmanager.h"
#include "model/training/qmtrainingexceptionviewmodel.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QSqlError>
#include <QList>
#include <QDate>
#include <QColor>
#include <QSortFilterProxyModel>
#include <QHash>

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

QMQualiResultRecord* QMQualiResultModel::getResultFromRecord(const QSqlRecord &record)
{
    auto *result = new QMQualiResultRecord(this);

    result->setFirstName(record.value("employee_name").toString());
    result->setFunction(record.value("func_name").toString());
    result->setInterval(record.value("train_interval").toInt());
    result->setLastDate(record.value("train_date_1").toString());
    result->setNextDate(record.value("train_date_2").toString());
    result->setTraining(record.value("train_name").toString());
    result->setQualiState(record.value("quali_state").toString());
    result->setTrainingGroup(record.value("train_group").toString());

    // Calculate and set further information.
    auto lastDate = QDate::fromString(result->getLastDate(), Qt::DateFormat::ISODate);
    QDate nextDate;
    if (result->getInterval() > 0)
    {
        nextDate = lastDate.addYears(result->getInterval());
    }

    if (result->getQualiState().compare(tr("Pflicht")) == 0)
    {
        auto currDate = QDate::currentDate();

        if (lastDate.isValid())
        {
            if (result->getInterval() == 0)
            {
                result->setTrainingState(tr("Gut"));
            }
            else
            {
                if (nextDate < currDate)
                {
                    result->setTrainingState(tr("Schlecht"));
                    result->setInformation(tr("Schulung planen!"));
                }
                else
                {
                    auto &settings = QMApplicationSettings::getInstance();
                    auto monthExpire = settings.read("QualiResult/MonthExpire", 6).toInt();

                    result->setTrainingState(tr("Gut"));

                    if (nextDate.addMonths(-1*monthExpire) < currDate && !nextDate.isValid())
                    {
                        result->setInformation(tr("Schulung planen!"));
                    }
                }
            }
        }
        else
        {
            result->setTrainingState(tr("Schlecht"));
            result->setInformation(tr("Schulung planen!"));
        }
    }
    else
    {
        result->setTrainingState(tr("Gut"));
    }

    return result;
}

bool QMQualiResultModel::updateQualiInfo(const QString &filterName, const QString &filterFunc,
    const QString &filterTrain, const QString &filterEmployeeGroup, bool showTemporarilyDeactivated,
    bool showPersonnelLeasing, bool showTrainee, bool showApprentice)
{
    resetModel();

    // Informate listener.
    emit beforeUpdateQualiInfo(tr("Datenbankabfrage"), 100);

    // Generate filter string for query.

    QStringList employeeFilterValues;
    QString employeeFilterQuery = "";
    for (const QString &employee : filterName.split(";"))
    {
        if (!employee.isEmpty())
        {
            employeeFilterValues << "employee_name = '" + employee + "'";
        }
    }
    employeeFilterQuery = (!employeeFilterValues.isEmpty()) ? "(" + employeeFilterValues.join(" OR ") + ") and " : "";

    QStringList employeeGroupFilterValues;
    QString employeeGroupFilterQuery = "";
    for (const QString &employeeGroup : filterEmployeeGroup.split(";"))
    {
        if (!employeeGroup.isEmpty())
        {
            employeeGroupFilterValues << "employee_group = '" + employeeGroup + "'";
        }
    }
    employeeGroupFilterQuery = (!employeeGroupFilterValues.isEmpty()) ? "(" + employeeGroupFilterValues.join(" OR ") +
            ") and " : "";

    QStringList funcFilterValues;
    QString funcFilterQuery = "";
    for (const QString &func : filterFunc.split(";"))
    {
        if (!func.isEmpty())
        {
            funcFilterValues << "func_name = '" + func + "'";
        }
    }
    funcFilterQuery = (!funcFilterValues.isEmpty()) ? "(" + funcFilterValues.join(" OR ") + ") and " : "";

    QStringList trainFilterValues;
    QString trainFilterQuery = "";
    for (const QString &train : filterTrain.split(";"))
    {
        if (!train.isEmpty())
        {
            trainFilterValues << "train_name = '" + train + "'";
        }
    }
    trainFilterQuery = (!trainFilterValues.isEmpty()) ? "(" + trainFilterValues.join(" OR ") + ") and " : "";

    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return false;
    }

    // Test if a query should include temporarily deactivated employee or not.
    QString temporarilyDeactivatedQuery = "";
    if (!showTemporarilyDeactivated)
    {
        temporarilyDeactivatedQuery = "Employee.temporarily_deactivated = 0 and ";
    }

    // Test if a query should include personnel leasing employee or not.
    QString personnelLeasingQuery = "";
    if (!showPersonnelLeasing)
    {
        personnelLeasingQuery = "Employee.personnel_leasing = 0 and ";
    }

    // Test if a query should include trainee employee or not.
    QString traineeQuery = "";
    if (!showTrainee)
    {
        traineeQuery = "Employee.trainee = 0 and ";
    }

    // Test if a query should include apprentice employee or not.
    QString apprenticeQuery = "";
    if (!showApprentice)
    {
        apprenticeQuery = "Employee.apprentice = 0 and ";
    }

    // Most important filter should be done for employee ams manager permissions. The ams system restricts the access
    // to employees. Only access to employees is allowed, that are in the permission list of the
    auto amsManager = QMAMSManager::getInstance();

    QStringList primaryKeysString;
    QList<int> primaryKeyInt = amsManager->getEmployeePrimaryKeys();
    for (int i = 0; i < primaryKeyInt.count(); i++)
    {
        if (i != 0)
        {
            primaryKeysString << ",";
        }

        primaryKeysString << QString("%1").arg(primaryKeyInt.at(i));
    }

    QString amsQuery = "Employee.id IN (" + primaryKeysString.join("") + ") and";

    auto db = QSqlDatabase::database("default");

    QString strQualiMatrixQuery =
            "SELECT DISTINCT "
            "   Train.id as train_id, "
            "   Employee.id as employee_id, "
            "   Employee.name as employee_name, "
            "   Shift.name as employee_group, "
            "   Func.name as func_name, "
            "   Train.name as train_name, "
            "   TrainGroup.name train_group, "
            "   Train.interval as train_interval, "
            "   QualiState.name as quali_state, "
            "   TrainDataState1.date as train_date_1, "
            "   TrainDataState1.state as train_state_1, "
            "   TrainDataState2.date as train_date_2, "
            "   TrainDataState2.state as train_state_2, "
            "   TrainException.explanation as train_exception "
            "FROM "
            "   Employee, Train, Func, EmployeeFunc, QualiData, QualiState, TrainGroup, Shift "
            "LEFT OUTER JOIN TrainData TrainDataState1 ON "
            "   Train.id = TrainDataState1.train and "
            "   Employee.id = TrainDataState1.employee and "
            "   TrainDataState1.state = 1 "
            "LEFT OUTER JOIN TrainData TrainDataState2 ON "
            "   Train.id = TrainDataState2.train and "
            "   Employee.id = TrainDataState2.employee and "
            "   TrainDataState2.state = 2 "
            "LEFT OUTER JOIN TrainException ON "
            "   Train.id = TrainException.train and "
            "   Employee.id = TrainException.employee "
            "WHERE "
            + amsQuery +
            "   Employee.shift = Shift.id and "
            "   Train.'group' = TrainGroup.id and "
            "   QualiData.train = Train.id and "
            "   QualiData.func = Func.id and "
            "   EmployeeFunc.func = Func.id and "
            "   EmployeeFunc.employee = Employee.id and "
            "   QualiData.qualistate = QualiState.id and "
            "   Employee.active = 1 and "
            + employeeFilterQuery
            + funcFilterQuery
            + employeeGroupFilterQuery
            + trainFilterQuery
            + temporarilyDeactivatedQuery
            + personnelLeasingQuery
            + traineeQuery
            + apprenticeQuery +
            "(train_date_1 = ( "
            "   SELECT "
            "       MAX(date) "
            "   FROM "
            "       TrainData "
            "   WHERE "
            "       Train.id = TrainData.train and "
            "       Employee.id = TrainData.employee and "
            "       TrainData.state = 1 "
            ") OR train_date_1 IS NULL) and "
            "(train_date_2 = ( "
            "   SELECT "
            "       MAX(date) "
            "   FROM "
            "       TrainData "
            "   WHERE "
            "       Train.id = TrainData.train and "
            "       Employee.id = TrainData.employee and "
            "       TrainData.state = 2 "
            ") OR train_date_2 IS NULL) "
            "ORDER BY "
            "   Employee.name";

    QSqlQuery query(strQualiMatrixQuery, db);
    query.last();
    int numElem = query.at() + 1;
    query.seek(-1);

    auto &settings = QMApplicationSettings::getInstance();
    auto ignoreList = settings.read("QualiResult/IgnoreList", QStringList()).toStringList();
    auto doIgnore = settings.read("QualiResult/DoIgnore", true).toBool();

    // Get the TrainExceptionView model.
    QMTrainingExceptionViewModel trainExceptViewModel(this, db);
    trainExceptViewModel.select();

    // Create hash table from training exception model.
    QHash<QString, QString> trainExceptHash;
    for (int i = 0; i < trainExceptViewModel.rowCount(); i++)
    {
        auto name = trainExceptViewModel.data(trainExceptViewModel.index(i, 1)).toString();
        auto train = trainExceptViewModel.data(trainExceptViewModel.index(i, 2)).toString();
        auto desc = trainExceptViewModel.data(trainExceptViewModel.index(i, 3)).toString();

        trainExceptHash.insert(name + "_" + train, desc);
    }

    int i = 0;
    while (query.next())
    {
        emit updateUpdateQualiInfo(i++*100/numElem);

        // Read in the first entry.
        QMQualiResultRecord *result = getResultFromRecord(query.record());

        // Remove all entries, that are set on ignore for qualification result.
        if (doIgnore)
        {
            auto found = false;
            for (const QString &ignoreEntry : ignoreList)
            {
                if (result->getTraining().contains(ignoreEntry) ||
                    result->getTrainingGroup().contains(ignoreEntry))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                delete result;
                continue;
            }
        }

        // Remove all entries, an expetion exist for.
        if (trainExceptHash.contains(result->getFirstName() + "_" + result->getTraining()))
        {
            delete result;
            continue;
        }

        resultRecords->append(result);
    }

    // After finish the results are not accesible anymore.
    query.finish();

    beginInsertRows(QModelIndex(), 0, (int) resultRecords->size() - 1);
    endInsertRows();

    // Informate listener.
    emit afterUpdateQualiInfo();

    return true;
}

QMQualiResultRecord* QMQualiResultModel::getQualiResultRecord(const int &num)
{
    if (num >= resultRecords->count() || num < 0)
    {
        return nullptr;
    }

    return resultRecords->at(num);
}

QVariant QMQualiResultModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return tr("Angemeldete Schulung");
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

    return {};
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
