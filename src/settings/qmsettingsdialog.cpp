// qmsettings.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix. If not,
// see <http://www.gnu.org/licenses/>.

#include "qmsettingsdialog.h"
#include "ui_qmsettingsdialog.h"
#include "qmemployeesettingswidget.h"
#include "qmfuncsettingswidget.h"
#include "qmtrainsettingswidget.h"
#include "qmapplicationsettings.h"
#include "qmgeneralsettingswidget.h"
#include "qmqualimatrixsettingswidget.h"
#include "qmqualiresultsettingswidget.h"
#include "ams/qmamsusersettingswidget.h"
#include "ams/qmamsgroupsettingswidget.h"
#include "framework/delegate/qmtreesettingsdelegate.h"
#include "ams/qmamslogindialog.h"
#include "ams/qmamsmanager.h"

#include <QWidget>
#include <QSqlRelationalDelegate>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QSortFilterProxyModel>
#include <QCloseEvent>
#include <QScrollArea>
#include <QScrollBar>

QMSettingsDialog::QMSettingsDialog(QWidget *parent)
    : QMDialog(parent)
{
    ui = new Ui::QMSettingsDialog;
    ui->setupUi(this);

    changed = false;
    currentGroup = 0;
    lastGroup = 0;

    // Stack widget and tree are connected in functionality. So create both
    // before.
    initStackWidgets();
    initTreeWidgets();

    // Load general settings from settings certificate.
    loadSettings();

    // After loading all settings, signal has been called, that changed the
    // settingsChanged state so it has to be returned.
    changed = false;
    ui->pbApply->setEnabled(false);
    ui->swSettingGroups->setCurrentIndex(0);
    ui->twSettingGroups->setItemDelegate(new QMTreeSettingsDelegate(this, 25));
}

QMSettingsDialog::~QMSettingsDialog()
{
    delete ui;
}

void QMSettingsDialog::restartNeededInformation()
{
    QMessageBox::information(this, tr("Einstellung geändert"),
        tr("Die neue Einstellung erfordert einen Neustart der Anwendung."));
}

void QMSettingsDialog::initStackWidgets()
{
    appendConnectSettingsWidget(new QMGeneralSettingsWidget(this));
    appendConnectSettingsWidget(new QMQualiMatrixSettingsWidget(this));
    appendConnectSettingsWidget(new QMQualiResultSettingsWidget(this));

    // If do not have the permission, make a lot of stuff disable.
    // Permission check.
    auto amsManager = QMAMSManager::getInstance();
    if (amsManager->checkPermission(AccessMode::PER_DATA_CONFIG))
    {
        appendConnectSettingsWidget(new QMTrainSettingsWidget(this));
        appendConnectSettingsWidget(new QMFuncSettingsWidget(this));
        appendConnectSettingsWidget(new QMEmployeeSettingsWidget(this));
    }

    // Don't show permission configuration if administrator ist not logged in.
    if (amsManager->getLoginUserName().compare("administrator") == 0)
    {
        appendConnectSettingsWidget(new QMAMSUserSettingsWidget(this));
        appendConnectSettingsWidget(new QMAMSGroupSettingsWidget(this));
    }
}

void QMSettingsDialog::appendConnectSettingsWidget(
        QMSettingsWidget *settingsWidget)
{
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidget(settingsWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);

    ui->swSettingGroups->addWidget(scrollArea);
    connect(settingsWidget, &QMSettingsWidget::settingsChanged, this,
        &QMSettingsDialog::settingsChanged);
}

void QMSettingsDialog::settingsChanged()
{
    ui->pbApply->setEnabled(true);
    changed = true;
}

