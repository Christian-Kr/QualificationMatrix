// qmqualiresultreportdocument.cpp is part of QualificationMatrix
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

#include "qmqualiresultreportdocument.h"

#include <QTextTable>
#include <QTextLength>

QMQualiResultReportDocument::QMQualiResultReportDocument(QObject *parent)
    : QTextDocument(parent)
{}

void QMQualiResultReportDocument::createDocument()
{
    QTextCursor cursor(this);

    // Set default font.
    QFont font;
    font.setPointSize(8);
    setDefaultFont(font);

    QTextTable *headerTable = cursor.insertTable(1, 1);

    QTextTableFormat tableHeaderFormat = headerTable->format();
    tableHeaderFormat.setCellPadding(10);
    tableHeaderFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableHeaderFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    headerTable->setFormat(tableHeaderFormat);

    // Build header table.
    QTextTable *table = cursor.insertTable(4, 3);
    table->mergeCells(0, 0, 4, 2);

    QTextTableFormat tableFormat = table->format();
    tableFormat.setCellPadding(2);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    table->setFormat(tableFormat);

    // Fill header table with text.
//    cursor.insertImage(imagePath);
//    cursor.movePosition(QTextCursor::NextCell);
//    cursor.insertText(tr("Unterweisung am: %1").arg(date.toString("dd.MM.yyyy")));
//    cursor.movePosition(QTextCursor::NextCell);
//    cursor.insertText(tr("Organisation: %1").arg(organisation));
//    cursor.movePosition(QTextCursor::NextCell);
//    cursor.insertText(tr("Unterweisender: %1").arg(trainer));
//    cursor.movePosition(QTextCursor::NextCell);
//    cursor.insertText(tr("Unterschrift:"));
//    cursor.movePosition(QTextCursor::NextBlock);
//    cursor.movePosition(QTextCursor::NextBlock);

//    QTextCharFormat textHeaderFormat;
//    textHeaderFormat.setFontPointSize(16);
//    cursor.insertText(tr("Unterweisungsprotokoll"), textHeaderFormat);
//    cursor.movePosition(QTextCursor::NextBlock);
//    cursor.insertHtml("<br>");

//    // Build info table.
//    cursor.movePosition(QTextCursor::NextBlock);
//    QTextTable *infoTable = cursor.insertTable(2, 2);

//    QTextTableFormat infoTableFormat = table->format();
//    infoTableFormat.setCellPadding(2);
//    infoTableFormat.setCellSpacing(5);
//    infoTableFormat.setBorder(0.5);
//    infoTableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
//    infoTableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
//    infoTableFormat.setColumnWidthConstraints({
//        QTextLength(QTextLength::PercentageLength, 15),
//        QTextLength(QTextLength::PercentageLength, 85)});
//    infoTable->setFormat(infoTableFormat);

//    // Fill header table with text.
//    QTextCharFormat textFormat;
//    textFormat.setFontPointSize(12);
//    cursor.insertText(tr("Thema: "), textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    cursor.insertText(train, textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    textFormat.setFontPointSize(10);
//    cursor.insertText(tr("Inhalte: "), textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    cursor.insertText(contents, textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    // Format table cells.
//    QTextCharFormat format = table->cellAt(0, 0).format();
//    format.setBackground(QColor("#d9d9d9"));

//    // Build info table.
//    cursor.movePosition(QTextCursor::NextBlock);
//    cursor.insertHtml("<br><br>");
//    cursor.movePosition(QTextCursor::NextBlock);
//    QTextTable *employeeTable = cursor.insertTable(employees.size() + 1 + emptyEmployees, 4);

//    QTextTableFormat employeeTableFormat = employeeTable->format();
//    employeeTableFormat.setCellPadding(4);
//    employeeTableFormat.setCellSpacing(0);
//    employeeTableFormat.setBorder(0.5);
//    employeeTableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
//    employeeTableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
//    employeeTableFormat.setColumnWidthConstraints({
//        QTextLength(QTextLength::PercentageLength, 8),
//        QTextLength(QTextLength::PercentageLength, 57),
//        QTextLength(QTextLength::PercentageLength, 20),
//        QTextLength(QTextLength::PercentageLength, 15)});
//    employeeTable->setFormat(employeeTableFormat);

//    textFormat.setFontPointSize(10);

//    cursor.insertText(tr("Nr."), textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    cursor.insertText(tr("Name"), textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    cursor.insertText(tr("Unterschrift"), textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    cursor.insertText(tr("Datum"), textFormat);
//    cursor.movePosition(QTextCursor::NextCell);

//    // Format header cells.
//    QTextTableCell cell = employeeTable->cellAt(0, 0);

//    // Set background of header.
//    format = cell.format();
//    format.setBackground(QColor("#d9d9d9"));

//    cell.setFormat(format);
//    employeeTable->cellAt(0, 1).setFormat(format);
//    employeeTable->cellAt(0, 2).setFormat(format);
//    employeeTable->cellAt(0, 3).setFormat(format);

//    // Sort the list of employees.
//    switch (sortType)
//    {
//    case EmployeeSort::SortFirstName:
//        std::sort(employees.begin(), employees.end(),
//        [this](const QString &test1, const QString &test2) -> bool {
//            QString test1n;
//            if (test1.contains("Dr"))
//            {
//                QStringList splitted = test1.split(" ");
//                if (splitted.size() > 1)
//                {
//                    test1n = splitted.at(1);
//                }
//                else
//                {
//                    test1n = splitted.first();
//                }
//            }
//            else
//            {
//                test1n = test1;
//            }

//            QString test2n;
//            if (test2.contains("Dr."))
//            {
//                QStringList splitted = test2.split(" ");
//                if (splitted.size() > 1)
//                {
//                    test2n = splitted.at(1);
//                }
//                else
//                {
//                    test2n = splitted.first();
//                }
//            }
//            else
//            {
//                test2n = test2.split(" ").first();
//            }

//            return test1n < test2n;
//        });
//        break;
//    case EmployeeSort::SortLastName:
//        std::sort(employees.begin(), employees.end(),
//        [this](const QString &test1, const QString &test2) -> bool {
//          QString test1n = test1.split(" ").last();
//          QString test2n = test2.split(" ").last();
//          return test1n < test2n;
//        });
//        break;
//    default:
//        break;
//    }

//    // Go through all header cells and set them.
//    for (int i = 1; i < employees.size() + 1 + emptyEmployees; i++)
//    {
//        if (i > 0)
//        {
//            cursor.insertText(QString::number(i), textFormat);
//            cursor.movePosition(QTextCursor::NextCell);

//            if (i < employees.size() + 1) {
//                cursor.insertText(employees.at(i - 1), textFormat);
//            }

//            cursor.movePosition(QTextCursor::NextCell);
//            cursor.movePosition(QTextCursor::NextCell);
//            cursor.movePosition(QTextCursor::NextCell);
//        }
//    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    //employeeTableFormat.setBorderCollapse(true);
#endif
}
