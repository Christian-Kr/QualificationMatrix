//
// qualimatrixmodel.h is part of QualificationMatrix
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

#ifndef QMQUALIMATRIXMODEL_H
#define QMQUALIMATRIXMODEL_H

#include <QAbstractTableModel>

#include <memory>

// forward declarations
class QSqlRelationalTableModel;

class QSortFilterProxyModel;

/**
 * @brief a model for holding the table view data representation
 * @author Christian Kr, Copyright (c) 2020
 */
class QualiMatrixModel: public QAbstractTableModel
{
Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     */
    explicit QualiMatrixModel(QObject *parent = nullptr);

    ~QualiMatrixModel() override;

    /**
     * @brief Override from QAbstractTableModel
     */
    QVariant headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole
    ) const override;

    /**
     * @brief Override from QAbstractTableModel
     */
    bool setHeaderData(
        int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole
    ) override;

    /**
     * @brief Override from QAbstractTableModel
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Override from QAbstractTableModel
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Override from QAbstractTableModel
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Override from QAbstractTableModel
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * @brief Override from QAbstractTableModel
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief load/reload the models from database object
     */
    void updateModels();

    /**
     * @brief Set the filter to the function names and update.
     * @param filter
     */
    void setFuncFilter(const QString &filter);

    /**
     * @brief Set the filter to the training names and update.
     * @param filter
     */
    void setTrainFilter(const QString &filter);

    /**
     * @brief Set the filter to the function group names and update.
     * @param filter
     */
    void setFuncGroupFilter(const QString &filter);

    /**
     * @brief Set the filter to the training group names and update.
     * @param filter
     */
    void setTrainGroupFilter(const QString &filter);

    /**
     * @brief Due to performance reasons, use caching of model data.
     */
    void buildCache();

signals:

    /**
     * @brief Emit before the cache will be build.
     * @param maxSteps The maximum number of steps.
     */
    void beforeBuildCache(int maxSteps);

    /**
     * @brief Emited on every loop of cache build.
     * @param currentStep The current number of steps.
     */
    void updateBuildCache(int currentStep);

    /**
     * @brief Emit when the build cache has been finished.
     */
    void afterBuildCache();

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
    std::shared_ptr<QSqlRelationalTableModel> qualiModel;

    QSortFilterProxyModel *funcFilterModel;
    QSortFilterProxyModel *trainFilterModel;
    QSortFilterProxyModel *funcFilterGroupModel;
    QSortFilterProxyModel *trainFilterGroupModel;

    QHash<QString, QString> *cache;
};

#endif // QMQUALIMATRIXMODEL_H
