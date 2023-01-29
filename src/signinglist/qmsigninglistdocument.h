// qmsigninglistdocument.cpp is part of QualificationMatrix
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

#ifndef QMSIGNINGLISTDOCUMENT_H
#define QMSIGNINGLISTDOCUMENT_H

#include <QTextDocument>
#include <QDate>
#include <QStringList>

// Forward declaration.
class QString;

/// Enum for sort types of employee list.
enum class EmployeeSort { SortNo = 0, SortFirstName = 1, SortLastName = 2 };

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

    /// Set sort type for employees.
    /// \param sortType
    void setSortType(EmployeeSort sortType) { m_sortType = sortType; }

    /// Set the employees.
    /// \param employees List with employees to print.
    void setEmployees(const QStringList &employees) { m_employees = employees; }

    /// Set the trainer name.
    /// \param trainer Name of the trainer.
    void setTrainer(const QString &trainer) { m_trainer = trainer; }

    /// Set the organisation doing the training.
    /// \param organisation The organisation doing the training.
    void setOrganisationName(const QString &organisation) { m_organisation = organisation; }

    /// Set the name of the training.
    /// \param train The training name.
    void setTrainingName(const QString &train) { m_train = train; }

    /// Set the description of the training contents.
    /// \param contents The contents of the training.
    void setTrainingContents(const QString &contents) { m_contents = contents; }

    /// Set date of the training.
    /// \param date The date of the training.
    void setTrainingDate(const QDate &date) { m_date = date; }

    /// Set path of image.
    /// \param path
    void setImagePath(const QString &path) { m_imagePath = path; }

    /// Set empty rows for unknown employees.
    /// \param emptyEmployees
    void setEmptyEmployees(int emptyEmployees) { m_emptyEmployees = emptyEmployees; }

    /// Create empty signing fields for the trainer.
    /// \param createEmptyTrainerFields
    void setCreateEmptyTrainerFields(bool createEmptyTrainerFields)
            { m_createEmptyTrainerFields = createEmptyTrainerFields; }

    /// Create the document with all the content.
    void createDocument();

    /// Add training date.
    /// \param insert true if insert training date, else false
    void setInsertTrainingDate(bool insert) { m_insertTrainingDate = insert; }

    /// Add the given number of empty trainer fields.
    /// \param numberEmptyTrainerFields the total number of empty trainer fields
    void setNumberEmptyTrainerFields(int numberEmptyTrainerFields);

private:
    int m_numberEmptyTrainerFields;
    int m_emptyEmployees;
    bool m_createEmptyTrainerFields;
    bool m_insertTrainingDate;
    EmployeeSort m_sortType;
    QStringList m_employees;
    QString m_trainer;
    QString m_organisation;
    QString m_train;
    QString m_contents;
    QDate m_date;
    QString m_imagePath;
};


#endif //QMSIGNINGLISTDOCUMENT_H
