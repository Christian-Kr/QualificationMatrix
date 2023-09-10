// qmappcachemanager.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmappcachemanager.h"
#include "settings/qmapplicationsettings.h"
#include "config.h"

#include <QProgressDialog>
#include <QDir>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QMAppCacheManager *QMAppCacheManager::instance = nullptr;

QMAppCacheManager::QMAppCacheManager()
    : QObject()
    , m_metaInfoSource(std::make_unique<QMAppCacheMetaInfo>())
    , m_metaInfoTarget(std::make_unique<QMAppCacheMetaInfo>())
    , m_lastErrorType(AppCacheError::NONE)
{}

QMAppCacheManager::~QMAppCacheManager() = default;

bool QMAppCacheManager::trigger()
{
    readSettings();

    // check that both paths exist and are directories
    auto sourceFileInfo = QFileInfo(m_sourcePath);

    if (!sourceFileInfo.exists())
    {
        m_lastErrorType = AppCacheError::SOURCE_NOT_EXIST;
        return false;
    }

    if (!sourceFileInfo.isDir())
    {
        m_lastErrorType = AppCacheError::SOURCE_NOT_DIR;
        return false;
    }

    auto targetFileInfo = QFileInfo(m_targetPath);

    if (!targetFileInfo.exists())
    {
        m_lastErrorType = AppCacheError::TARGET_NOT_EXIST;
        return false;
    }

    if (!targetFileInfo.isDir())
    {
        m_lastErrorType = AppCacheError::TARGET_NOT_DIR;
        return false;
    }

    // read in the meta file from the source - this files MUST always exist, if this fails, the
    // whole caching process should fail
    if (!readMetaInfo(m_metaInfoSource, m_sourcePath))
    {
        return false;
    }

    // read in the meta file from the target - this file might not exist, if this is the first time
    // the caching process is running
    if (!readMetaInfo(m_metaInfoTarget, m_targetPath))
    {
        if (m_lastErrorType != AppCacheError::META_NOT_EXIST)
        {
            return false;
        }
    }

    // copy all files from meta info source
    if (!copyFiles())
    {
        qDebug() << m_lastErrorText;
        return false;
    }

    return true;
}

bool QMAppCacheManager::copyFiles()
{
    // the target folder will be renamed first and a new one will be created - after that all files
    // will be copied to the target directory

    // rename target folder
    QString tmpTargetPathNew = m_targetPath + "_backup_"
            + QDateTime::currentDateTime().toString("yyyyMMddThhmmsszzz");

    if (!QDir().rename(m_targetPath, tmpTargetPathNew))
    {
        m_lastErrorType = AppCacheError::COPY_FAILED;
        m_lastErrorText = "cannot rename target folder";
        return false;
    }

    // create a new target folder
    if (!QDir().mkdir(m_targetPath))
    {
        m_lastErrorType = AppCacheError::COPY_FAILED;
        m_lastErrorText = "cannot create new target folder";
        return false;
    }

    // copy all files
    QList<QMAppCacheMetaInfoFile> metaInfoFiles = m_metaInfoSource->files;

    for (const auto &metaInfoFile : metaInfoFiles)
    {
        QString sourceFile = m_sourcePath + metaInfoFile.folderPath + metaInfoFile.name;
        QString targetFile = m_targetPath + metaInfoFile.folderPath + metaInfoFile.name;

        if (!QFile::exists(sourceFile))
        {
            m_lastErrorType = AppCacheError::COPY_FAILED;
            m_lastErrorText = "source file for copy does not exist";
            return false;
        }

        if (!QFile::copy(sourceFile, targetFile))
        {
            m_lastErrorType = AppCacheError::COPY_FAILED;
            m_lastErrorText = "cannot copy file";
            return false;
        }
    }

    return true;
}

bool QMAppCacheManager::readSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    m_sourcePath = settings.read("AppCache/SourcePath", "").toString();
    m_targetPath = settings.read("AppCache/TargetPath", "").toString();

    return true;
}

bool QMAppCacheManager::readMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
        const QString &path)
{
    // reset metaInfo
    metaInfo->versionMajor = -1;
    metaInfo->versionMinor = -1;

    metaInfo->files.clear();

    // open the appcache meta info file
    auto metaFile = QFile(path + QDir::separator() + APPCACHE_META);

    if (!metaFile.exists())
    {
        m_lastErrorType = AppCacheError::META_NOT_EXIST;
        return false;
    }

    // start to read meta information
    if (!metaFile.open(QFile::ReadOnly))
    {
        m_lastErrorType = AppCacheError::META_NOT_READABLE;
        return false;
    }

    // open the file and read in all byte data
    auto metaFileByteArray = metaFile.readAll();
    metaFile.close();

    return parseMetaInfo(metaInfo, metaFileByteArray);
}

