// qmcombobox.h is part of QualificationMatrix
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

#ifndef QMCOMBOBOX_H
#define QMCOMBOBOX_H

#include <QComboBox>

/// Custom combobox component that has been extended to some useful functionalitites.
/// \author Christian Kr, Copyright 2022
class QMComboBox : public QComboBox
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMComboBox(QWidget *parent = nullptr);

    /// Get focus out validation state.
    /// \returns true if validation on focus out is active, else false
    [[nodiscard]] bool getFocusOutListValidation() const { return m_focusOutListValidation; }

    /// Set the focus out validation state.
    /// \param focusOutValidation true if validation on focus out should be active
    void setFocusOutListValidation(bool focusOutListValidation) {m_focusOutListValidation = focusOutListValidation; }

protected:
    /// Override from QComboBox
    /// \param e the focus event
    void focusOutEvent(QFocusEvent *e) override;

private:
    bool m_focusOutListValidation;
};

#endif // QMCOMBOBOX_H
