// qmemployeedetailsdialog.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEEDETAILSDIALOG_H
#define QMEMPLOYEEDETAILSDIALOG_H

#include "framework/qmdialog.h"

#include <memory>
#include <QSortFilterProxyModel>

// Forward declaration for faster compiling.
class QMSqlRelationalTableModel;
class QSqlRelationalTableModel;
class QSqlTableModel;

namespace Ui
{
class QMEmployeeDetailsDialog;
}

/**
 * @brief Show details about an employee and lets change some settings.
 * @author Christian Kr, Copyright (c) 2020
 */
class QMEmployeeDetailsDialog: public QMDialog
{
Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     * @param employeeId Unique id in table. This is the primary key.
     * @param emnployeeName The name of the employee.
     * @param employeeGroup The employees' group.
     */
    explicit QMEmployeeDetailsDialog(
        QString employeeId, QString employeeName, QString employeeGroup, bool activated,
        QWidget *parent = nullptr);

    ~QMEmployeeDetailsDialog() override;

    /**
     * @brief Override from QDialog.
     */
    void accept() override;

    /**
     * @brief Override from QDialog.
     */
    void reject() override;

    /**
     * @brief Apply all changes..
     */
    void applyChanges();

    /**
     * @brief Revert all changes.
     */
    void revertChanges();

public slots:

    /**
     * @brief Add a new func.
     */
    void addFunc();

    /**
     * @brief Add a new train exception.
     */
    void addTrainException();

    /**
     * @brief Remove a selected function.
     */
    void removeFunc();

    /**
     * @brief Remove a selected train exception.
     */
    void removeTrainException();

    /**
     * @brief Updates all data that exist are needed.
     */
    void updateData();

private:
    Ui::QMEmployeeDetailsDialog *ui;

    QString id;
    QString name;
    QString group;
    bool active;

    std::shared_ptr<QMSqlRelationalTableModel> funcModel;
    std::shared_ptr<QSqlRelationalTableModel> employeeFuncModel;
    std::shared_ptr<QMSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> trainExceptionModel;
    std::shared_ptr<QSqlTableModel> shiftModel;

    QSortFilterProxyModel *employeeFilterModel;
    QSortFilterProxyModel *employeeFuncFilterModel;
    QSortFilterProxyModel *trainExceptionFilterModel;
};

#endif // QMEMPLOYEEDETAILSDIALOG_H
