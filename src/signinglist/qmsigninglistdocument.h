//
// qmsigninglistdocument.cpp is part of QualificationMatrix
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

#ifndef QMSIGNINGLISTDOCUMENT_H
#define QMSIGNINGLISTDOCUMENT_H

#include <QTextDocument>
#include <QDate>

// Forward declaration.
class QString;
class QStringList;

/// Class with design for signing lists.
/// \author Christian Kr, Copyright 2020
class QMSigningListDocument: public QTextDocument
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent obeject of the widget.
    explicit QMSigningListDocument(QObject *parent = nullptr);

    /// Destructor
    ~QMSigningListDocument() override = default;

    /// Set the employees.
    /// \param employees List with employees to print.
    void setEmployees(QStringList employees) { this->employees = employees; }

    /// Set the trainer name.
    /// \param trainer Name of the trainer.
    void setTrainer(QString trainer) { this->trainer = trainer; }

    /// Set the organisation doing the training.
    /// \param organisation The organisation doing the training.
    void setOrganisationName(QString organisation) { this->organisation = organisation; }

    /// Set the name of the training.
    /// \param train The training name.
    void setTrainingName(QString train) { this->train = train; }

    /// Set the description of the training contents.
    /// \param contents The contents of the training.
    void setTrainingContents(QString contents) { this->contents = contents; }

    /// Set date of the training.
    /// \param date The date of the training.
    void setTrainingDate(QDate date) { this->date = date; }

    /// Create the document with all the content.
    void createDocument();

private:
    QStringList employees;
    QString trainer;
    QString organisation;
    QString train;
    QString contents;
    QDate date;
};


#endif //QMSIGNINGLISTDOCUMENT_H