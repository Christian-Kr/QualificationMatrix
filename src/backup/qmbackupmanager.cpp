// qmbackupmanager.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmbackupmanager.h"
#include "settings/qmapplicationsettings.h"
#include "config.h"

#include <QFile>
#include <QSqlDatabase>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

QMBackupManager *QMBackupManager::instance = nullptr;

QMBackupMetaInfo::QMBackupMetaInfo()
    : metaFile(std::make_unique<QFile>())
    , integrityHash(std::make_unique<QString>())
    , relFilePath(std::make_unique<QString>())
    , creationDate(std::make_unique<QDateTime>())
{}

QMBackupMetaInfo::QMBackupMetaInfo(const QMBackupMetaInfo &other) noexcept
    : metaFile(std::make_unique<QFile>(other.metaFile->fileName()))
    , integrityHash(std::make_unique<QString>(*(other.integrityHash)))
    , relFilePath(std::make_unique<QString>(*(other.relFilePath)))
    , creationDate(std::make_unique<QDateTime>(*(other.creationDate)))
{}

QMBackupMetaInfo& QMBackupMetaInfo::operator=(const QMBackupMetaInfo &other)
{
    metaFile->setFileName(other.metaFile->fileName());
    *integrityHash = *(other.integrityHash);
    *relFilePath = *(other.relFilePath);
    *creationDate = *(other.creationDate);

    return *this;
}

bool QMBackupMetaInfo::operator<(const QMBackupMetaInfo &other) const
{
    return *creationDate < *(other.creationDate);
}

QMBackupManager::QMBackupManager()
    : QObject()
    , m_generalInformation(std::make_unique<QMGeneralInformation>())
    , m_databaseFileInfo(std::make_unique<QFileInfo>())
    , m_backupFolderPath(std::make_unique<QString>())
{}

QMBackupManager::~QMBackupManager() = default;

bool QMBackupManager::trigger()
{
    // Read database file and set path variables.
    if (!initializeDatabaseFileLocation())
    {
        return false;
    }

    // Read general backup systems settings.
    if (!readSettings())
    {
        return false;
    }


    switch (m_generalInformation->mode)
    {
        case BackupMode::ONCE_A_DAY:
        {
            // Test if the last backup file has been created today. If yes: Backup process has been finished.

            QList<QMBackupMetaInfo> backupMetaInfoList = getBackupMetaInfoList();
            std::sort(backupMetaInfoList.begin(), backupMetaInfoList.end());

            if (!backupMetaInfoList.empty()) {
                if (backupMetaInfoList.last().creationDate->date() == QDate::currentDate()) {
                    return true;
                }
            }

            break;
        }
        case BackupMode::ONLY_AFTER_CHANGE:
        {
            // Test if the last backup file has the same hash value in comparision to current one. If yes: Backup
            // process has been finished.

            QList<QMBackupMetaInfo> backupMetaInfoList = getBackupMetaInfoList();
            std::sort(backupMetaInfoList.begin(), backupMetaInfoList.end());

            if (!backupMetaInfoList.empty())
            {
                auto lastHash = *(backupMetaInfoList.last().integrityHash);

                // Create a hash value of the database file.
                QFile databaseFile(m_databaseFileInfo->absoluteFilePath());

                if (!databaseFile.open(QFile::ReadOnly))
                {
                    qCritical() << "BackupManager: The db file can not be opened for read (" <<
                            databaseFile.fileName() << ")";
                    return false;
                }

                auto dbHash = QString(QCryptographicHash::hash(
                        databaseFile.readAll(), QCryptographicHash::Md5).toHex());
                databaseFile.close();

                if (dbHash.compare(lastHash) == 0)
                {
                    return true;
                }
            }

            break;
        }
        case BackupMode::EVERY_TRIGGER:
        {
            break;
        }
        default:
            qCritical() << "BackupManager: Unknown backup mode";
            return false;
    }

    // Run the backup.
    return run();
}

