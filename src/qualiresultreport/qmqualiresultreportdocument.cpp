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
#include "ams/qmamsmanager.h"
#include "qmqualiresultreportitem.h"
#include "settings/qmapplicationsettings.h"

#include <QTextTable>
#include <QTextLength>
#include <QDate>

QMQualiResultReportDocument::QMQualiResultReportDocument(QObject *parent)
    : QTextDocument(parent)
{}

void QMQualiResultReportDocument::createDocument(QList<QMQualiResultReportItem> &resultItems, QDate date)
{
    // TODO: Keine Pflichtschulungen wenn niemand eine Schulung braucht.

    QTextCursor cursor(this);

    // Set default font.
    QFont font;
    font.setPointSize(8);
    setDefaultFont(font);

    QTextTable *headerTable = cursor.insertTable(1, 1);

    QTextTableFormat tableHeaderFormat = headerTable->format();
    tableHeaderFormat.setCellPadding(0);
    tableHeaderFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableHeaderFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    headerTable->setFormat(tableHeaderFormat);

    QTextCharFormat textFormat;
    textFormat.setFontPointSize(10);
    textFormat.setFontWeight(QFont::Bold);
    cursor.insertText(tr("Schulungsbericht"), textFormat);
    cursor.movePosition(QTextCursor::NextBlock);

    QTextTable *headerInfoTable = cursor.insertTable(3, 2);
    tableHeaderFormat.setCellPadding(0);
    tableHeaderFormat.setWidth(QTextLength(QTextLength::PercentageLength, 80));
    headerInfoTable->setFormat(tableHeaderFormat);

    textFormat.setFontPointSize(10);
    textFormat.setFontWeight(QFont::Normal);
    cursor.insertText(tr("Ersteller:"), textFormat);
    cursor.movePosition(QTextCursor::NextCell);
    cursor.insertText(QMAMSManager::getInstance()->getLoginFullName(), textFormat);

    cursor.movePosition(QTextCursor::NextCell);
    cursor.insertText(tr("Datum - Betrachtung:"), textFormat);
    cursor.movePosition(QTextCursor::NextCell);
    cursor.insertText(date.toString("dd.MM.yyyy"), textFormat);

    cursor.movePosition(QTextCursor::NextCell);
    cursor.insertText(tr("Datum - Erstellung:"), textFormat);
    cursor.movePosition(QTextCursor::NextCell);
    cursor.insertText(QDate::currentDate().toString("dd.MM.yyyy"), textFormat);
    cursor.movePosition(QTextCursor::NextBlock);
    cursor.insertHtml("<br><br>");

    QTextTable *contentTable = cursor.insertTable((int) resultItems.size() + 1, 3);
    tableHeaderFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    contentTable->setFormat(tableHeaderFormat);

    QTextTableFormat contentTableFormat = contentTable->format();
    contentTableFormat.setCellPadding(5);
    contentTableFormat.setCellSpacing(0);
    contentTableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    contentTableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    contentTableFormat.setColumnWidthConstraints({QTextLength(QTextLength::PercentageLength, 7),
            QTextLength(QTextLength::PercentageLength, 78),
            QTextLength(QTextLength::PercentageLength, 15)});
    contentTable->setFormat(contentTableFormat);

    cursor.insertText(tr("Nr."), textFormat);
    cursor.movePosition(QTextCursor::NextCell);

    cursor.insertText(tr("Schulungsname"), textFormat);
    cursor.movePosition(QTextCursor::NextCell);

    cursor.insertText(tr("Ergebnis"), textFormat);
    cursor.movePosition(QTextCursor::NextCell);

    textFormat.setBackground(QColor("#d9d9d9"));
    contentTable->cellAt(0, 0).setFormat(textFormat);
    contentTable->cellAt(0, 1).setFormat(textFormat);
    contentTable->cellAt(0, 2).setFormat(textFormat);

    textFormat.setFontWeight(QFont::Normal);
    textFormat.setBackground(QColor(Qt::white));
    textFormat.setFontPointSize(8);

    auto &settings = QMApplicationSettings::getInstance();

    QColor okColor(settings.read("QualiResult/OkColor", "#ffffff").toString());
    QColor badColor(settings.read("QualiResult/BadColor", "#ffffff").toString());

    int num = 0;
    for (auto &resultItem : resultItems)
    {
        cursor.insertText(QString("%1").arg(++num), textFormat);
        cursor.movePosition(QTextCursor::NextCell);

        cursor.insertText(resultItem.getTrainName(), textFormat);
        cursor.movePosition(QTextCursor::NextCell);

        if (resultItem.getTrainResultTotal() > 0)
        {
            int frac = 100 * resultItem.getTrainResultOk() / resultItem.getTrainResultTotal();

            if (frac > 90)
            {
                textFormat.setBackground(okColor);
            }
            else
            {
                textFormat.setBackground(badColor);
            }

            cursor.insertText(QString("%1/%2 - %3 %").arg(resultItem.getTrainResultOk())
                    .arg(resultItem.getTrainResultTotal())
                    .arg(frac), textFormat);
        }

        cursor.movePosition(QTextCursor::NextCell);
        contentTable->cellAt(num, 2).setFormat(textFormat);
        textFormat.setBackground(QColor(Qt::white));
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    contentTableFormat.setBorderCollapse(true);
#endif
}
