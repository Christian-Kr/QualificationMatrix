//
// qmcertificateintegritycheckdialog.cpp is part of QualificationMatrix
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

#include "qmcertificateintegritycheckdialog.h"
#include "ui_qmcertificateintegritycheckdialog.h"
#include "model/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"

#include <QSqlTableModel>
#include <QKeyEvent>

#include <QDebug>

QMCertificateIntegrityCheckDialog::QMCertificateIntegrityCheckDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMCertificateIntegrityCheckDialog;
    ui->setupUi(this);

    updateData();
}

QMCertificateIntegrityCheckDialog::~QMCertificateIntegrityCheckDialog()
{
    delete ui;
}

void QMCertificateIntegrityCheckDialog::accept()
{
    // TODO: Run check.

    QDialog::accept();
}

void QMCertificateIntegrityCheckDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMCertificateIntegrityCheckDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("CertificateIntegrityCheckDialog/Width", width());
    settings.write("CertificateIntegrityCheckDialog/Height", height());
}

void QMCertificateIntegrityCheckDialog::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    certificateModel = dm->getCertificateModel();
}

void QMCertificateIntegrityCheckDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        return;
    }

    QDialog::keyPressEvent(event);
}
