// qmqualiresultreportdialog.cpp is part of QualificationMatrix
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

#include "qmqualiresultreportdialog.h"
#include "ui_qmqualiresultreportdialog.h"
#include "model/qmemployeeviewmodel.h"
#include "model/qmshiftviewmodel.h"
#include "model/qmtrainingviewmodel.h"
#include "model/qmtraininggroupviewmodel.h"
#include "settings/qmapplicationsettings.h"

QMQualiResultReportDialog::QMQualiResultReportDialog(QWidget *parent)
    : QMDialog(parent)
{
    ui = new Ui::QMQualiResultReportDialog;
    ui->setupUi(this);

    updateData();

    auto &settings = QMApplicationSettings::getInstance();
}

QMQualiResultReportDialog::~QMQualiResultReportDialog()
{
    delete ui;
}

void QMQualiResultReportDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("QualiResultReportDialog/Width", width());
    settings.write("QualiResultReportDialog/Height", height());
}

void QMQualiResultReportDialog::updateData()
{
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    employeeViewModel->select();

    shiftViewModel = std::make_unique<QMShiftViewModel>(this, db);
    shiftViewModel->select();

    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    trainGroupViewModel = std::make_unique<QMTrainingGroupViewModel>(this, db);
    trainGroupViewModel->select();

    // Set ui elements.
    ui->lvEmployees->setModel(employeeViewModel.get());
    ui->lvEmployees->setModelColumn(1);

    ui->lvTrainings->setModel(trainViewModel.get());
    ui->lvTrainings->setModelColumn(1);
}
