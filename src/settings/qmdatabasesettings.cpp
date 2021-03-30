// qmdatabasesettings.cpp is part of QualificationMatrix
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

#include "qmdatabasesettings.h"
#include "model/qminfomodel.h"

#include <QVariant>

QMDatabaseSettings::QMDatabaseSettings(std::shared_ptr<QMInfoModel> model, QObject *parent)
    : QMSettings(parent)
{
    infoModel = model;
}

QVariant QMDatabaseSettings::read(const QString &name)
{
    int row = getNameRow(name);
    if (row == -1) {
        return QVariant();
    }

    return infoModel->data(infoModel->index(row, 2));
}

QVariant QMDatabaseSettings::read(const QString &name, const QVariant &defValue)
{
    QVariant value = read(name);
    if (value.isNull()) {
        return defValue;
    }
    else {
        return value;
    }
}

void QMDatabaseSettings::write(const QString &name, const QVariant &value)
{
    int row = getNameRow(name);
    if (row == -1) {
        return;
    }

    infoModel->setData(infoModel->index(row, 2), value);
    infoModel->submitAll();
}

int QMDatabaseSettings::getNameRow(const QString &name)
{
    for (int i = 0; i < infoModel->rowCount(); i++) {
        QString tmpName = infoModel->data(infoModel->index(i, 1)).toString();
        if (tmpName == name) {
            return i;
        }
    }

    return -1;
}
