//
// traindatawidget.h is part of QualificationMatrix
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

#ifndef TRAINDATAWIDGET_H
#define TRAINDATAWIDGET_H

#include <QWidget>

#include <memory>

// Forward declarations for faster compiling.
class QSqlRelationalTableModel;

class QSqlTableModel;

class QSortFilterProxyModel;

class QProgressDialog;

namespace Ui
{
class TrainDataWidget;
}

/**
 * @brief Widget for training data to manipulate.
 * @author Christian Kr, Copyright (c) 2020
 */
class TrainDataWidget: public QWidget
{
Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     */
    explicit TrainDataWidget(QWidget *parent = nullptr);

    ~TrainDataWidget() override;

public slots:

    /**
     * @brief Update the models, cause they might have changed.
     */
    void updateData();

    /**
     * @brief Update table with filter from ui components.
     */
    void updateFilter();

    /**
     * @brief Reset the content of the filter.
     */
    void resetFilter();

    /**
     * @brief Add single entry into train data model.
     */
    void addSingleEntry();

    /**
     * @brief Delete selected train data elements.
     */
    void deleteSelected();

    /**
     * @brief Show the import csv dialog.
     */
    void importCsv();

private:
    Ui::TrainDataWidget *ui;

    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> trainDataModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;

    QSortFilterProxyModel *employeeFilterModel;
    QSortFilterProxyModel *trainFilterModel;
    QSortFilterProxyModel *trainDataStateFilterModel;

    QProgressDialog *progressDialog;
};

#endif // TRAINDATAWIDGET_H
