// qmqualimatrixmodel.h is part of QualificationMatrix
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

#ifndef QMQUALIMATRIXMODEL_H
#define QMQUALIMATRIXMODEL_H

#include <QAbstractTableModel>

#include <memory>

class QMSqlRelationalTableModel;
class QSqlRelationalTableModel;
class QSortFilterProxyModel;

/// A model for holding the table view data representation
/// \author Christian Kr, Copyright (c) 2020
class QMQualiMatrixModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMQualiMatrixModel(QObject *parent = nullptr);

    /// Destructor
    ~QMQualiMatrixModel() override;

    /// Override from QAbstractTableModel
    /// \param orientation
    /// \param role
    /// \param section
    /// \return
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /// Override from QAbstractTableModel
    /// \param orientation
    /// \param section
    /// \param role
    /// \param value
    /// \return
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

    /// Override from QAbstractTableModel
    /// \param parent
    /// \return
    int rowCount(const QModelIndex &parent) const override;

    /// Override from QAbstractTableModel
    /// \param parent
    /// \return
    int columnCount(const QModelIndex &parent) const override;

    /// Override from QAbstractTableModel
    /// \param index
    /// \param role
    /// \return
    QVariant data(const QModelIndex &index, int role) const override;

    /// Override from QAbstractTableModel
    /// \param index
    /// \param role
    /// \param value
    /// \return
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /// Override from QAbstractTableModel
    /// \param index
    /// \return
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// Load/reload the models from database object
    void updateModels();

    /// Set the filter to the function names and update.
    /// \param filter
    void setFuncFilter(const QString &filter);

    /// Set the filter to the training names and update.
    /// \param filter
    void setTrainFilter(const QString &filter);

    /// Set the filter to the function group names and update.
    /// \param filter
    void setFuncGroupFilter(const QString &filter);

    /// Set the filter to the training group names and update.
    /// \param filter
    void setTrainGroupFilter(const QString &filter);

    /// Set the filter for legally necessary trainings.
    /// \param filter True if activate, else false.
    void setTrainLegalFilter(bool filter);

    /// Due to performance reasons, use caching of model data.
    void buildCache();

signals:
    /// Emit before the cache will be build.
    /// \param maxSteps The maximum number of steps.
    /// \param info Information message about what will happen.
    void beforeBuildCache(QString info, int maxSteps);

    /// Emited on every loop of cache build.
    /// \param currentStep The current number of steps.
    void updateBuildCache(int currentStep);

    /// Emit when the build cache has been finished.
    void afterBuildCache();

private:
    /// Get the state row in the qualiModel from func and train index.
    /// \param funcRow the function index
    /// \param trainRow the training index
    /// \return any number if row found, else < 0.
    int qualiStateRowFromFuncTrain(const int &funcRow, const int &trainRow) const;

    std::shared_ptr<QSqlRelationalTableModel> funcModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QMSqlRelationalTableModel> qualiModel;

    QSortFilterProxyModel *funcFilterModel;
    QSortFilterProxyModel *trainFilterModel;
    QSortFilterProxyModel *funcFilterGroupModel;
    QSortFilterProxyModel *trainFilterGroupModel;
    QSortFilterProxyModel *trainFilterLegalModel;

    QHash<QString, QString> *cache;
};

#endif // QMQUALIMATRIXMODEL_H
