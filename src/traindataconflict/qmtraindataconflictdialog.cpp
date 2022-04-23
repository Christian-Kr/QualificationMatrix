// qmtraindataconflictdialog.cpp is part of QualificationMatrix
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

#include "qmtraindataconflictdialog.h"
#include "ui_qmtraindataconflictdialog.h"
#include "settings/qmapplicationsettings.h"

QMTrainDataConflictDialog::QMTrainDataConflictDialog(QWidget *parent)
    : QMDialog(parent)
{
    ui = new Ui::QMTrainDataConflictDialog;
    ui->setupUi(this);

    auto &settings = QMApplicationSettings::getInstance();
}

QMTrainDataConflictDialog::~QMTrainDataConflictDialog()
{
    delete ui;
}

void QMTrainDataConflictDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();
}

void QMTrainDataConflictDialog::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();
}