void QMSettingsDialog::initTreeWidgets()
{
    int i = 0;

    auto twiGeneral = new QTreeWidgetItem(ui->twSettingGroups);
    twiGeneral->setText(0, tr("Allgemein"));
    twiGeneral->setData(0, Qt::UserRole, i++);

    auto twiQualiMatrix = new QTreeWidgetItem(ui->twSettingGroups);
    twiQualiMatrix->setText(0, tr("Qualikationsmatrix"));
    twiQualiMatrix->setData(0, Qt::UserRole, i++);

    auto twiQualiResult = new QTreeWidgetItem(ui->twSettingGroups);
    twiQualiResult->setText(0, tr("Qualifizierungsergebnis"));
    twiQualiResult->setData(0, Qt::UserRole, i++);

    // If do not have the permission, make a lot of stuff disable.
    // Permission check.
    auto amsManager = QMAMSManager::getInstance();
    if (amsManager->checkPermission(AccessMode::PER_DATA_CONFIG))
    {
        auto twiDatasets = new QTreeWidgetItem(ui->twSettingGroups);
        twiDatasets->setText(0, tr("Datensätze"));

        auto twiTrainings = new QTreeWidgetItem(twiDatasets);
        twiTrainings->setText(0, tr("Schulungen"));
        twiTrainings->setData(0, Qt::UserRole, i++);

        auto twiFunctions = new QTreeWidgetItem(twiDatasets);
        twiFunctions->setText(0, tr("Funktionen"));
        twiFunctions->setData(0, Qt::UserRole, i++);

        auto twiEmployee = new QTreeWidgetItem(twiDatasets);
        twiEmployee->setText(0, tr("Mitarbeiter"));
        twiEmployee->setData(0, Qt::UserRole, i++);
    }


    // Don't show permission configuration if administrator ist not logged in.
    if (amsManager->getLoginUserName().compare("administrator") == 0)
    {
        auto twiAMS = new QTreeWidgetItem(ui->twSettingGroups);
        twiAMS->setText(0, tr("Rechtverwaltung"));

        auto twiAMSUser = new QTreeWidgetItem(twiAMS);
        twiAMSUser->setText(0, tr("Benutzer"));
        twiAMSUser->setData(0, Qt::UserRole, i++);

        auto twiAMSGroup = new QTreeWidgetItem(twiAMS);
        twiAMSGroup->setText(0, tr("Gruppen"));
        twiAMSGroup->setData(0, Qt::UserRole, i++);
    }

    ui->twSettingGroups->expandAll();
}

void QMSettingsDialog::modelDataChanged(
    const QModelIndex &, const QModelIndex &, const QVector<int> &)
{
    changed = true;
    ui->pbApply->setEnabled(true);
}

void QMSettingsDialog::accept()
{
    if (changed)
    {
        auto res = QMessageBox::question(
            this, tr("Einstellungen geändert"),
            tr("Die Einstellungen wurden geändert, sollen sie gespeichert "
                "werden?"),
            QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::Yes)
        {
            apply();
        }
        else
        {
            // The settings of the current dialog have to be reset, if they are
            // a database value, cause changes to database are manually
            // temporary. Other setting widgets not writing to the database
            // are not that important, cause they will be reloaded when the
            // dialog gets recreated.
            if (!resetSettingsGroup(ui->swSettingGroups->currentIndex()))
            {
                QMessageBox::critical(
                    this, tr("Einstellungen zurücksetzen"),
                    tr("Die Einstellungen konnten nicht zurückgesetzt "
                        "werden!"));
                return;
            }
        }
    }

    close();
}

void QMSettingsDialog::reject()
{
    accept();
}

void QMSettingsDialog::apply()
{
    auto widget = ui->swSettingGroups->currentWidget();
    if (widget == nullptr)
    {
        qWarning() << "loadSettings: widget does not exist";
        return;
    }

    auto scrollArea = dynamic_cast<QScrollArea *>(widget);
    if (scrollArea == nullptr)
    {
        qWarning() << "loadSettings: dynamic_cast failed to QScrollArea";
        return;
    }

    auto settingsWidget = dynamic_cast<QMSettingsWidget *>(
            scrollArea->widget());
    if (settingsWidget == nullptr)
    {
        qWarning() << "loadSettings: dynamic_cast failed to QMSettingsWidget";
        return;
    }

    settingsWidget->saveSettings();
    changed = false;
    ui->pbApply->setEnabled(false);
}

void QMSettingsDialog::loadSettings()
{
    // Load all settings from every widget. The widgets in stacked widget
    // should all be derived from QMSettingsWidget. Therefore every object as
    // a widget from stacked widget should be able to cast into
    // QMSettingsWidget.

    auto widgetsCount = ui->swSettingGroups->count();

    for (int i = 0; i < widgetsCount; i++)
    {
        auto widget = ui->swSettingGroups->widget(i);
        if (widget == nullptr)
        {
            qWarning() << "loadSettings: widget does not exist";
            continue;
        }

        auto scrollArea = dynamic_cast<QScrollArea *>(widget);
        if (scrollArea == nullptr)
        {
            qWarning() << "loadSettings: dynamic_cast failed to QScrollArea";
            return;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(
                scrollArea->widget());
        if (settingsWidget == nullptr)
        {
            qWarning() << "loadSettings: dynamic_cast failed to "
                "QMSettingsWidget";
            continue;
        }

        settingsWidget->loadSettings();
    }
}

void QMSettingsDialog::windowLoaded()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto width = settings.read("SettingsWin/Width", 500).toInt();
    auto height = settings.read("SettingsWin/Height", 500).toInt();
    resize(width, height);

    auto parent = dynamic_cast<QWidget *>(this->parent());
    auto parentRect = parent->geometry();
    move((parentRect.width() - width) / 2 + parentRect.x(),
        (parentRect.height() - height) / 2 + parentRect.y());
}

void QMSettingsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    windowLoaded();
}

