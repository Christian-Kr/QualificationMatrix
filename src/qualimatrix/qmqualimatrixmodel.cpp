// qmqualimatrixmodel.cpp is part of QualificationMatrix
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

#include "qmqualimatrixmodel.h"
#include "model/qmdatamanager.h"
#include "model/qmfunctionviewmodel.h"
#include "model/qmtrainingviewmodel.h"
#include "model/qmqualificationmatrixmodel.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include <QSqlQuery>

#include <QDebug>

QMQualiMatrixModel::QMQualiMatrixModel(QObject *parent)
    : QAbstractTableModel(parent)
    , funcFilterModel(new QSortFilterProxyModel(this))
    , trainFilterModel(new QSortFilterProxyModel(this))
    , funcFilterGroupModel(new QSortFilterProxyModel(this))
    , trainFilterGroupModel(new QSortFilterProxyModel(this))
    , cache(new QHash<QString, QString>())
    , trainFilterLegalModel(new QSortFilterProxyModel(this))
{}

QMQualiMatrixModel::~QMQualiMatrixModel()
{
    delete cache;
}

void QMQualiMatrixModel::updateModels()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    funcViewModel = std::make_unique<QMFunctionViewModel>(this, db);
    funcViewModel->select();

    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    qualiModel = std::make_unique<QMQualificationMatrixModel>(this, db);
    qualiModel->select();

    // Update the filter models.
    funcFilterGroupModel->setSourceModel(funcViewModel.get());
    funcFilterGroupModel->setFilterKeyColumn(2);
    funcFilterModel->setSourceModel(funcFilterGroupModel);
    funcFilterModel->setFilterKeyColumn(1);

    trainFilterGroupModel->setSourceModel(trainViewModel.get());
    trainFilterGroupModel->setFilterKeyColumn(2);
    trainFilterGroupModel->sort(2);
    trainFilterLegalModel->setSourceModel(trainFilterGroupModel);
    trainFilterLegalModel->setFilterKeyColumn(4);
    trainFilterModel->setSourceModel(trainFilterLegalModel);
    trainFilterModel->setFilterKeyColumn(1);

    emit headerDataChanged(Qt::Orientation::Horizontal, 0, trainFilterModel->rowCount() - 1);
    emit headerDataChanged(Qt::Orientation::Vertical, 0, funcFilterModel->rowCount() - 1);

    // Rebuild the cache.
    buildCache();
}

void QMQualiMatrixModel::buildCache()
{
    emit beforeBuildCache(tr("Erstelle cache"), funcFilterModel->rowCount() * trainFilterModel->rowCount());

    // Clear all data in cache.
    cache->clear();

    // Build a temporary cache from qualiModel. This will improve overall update speed.
    QHash<QString, QString> tmpCache;
    for (int i = 0; i < qualiModel->rowCount(); i++)
    {
        auto qualiDataFunc = qualiModel->data(qualiModel->index(i, 1)).toString();
        auto qualiDataTrain = qualiModel->data(qualiModel->index(i, 2)).toString();
        auto qualiStateRow = qualiModel->data(qualiModel->index(i, 3)).toString();

        tmpCache.insert(QString("%1_%2").arg(qualiDataFunc, qualiDataTrain), qualiStateRow);
    }

    // Now create main cache.
    for (int i = 0; i < funcFilterModel->rowCount(); i++)
    {
        for (int j = 0; j < trainFilterModel->rowCount(); j++)
        {
            emit updateBuildCache(i * trainFilterModel->rowCount() + j);

            QString qualiDataFunc = funcFilterModel->data(funcFilterModel->index(i, 1)).toString();
            QString qualiDataTrain = trainFilterModel->data(trainFilterModel->index(j, 1))
                .toString();
            QString key = QString("%1_%2").arg(qualiDataFunc, qualiDataTrain);

            if (tmpCache.contains(key))
            {
                cache->insert(QString("%1_%2").arg(i).arg(j), tmpCache.value(key));
            }
        }
    }

    emit afterBuildCache();
}

QVariant QMQualiMatrixModel::headerData(int section, Qt::Orientation orientation, int) const
{
    switch (orientation)
    {
        case Qt::Orientation::Horizontal:
            return trainFilterModel->data(trainFilterModel->index(section, 1));
        case Qt::Orientation::Vertical:
            return funcFilterModel->data(funcFilterModel->index(section, 1));
    }

    return QVariant();
}

bool QMQualiMatrixModel::setHeaderData(int section, Qt::Orientation orientation,
    const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        emit headerDataChanged(orientation, section, section);
        return true;
    }

    return false;
}

int QMQualiMatrixModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return funcFilterModel->rowCount();
}

int QMQualiMatrixModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return trainFilterModel->rowCount();
}

QVariant QMQualiMatrixModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        auto key = QString("%1_%2").arg(index.row()).arg(index.column());

        if (cache->contains(key))
        {
            return QVariant(cache->value(key));
        }
    }

    if (role == Qt::ToolTipRole)
    {
        return tr("P = Pflicht; A = Angebot; S = Sonstiges");
    }

    return QVariant();
}

