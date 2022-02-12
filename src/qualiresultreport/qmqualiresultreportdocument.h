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

#ifndef QMQUALIRESULTREPORTDOCUMENT_H
#define QMQUALIRESULTREPORTDOCUMENT_H

#include <QTextDocument>

/// Class with design for quali result reports.
/// \author Christian Kr, Copyright 2022
class QMQualiResultReportDocument: public QTextDocument
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent obeject of the widget.
    explicit QMQualiResultReportDocument(QObject *parent = nullptr);

    /// Destructor
    ~QMQualiResultReportDocument() override = default;

    /// Create the document with all the content.
    void createDocument();
};


#endif // QMQUALIRESULTREPORTDOCUMENT_H
