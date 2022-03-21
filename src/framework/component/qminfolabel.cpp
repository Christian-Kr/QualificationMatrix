// qminfolabel.cpp is part of QualificationMatrix
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

#include "qminfolabel.h"

#include <QTimer>

QMInfoLabel::QMInfoLabel(QWidget *parent)
    : QLabel(parent),
    timer(new QTimer(this))
{
    /// Be sure only settings will be called, that are available during this state.
    loadSettings();

    timer->setSingleShot(true);
    timer->setInterval(3000);
    connect(timer, &QTimer::timeout, this, &QMInfoLabel::hide);

    setVisible(false);
}

void QMInfoLabel::loadSettings()
{
    // TODO: Load settings.
}

void QMInfoLabel::showWarnMessage(QString msg)
{
    timer->stop();
    timer->start();
    timer->setInterval(5000);

    // Set the style for information message.
    setStyleSheet("padding: 10px; margin-left: 10px; margin-right: 10px;"
          "background: rgba(255, 224, 179, 80%); border-style: solid; border-width: 1px; "
          "border-color: #000000;");

    if (!isVisible())
    {
        setVisible(true);
    }

    setText(msg);
}

void QMInfoLabel::showInfoMessage(QString msg)
{
    timer->stop();
    timer->start();
    timer->setInterval(3000);

    // Set the style for information message.
    setStyleSheet("padding: 10px; margin-left: 10px; margin-right: 10px;"
          "background: rgba(160, 223, 250, 80%); border-style: solid; border-width: 1px; "
          "border-color: #000000;");

    if (!isVisible())
    {
        setVisible(true);
    }

    setText(msg);
}
