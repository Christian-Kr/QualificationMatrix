//
// qualimatrixmodel.cpp is part of QualificationMatrix
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

#include "qmqualimatrixmodel.h"
#include "model/qmdatamanager.h"

#include <QSqlRelationalTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QSqlQuery>

QualiMatrixModel::QualiMatrixModel(QObject *parent)
    : QAbstractTableModel(parent), funcModel(nullptr), trainModel(nullptr), qualiModel(nullptr),
      funcFilterModel(new QSortFilterProxyModel(this)),
      trainFilterModel(new QSortFilterProxyModel(this)),
      funcFilterGroupModel(new QSortFilterProxyModel(this)),
      trainFilterGroupModel(new QSortFilterProxyModel(this)), cache(new QHash<QString, QString>())
{
}

QualiMatrixModel::~QualiMatrixModel()
{
    delete cache;
}

void QualiMatrixModel::updateModels()
{
    auto dm = QMDataManager::getInstance();
    funcModel = dm->getFuncModel();
    trainModel = dm->getTrainModel();
    qualiModel = dm->getQualiModel();

    // Update the filter models.
    funcFilterGroupModel->setSourceModel(funcModel.get());
    funcFilterGroupModel->setFilterKeyColumn(2);
    funcFilterModel->setSourceModel(funcFilterGroupModel);
    funcFilterModel->setFilterKeyColumn(1);

    trainFilterGroupModel->setSourceModel(trainModel.get());
    trainFilterGroupModel->setFilterKeyColumn(2);
    trainFilterModel->setSourceModel(trainFilterGroupModel);
    trainFilterModel->setFilterKeyColumn(1);

    emit headerDataChanged(Qt::Orientation::Horizontal, 0, trainFilterModel->rowCount() - 1);
    emit headerDataChanged(Qt::Orientation::Vertical, 0, funcFilterModel->rowCount() - 1);

    // Rebuild the cache.
    buildCache();
}

void QualiMatrixModel::buildCache()
{
    emit beforeBuildCache(funcFilterModel->rowCount() * trainFilterModel->rowCount());

    // Clear all data in cache.
    cache->clear();

    for (int i = 0; i < funcFilterModel->rowCount(); i++) {
        for (int j = 0; j < trainFilterModel->rowCount(); j++) {
            emit updateBuildCache(i * trainFilterModel->rowCount() + j);

            int qualiStateRow = qualiStateRowFromFuncTrain(i, j);
            if (qualiStateRow >= 0) {
                cache->insert(
                    QString("%1_%2").arg(i).arg(j),
                    qualiModel->data(qualiModel->index(qualiStateRow, 3)).toString());
            }
        }
    }

    emit afterBuildCache();
}

QVariant QualiMatrixModel::headerData(int section, Qt::Orientation orientation, int) const
{
    switch (orientation) {
        case Qt::Orientation::Horizontal:
            return trainFilterModel->data(trainFilterModel->index(section, 1));
        case Qt::Orientation::Vertical:
            return funcFilterModel->data(funcFilterModel->index(section, 1));
    }

    return QVariant();
}

bool QualiMatrixModel::setHeaderData(
    int section, Qt::Orientation orientation, const QVariant &value, int role
)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }

    return false;
}

int QualiMatrixModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return funcFilterModel->rowCount();
}

int QualiMatrixModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return trainFilterModel->rowCount();
}

QVariant QualiMatrixModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QString key = QString("%1_%2").arg(index.row()).arg(index.column());
        if (cache->contains(key)) {
            return QVariant(cache->value(key));
        }
    }

    if (role == Qt::ToolTipRole) {
        return tr("P = Pflicht; A = Angebot; S = Sonstiges");
    }

    return QVariant();
}

int QualiMatrixModel::qualiStateRowFromFuncTrain(const int &funcRow, const int &trainRow) const
{
    QString funcName = funcFilterModel->data(funcFilterModel->index(funcRow, 1)).toString();
    QString trainName = trainFilterModel->data(trainFilterModel->index(trainRow, 1)).toString();

    for (int i = 0; i < qualiModel->rowCount(); i++) {
        QString qualiDataFunc = qualiModel->data(qualiModel->index(i, 1)).toString();
        QString qualiDataTrain = qualiModel->data(qualiModel->index(i, 2)).toString();

        if (funcName == qualiDataFunc && trainName == qualiDataTrain) {
            return i;
        }
    }

    return -1;
}

bool QualiMatrixModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (qualiModel == nullptr || funcModel == nullptr || trainModel == nullptr) {
        return false;
    }

    int tmpValue = value.toInt();

    // If value is already the target value, exit.
    if (data(index, role) != value) {
        int qualiStateRow = qualiStateRowFromFuncTrain(index.row(), index.column());
        if (qualiStateRow < 0) {
            // no entry found, create a new one
            int rowNew = qualiModel->rowCount();
            int funcNameID = funcFilterModel->data(funcFilterModel->index(index.row(), 0)).toInt();
            int trainNameID = trainFilterModel->data(trainFilterModel->index(index.column(), 0))
                .toInt();

            qualiModel->insertRow(rowNew);
            qualiModel->setData(qualiModel->index(rowNew, 1), funcNameID);
            qualiModel->setData(qualiModel->index(rowNew, 2), trainNameID);
            qualiModel->setData(qualiModel->index(rowNew, 3), tmpValue);
            qualiModel->submitAll();
        }
        else {
            if (tmpValue < 0) {
                // The recieved value is negative, meaning to delete the line
                qualiModel->removeRow(qualiStateRow);
                qualiModel->submitAll();
            }
            else {
                qualiModel->setData(qualiModel->index(qualiStateRow, 3), tmpValue);
            }
        }

        buildCache();
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags QualiMatrixModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void QualiMatrixModel::setFuncGroupFilter(const QString &filter)
{
    funcFilterGroupModel->setFilterFixedString(filter);
}

void QualiMatrixModel::setTrainGroupFilter(const QString &filter)
{
    trainFilterGroupModel->setFilterFixedString(filter);
}

void QualiMatrixModel::setFuncFilter(const QString &filter)
{
    funcFilterModel->setFilterFixedString(filter);
}

void QualiMatrixModel::setTrainFilter(const QString &filter)
{
    trainFilterModel->setFilterFixedString(filter);
}
