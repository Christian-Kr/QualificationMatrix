//
// qmcertificatedialog.cpp is part of QualificationMatrix
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

#include "qmnewcertificatedialog.h"
#include "ui_qmnewcertificatedialog.h"
#include "model/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"

#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDate>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QTemporaryFile>

#include <QDebug>

QMNewCertificateDialog::QMNewCertificateDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMNewCertificateDialog;
    ui->setupUi(this);
}

QMNewCertificateDialog::~QMNewCertificateDialog()
{
    delete ui;
}

void QMNewCertificateDialog::accept()
{
    QDialog::accept();
}

void QMNewCertificateDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMNewCertificateDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("NewCertificateDialog/Width", width());
    settings.write("NewCertificateDialog/Height", height());
}

void QMNewCertificateDialog::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();
    certificateModel = dm->getCertificateModel();
    trainDataCertificateModel = dm->getTrainDataCertificateModel();
}

void QMNewCertificateDialog::openCertificatePath()
{
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Nachweis hinzufügen"), QDir::homePath(),
        tr("All files (*.*);;JPEG (*.jpg *.jpeg);;PDF (*.pdf)" ));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isReadable() || !file.exists())
    {
        qWarning() << "certificate file does not exist or is not readable" << fileName;
        return;
    }


}

void QMNewCertificateDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        return;
    }

    QDialog::keyPressEvent(event);
}
