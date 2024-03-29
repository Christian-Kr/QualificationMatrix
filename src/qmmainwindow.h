// qmmainwindow.h is part of QualificationMatrix
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

#ifndef QMMAINWINDOW_H
#define QMMAINWINDOW_H

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMMainWindow;
}
QT_END_NAMESPACE

// Forward declarations
class QProgressDialog;
class QMQualiResultWidget;
class QMQualiMatrixWidget;
class QMTrainDataWidget;
class QTranslator;
class QSqlDatabase;
class QMSigningListDialog;
class QMAMSLoginDialog;
class QMNewCertificateDialog;
class QMFavoriteDatabaseDialog;

enum class LoginState;

// Enum: The WIN_MODE. Every time the mode will be switched and a new widget will be shown, the old objects will be
// deleted. This makes sure, nothing unexpected happens or blocks access in unused application parts.
enum class WIN_MODE {
    NONE,           // Show no widget. This normally happens, when no database has been loaded.
    RESULT,         // Show the qualification result widget.
    MATRIX,         // Show the qualification matrix widget.
    TRAININGDATA    // Show the training data widget.
};

/// Main windows class for this application.
/// \author Christian Kr, Copyright 2020
class QMMainWindow: public QMainWindow
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object, if there is one.
    explicit QMMainWindow(QWidget *parent = nullptr);

    /// Destructor
    ~QMMainWindow() override;

public slots:
    /// AMS login user.
    void amsLogin();

    /// Login dialog has finished and result has to be handle.
    /// \param result The result of the user (e.g. accpeted or rejected).
    [[maybe_unused]] void amsLoginDialogFinished(int result);

    /// AMS logout user.
    void amsLogout();

    /// Show the dialog to create a signing list.
    [[maybe_unused]] void showCreateSigningList();

    /// Show the about dialog of this application.
    [[maybe_unused]] void showAbout();

    /// Show the about dialog of Qt.
    [[maybe_unused]] void showAboutQt();

    /// Show the central settings dialog of this application.
    [[maybe_unused]] void showSettings();

    /// Show a progress dialog to informate about a working progress.
    /// \param title The title of the progress dialog.
    /// \param text The text to give the user an information about the work that will be done.
    /// \param minSteps The minimum steps of updates for the progress widget.
    /// \param maxSteps The maximum steps of updates for the progress widget.
    void showProgress(const QString &title, const QString &text, const int &minSteps, const int &maxSteps);

    /// Save all readed settings centralized on one place.
    void saveSettings();

    /// Informate before a workload starts. This will show a progress dialog with progress information.
    /// \param maxSteps Maximum number of steps in progress.
    /// \param info Information about what happens.
    void workloadStarts(const QString& info, int maxSteps);

    /// Only call when progressdialog object has been created and dialog is visible.
    /// \param currentStep Needs be lower than maximum step.
    void workloadUpdates(int currentStep);

    /// Should only be called after progressdialog has been shown.
    void closeProgress();

    /// Close the database, if there is currently an open one.
    ///
    /// \param silent true if closing should be silent without message box, else false
    /// \param showFavs true if favorite dialog should be shown afterwarts, else false; when this
    ///         when this value is true, additional tests will be done whether to show the dialog
    ///         or not
    /// \return False if it cannot be close or don't exist, else true.
    bool closeDatabase(bool silent = false, bool showFavs = true);

    /// Shows the database dialog to manage the database information.
    [[maybe_unused]] void openDatabase();

    /// Manage database information without the database dialog.
    /// \return True if it could be load and open from settings, else false.
    bool manageDatabaseFromSettings();

    /// Save a database backup.
    /// \param db The database to save.
    /// \return True if success, else false.
    bool saveSingleDatabaseBackup(const QSqlDatabase &db);

    /// Open dialog for managing the certificate files.
    [[maybe_unused]] void manageCertificate();

    /// Show dialog for certificate integrity check.
    [[maybe_unused]] void showCertificateIntegrityCheck();

    /// Enter the result window mode.
    [[maybe_unused]] void enterResultMode();

    /// Enter the quali matrix window mode.
    [[maybe_unused]] void enterQualiMatrixMode();

    /// Enter the training data window mode.
    [[maybe_unused]] void enterTrainingDataMode();

    /// Enter the given window mode.
    /// \param mode The window mode to enter.
    void enterWindowMode(WIN_MODE mode);

    /// Closes the current window mode and deletes all objects.
    /// \return True if success, else false. False could also be a user abort.
    bool closeCurrentWindowMode();

    /// Handle the login change signal.
    /// \param before Login state before the change.
    /// \param current Current login state after change.
    void handleLoginChange(LoginState before, LoginState current);

    /// Initialize database information.
    void initDatabaseSettings();

    /// Add a new certificate.
    [[maybe_unused]] void addCertificate();

    /// Dialog has been closed.
    [[maybe_unused]] void addCertificateFinished(int);

    /// Create a new empty database to be used as a starting point for the application.
    [[maybe_unused]] void createEmptyDatabase();

    /// Show the favorites dialog.
    [[maybe_unused]] void showFavorites();

    /// Open the favorite database file given.
    [[maybe_unused]] void openFavoriteDatabase(QString dbFileName);

protected:
    /// Override from QMainWindow. This function will be called on closeing the widget.
    /// \param event The close event object that will be created.
    void closeEvent(QCloseEvent *event) override;

private:
    /// Save QSqlDatabase information to QMApplicationSettings.
    static void saveDatabaseSettings();

    /// Load QSqlDatabase information from QMApplicationSettings. A new database object will be created. If one exist
    /// with the name, the database will be closed and the object removed.
    /// \param dbName The name of the db that should be created from settings.
    /// \return True if database from was successfull, else false.
    bool loadDatabaseFromSettings(const QString &dbName = "default");

    /// Some initial actions after database has been successfully loaded.
    void initAfterDatabaseOpened();

    Ui::QMMainWindow *ui;

    std::unique_ptr<QMNewCertificateDialog> m_newCertificateDialog;
    std::unique_ptr<QMSigningListDialog> m_signingListDialog;
    std::unique_ptr<QMAMSLoginDialog> m_amsLoginDialog;
    std::unique_ptr<QProgressDialog> m_progressDialog;
    std::unique_ptr<QMQualiResultWidget> m_qualiResultWidget;
    std::unique_ptr<QMQualiMatrixWidget> m_qualiMatrixWidget;
    std::unique_ptr<QMTrainDataWidget> m_trainDataWidget;
    std::unique_ptr<QMFavoriteDatabaseDialog> m_favoriteDatabaseDialog;

    WIN_MODE winMode;
    WIN_MODE lastWinMode;
};

#endif // QMMAINWINDOW_H
