// qmsettingswidget.cpp is part of QualificationMatrix
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

#include "qmsettingswidget.h"
#include "ams/qmamsmanager.h"

#include <QShowEvent>
#include <QDebug>

QMSettingsWidget::QMSettingsWidget(QWidget *parent, bool adminAccess)
    : QWidget(parent)
    , admin(adminAccess)
{}

void QMSettingsWidget::emitSettingsChanged()
{
    emit settingsChanged();
}

void QMSettingsWidget::showEvent(QShowEvent *event)
{
    if (admin)
    {
        // Test if administrator is logged in to get access to the admin interface.
        auto am = QMAMSManager::getInstance();
        if (!am->checkAdminPermission())
        {
            qDebug() << "Try to open admin widget without being logged in as an administrator";
            setVisible(false);
            return;
        }
    }

    QWidget::showEvent(event);
}