bool QMAppCacheManager::parseMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
        const QByteArray &data)
{
    // parse the file content
    auto *parseError = new QJsonParseError();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, parseError);

    // if result is null, then parsing might have failed
    if (jsonDoc.isNull())
    {
        m_lastErrorType = AppCacheError::META_PARSE_FAILED;
        return false;
    }

    // get general information
    QJsonObject jsonRootObject = jsonDoc.object();

    // parse version
    if (!parseVersionFromMetaInfo(metaInfo, jsonRootObject))
    {
        return false;
    }

    // parse files
    if (!parseFilesFromMetaInfo(metaInfo, jsonRootObject))
    {
        return false;
    }

    return true;
}

bool QMAppCacheManager::parseVersionFromMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
        QJsonObject &jsonObject)
{
    // parse version of software
    if (jsonObject.contains("Version") && jsonObject["Version"].isObject())
    {
        auto jsonAppVersion = jsonObject.take("Version").toObject();

        if (jsonAppVersion.contains("Minor") && jsonAppVersion["Minor"].isDouble())
        {
            metaInfo->versionMinor = jsonAppVersion.take("Minor").toInt();
        }
        else
        {
            m_lastErrorType = AppCacheError::META_PARSE_FAILED;
            m_lastErrorText = "minor version number does not exist or is of wrong type";
            return false;
        }

        if (jsonAppVersion.contains("Major") && jsonAppVersion["Major"].isDouble())
        {
            metaInfo->versionMajor = jsonAppVersion.take("Major").toInt();
        }
        else
        {
            m_lastErrorType = AppCacheError::META_PARSE_FAILED;
            m_lastErrorText = "major version number does not exist or is of wrong type";
            return false;
        }

        if (!jsonAppVersion.isEmpty())
        {
            m_lastErrorType = AppCacheError::META_PARSE_FAILED;
            m_lastErrorText = "unknown objects";
            return false;
        }
    }
    else
    {
        m_lastErrorType = AppCacheError::META_PARSE_FAILED;
        m_lastErrorText = "no version object found";
        return false;
    }

    return true;
}

bool QMAppCacheManager::parseFilesFromMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
        QJsonObject &jsonObject)
{
    // parse files that should be copied, alongside with a hash value
    if (jsonObject.contains("Files") && jsonObject["Files"].isArray())
    {
        auto jsonFilesArray = jsonObject.take("Files").toArray();

        for (const auto &item: jsonFilesArray)
        {
            auto itemObject = item.toObject();

            QMAppCacheMetaInfoFile metaInfoFile;

            if (itemObject.contains("Name") && itemObject["Name"].isString())
            {
                metaInfoFile.name = itemObject.take("Name").toString();
            }
            else
            {
                m_lastErrorType = AppCacheError::META_PARSE_FAILED;
                m_lastErrorText = "name for a file does not exist or is of wrong type";
                return false;
            }

            if (itemObject.contains("FolderPath") && itemObject["FolderPath"].isString())
            {
                metaInfoFile.folderPath = itemObject.take("FolderPath").toString();
            }
            else
            {
                m_lastErrorType = AppCacheError::META_PARSE_FAILED;
                m_lastErrorText = "folder path for a file does not exist or is of wrong type";
                return false;
            }

            if (itemObject.contains("Hash") && itemObject["Hash"].isString())
            {
                metaInfoFile.hash = itemObject.take("Hash").toString();
            }
            else
            {
                m_lastErrorType = AppCacheError::META_PARSE_FAILED;
                m_lastErrorText = "hash for a file does not exist or is of wrong type";
                return false;
            }

            if (!itemObject.isEmpty())
            {
                m_lastErrorType = AppCacheError::META_PARSE_FAILED;
                m_lastErrorText = "unknown objects";
                return false;
            }

            metaInfo->files.append(metaInfoFile);
        }
    }
    else
    {
        m_lastErrorType = AppCacheError::META_PARSE_FAILED;
        m_lastErrorText = "no files array found";
        return false;
    }

    return true;
}
