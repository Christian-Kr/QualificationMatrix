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
    , m_focusOutListValidation(false)
{}

int QMComboBox::findData(const QString &text) const
{
    // try to get the model, if there is non, just return "not found" with -1
    auto elementModel = model();
    if (elementModel == nullptr)
    {
        return -1;
    }

    // go through every element of the model column and search for the element - not that efficient, but it works
    auto selectedColumn = modelColumn();
    auto rowCount = elementModel->rowCount();
    auto index = -1;

    for (auto i = 0; i < rowCount; i++)
    {
        auto element = elementModel->data(elementModel->index(i, selectedColumn), Qt::DisplayRole).toString();
        if (element.contains(text, Qt::CaseInsensitive)) {
            index = i;
            break;
        }
    }

    return index;
}

void QMComboBox::focusOutEvent(QFocusEvent *e)
{
    if (m_focusOutListValidation)
    {
        // do validation when losing focus to be sure no bad value is in combo box
        auto index = findData(currentText());

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
            }
        }
            // if any element fits, this might be full or just start with
        else
        {
            setCurrentIndex(index);
        }
    }

    // handle in parent
    QComboBox::focusInEvent(e);
}