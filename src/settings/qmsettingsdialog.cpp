//
// qmsettings.cpp is part of QualificationMatrix
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

#include "qmsettingsdialog.h"
#include "ui_qmsettingsdialog.h"
#include "qmemployeesettingswidget.h"
#include "qmfuncsettingswidget.h"
#include "qmtrainsettingswidget.h"
#include "qmapplicationsettings.h"
#include "qmgeneralsettingswidget.h"
#include "qmqualimatrixsettingswidget.h"
#include "qmqualiresultsettingswidget.h"

#include <QWidget>
#include <QSqlRelationalDelegate>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QCloseEvent>

QMSettingsDialog::QMSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMSettingsDialog;
    ui->setupUi(this);

    changed = false;
    currentGroup = 0;
    lastGroup = 0;

    // Stack widget and tree are connected in functionality. So create both before.
    initStackWidgets();
    initTreeWidgets();

    // Load general settings from settings certificate.
    loadSettings();

    // After loading all settings, signal has been called, that changed the settingsChanged state
    // so it has to be returned.
    changed = false;
    ui->pbApply->setEnabled(false);
    ui->swSettingGroups->setCurrentIndex(0);
}

QMSettingsDialog::~QMSettingsDialog()
{
    delete ui;
}

void QMSettingsDialog::restartNeededInformation()
{
    QMessageBox::information(
        this, tr("Einstellung geändert"),
        tr("Die neue Einstellung erfordert einen Neustart der Anwendung."));
}

void QMSettingsDialog::initStackWidgets()
{
    appendConnectSettingsWidget(new QMGeneralSettingsWidget(this));
    appendConnectSettingsWidget(new QMQualiMatrixSettingsWidget(this));
    appendConnectSettingsWidget(new QMQualiResultSettingsWidget(this));
    appendConnectSettingsWidget(new QMTrainSettingsWidget(this));
    appendConnectSettingsWidget(new QMFuncSettingsWidget(this));
    appendConnectSettingsWidget(new QMEmployeeSettingsWidget(this));
}

