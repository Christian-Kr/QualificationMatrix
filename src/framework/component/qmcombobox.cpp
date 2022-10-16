// qmcombobox.cpp is part of QualificationMatrix
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

#include "qmcombobox.h"

QMComboBox::QMComboBox(QWidget *parent)
    : QComboBox(parent)
{}

void QMComboBox::focusOutEvent(QFocusEvent *e)
{
    // do validation when losing focus to be sure no bad value is in combo box
    auto index = findData(currentText(), Qt::MatchStartsWith);

    // if no element fits, try to set one
    if (index == -1)
    {
        // if there is no element to select, just empty everything
        if (count() == 0)
        {
            setCurrentText("");
            setCurrentIndex(-1);
        }
        // if there are possible elements, select the one that fits
        else
        {
            setCurrentIndex(0);
            setCurrentText(currentText());
        }
    }
    // if any element fits, this might be full or just start with
    else
    {
        setCurrentIndex(index);
    }

    // handle in parent
    QComboBox::focusInEvent(e);
}