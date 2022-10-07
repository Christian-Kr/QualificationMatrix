// qmqualiresultwidget.cpp is part of QualificationMatrix
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

#include "qmqualiresultwidget.h"
#include "ui_qmqualiresultwidget.h"
#include "qmqualiresultmodel.h"
#include "framework/dialog/qmextendedselectiondialog.h"
#include "settings/qmapplicationsettings.h"
#include "model/function/qmfunctionviewmodel.h"
#include "model/training/qmtrainingviewmodel.h"
#include "model/employee/qmemployeeviewmodel.h"
#include "model/trainingdata/qmtrainingdatastateviewmodel.h"
#include "model/employee/qmshiftviewmodel.h"
#include "qualiresultreport/qmqualiresultreportdialog.h"
#include "qualiresult/qmqualiresultrecord.h"
#include "framework/dialog/qmselectfromlistdialog.h"

#include <QSortFilterProxyModel>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QTextTable>
#include <QTextDocument>
#include <QTextCursor>
#include <QPainter>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QDesktopServices>
#include <QTemporaryFile>

QMQualiResultWidget::QMQualiResultWidget(QWidget *parent)
    : QMWinModeWidget(parent)
    , ui(new Ui::QMQualiResultWidget)
    , qualiResultFilterTRState(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // Initializing settings for table view.
    ui->tvQualiResult->horizontalHeader()->setStretchLastSection(false);
    ui->tvQualiResult->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvQualiResult->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvQualiResult->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvQualiResult->setEnabled(true);
    ui->tvQualiResult->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cbTrainResultState->setCurrentText("");

    // Connect to double click on table view.
    connect(ui->tvQualiResult, &QTableView::doubleClicked, this, &QMQualiResultWidget::onDoubleClick);

    loadSettings();
}

QMQualiResultWidget::~QMQualiResultWidget()
{
    // Save settings before deleting the object.
    saveSettings();

    delete ui;
}

void QMQualiResultWidget::onDoubleClick(QModelIndex)
{
    showCertificate();
}

void QMQualiResultWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("QualiResult/FilterSplitter", ui->splitter->saveState());
}

void QMQualiResultWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    ui->dwFilter->setVisible(settings.read("QualiResult/ShowFilter", true).toBool());
    filterVisibilityChanged();

    ui->splitter->restoreState(settings.read("QualiResult/FilterSplitter").toByteArray());
}

void QMQualiResultWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    // Create the model objects.
    qualiResultModel = std::make_unique<QMQualiResultModel>();
    connect(qualiResultModel.get(), &QMQualiResultModel::beforeUpdateQualiInfo, this, &QMWinModeWidget::startWorkload);
    connect(qualiResultModel.get(), &QMQualiResultModel::updateUpdateQualiInfo, this, &QMWinModeWidget::updateWorkload);
    connect(qualiResultModel.get(), &QMQualiResultModel::afterUpdateQualiInfo, this, &QMWinModeWidget::endWorkload);

    funcViewModel = std::make_unique<QMFunctionViewModel>(this, db);
    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainDataStateViewModel = std::make_unique<QMTrainingDataStateViewModel>(this, db);
    employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    employeeGroupViewModel = std::make_unique<QMShiftViewModel>(this, db);

    // Set models to different ui controls.
    qualiResultFilterTRState->setSourceModel(qualiResultModel.get());
    qualiResultFilterTRState->setFilterKeyColumn(7);

    // Update the views.
    ui->tvQualiResult->setModel(qualiResultFilterTRState);

    // Update all table views with the new models.
    ui->cbFilterFunc->setModel(funcViewModel.get());
    ui->cbFilterFunc->setModelColumn(1);

    ui->cbFilterTrain->setModel(trainViewModel.get());
    ui->cbFilterTrain->setModelColumn(1);

    ui->cbFilterEmployeeGroup->setModel(employeeGroupViewModel.get());
    ui->cbFilterEmployeeGroup->setModelColumn(1);

    ui->cbFilterEmployee->setModel(employeeViewModel.get());
    ui->cbFilterEmployee->setModelColumn(1);

    resetFilter();
}

void QMQualiResultWidget::resetFilter()
{
    ui->cbFilterFunc->setCurrentText("");
    ui->cbFilterTrain->setCurrentText("");
    ui->cbFilterEmployee->setCurrentText("");
    ui->cbFilterEmployeeGroup->setCurrentText("");
}

