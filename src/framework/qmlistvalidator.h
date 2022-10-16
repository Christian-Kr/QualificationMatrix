// qmlistvalidator.h is part of QualificationMatrix
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

#ifndef QMLISTVALIDATOR_H
#define QMLISTVALIDATOR_H

#include <QValidator>

#include <memory>

class QAbstractTableModel;

/// Custom validator against a list of entries.
/// \author Christian Kr, Copyright 2022
class QMListValidator : public QValidator
{
    Q_OBJECT

public:
    /// Constructor
    /// \param list validation list
    /// \param parent the parent object
    explicit QMListValidator(const QStringList &list, QObject *parent = nullptr);

    /// Contructor with model
    /// \param model the model to get the list from
    /// \param column the column number of the model to use
    /// \param parent the parent object
    explicit QMListValidator(const QAbstractTableModel &model, int column, QObject *parent = nullptr);

    /// Override from QValidator. Changing input and position will change the values for validated input field.
    /// \param input text string to validate
    /// \param pos position where the current cursor is
    QValidator::State validate(QString &input, int &pos) const override;

private:
    std::unique_ptr<QStringList> m_list;
};

#endif // QMLISTVALIDATOR_H
