// qmgeneralsettingswidget.cpp is part of QualificationMatrix
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

#include "qmgeneralsettingswidget.h"
#include "ui_qmgeneralsettingswidget.h"
#include "qmapplicationsettings.h"

#include <QDir>
#include <QStyleFactory>
#include <QFileDialog>

QMGeneralSettingsWidget::QMGeneralSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, false),
    ui(new Ui::QMGeneralSettingsWidget)
{
    ui->setupUi(this);

    initLanguageFiles();
    initStyles();
}

QMGeneralSettingsWidget::~QMGeneralSettingsWidget()
{
    delete ui;
}

void QMGeneralSettingsWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto lang = settings.read("General/Lang", "de").toString();
    for (int i = 0; i < ui->cbLanguage->count(); i++)
    {
        if (ui->cbLanguage->itemText(i) == lang)
        {
            ui->cbLanguage->setCurrentIndex(i);
            break;
        }
    }

    auto theme = settings.read("General/Style", "Fusion").toString();
    for (int i = 0; i < ui->cbStyle->count(); i++)
    {
        if (ui->cbStyle->itemText(i) == theme)
        {
            ui->cbStyle->setCurrentIndex(i);
            break;
        }
    }

    ui->gbCentralizedSettings->setChecked(settings.read("General/Centralized", false).toBool());
    ui->leCentralizedSettingsFile->setText(settings.read("General/CentralizedPath", "").toString());

    auto loadLastDatabase = settings.read("Database/LoadLastOnStartup", true).toBool();
    ui->cbLoadLastDatabase->setChecked(loadLastDatabase);

    ui->cbAutoLogin->setChecked(settings.read("AMS/AutoLogin", false).toBool());
    ui->cbSaveLastLoginName->setChecked(settings.read("AMS/SaveLastLogin", false).toBool());
}

void QMGeneralSettingsWidget::initLanguageFiles()
{
    // Get a list of all language files. For now, files will only be searched in path, where the
    // executable exist. The format of the certificate must be qm_<lang>.qm.

    QDir langDir("");
    QStringList filters;
    filters << "qm_*.qm";

    langDir.setNameFilters(filters);
    langDir.setSorting(QDir::Name);

    auto langFileList = langDir.entryInfoList();
    ui->cbLanguage->clear();

    for (const auto &i : langFileList)
    {
        QString langFileName = i.baseName().split("_").at(1);
        ui->cbLanguage->addItem(langFileName);
    }
}

void QMGeneralSettingsWidget::initStyles()
{
    auto styles = QStyleFactory::keys();
    ui->cbStyle->addItems(styles);
}

void QMGeneralSettingsWidget::chooseCentralizedSettingsFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Zentrale Einstellungsdatei öffnen"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        ui->leCentralizedSettingsFile->setText(fileName);
    }

    emitSettingsChanged();
}

void QMGeneralSettingsWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("General/CentralizedPath", ui->leCentralizedSettingsFile->text());
    settings.write("General/Centralized", ui->gbCentralizedSettings->isChecked());

    settings.write("Database/LoadLastOnStartup", ui->cbLoadLastDatabase->isChecked());

    settings.write("General/Lang", ui->cbLanguage->currentText());
    settings.write("General/Style", ui->cbStyle->currentText());

    settings.write("AMS/AutoLogin", ui->cbAutoLogin->isChecked());
    settings.write("AMS/SaveLastLogin", ui->cbSaveLastLoginName->isChecked());
}

void QMGeneralSettingsWidget::revertChanges()
{
    loadSettings();
}