[[maybe_unused]] void QMQualiResultWidget::resetModel()
{
    qualiResultModel->resetModel();
}

[[maybe_unused]] void QMQualiResultWidget::updateFilterAndCalculate()
{
    if (qualiResultModel == nullptr)
    {
        return;
    }

    QString tmpFunc = ui->cbFilterFunc->currentText();
    QString tmpTrain = ui->cbFilterTrain->currentText();
    QString tmpEmployee = ui->cbFilterEmployee->currentText();
    QString tmpEmployeeGroup = ui->cbFilterEmployeeGroup->currentText();

    ui->tvQualiResult->clearSelection();
    qualiResultModel->updateQualiInfo(tmpEmployee, tmpFunc, tmpTrain, tmpEmployeeGroup,
            ui->cbShowTempDeactEmployees->isChecked(), ui->cbShowPersonnelLeasings->isChecked(),
            ui->cbShowTrainees->isChecked(), ui->cbShowApprentices->isChecked());
    qualiResultFilterTRState->setFilterRegularExpression(ui->cbTrainResultState->currentText());
    ui->tvQualiResult->resizeColumnsToContents();
}

[[maybe_unused]] void QMQualiResultWidget::switchFilterVisibility()
{
    ui->dwFilter->setVisible(!ui->dwFilter->isVisible());

    auto &settings = QMApplicationSettings::getInstance();

    settings.write("QualiResult/ShowFilter", ui->dwFilter->isVisible());

}

void QMQualiResultWidget::filterVisibilityChanged()
{
    if (ui->dwFilter->isVisible())
    {
        ui->tbFilterVisible->setText(tr("Filter ausblenden"));
    }
    else
    {
        ui->tbFilterVisible->setText(tr("Filter einblenden"));
    }
}

[[maybe_unused]] void QMQualiResultWidget::saveToCsv()
{
    // Ask where to save the csv certificate.
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Qualifizierungsresultat speichern"), QDir::homePath(),
        tr("Comma-separated values (*.csv)"));
    if (fileName.isEmpty())
    {
        return;
    }

    // Open certificate for writing and write the whole content to the certificate.
    QFile csvFile(fileName);

    if (!csvFile.open(QFile::ReadWrite))
    {
        QMessageBox::critical(
            this, tr("Qualifizierungsresultat speichern"), tr(
                "Datei konnte nicht zum Schreiben geöffnet werden. Bitte prüfen Sie die"
                " Berechtigungen."
                "\n\nAktion wird abgebrochen."
            ));
        return;
    }

    QTextStream csvStream(&csvFile);

    int colCount = qualiResultFilterTRState->columnCount();
    int rowCount = qualiResultFilterTRState->rowCount();

    // Write header.
    for (int i = 0; i < colCount; i++)
    {
        csvStream << qualiResultFilterTRState->headerData(i, Qt::Horizontal).toString();
        if (i < colCount - 1)
        {
            csvStream << ";";
        }
        else
        {
            csvStream << "\n";
        }
    }

    // Write data.
    for (int i = 0; i < rowCount; i++)
    {
        for (int j = 0; j < colCount; j++)
        {
            csvStream << qualiResultFilterTRState->data(qualiResultFilterTRState->index(i, j)).toString();
            if (j < colCount - 1)
            {
                csvStream << ";";
            }
            else
            {
                csvStream << "\n";
            }
        }
    }

    csvFile.close();

    QMessageBox::information(
        this, tr("Qualifizierungsresultat speichern"),
        tr("Die Daten wurden erfolgreich gespeichert."));
}

[[maybe_unused]] void QMQualiResultWidget::printToPDF()
{
    // Set up default printer.
    auto *printer = new QPrinter();
    printer->setPageOrientation(QPageLayout::Orientation::Landscape);

    // Call printer dialog.
    QPrintPreviewDialog previewDialog(printer, this);
    connect(
        &previewDialog, &QPrintPreviewDialog::paintRequested, this,
        &QMQualiResultWidget::paintPdfRequest
    );
    previewDialog.exec();
}