void QMSettingsDialog::appendConnectSettingsWidget(QMSettingsWidget *settingsWidget)
{
    ui->swSettingGroups->addWidget(settingsWidget);
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
    auto twiGeneral = new QTreeWidgetItem(ui->twSettingGroups);
    twiGeneral->setText(0, tr("Allgemein"));
    twiGeneral->setData(0, Qt::UserRole, 0);

    auto twiQualiMatrix = new QTreeWidgetItem(ui->twSettingGroups);
    twiQualiMatrix->setText(0, tr("Qualikationsmatrix"));
    twiQualiMatrix->setData(0, Qt::UserRole, 1);

    auto twiQualiResult = new QTreeWidgetItem(ui->twSettingGroups);
    twiQualiResult->setText(0, tr("Qualifizierungsergebnis"));
    twiQualiResult->setData(0, Qt::UserRole, 2);

    auto twiDatasets = new QTreeWidgetItem(ui->twSettingGroups);
    twiDatasets->setText(0, tr("Datensätze"));

    auto twiTrainings = new QTreeWidgetItem(twiDatasets);
    twiTrainings->setText(0, tr("Schulungen"));
    twiTrainings->setData(0, Qt::UserRole, 3);

    auto twiFunctions = new QTreeWidgetItem(twiDatasets);
    twiFunctions->setText(0, tr("Funktionen"));
    twiFunctions->setData(0, Qt::UserRole, 4);

    auto twiEmployee = new QTreeWidgetItem(twiDatasets);
    twiEmployee->setText(0, tr("Mitarbeiter"));
    twiEmployee->setData(0, Qt::UserRole, 5);

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
            tr("Die Einstellungen wurden geändert, sollen sie gespeichert werden?"),
            QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::Yes)
        {
            apply();
        }
        else
        {
            // The settings of the current dialog have to be reset, if they are a database value,
            // cause changes to database are manually temporary. Other setting widgets not writing
            // to the database are not that important, cause they will be reloaded when the dialog
            // gets recreated.
            if (!resetSettingsGroup(ui->swSettingGroups->currentIndex()))
            {
                QMessageBox::critical(
                    this, tr("Einstellungen zurücksetzen"),
                    tr("Die Einstellungen konnten nicht zurückgesetzt werden!"));
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

    auto settingsWidget = dynamic_cast<QMSettingsWidget *>(widget);
    if (settingsWidget == nullptr)
    {
        qWarning() << "loadSettings: dynamic_cast failed";
        return;
    }

    settingsWidget->saveSettings();
    changed = false;
    ui->pbApply->setEnabled(false);
}

void QMSettingsDialog::loadSettings()
{
    // Load all settings from every widget. The widgets in stacked widget should all be derived
    // from QMSettingsWidget. Therefore every object as a widget from stacked widget should be
    // able to cast into QMSettingsWidget.

    auto widgetsCount = ui->swSettingGroups->count();

    for (int i = 0; i < widgetsCount; i++)
    {
        auto widget = ui->swSettingGroups->widget(i);
        if (widget == nullptr)
        {
            qWarning() << "loadSettings: widget does not exist";
            continue;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(widget);
        if (settingsWidget == nullptr)
        {
            qWarning() << "loadSettings: dynamic_cast failed";
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
            qWarning() << "saveSettings: widget does not exist";
            continue;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(widget);
        if (settingsWidget == nullptr)
        {
            qWarning() << "saveSettings: dynamic_cast failed";
            continue;
        }

        settingsWidget->saveSettings();
    }

    changed = false;
    ui->pbApply->setEnabled(false);
}

bool QMSettingsDialog::saveSettingsGroup(const int group)
{
    auto &settings = QMApplicationSettings::getInstance();

    switch (group) {
        case 0:
            // general


            return true;
        case 1:


            return true;
        case 2:

            return true;
        default:
            return saveSettingsGroupDatabase(group);
    }
}

bool QMSettingsDialog::saveSettingsGroupDatabase(const int group)
{
    // Ask if you really want to save, cause we have changes to database.
    auto res = QMessageBox::question(
        this, tr("Anwenden"),
        tr("Eventuelle Änderungen an der Datenbank werden jetzt geschrieben."
           "\n\nSind Sie sich sicher?"), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
        dynamic_cast<QMSettingsWidget *>(ui->swSettingGroups->widget(group))->saveSettings();
        return true;
    }

    return false;
}

void QMSettingsDialog::changeSettingsGroup(QTreeWidgetItem *item, const int)
{
    // See if any setting has changed and save them if the user wants to.

    if (changed)
    {
        QMessageBox::StandardButton res = QMessageBox::question(
            this, tr("Einstellungen geändert"),
            tr("Die Einstellungen wurden geändert, sollen sie gespeichert werden?"),
            QMessageBox::Yes | QMessageBox::No);

        auto widget = ui->swSettingGroups->currentWidget();
        if (widget == nullptr)
        {
            qWarning() << "changeSettingsGroup: widget does not exist";
            return;
        }

        auto settingsWidget = dynamic_cast<QMSettingsWidget *>(widget);
        if (settingsWidget == nullptr)
        {
            qWarning() << "changeSettingsGroup: dynamic_cast failed";
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
                qWarning() << "changeSettingsGroup: unknown dialog button pressed";
                break;
        }

        changed = false;
        ui->pbApply->setEnabled(false);
    }

    // Make the new settings group visible.
    if (item->data(0, Qt::UserRole).isValid())
    {
        lastGroup = ui->swSettingGroups->currentIndex();
        ui->swSettingGroups->setCurrentIndex(item->data(0, Qt::UserRole).toInt());
        currentGroup = ui->swSettingGroups->currentIndex();
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

    auto settingWidget = dynamic_cast<QMSettingsWidget *>(ui->swSettingGroups->widget(group));
    if (settingWidget == nullptr)
    {
        return false;
    }

    // Ask if you really want to revert changes.
    auto res = QMessageBox::question(
        this, tr("Zurücksetzen"),
        tr("Eventuelle Änderungen an der Datenbank werden jetzt zurückgesetzt."
           "\n\nSind Sie sich sicher?"), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
        settingWidget->revertChanges();
        return true;
    }

    return false;
}
