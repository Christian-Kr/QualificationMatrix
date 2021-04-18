// appcacheconfigjson.cpp is part of QualificationMatrix
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

#include "appcacheconfigjson.h"

#include <memory>
#include <QJsonDocument>
#include <QFileInfo>
#include <QFile>
#include <QJsonParseError>

AppCacheConfigJSON::AppCacheConfigJSON(QObject *parent)
    : AppCacheConfig(parent)
{}

bool AppCacheConfigJSON::parse(QString fileName)
{
    if (!QFileInfo::exists(fileName))
    {
        // TODO: Error string handling.
        return false;
    }

    QFile file;
    file.setFileName(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // TODO: Error string handling.
        return false;
    }

    auto content = file.readAll();
    file.close();

    std::unique_ptr<QJsonParseError> parseError = std::make_unique<QJsonParseError>();
    QJsonDocument doc = QJsonDocument::fromJson(content, parseError.get());

    if (parseError->error != QJsonParseError::NoError)
    {
        // TODO: Error string handling.
        return false;
    }

    // At least, a document needs one array for the files and one object with the version number.

}

QList<QString> AppCacheConfigJSON::getCacheFiles()
{
    // TODO: Get all files that should be cached.
}

QString AppCacheConfigJSON::version()
{
    // TODO: Get the version of the config, defining the version of the application that should be cached.
}