int QMQualiMatrixModel::qualiStateRowFromFuncTrain(const int &funcRow, const int &trainRow) const
{
    auto funcName = funcFilterModel->data(funcFilterModel->index(funcRow, 1)).toString();
    auto trainName = trainFilterModel->data(trainFilterModel->index(trainRow, 1)).toString();

    for (int i = 0; i < qualiModel->rowCount(); i++)
    {
        auto qualiDataFunc = qualiModel->data(qualiModel->index(i, 1)).toString();
        auto qualiDataTrain = qualiModel->data(qualiModel->index(i, 2)).toString();

        if (funcName == qualiDataFunc && trainName == qualiDataTrain) {
            return i;
        }
    }

    return -1;
}

bool QMQualiMatrixModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (qualiModel == nullptr || funcViewModel == nullptr || trainViewModel == nullptr)
    {
        return false;
    }

    auto tmpValue = value.toInt();

    // If value is already the target value, exit. This means: If entry should be a must have and it already is, then
    // there is no need for going further.
    if (data(index, role) == value)
    {
        return true;
    }

    // Try to get the row of the quali data entry with the given table index for function and training. If the result
    // is smaller than 0, there might be no entry. In that case, a new entry has to be created.
    auto qualiStateRow = qualiStateRowFromFuncTrain(index.row(), index.column());

    if (qualiStateRow < 0)
    {
        // The new row id.
        auto rowNew = qualiModel->rowCount();

        // Get the ids of the function and training for the new entry.
        auto funcNameID = funcFilterModel->data(funcFilterModel->index(index.row(), 0)).toInt();
        auto trainNameID = trainFilterModel->data(trainFilterModel->index(index.column(), 0)).toInt();

        // If inserting a new row to the temporary table, do nothing and stop.
        if (!qualiModel->insertRow(rowNew))
        {
            return false;
        }

        // If setting one value failed, stop setting the rest, delete the temporary row.
        if (!qualiModel->setData(qualiModel->index(rowNew, 1), funcNameID) ||
                !qualiModel->setData(qualiModel->index(rowNew, 2), trainNameID) ||
                !qualiModel->setData(qualiModel->index(rowNew, 3), tmpValue))
        {
            qualiModel->removeRow(rowNew);
            qCritical() << "could not set a value of a new entry to database";

            return false;
        }

        // If the new temporary row cannot be written to the database, delete the temporary row.
        if (!qualiModel->submitAll())
        {
            qualiModel->removeRow(rowNew);
            qCritical() << "writing new row to database failed";

            return false;
        }

        // Create a new entry in cache, to prevent from rebuild the whole cache. Cause there is
        // no entry in the table - which has now been added - there should not be an entry in
        // the cache, till now.
        QString key = QString("%1_%2").arg(index.row()).arg(index.column());
        if (cache->contains(key))
        {
            qCritical() << "new entry exist in cache";
        }
        else
        {
            cache->insert(key, qualiModel->data(qualiModel->index(rowNew, 3)).toString());
        }
    }
    else
    {
        if (tmpValue < 0)
        {
            // The receive value is negative, meaning to delete the line
            qualiModel->removeRow(qualiStateRow);
            qualiModel->submitAll();

            // Delete the entry in the cache.
            QString key = QString("%1_%2").arg(index.row()).arg(index.column());
            if (!cache->remove(key))
            {
                qCritical() << "existing cache entry can not be removed";
            }
        }
        else
        {
            qualiModel->setData(qualiModel->index(qualiStateRow, 3), tmpValue);

            if (!qualiModel->submitAll())
            {
                qCritical() << "could not add new entry to database";
                return false;
            }

            // Set the entry in the cache.
            QString key = QString("%1_%2").arg(index.row()).arg(index.column());
            cache->insert(key, qualiModel->data(qualiModel->index(qualiStateRow, 3)).toString());
        }
    }

    //buildCache();
    emit dataChanged(index, index, QVector<int>() << role);

    return true;
}

Qt::ItemFlags QMQualiMatrixModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void QMQualiMatrixModel::setFuncGroupFilter(const QString &filter)
{
    funcFilterGroupModel->setFilterRegExp(filter);
}

void QMQualiMatrixModel::setTrainGroupFilter(const QString &filter)
{
    trainFilterGroupModel->setFilterRegExp(filter);
}

void QMQualiMatrixModel::setFuncFilter(const QString &filter)
{
    funcFilterModel->setFilterRegExp(filter);
}

void QMQualiMatrixModel::setTrainFilter(const QString &filter)
{
    trainFilterModel->setFilterRegExp(filter);
}

void QMQualiMatrixModel::setTrainLegalFilter(bool filter)
{
    if (filter)
    {
        trainFilterLegalModel->setFilterFixedString("1");
    }
    else
    {
        trainFilterLegalModel->setFilterFixedString("");
    }
}