void QMQualiResultWidget::paintPdfRequest(QPrinter *printer)
{
    QTextDocument document;
    QTextCursor cursor(&document);
    QSortFilterProxyModel *model = qualiResultFilterTRState;

    // Set default font.
    QFont font;
    font.setPointSize(8);
    document.setDefaultFont(font);

    // Create new table object and format it.
    QTextTable *table = cursor.insertTable(model->rowCount() + 1, model->columnCount() + 1);
    QTextTableFormat tableFormat = table->format();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    tableFormat.setBorderCollapse(true);
#endif

    tableFormat.setCellPadding(2);
    tableFormat.setHeaderRowCount(1);
    table->setFormat(tableFormat);

    // Go through all header cells and set them.
    for (int i = 0; i < model->columnCount() + 1; i++)
    {
        QTextTableCell cell = table->cellAt(0, i);

        // Set background of header.
        QTextCharFormat format = cell.format();
        format.setBackground(QColor("#d9d9d9"));
        cell.setFormat(format);

        if (i > 0)
        {
            cursor.insertText(model->headerData(i - 1, Qt::Horizontal).toString());
        }

        cursor.movePosition(QTextCursor::NextCell);
    }

    auto &settings = QMApplicationSettings::getInstance();

    // Put data of the model into the table and style them.
    for (int i = 0; i < model->rowCount(); i++)
    {
        for (int j = 0; j < model->columnCount() + 1; j++)
        {
            QTextTableCell cell = table->cellAt(i + 1, j);

            // Set styles. The background of every second row should be light grey.
            QTextCharFormat format = cell.format();

            if ((i + 1) % 2 == 0)
            {
                format.setBackground(QColor("#f2f2f2"));
            }

            // For better readability fill the whole row with state color.
            auto state = model->data(model->index(i, 8 - 1)).toString();
            if (state == "Schlecht")
            {
                format.setBackground(QColor(settings.read("QualiResult/BadColor", "#ffffff").toString()));
            }
            else if (state == "Gut")
            {
                format.setBackground(QColor(settings.read("QualiResult/OkColor", "#ffffff").toString()));
            }

            cell.setFormat(format);

            // First number column.
            if (j == 0)
            {
                cursor.insertText(QString::number(i + 1));
            }
            else
            {
                cursor.insertText(model->data(model->index(i, j - 1)).toString());
            }
            cursor.movePosition(QTextCursor::NextCell);
        }
    }

    // Default printer settings.
    document.print(printer);
}

[[maybe_unused]] void QMQualiResultWidget::resetFunc()
{
    ui->cbFilterFunc->clearEditText();
}

[[maybe_unused]] void QMQualiResultWidget::resetTrain()
{
    ui->cbFilterTrain->clearEditText();
}

[[maybe_unused]] void QMQualiResultWidget::resetTrainResultState()
{
    ui->cbTrainResultState->clearEditText();
}

[[maybe_unused]] void QMQualiResultWidget::resetEmployee()
{
    ui->cbFilterEmployee->clearEditText();
}

[[maybe_unused]] void QMQualiResultWidget::resetEmployeeGroup()
{
    ui->cbFilterEmployeeGroup->clearEditText();
}

