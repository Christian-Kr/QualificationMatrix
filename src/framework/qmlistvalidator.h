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

/// Custom validator against a list of entries.
/// \author Christian Kr, Copyright 2022
class QMListValidator : public QValidator
{
    Q_OBJECT

public:
    /// Constructor
    /// \param list validation list
    /// \param parent parent object
    explicit QMListValidator(const QStringList &list, QObject *parent = nullptr);

private:
    std::unique_ptr<QStringList> m_list;
};

#endif // QMLISTVALIDATOR_H