bool QMBackupManager::run()
{
    // Create a hash value of the database file.
    QFile databaseFile(m_databaseFileInfo->absoluteFilePath());

    if (!databaseFile.open(QFile::ReadOnly))
    {
        qCritical() << "BackupManager: The db file can not be opened for read (" << databaseFile.fileName() << ")";
        return false;
    }

    auto dbHash = QString(QCryptographicHash::hash(databaseFile.readAll(), QCryptographicHash::Md5).toHex());
    databaseFile.close();

    // Copy the file to the new location. The location is inside the base path of the backup, which itself is inside
    // the same path as the database file.
    QDateTime creationDate = QDateTime::currentDateTime();
    auto copyPath = *m_backupFolderPath + QDir::separator() + m_databaseFileInfo->baseName() + "_" +
            creationDate.toString("yyyyMMdd_hhmmss") + ".qmsql";

    if (!databaseFile.copy(copyPath))
    {
        qCritical() << "BackupManager: Backup file cannot be created (" << copyPath << ")";
        return false;
    }

    // Test the new file for the right hash.
    QFile copyFile(copyPath);

    if (!copyFile.exists())
    {
        qCritical() << "BackupManager: The copied file does not exist (" << copyFile.fileName() << ")";
        return false;
    }

    // The file exist, but we are not able to open it. Therefore this error is more critical than a not existing file.
    // The user has to manually repair the failure. It might be a good idea to run a integrity check on all files and
    // after that, give the user an information what to do.
    if (!copyFile.open(QFile::ReadOnly))
    {
        qCritical() << "BackupManager: The db copy file can not be opened for read (" << copyPath << ")";
        return false;
    }

    auto copyHash = QString(QCryptographicHash::hash(copyFile.readAll(), QCryptographicHash::Md5).toHex());
    copyFile.close();

    // The file exist, it has been opened to reading, but the hash value before and after are not equal. This means,
    // that some information has been lost during the copy process. The copy should be deleted. The user might want
    // to start a new backup.
    if (dbHash.compare(copyHash) != 0)
    {
        qCritical() << "BackupManager: The integrity check of the copied file failed (" << copyPath << ")";

        // If the copied file cannot not be deleted, this might be more critical. The user should manually act.
        if (!copyFile.remove())
        {
            qCritical() << "BackupManager: The db copy cannot be removed after a failed copy process ("
                    << copyPath << ")";
        }

        return false;
    }

    // Create a config file with the given information about the new backup file.
    QMBackupMetaInfo backupMetaInfo;

    auto metaFilePath = *m_backupFolderPath + QDir::separator() + m_databaseFileInfo->baseName() + "_" +
            creationDate.toString("yyyyMMdd_hhmmss") + ".json";
    backupMetaInfo.metaFile->setFileName(metaFilePath);

    *(backupMetaInfo.creationDate) = creationDate;
    *(backupMetaInfo.relFilePath) = m_databaseFileInfo->baseName() + "_" +
            creationDate.toString("yyyyMMdd_hhmmss") + ".qmsql";
    *(backupMetaInfo.integrityHash) = copyHash;

    backupMetaInfo.integrityCheckPassed = true;

    if (!writeBackupMetaInfo(backupMetaInfo))
    {
        return false;
    }

    // Get all backups meta info files, find the oldest backups and delete every backup file that is more than the
    // maximum number of total backups defined.

    QList<QMBackupMetaInfo> backupMetaInfoList = getBackupMetaInfoList();
    std::sort(backupMetaInfoList.begin(), backupMetaInfoList.end());

    while (backupMetaInfoList.size() > m_generalInformation->maxBackups && m_generalInformation->maxBackups > 0)
    {
        QMBackupMetaInfo tmpBackupMetaInfo = backupMetaInfoList.takeFirst();

        // Delete both files.
        if (!QFile::remove(tmpBackupMetaInfo.metaFile->fileName()))
        {
            qWarning() << "BackupManager: Cannot delete meta file (" << tmpBackupMetaInfo.metaFile->fileName() << ")";
        }

        if (!QFile::remove(*m_backupFolderPath + QDir::separator() + *(tmpBackupMetaInfo.relFilePath)))
        {
            qWarning() << "BackupManager: Cannot delete backup file (" <<
                *(tmpBackupMetaInfo.relFilePath) << ")";
        }
    }

    return true;
}

bool QMBackupManager::initializeDatabaseFileLocation()
{
    // For the backup system to work, a database connection to a local database file is necessary. Otherwise the
    // system cannot locate where to store the files.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning() << "BackupManager: Database is not connected";
        return false;
    }

    auto db = QSqlDatabase::database("default");

    // For a local file, the database name is the path to the database as a file. If the file does not exist, there is
    // no local connection to database file. Then the backup manager will not work.
    if (!QFileInfo::exists(db.databaseName()))
    {
        qWarning() << "BackupManager: Database file does not exist as a path (" << db.databaseName() << ")";
        return false;
    }

    m_databaseFileInfo->setFile(db.databaseName());

    return true;
}

bool QMBackupManager::readSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Read the backup folder. The backup folder ist relative to the database file path. Therefore an open database
    // file must exist. If the folder does not exist, create it.
    auto tmpBackupFolder = settings.read("Backup/Folder", "backup").toString();

    // Create the final absolute path for the backup folder and test whether it exists or not.
    *m_backupFolderPath = m_databaseFileInfo->absolutePath() + QDir::separator() + tmpBackupFolder;

    auto backupFolderPathDir = QDir(*m_backupFolderPath);
    if (!backupFolderPathDir.exists())
    {
        // The backup path does not exist, so try to create it.
        if (!backupFolderPathDir.mkpath(*m_backupFolderPath))
        {
            qWarning() << "BackupManager: The backup folder path does not exist and cannot be created ("
                    << *m_backupFolderPath << ")";
            return false;
        }
    }

    // Read in the backup mode.
    m_generalInformation->mode = static_cast<BackupMode>(settings.read("Backup/Mode", 2).toInt());

    // What is the maximum number of backups that should exist? More than the maximum number will be deleted
    // beginning with the oldest one.
    m_generalInformation->maxBackups = settings.read("Backup/Max", 60).toInt();

    return true;
}

