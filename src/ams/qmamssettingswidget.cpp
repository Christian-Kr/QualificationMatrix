// qmamssettingswidget.cpp is part of QualificationMatrix
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

#include "qmamssettingswidget.h"
#include "ui_qmamssettingswidget.h"
#include "ams/model/qmamsusermodel.h"
#include "ams/model/qmamsgroupmodel.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QDebug>

QMAMSSettingsWidget::QMAMSSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, true)
    , ui(new Ui::QMAMSSettingsWidget)
{
    ui->setupUi(this);
}

QMAMSSettingsWidget::~QMAMSSettingsWidget()
{
    delete ui;
}

void QMAMSSettingsWidget::saveSettings()
{
    // TODO: Save settings.
}

void QMAMSSettingsWidget::revertChanges()
{
    // TODO: Revert all changes and load last state.
}

void QMAMSSettingsWidget::loadSettings()
{
    updateData();
}

void QMAMSSettingsWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    amsUserModel = std::make_unique<QMAMSUserModel>(this, db);
    amsUserModel->select();

    amsGroupModel = std::make_unique<QMAMSGroupModel>(this, db);
    amsGroupModel->select();

    ui->tvUser->setModel(amsUserModel.get());
    ui->tvUser->hideColumn(0);
    ui->tvUser->hideColumn(3);
    ui->tvUser->resizeColumnsToContents();

    ui->lvGroup->setModel(amsGroupModel.get());
}

void QMAMSSettingsWidget::addUser()
{
    // TODO: Implement

    auto record = amsUserModel->record();
    record.setValue("name", tr("Vollständiger Name"));
    record.setValue("username", tr("Benutzername"));

    // TODO: Get password and add it.

    if (!amsUserModel->insertRecord(-1, record) | !amsUserModel->submitAll())
    {
        qWarning() << "could not save a new record";
    }
}

void QMAMSSettingsWidget::addGroup()
{
    // TODO: Implement
}

void QMAMSSettingsWidget::removeUser()
{
    // TODO: Implement
}

void QMAMSSettingsWidget::removeGroup()
{
    // TODO: Implement
}

void QMAMSSettingsWidget::configUser()
{
    // TODO: Implement
}

void QMAMSSettingsWidget::configGroup()
{
    // TODO: Implement
}