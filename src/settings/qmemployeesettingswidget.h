// qmemployeesettingswidget.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEESETTINGSWIDGET_H
#define QMEMPLOYEESETTINGSWIDGET_H

#include "qmsettingswidget.h"

#include <memory>
#include "framework/qmsqlrelationaltablemodel.h"

// Forward declaration for faster compiling.
class QMSqlRelationalTableModel;
class QSortFilterProxyModel;
class QSqlRelationalTableModel;
class QSqlTableModel;

namespace Ui
{
class QMEmployeeSettingsWidget;
}

/**
 * @brief Shows a configuration widget for employee data.
 * @author Christian Kr, Copyright 2020
 */
class QMEmployeeSettingsWidget: public QMSettingsWidget
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent obejct for the qt system.
    explicit QMEmployeeSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMEmployeeSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this function, it
    /// should take a QSettings object or data Model (bspw. SqlModel) to save every changes to a
    /// certificate based system.
    void saveSettings() override;

    /// Override from QMSettingsWidget.
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

    /// Update table view like setting hidden columns.
    void updateTableView();

public slots:
    /// Hide or show employees that are deactivated.
    /// \param state State of checkbox
    void switchDeactivatedVisibility(bool state);

    /// Deactivates the selected employee.
    void deactivate();

    /// Add a new employee.
    void addEmployee();

    /// Add a new employee group.
    void addEmployeeGroup();

    /// Remove a selected employee group.
    void removeEmployeeGroup();

    /// Updates all data that exist are needed.
    void updateData();

    /// Update all filter.
    void updateFilter();

    /// Reset all filter.
    void resetFilter();

    /// Show details of selected employees.
    void showEmployeeDetails();

private:
    Ui::QMEmployeeSettingsWidget *ui;

    std::shared_ptr<QMSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlTableModel> shiftModel;

    QSortFilterProxyModel *employeeFilterModel;
    QSortFilterProxyModel *employeeActivatedFilterModel;
};

#endif // QMEMPLOYEESETTINGSWIDGET_H