[[maybe_unused]] void QMQualiResultWidget::extSelEmployee()
{
    QMExtendedSelectionDialog extSelDialog(this, employeeViewModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.isEmpty() || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbFilterEmployee->setCurrentText(extSelDialog.getSelectedElements().join(";"));
}

[[maybe_unused]] void QMQualiResultWidget::extSelTrain()
{
    QMExtendedSelectionDialog extSelDialog(this, trainViewModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.isEmpty() || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbFilterTrain->setCurrentText(extSelDialog.getSelectedElements().join(";"));
}

[[maybe_unused]] void QMQualiResultWidget::extSelFunc()
{
    QMExtendedSelectionDialog extSelDialog(this, funcViewModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.isEmpty() || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbFilterFunc->setCurrentText(extSelDialog.getSelectedElements().join(";"));
}

[[maybe_unused]] void QMQualiResultWidget::extSelEmployeeGroup()
{
    QMExtendedSelectionDialog extSelDialog(this, employeeGroupViewModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.isEmpty() || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbFilterEmployeeGroup->setCurrentText(extSelDialog.getSelectedElements().join(";"));
}

[[maybe_unused]] void QMQualiResultWidget::extSelTrainResultState()
{
    QMExtendedSelectionDialog extSelDialog(this, ui->cbTrainResultState->model(), 0);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.isEmpty() || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbTrainResultState->setCurrentText(extSelDialog.getRegExpText());
}

[[maybe_unused]] void QMQualiResultWidget::showCreateReportDialog()
{
    QMQualiResultReportDialog createReportDialog(this);
    createReportDialog.setModal(true);
    createReportDialog.exec();
}

void QMQualiResultWidget::showCertificate()
{
    auto selIndex = ui->tvQualiResult->selectionModel()->currentIndex();

    if (!selIndex.isValid())
    {
        QMessageBox::information(this, tr("Nachweis"), tr("Kein Eintrag ausgewählt."));
        return;
    }

    auto trainDate = qualiResultFilterTRState->data(qualiResultFilterTRState->index(selIndex.row(), 5),
            Qt::DisplayRole).toString();

    if (trainDate.isEmpty())
    {
        QMessageBox::information(this, tr("Nachweis"), tr("Keine durchgeführte Schulung vorhanden."));
        return;
    }

    auto name = qualiResultFilterTRState->data(
            qualiResultFilterTRState->index(selIndex.row(), 0), Qt::DisplayRole).toString();
    auto training = qualiResultFilterTRState->data(
            qualiResultFilterTRState->index(selIndex.row(), 2), Qt::DisplayRole).toString();

    auto db = QSqlDatabase::database("default");

    QString strCertQuery =
        "SELECT "
        "   Certificate.path as cert_path, "
        "   Certificate.md5_hash as cert_hash, "
        "   Certificate.type as cert_type, "
        "   Certificate.name as cert_name, "
        "   TrainDataCertificate.train_data as train_data_id, "
        "   TrainData.date as train_data_date, "
        "   Train.name as train_name, "
        "   Employee.name as employee_name "
        "FROM "
        "   Certificate, TrainDataCertificate, TrainData, Train, Employee "
        "WHERE "
        "   TrainData.employee = Employee.id and "
        "   TrainDataCertificate.certificate = Certificate.id and "
        "   TrainData.train = Train.id and "
        "   TrainDataCertificate.train_data = TrainData.id and "
        "   train_name = '" + training + "' and "
        "   employee_name = '" + name + "' and "
        "   train_data_date = '" + trainDate + "'";

    QSqlQuery query(strCertQuery, db);

    // Read in all certificate rows.
    struct CertData
    {
        QString path;
        QString hash;
        QString type;
        QString name;
    };

    QList<CertData> certList;

    while (query.next())
    {
        CertData tmpCert;

        tmpCert.path = query.record().value("cert_path").toString();
        tmpCert.hash = query.record().value("cert_hash").toString();
        tmpCert.type = query.record().value("cert_type").toString();
        tmpCert.name = query.record().value("cert_name").toString();

        certList.append(tmpCert);
    }

    query.finish();

    // If there is no certificate result, there is nothing to show.
    if (certList.size() < 1)
    {
        QMessageBox::information(this, tr("Nachweis"), tr("Kein Nachweis gefunden. Eventuell existiert ein geplanter "
                "Schulungseintrag der noch nicht durchgeführt wurde."));
        return;
    }

    // If there is more than one certificate, the user should select one certificate to show.
    int certNum = 0;
    if (certList.size() > 1)
    {
        QStringList selectionList;
        for (CertData certData : certList)
        {
            selectionList.append(certData.name);
        }

        QMSelectFromListDialog selectDialog(this, tr("Nachweis zum öffnen auswählen"), SelectionMode::SINGLE);
        selectDialog.setSelectionList(selectionList);

        if (selectDialog.exec() == QDialog::Rejected || selectDialog.getSelected().size() < 1)
        {
            return;
        }
        else
        {
            certNum = selectDialog.getSelected().first();
        }
    }

    CertData showCertData = certList.at(certNum);

    // Open the selected or found certificate file.
    QFile file(showCertData.path);

    if (!file.exists())
    {
        QMessageBox::critical(this, tr("Nachweis"),
                tr("Es gibt einen Nachweis, aber die Datei konnte nicht gefunden werden."));
        return;
    }

    file.open(QIODevice::ReadOnly);

    QTemporaryFile temp(QDir::tempPath() + QDir::separator() + showCertData.hash + "XXXXXX" + "." + 
            showCertData.type, this);
    temp.setAutoRemove(false);
    temp.open();

    temp.write(file.readAll());
    file.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(temp.fileName()));
}