bool QMBackupManager::writeBackupMetaInfo(QMBackupMetaInfo &backupMetaInfo)
{
    // Here start to read in all database meta information.
    if (!backupMetaInfo.metaFile->open(QFile::ReadWrite))
    {
        qCritical() << "BackupManager: The meta information file can not be opened for read ("
                << backupMetaInfo.metaFile->fileName() << ")";
        return false;
    }

    QJsonObject jsonRootObject;
    jsonRootObject.insert("integrity_check_passed", QJsonValue(backupMetaInfo.integrityCheckPassed));
    jsonRootObject.insert("integrity_hash", QJsonValue(*(backupMetaInfo.integrityHash)));
    jsonRootObject.insert("rel_file_path", QJsonValue(*(backupMetaInfo.relFilePath)));
    jsonRootObject.insert("creation_date", QJsonValue(backupMetaInfo.creationDate->toString(Qt::DateFormat::ISODate)));

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonRootObject);

    backupMetaInfo.metaFile->write(jsonDoc.toJson());
    backupMetaInfo.metaFile->close();

    return true;
}

bool QMBackupManager::readBackupMetaInfo(QMBackupMetaInfo &backupMetaInfo)
{
    // Get the file from the given backupMetaInfo and read in all information from the file.

    // Try to open the meta information file.
    if (!backupMetaInfo.metaFile->exists())
    {
        qWarning() << "BackupManager: The meta information file does not exist and cannot be created ("
                << backupMetaInfo.metaFile->fileName() << ")";
        return false;
    }

    // Here start to read in all database meta information.
    if (!backupMetaInfo.metaFile->open(QFile::ReadOnly))
    {
        qCritical() << "BackupManager: The meta information file can not be opened for read ("
                << backupMetaInfo.metaFile->fileName() << ")";
        return false;
    }

    // Open the file.
    auto metaInfoFileByteArray = backupMetaInfo.metaFile->readAll();
    backupMetaInfo.metaFile->close();

    // Parse the file content.
    auto *error = new QJsonParseError();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(metaInfoFileByteArray, error);

    // If the result of fromJson is null, then parsing might have failed.
    if (jsonDoc.isNull())
    {
        qCritical() << "BackupManager: Config cannot be parsed (" << error->errorString() << ")";
        return false;
    }

    // Get general information.
    QJsonObject jsonRootObject = jsonDoc.object();

    if (jsonRootObject.contains("integrity_check_passed"))
    {
        backupMetaInfo.integrityCheckPassed = jsonRootObject["integrity_check_passed"].toBool();
    }

    if (jsonRootObject.contains("integrity_hash"))
    {
        *(backupMetaInfo.integrityHash) = jsonRootObject["integrity_hash"].toString();
    }

    if (jsonRootObject.contains("rel_file_path"))
    {
        *(backupMetaInfo.relFilePath) = jsonRootObject["rel_file_path"].toString();
    }

    if (jsonRootObject.contains("creation_date"))
    {
        QString strDateTime = jsonRootObject["creation_date"].toString();
        *(backupMetaInfo.creationDate) = QDateTime::fromString(strDateTime, Qt::DateFormat::ISODate);
    }

    return true;
}

QList<QMBackupMetaInfo> QMBackupManager::getBackupMetaInfoList()
{
    // Read database file and set path variables.
    if (!initializeDatabaseFileLocation())
    {
        return {};
    }

    // Read general backup systems settings.
    if (!readSettings())
    {
        return {};
    }

    // Get a list of all json backup files in the backup path folder and parse them into objects to get the full list
    // of backup files.
    QList<QMBackupMetaInfo> backupMetaInfoList;

    QDir backupFolderDir(*m_backupFolderPath);
    auto metaInfoFileInfoList = backupFolderDir.entryInfoList(QStringList() << "*.json", QDir::Files);

    for (const auto& metaInfoFileInfo : metaInfoFileInfoList)
    {
        QMBackupMetaInfo backupMetaInfo;
        backupMetaInfo.metaFile->setFileName(metaInfoFileInfo.absoluteFilePath());

        if (readBackupMetaInfo(backupMetaInfo))
        {
            backupMetaInfoList.append(backupMetaInfo);
        }
        else
        {
            qWarning() << "BackupManager: Cannot read backup meta info file (" <<
                    metaInfoFileInfo.absoluteFilePath() << ")";
        }
    }

    return backupMetaInfoList;
}
