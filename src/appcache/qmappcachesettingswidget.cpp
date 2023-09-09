// qmappcachesettingswidget.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmappcachesettingswidget.h"
#include "ui_qmappcachesettingswidget.h"
#include "settings/qmapplicationsettings.h"

#include <QFileDialog>

QMAppCacheSettingsWidget::QMAppCacheSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, false)
    , m_ui(new Ui::QMAppCacheSettingsWidget)
{
    m_ui->setupUi(this);
}

QMAppCacheSettingsWidget::~QMAppCacheSettingsWidget()
{
    delete m_ui;
}

void QMAppCacheSettingsWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("AppCache/Active", m_ui->gbAppCache->isChecked());

    settings.write("AppCache/SourcePath", m_ui->leSourcePath->text());
    settings.write("AppCache/TargetPath", m_ui->leTargetPath->text());

    emit settingsApplied();
}

void QMAppCacheSettingsWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto appCacheActive = settings.read("AppCache/Active", false).toBool();
    m_ui->gbAppCache->setChecked(appCacheActive);

    auto sourcePath = settings.read("AppCache/SourcePath", "").toString();
    m_ui->leSourcePath->setText(sourcePath);

    auto targetPath = settings.read("AppCache/TargetPath", "").toString();
    m_ui->leTargetPath->setText(targetPath);
}

void QMAppCacheSettingsWidget::chooseSourceFolder()
{
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::FileMode::Directory);
    fileDialog.setWindowTitle(tr("Quellordner öffnen"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setDirectory(QDir::home());

    auto result = fileDialog.exec();

    if (result != QFileDialog::Accepted)
    {
        return;
    }

    auto selectedFiles = fileDialog.selectedFiles();

    Q_ASSERT(selectedFiles.count() == 1);

    m_ui->leSourcePath->setText(selectedFiles.first());
    emit settingsChanged();
}

void QMAppCacheSettingsWidget::chooseTargetFolder()
{
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::FileMode::Directory);
    fileDialog.setWindowTitle(tr("Zielordner öffnen"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setDirectory(QDir::home());

    auto result = fileDialog.exec();

    if (result != QFileDialog::Accepted)
    {
        return;
    }

    auto selectedFiles = fileDialog.selectedFiles();

    Q_ASSERT(selectedFiles.count() == 1);

    m_ui->leTargetPath->setText(selectedFiles.first());
    emit settingsChanged();
}

void QMAppCacheSettingsWidget::toggleAppCacheGroup()
{
    emit settingsChanged();
}