void QMSettingsDialog::saveSettings()
{
    for (int i = 0; i < ui->swSettingGroups->count(); i++)
    {
        auto widget = ui->swSettingGroups->widget(i);
        if (widget == nullptr)
        {
            qWarning() << "loadSettings: widget does not exist";
            continue;
        }

        auto scrollArea = dynamic_cast<QScrollArea *>(widget);
        if (scrollArea == nullptr)
        {
            qWarning() << "loadSettings: dynamic_cast failed to QScrollArea";
            return;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(
                scrollArea->widget());
        if (settingsWidget == nullptr)
        {
            qWarning() << "loadSettings: dynamic_cast failed to "
                "QMSettingsWidget";
            continue;
        }

        settingsWidget->saveSettings();
    }

    changed = false;
    ui->pbApply->setEnabled(false);
}

void QMSettingsDialog::changeSettingsGroup(QTreeWidgetItem *item, const int)
{
    // See if any setting has changed and save them if the user wants to.

    if (changed)
    {
        QMessageBox::StandardButton res = QMessageBox::question(
            this, tr("Einstellungen geändert"),
            tr("Die Einstellungen wurden geändert, sollen sie gespeichert "
                "werden?"),
            QMessageBox::Yes | QMessageBox::No);

        auto widget = ui->swSettingGroups->currentWidget();
        if (widget == nullptr)
        {
            qWarning() << "changeSettingsGroup: widget does not exist";
            return;
        }

        auto scrollArea = dynamic_cast<QScrollArea *>(widget);
        if (scrollArea == nullptr)
        {
            qWarning() << "changeSettingsGroup: dynamic_cast failed to "
                "QScrollArea";
            return;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(
                scrollArea->widget());
        if (settingsWidget == nullptr)
        {
            qWarning() << "changeSettingsGroup: dynamic_cast failed to "
                "QMSettingsWidget";
            return;
        }

        switch (res)
        {
            case QMessageBox::Yes:
                settingsWidget->saveSettings();
                break;
            case QMessageBox::No:
                settingsWidget->revertChanges();
                break;
            default:
                qWarning() << "changeSettingsGroup: unknown dialog button "
                    "pressed";
                break;
        }

        changed = false;
        ui->pbApply->setEnabled(false);
    }

    // Make the new settings group visible.
    if (item->data(0, Qt::UserRole).isValid())
    {
        lastGroup = ui->swSettingGroups->currentIndex();
        ui->swSettingGroups->setCurrentIndex(
                item->data(0, Qt::UserRole).toInt());
        currentGroup = ui->swSettingGroups->currentIndex();

        auto widget = ui->swSettingGroups->currentWidget();
        if (widget == nullptr)
        {
            qWarning() << "changeSettingsGroup: widget does not exist";
            return;
        }

        auto scrollArea = dynamic_cast<QScrollArea *>(widget);
        if (scrollArea == nullptr)
        {
            qWarning() << "changeSettingsGroup: dynamic_cast failed to "
                "QScrollArea";
            return;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(
                scrollArea->widget());
        if (settingsWidget == nullptr)
        {
            qWarning() << "changeSettingsGroup: dynamic_cast failed to "
                "QMSettingsWidget";
            return;
        }

        auto am = QMAMSManager::getInstance();
        if (settingsWidget->adminAccessNeeded() &&
            am->getLoginUserName().compare("administrator") != 0)
        {
            QMAMSLoginDialog loginDialog(this);
            loginDialog.setUsername("administrator");
            loginDialog.exec();

            settingsWidget->show();
        }
    }
}

void QMSettingsDialog::closeEvent(QCloseEvent *event)
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("SettingsWin/Width", width());
    settings.write("SettingsWin/Height", height());

    event->accept();
}

bool QMSettingsDialog::resetSettingsGroup(const int group)
{
    // If normal widget, reload settings.
    if (group >= 0 & group < 3)
    {
        loadSettings();
        return true;
    }

    auto widget = ui->swSettingGroups->widget(group);
    if (widget == nullptr)
    {
        qWarning() << "changeSettingsGroup: widget does not exist";
        return false;
    }

    auto scrollArea = dynamic_cast<QScrollArea *>(widget);
    if (scrollArea == nullptr)
    {
        qWarning() << "loadSettings: dynamic_cast failed to QScrollArea";
        return false;
    }

    auto settingsWidget = dynamic_cast<QMSettingsWidget *>(
            scrollArea->widget());
    if (settingsWidget == nullptr)
    {
        qWarning() << "changeSettingsGroup: dynamic_cast failed to "
            "QMSettingsWidget";
        return false;
    }

    // Ask if you really want to revert changes.
    auto res = QMessageBox::question(
        this, tr("Zurücksetzen"),
        tr("Eventuelle Änderungen an der Datenbank werden jetzt zurückgesetzt."
           "\n\nSind Sie sich sicher?"), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
        settingsWidget->revertChanges();
        return true;
    }

    return false;
}
