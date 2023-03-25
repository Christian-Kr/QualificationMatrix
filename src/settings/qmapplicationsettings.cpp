// qmapplicationsettings.cpp is part of QualificationMatrix
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

#include "qmapplicationsettings.h"

#include <QSettings>
#include <QCoreApplication>

QMApplicationSettings::QMApplicationSettings(QObject *parent)
    : QMSettings(parent), centralizedAvailable(false),
    priority(QMApplicationSettings::CENTRAL), centralized(nullptr), local(nullptr)
{
    // Initialize the local.
    initLocal();

    // Initialize the centralized setting, if wanted.
    readCentralized();
    if (centralizedAvailable)
    {
        initCentralized();
    }
}

QMApplicationSettings::~QMApplicationSettings()
{
    delete local;
    delete centralized;
}

void QMApplicationSettings::initLocal()
{
    delete local;

    local = new QSettings(
        QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
        QCoreApplication::applicationName()
    );
}

void QMApplicationSettings::initCentralized()
{
    delete centralized;

    centralized = new QSettings(centralizedPath, QSettings::IniFormat);
}

void QMApplicationSettings::readCentralized()
{
    QVariant tmp;

    tmp = read("General/Centralized");
    if (!tmp.isNull())
    {
        centralizedAvailable = tmp.toBool();
    }

    tmp = read("General/CentralizedPath");
    if (!tmp.isNull())
    {
        centralizedPath = tmp.toString();
    }
}

QVariant QMApplicationSettings::read(const QString &name, const QVariant &defValue)
{
    auto tmp = read(name);
    if (tmp.isNull())
    {
        return defValue;
    }

    return tmp;
}

QVariant QMApplicationSettings::read(const QString &name)
{
    // If priority is set to CENTRAL, start with watching there, else the other way around.

    if (priority == QMApplicationSettings::CENTRAL)
    {

        // If there is a centralized config, return it.

        if (centralized != nullptr && centralizedAvailable)
        {
            QVariant tmp = centralized->value(name);
            if (!tmp.isNull())
            {
                return tmp;
            }
        }
    }
    else
    {

        // If there is a local config, return it.

        if (local != nullptr)
        {
            QVariant tmp = local->value(name);
            if (!tmp.isNull())
            {
                return tmp;
            }
        }

        // If there is no local, try to find a centralized one.

        if (centralized != nullptr && centralizedAvailable)
        {
            QVariant tmp = centralized->value(name);
            if (!tmp.isNull())
            {
                return tmp;
            }
        }
    }

    // Return the result of the local value.

    if (local != nullptr)
    {
        // If only value gets called and the setting does not exist, it will create a variant object with a default
        // value. Therefore, you cannot just ask with function value(...) to know whether the setting exist or not.

        if (!local->contains(name))
        {
            return {};
        }
        else
        {
            return local->value(name);
        }
    }

    return {};
}

void QMApplicationSettings::write(const QString &name, const QVariant &value)
{
    // Writing settings should never be done for centralized ones. They have to be configure
    // manually by an external editor.

//    // If there is a higher priority to central one, settings existing in central one should not
//    // be written at all, if there is no write permission to central certificate.
//
//    if (priority == QMApplicationSettings::CENTRAL)
//    {
//        if (centralized != nullptr && centralizedAvailable)
//        {
//            QVariant tmp = centralized->value(name);
//            if (!tmp.isNull())
//            {
//
//                if (!centralized->isWritable())
//                {
//                    // The setting exist in centralized, but is not writeable, so do nothing.
//                    return;
//                }
//                else
//                {
//
//                    // The settings exist in centralized AND is writeable, so write the new value.
//                    centralized->setValue(name, value);
//                    return;
//                }
//            }
//        }
//    }

    // When the function comes to this line, writing to local setting should happen.

    if (local != nullptr)
    {
        local->setValue(name, value);
        return;
    }

    // If local object is not set, log.

    qWarning() << "writing a settings value although the local object does not exist";
}

void QMApplicationSettings::clear()
{
    initLocal();

    readCentralized();
    if (centralizedAvailable)
    {
        initCentralized();
    }
}
