#include "core/ExampleNotes.h"
#include "core/NoteManager.h"
#include "core/Settings.h"
#include "integration/DBusService.h"
#include "integration/DesktopHelper.h"
#include "integration/PortalShortcuts.h"
#include "storage/Crypto.h"
#include "ui/FirstRunDialog.h"
#include "ui/MainWindow.h"
#include "ui/TrayIcon.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QStyleFactory>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
#include <cstdlib> // for unsetenv

static const QString SOCKET_NAME = "linnote-single-instance";

int main(int argc, char *argv[]) {
  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: Pre-Application Initialization
  // These must be called BEFORE QApplication is created
  // ═══════════════════════════════════════════════════════════════════════════

  // Suppress Wayland protocol noise (zwp_text_input warnings, etc.)
  QLoggingCategory::setFilterRules(QStringLiteral("qt.qpa.wayland*=false"));

  // Prevent GNOME startup notification cursor hang (30s loading cursor)
  // This must be done before QApplication creation
  unsetenv("DESKTOP_STARTUP_ID");

  QApplication app(argc, argv);
  app.setApplicationName("linnote");
  app.setApplicationVersion("1.0");
  app.setOrganizationName("LinNote");
  app.setOrganizationDomain("linnote.app");

  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: Desktop File Name for GNOME Dock
  // This ensures the app icon and name display correctly in GNOME's dock
  // ═══════════════════════════════════════════════════════════════════════════
  app.setDesktopFileName("linnote");

  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: Style Hardening for GTK-based Desktops
  // Prevents theme collision artifacts (popup ghosting, dropdown bugs)
  // ═══════════════════════════════════════════════════════════════════════════
  if (LinNote::isGtkBasedDesktop()) {
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    qDebug() << "LinNote: Using Fusion style for"
             << LinNote::desktopEnvironmentName() << "compatibility";
  }

  // === Single Instance Check ===
  // Try to connect to existing instance
  QLocalSocket socket;
  socket.connectToServer(SOCKET_NAME);
  if (socket.waitForConnected(500)) {
    // Another instance is running - send activate signal
    socket.write("activate");
    socket.waitForBytesWritten();
    socket.disconnectFromServer();

    // Show notification and exit
    QMessageBox::information(nullptr, "LinNote",
                             "LinNote is already running!\n\n"
                             "The existing window has been brought to front.");
    return 0;
  }

  // Create server for this instance
  QLocalServer *server = new QLocalServer(&app);
  // Remove stale socket file if exists
  QLocalServer::removeServer(SOCKET_NAME);
  server->listen(SOCKET_NAME);

  // Don't quit when last window is closed (tray icon keeps running)
  app.setQuitOnLastWindowClosed(false);

  // Set application icon
  app.setWindowIcon(QIcon(":/app/icons/app/linnote-128.png"));

  // Command line parser
  QCommandLineParser parser;
  parser.setApplicationDescription("A quick scratchpad for Linux");
  parser.addHelpOption();
  parser.addVersionOption();

  // URL Scheme support
  QCommandLineOption newNoteOption(QStringList() << "n" << "new",
                                   "Create new note with text", "text");
  QCommandLineOption searchOption(QStringList() << "s" << "search",
                                  "Search notes", "query");
  parser.addOption(newNoteOption);
  parser.addOption(searchOption);
  parser.addPositionalArgument("url", "linnote:// URL to open");

  parser.process(app);

  // First-run wizard
  if (Settings::instance()->isFirstRun()) {
    FirstRunDialog firstRunDialog;
    if (firstRunDialog.exec() == QDialog::Accepted) {
      // Apply preferences
      Settings::instance()->setThemeName(firstRunDialog.selectedTheme());
      Settings::instance()->setDarkMode(firstRunDialog.darkModeEnabled());
      Settings::instance()->setBaseCurrency(firstRunDialog.selectedCurrency());
      Settings::instance()->setAutoPasteEnabled(
          firstRunDialog.autoPasteEnabled());

      // Apply encryption settings
      Settings::instance()->setEncryptionEnabled(
          firstRunDialog.encryptionEnabled());

      if (firstRunDialog.encryptionEnabled()) {
        // Hash the password and store using Crypto::hashPassword
        QString password = firstRunDialog.password();
        QString hash = Crypto::hashPassword(password);
        Settings::instance()->setMasterPasswordHash(hash);
        Settings::instance()->setRecoveryKey(firstRunDialog.recoveryKey());
      }

      // Apply global hotkey (this triggers KDE shortcut registration)
      Settings::instance()->setGlobalHotkey(firstRunDialog.selectedHotkey());

      // Apply autostart setting
      if (firstRunDialog.startOnBootEnabled()) {
        QString autostartDir = QDir::homePath() + "/.config/autostart";
        QString autostartPath = autostartDir + "/linnote.desktop";
        QDir().mkpath(autostartDir);
        QFile file(autostartPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
          QTextStream out(&file);
          out << "[Desktop Entry]\n";
          out << "Type=Application\n";
          out << "Name=LinNote\n";
          out << "Comment=Linux Scratchpad\n";
          out << "Exec=" << QCoreApplication::applicationFilePath() << "\n";
          out << "Icon=linnote\n";
          out << "Terminal=false\n";
          out << "Categories=Utility;TextEditor;\n";
          out << "X-GNOME-Autostart-enabled=true\n";
          out << "StartupNotify=false\n";
          file.close();
        }
      }

      Settings::instance()->setFirstRunComplete();
    } else {
      // User closed dialog - still mark first run complete with no encryption
      Settings::instance()->setFirstRunComplete();
    }
  }

  // Create main window
  MainWindow mainWindow;

  // Connect single instance server to activate window
  QObject::connect(server, &QLocalServer::newConnection,
                   [&mainWindow, server]() {
                     QLocalSocket *client = server->nextPendingConnection();
                     if (client) {
                       client->waitForReadyRead(500);
                       // Activate window
                       mainWindow.showAndFocus();
                       client->disconnectFromServer();
                       client->deleteLater();
                     }
                   });

  // Check if this is a fresh setup - offer example notes only once
  if (!Settings::instance()->examplesShown() &&
      mainWindow.noteManager()->noteCount() <= 1) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        &mainWindow, "Example Notes",
        "Would you like to see example notes?\n\n"
        "These examples will show you how each feature works.\n\n"
        "Essential Shortcuts:\n"
        "• Ctrl+N           New page\n"
        "• Ctrl+D           Delete page\n"
        "• Ctrl+Tab         Next page\n"
        "• Ctrl+Shift+Tab   Previous page\n"
        "• Ctrl+F           Search\n"
        "• Esc              Hide window",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    Settings::instance()->setExamplesShown(true); // Mark as shown

    if (reply == QMessageBox::Yes) {
      ExampleNotes::createExamples(mainWindow.noteManager());
    }
  }

  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: System Tray Availability Guard
  // Vanilla GNOME has no system tray - requires extension or fallback to
  // DockOnly
  // ═══════════════════════════════════════════════════════════════════════════
  TrayIcon trayIcon(&mainWindow);
  Settings::DisplayMode displayMode = Settings::instance()->displayMode();

  // Check if system tray is actually available
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    qWarning() << "LinNote: System tray not available on"
               << LinNote::desktopEnvironmentName();

    if (displayMode == Settings::TrayOnly) {
      // TrayOnly mode is not possible - use DockOnly for this session
      qWarning() << "LinNote: TrayOnly mode not possible, using DockOnly";
      displayMode = Settings::DockOnly; // Session override, don't save

      // Show one-time informative dialog (non-blocking)
      QMessageBox::information(
          &mainWindow, QObject::tr("System Tray Unavailable"),
          QObject::tr(
              "Your desktop environment doesn't have a system tray.\n\n"
              "LinNote will appear in the dock/taskbar instead.\n\n"
              "Tip: On GNOME, you can install the 'AppIndicator Support' "
              "extension to enable system tray icons."));
    }
  }

  // Show tray icon if available and mode requires it
  if (displayMode != Settings::DockOnly &&
      QSystemTrayIcon::isSystemTrayAvailable()) {
    trayIcon.show();
  }

  // Quit behavior based on effective display mode
  // If tray is shown, don't quit when last window closed
  app.setQuitOnLastWindowClosed(displayMode == Settings::DockOnly);

  // Register DBus service for external control
  // This allows triggering via: dbus-send --session --type=method_call \
  //   --dest=org.linnote.LinNote /LinNote org.linnote.LinNote.Toggle
  DBusService dbusService(&mainWindow);
  dbusService.registerService();

  // Setup global shortcuts via portal (may not work on all systems)
  PortalShortcuts portalShortcuts;
  QObject::connect(&portalShortcuts, &PortalShortcuts::toggleRequested,
                   &mainWindow, &MainWindow::toggleVisibility);
  portalShortcuts.registerShortcuts();

  // Handle CLI arguments
  QString newNoteText = parser.value(newNoteOption);
  QString searchQuery = parser.value(searchOption);
  QStringList positionalArgs = parser.positionalArguments();

  // Check for linnote:// URL in positional args
  if (!positionalArgs.isEmpty()) {
    QString urlArg = positionalArgs.first();
    if (urlArg.startsWith("linnote://")) {
      dbusService.OpenUrl(urlArg);
    }
  } else if (!newNoteText.isEmpty()) {
    mainWindow.showAndFocus();
    mainWindow.createNewNoteWithText(newNoteText);
  } else if (!searchQuery.isEmpty()) {
    mainWindow.showAndFocus();
    mainWindow.searchNotes(searchQuery);
  } else {
    // Show window on first launch
    mainWindow.showAndFocus();
  }

  return app.exec();
}
