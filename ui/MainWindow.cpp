#include "MainWindow.h"
#include "ConfettiWidget.h"

#include "PageSelector.h"
#include "PasswordDialog.h"
#include "SearchBar.h"
#include "SearchModal.h"
#include "SettingsDialog.h"
#include "core/NoteManager.h"
#include "core/OcrHelper.h"
#include "core/Settings.h"
#include "core/SlashCommand.h"
#include "core/Theme.h"
#include "core/Timer.h"
#include "core/UrlShortener.h"
#include "integration/DesktopHelper.h"
#include "storage/BackupManager.h"
#include "storage/Crypto.h"
#include "storage/Export.h"
#include <QApplication>
#include <QCursor>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QShortcut>
#include <QShowEvent>
#include <QSizeGrip>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

#ifdef HAVE_KWINDOWSYSTEM
#include <KWindowSystem>
#include <KX11Extras>
#include <netwm_def.h>
#endif

#ifdef HAVE_LAYERSHELLQT
#include <LayerShellQt/Shell>
#include <LayerShellQt/Window>
#endif

#ifdef HAVE_KGLOBALACCEL
#include <KGlobalAccel>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_editor(nullptr),
      m_clipboardManager(new ClipboardManager(this)),
      m_noteManager(new NoteManager(this)), m_pageSelector(nullptr),
      m_slashCommand(new SlashCommand(m_noteManager, this)), m_state(Hidden),
      m_autoPastedThisSession(false), m_updatingEditor(false),
      m_titleBar(nullptr), m_dragging(false), m_pinButton(nullptr),
      m_alwaysOnTop(false), m_timerWidget(nullptr), m_confetti(nullptr),
      m_searchModal(nullptr), m_suppressFocusOut(false),
      m_autoLockTimer(new QTimer(this)),
      m_backupManager(new BackupManager(this)) {
  setupUi();
  setupShortcuts();

  // Start auto-backup if enabled
  m_backupManager->startAutoBackup();

  // Register KDE global shortcut (like Yakuake does)
#ifdef HAVE_KGLOBALACCEL
  registerKdeGlobalShortcut();
  // Re-register when user changes hotkey in Settings
  connect(Settings::instance(), &Settings::settingsChanged, this,
          &MainWindow::registerKdeGlobalShortcut);
#endif

  // Setup auto-lock timer
  connect(m_autoLockTimer, &QTimer::timeout, this,
          &MainWindow::onAutoLockTimeout);

  // Frameless window - Qt::Tool + Popup hides from dock/panel on KDE
  Settings::DisplayMode displayMode = Settings::instance()->displayMode();
  if (displayMode == Settings::TrayOnly) {
    // Tray Only: hide from dock/taskbar/panel
    // Qt::Tool + Qt::BypassWindowManagerHint can help on some compositors
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool |
                   Qt::WindowDoesNotAcceptFocus);
  } else {
    // Dock Only or Both: show in dock/taskbar
    setWindowFlags(Qt::FramelessWindowHint);
  }
  setAttribute(Qt::WA_TranslucentBackground);

  // Reasonable default size
  resize(500, 400);

  // Center on screen where cursor is (multi-monitor support)
  QPoint cursorPos = QCursor::pos();
  QScreen *currentScreen = QGuiApplication::screenAt(cursorPos);
  if (!currentScreen) {
    currentScreen = QGuiApplication::primaryScreen();
  }
  if (currentScreen) {
    QRect screenGeometry = currentScreen->availableGeometry();
    move(screenGeometry.x() + (screenGeometry.width() - width()) / 2,
         screenGeometry.y() + (screenGeometry.height() - height()) / 2);
  }

  // Apply dark theme
  applyStyle();

  // Set window icon
  setWindowIcon(QIcon(":/app/icons/app/linnote-128.png"));

  // Connect to NoteManager
  connect(m_noteManager, &NoteManager::currentNoteChanged, this,
          &MainWindow::onCurrentNoteChanged);
  connect(m_noteManager, &NoteManager::noteModeChanged, this,
          &MainWindow::onNoteModeChanged);

  // Connect slash commands
  connect(m_slashCommand, &SlashCommand::openSettingsRequested, this,
          &MainWindow::openSettingsDialog);
  connect(m_slashCommand, &SlashCommand::ocrRequested, this,
          &MainWindow::captureOcr);
  connect(m_slashCommand, &SlashCommand::modeChanged, this,
          [this](int mode) { m_editor->setMode(static_cast<NoteMode>(mode)); });
  connect(m_slashCommand, &SlashCommand::textAnalysisRequested, this,
          [this](const QString &type) { m_editor->performTextAnalysis(type); });

  // AutoPaste connections
  connect(m_slashCommand, &SlashCommand::autoPasteRequested, this,
          [this](const QString &delimiter) {
            m_editor->startAutoPaste(delimiter);
            m_clipboardManager->startMonitoring();
          });
  connect(m_clipboardManager, &ClipboardManager::contentReceived, m_editor,
          &NoteEditor::onClipboardContent);
  connect(m_editor, &NoteEditor::autoPasteStopped, this,
          [this]() { m_clipboardManager->stopMonitoring(); });

  // Timer command connections
  connect(m_slashCommand, &SlashCommand::timerStartStopwatch, this, [this]() {
    ensureTimerWidget();
    m_timerWidget->setType(TimerType::Stopwatch);
    m_timerWidget->start();
  });
  connect(m_slashCommand, &SlashCommand::timerStartCountdown, this,
          [this](int seconds, const QString &name) {
            ensureTimerWidget();
            m_timerWidget->setType(TimerType::Countdown);
            m_timerWidget->setCountdownSeconds(seconds);
            if (!name.isEmpty()) {
              m_timerWidget->setCountdownName(name);
            }
            m_timerWidget->start();
          });
  connect(m_slashCommand, &SlashCommand::timerStartPomodoro, this,
          [this](int workMin, int breakMin) {
            ensureTimerWidget();
            m_timerWidget->setCustomPomodoro(workMin, breakMin);
            m_timerWidget->start();
          });
  connect(m_slashCommand, &SlashCommand::timerControl, this,
          [this](const QString &action) {
            if (!m_timerWidget)
              return;
            if (action == "pause") {
              m_timerWidget->toggle();
            } else if (action == "reset") {
              m_timerWidget->reset();
            } else if (action == "stop") {
              m_timerWidget->reset();
              m_timerWidget->hide();
            }
          });
  connect(m_slashCommand, &SlashCommand::timerStartBreathe, this, [this]() {
    ensureTimerWidget();
    // Breathe mode: 4 seconds in, 7 hold, 8 out (4-7-8 breathing)
    m_timerWidget->startBreatheMode();
  });

  // Auto-hide when focus is lost (clicking outside)
  // NOTE: Don't hide when always-on-top is enabled (pin mode)
  connect(qApp, &QApplication::focusChanged, this,
          [this](QWidget *old, QWidget *now) {
            Q_UNUSED(old);
            // Don't hide if always-on-top (pin) is enabled
            if (m_alwaysOnTop)
              return;
            // Don't hide if focus is still somewhere in our application
            if (!isVisible() || m_suppressFocusOut)
              return;

            // If focus is nullptr (went to another app), hide
            if (!now) {
              hideWindow();
              return;
            }

            // Check if the focused widget is part of our window hierarchy
            QWidget *w = now;
            while (w) {
              if (w == this)
                return; // Focus is on our window or child
              w = w->parentWidget();
            }

            // Check if focus is on a dialog or popup (like message box,
            // dropdown)
            if (now->inherits("QMenu") ||
                now->inherits("QComboBoxPrivateContainer") ||
                now->inherits("QMessageBox") || now->inherits("QDialog") ||
                now->inherits("QInputDialog")) {
              return; // Don't hide for dialogs and popups
            }

            // If focus went to another window, hide
            if (now->window() != this) {
              hideWindow();
            }
          });

  // Load initial note into editor
  if (m_noteManager->noteCount() > 0) {
    onCurrentNoteChanged(m_noteManager->currentIndex());
  }

  // Apply theme at startup
  applyTheme();
}

MainWindow::~MainWindow() { saveCurrentNoteContent(); }

MainWindow::State MainWindow::state() const { return m_state; }

NoteManager *MainWindow::noteManager() const { return m_noteManager; }

void MainWindow::applyStyle() {
  // Only structural styles - colors come from applyTheme()
  setStyleSheet(R"(
        QMainWindow {
            background: transparent;
        }
        #container {
            border-radius: 12px;
            border: 1px solid;
        }
        #titleBar {
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
        }
        #titleLabel {
            font-weight: bold;
            font-size: 13px;
        }
        #closeBtn, #minimizeBtn, #exportBtn, #settingsBtn, #pinBtn {
            background: transparent;
            border: none;
            font-size: 14px;
            padding: 4px 8px;
            border-radius: 4px;
        }
        QPlainTextEdit {
            border: none;
            border-bottom-left-radius: 12px;
            border-bottom-right-radius: 12px;
            padding: 12px;
        }
    )");
}

void MainWindow::setupUi() {
  // Main container
  QWidget *container = new QWidget(this);
  container->setObjectName("container");

  QVBoxLayout *mainLayout = new QVBoxLayout(container);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Enable mouse tracking for auto-hide feature
  container->setMouseTracking(true);
  container->installEventFilter(this);

  // Custom title bar
  m_titleBar = new QWidget();
  m_titleBar->setObjectName("titleBar");
  m_titleBar->setFixedHeight(36);
  m_titleBar->setMouseTracking(true);
  m_titleBar->installEventFilter(this);

  QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
  titleLayout->setContentsMargins(12, 0, 8, 0);
  titleLayout->setSpacing(4);

  // App icon in titlebar
  QLabel *iconLabel = new QLabel();
  iconLabel->setObjectName("appIcon");
  iconLabel->setPixmap(
      QPixmap(":/app/icons/app/linnote-32.png")
          .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  iconLabel->setToolTip("LinNote");
  iconLabel->setFixedSize(28, 28);
  titleLayout->addWidget(iconLabel);

  // Page selector
  m_pageSelector = new PageSelector(m_noteManager);
  m_pageSelector->setObjectName("pageSelector");
  titleLayout->addWidget(m_pageSelector);

  // Stretch to push buttons to right
  titleLayout->addStretch();

  // Common button style - transparent with hover effect
  QString titleBtnStyle = R"(
    QPushButton {
      background: transparent;
      border: none;
      padding: 2px;
      border-radius: 4px;
    }
    QPushButton:hover { background-color: rgba(128, 128, 128, 0.2); }
  )";
  // Auto-hide toggle button (▼ = auto-hide on, ▲ = auto-hide off)
  m_autoHideButton =
      new QPushButton(Settings::instance()->toolbarAutoHide() ? "▲" : "▼");
  m_autoHideButton->setObjectName("autoHideBtn");
  m_autoHideButton->setToolTip(tr("Toggle Toolbar Auto-Hide"));
  m_autoHideButton->setFixedSize(24, 24);
  m_autoHideButton->setStyleSheet(titleBtnStyle);
  connect(m_autoHideButton, &QPushButton::clicked, this, [this]() {
    bool newState = !Settings::instance()->toolbarAutoHide();
    Settings::instance()->setToolbarAutoHide(newState);
    m_autoHideButton->setText(newState ? "▲" : "▼");
    updateToolbarVisibility();
  });
  titleLayout->addWidget(m_autoHideButton);

  // Pin button (always on top toggle) - emoji
  m_pinButton = new QPushButton("📌");
  m_pinButton->setObjectName("pinBtn");
  m_pinButton->setToolTip(tr("Always on Top (Ctrl+T)"));
  m_pinButton->setFixedSize(24, 24);
  m_pinButton->setStyleSheet(titleBtnStyle);
  connect(m_pinButton, &QPushButton::clicked, this,
          &MainWindow::toggleAlwaysOnTop);
  titleLayout->addWidget(m_pinButton);

  // Export button - emoji
  QPushButton *exportBtn = new QPushButton("📤");
  exportBtn->setObjectName("exportBtn");
  exportBtn->setToolTip(tr("Export to .txt"));
  exportBtn->setFixedSize(24, 24);
  exportBtn->setStyleSheet(titleBtnStyle);
  connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportNote);
  titleLayout->addWidget(exportBtn);

  // Settings button - emoji
  QPushButton *settingsBtn = new QPushButton("⚙");
  settingsBtn->setObjectName("settingsBtn");
  settingsBtn->setToolTip(tr("Settings"));
  settingsBtn->setFixedSize(24, 24);
  settingsBtn->setStyleSheet(titleBtnStyle);
  connect(settingsBtn, &QPushButton::clicked, this,
          &MainWindow::openSettingsDialog);
  titleLayout->addWidget(settingsBtn);

  mainLayout->addWidget(m_titleBar);

  // Note editor
  m_editor = new NoteEditor();
  m_editor->setObjectName("noteEditor");
  m_editor->setPlaceholderText(tr("Start typing..."));
  m_editor->setMouseTracking(true);
  m_editor->installEventFilter(this);
  mainLayout->addWidget(m_editor);

  // Search bar (hidden by default)
  m_searchBar = new SearchBar(this);
  m_searchBar->setEditor(m_editor);
  m_searchBar->setNoteManager(m_noteManager);
  connect(m_searchBar, &SearchBar::closeRequested, this,
          [this]() { m_editor->setFocus(); });
  mainLayout->addWidget(m_searchBar);
  // Size grip as overlay (not in layout) - positioned at bottom-right corner
  m_sizeGrip = new QSizeGrip(container);
  m_sizeGrip->setStyleSheet(
      "QSizeGrip { background: transparent; width: 16px; height: 16px; }");
  m_sizeGrip->setFixedSize(16, 16);
  m_sizeGrip->raise();                  // Ensure it's on top
  m_sizeGrip->installEventFilter(this); // For resize focus suppression
  setCentralWidget(container);

  // Connect editor changes
  connect(m_editor, &NoteEditor::contentChanged, this,
          &MainWindow::onEditorContentChanged);

  // Connect command popup signal
  connect(m_editor, &NoteEditor::commandExecuted, this,
          [this](const QString &command) {
            qDebug() << "MainWindow: Executing popup command:" << command;
            m_slashCommand->execute(command);
          });
}

void MainWindow::setupShortcuts() {
  Settings *s = Settings::instance();

  // Ctrl+N - New page
  QShortcut *newPage = new QShortcut(s->shortcut("newPage"), this);
  connect(newPage, &QShortcut::activated, m_noteManager,
          [this]() { m_noteManager->createNote(); });

  // Ctrl+W - Delete current page (with confirmation)
  QShortcut *deletePage = new QShortcut(s->shortcut("deletePage"), this);
  connect(deletePage, &QShortcut::activated, this, [this]() {
    // Suppress focus-out during dialog (prevent app from hiding)
    setSuppressFocusOut(true);

    // Show confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Delete Note"),
        tr("Are you sure you want to delete this note?\nThis action cannot be "
           "undone."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    // Re-enable focus-out
    setSuppressFocusOut(false);

    if (reply == QMessageBox::Yes) {
      m_noteManager->deleteNoteAt(m_noteManager->currentIndex());
      // If that was the last note, create a new empty one
      if (m_noteManager->noteCount() == 0) {
        m_noteManager->createNote();
      }
      // Keep focus on editor
      m_editor->setFocus();
    }
  });

  // Ctrl+Tab - Next page
  QShortcut *nextPage = new QShortcut(s->shortcut("nextPage"), this);
  connect(nextPage, &QShortcut::activated, m_noteManager,
          &NoteManager::nextNote);

  // Ctrl+Shift+Tab - Previous page
  QShortcut *prevPage = new QShortcut(s->shortcut("prevPage"), this);
  connect(prevPage, &QShortcut::activated, m_noteManager,
          &NoteManager::previousNote);

  // Ctrl+1..9 - Direct page selection
  for (int i = 1; i <= 9; ++i) {
    QShortcut *goTo =
        new QShortcut(s->shortcut(QString("goToPage%1").arg(i)), this);
    connect(goTo, &QShortcut::activated, this, [this, i]() {
      if (i <= m_noteManager->noteCount()) {
        m_noteManager->goToNote(i - 1);
      }
    });
  }

  // Ctrl+Home - Go to first page
  QShortcut *firstPage = new QShortcut(s->shortcut("firstPage"), this);
  connect(firstPage, &QShortcut::activated, this,
          [this]() { m_pageSelector->goToFirstPage(); });

  // Ctrl+End - Go to last page
  QShortcut *lastPage = new QShortcut(s->shortcut("lastPage"), this);
  connect(lastPage, &QShortcut::activated, this,
          [this]() { m_pageSelector->goToLastPage(); });

  // Ctrl+E - Export note
  QShortcut *exportShortcut = new QShortcut(s->shortcut("export"), this);
  connect(exportShortcut, &QShortcut::activated, this, &MainWindow::exportNote);

  // Ctrl+T - Toggle always on top
  QShortcut *alwaysOnTop = new QShortcut(s->shortcut("alwaysOnTop"), this);
  connect(alwaysOnTop, &QShortcut::activated, this,
          &MainWindow::toggleAlwaysOnTop);

  // Ctrl+F - Find (Antinote-style overlay in main window)
  QShortcut *findShortcut = new QShortcut(s->shortcut("find"), this);
  connect(findShortcut, &QShortcut::activated, this, [this]() {
    if (!m_searchModal) {
      m_searchModal = new SearchModal(m_noteManager, centralWidget());
      connect(m_searchModal, &SearchModal::noteSelected, this,
              [this](int index) {
                m_noteManager->setCurrentIndex(index);
                onCurrentNoteChanged(index);
              });
    }
    m_searchModal->showAndFocus();
  });

  // Ctrl+H - Find and Replace
  QShortcut *replaceShortcut = new QShortcut(s->shortcut("replace"), this);
  connect(replaceShortcut, &QShortcut::activated, this, [this]() {
    m_searchBar->setEditor(m_editor);
    m_searchBar->showReplace();
  });

  // Ctrl+L - Toggle lock/unlock
  QShortcut *toggleLock = new QShortcut(s->shortcut("toggleLock"), this);
  connect(toggleLock, &QShortcut::activated, this,
          [this]() { m_pageSelector->toggleLock(); });

  // Ctrl+Shift+O - OCR capture
  QShortcut *ocrShortcut = new QShortcut(s->shortcut("ocr"), this);
  connect(ocrShortcut, &QShortcut::activated, this, &MainWindow::captureOcr);

  // Ctrl+Shift+L (customizable) - Shorten selected URL
  QShortcut *shortenUrlShortcut =
      new QShortcut(s->shortcut("shortenUrl"), this);
  connect(shortenUrlShortcut, &QShortcut::activated, this,
          &MainWindow::shortenSelectedUrl);
}

void MainWindow::onCurrentNoteChanged(int index) {
  if (index < 0)
    return;

  Note note = m_noteManager->noteAt(index);

  // Check if note is locked (has password hash)
  if (note.isLocked()) {
    // Check if note is session unlocked (temporarily unlocked in this session)
    if (m_noteManager->isSessionUnlocked(note.id())) {
      // Show decrypted content from cache
      QString decrypted = m_noteManager->getDecryptedContent(note.id());
      m_updatingEditor = true;
      m_editor->setContent(decrypted);
      m_editor->setMode(note.mode());
      m_editor->setReadOnly(false);
      m_updatingEditor = false;
    } else {
      // Not session unlocked - show locked message
      m_updatingEditor = true;
      m_editor->setContent(
          tr("🔒 This note is locked.\n\nClick the lock button to unlock."));
      m_editor->setReadOnly(true);
      m_updatingEditor = false;
    }
  } else {
    m_updatingEditor = true;
    m_editor->setContent(note.content());
    m_editor->setMode(note.mode());
    m_editor->setReadOnly(false);
    m_updatingEditor = false;
  }

  qDebug() << "Switched to note:" << note.title()
           << "mode:" << noteModeName(note.mode());
}

void MainWindow::onNoteModeChanged(const QString &id, NoteMode mode) {
  // Update editor mode if this is the current note
  Note current = m_noteManager->currentNote();
  if (current.id() == id) {
    m_editor->setMode(mode);

    // Show timer widget when entering Timer mode
    if (mode == NoteMode::Timer) {
      if (!m_timerWidget) {
        m_timerWidget = new TimerWidget(this);
        // Insert timer widget above the editor
        QVBoxLayout *mainLayout =
            qobject_cast<QVBoxLayout *>(centralWidget()->layout());
        if (mainLayout) {
          // Find editor and insert timer before it
          for (int i = 0; i < mainLayout->count(); ++i) {
            if (mainLayout->itemAt(i)->widget() == m_editor) {
              mainLayout->insertWidget(i, m_timerWidget);
              break;
            }
          }
        }
      }
      m_timerWidget->show();
      m_timerWidget->setType(TimerType::Stopwatch); // Default type

      // Connect timer finished signal to confetti/notification
      connect(m_timerWidget, &TimerWidget::finished, this,
              &MainWindow::onTimerFinished, Qt::UniqueConnection);

      qDebug() << "MainWindow: Timer widget shown";
    }
    // NOTE: We don't hide the timer when switching modes/pages
    // Timer stays visible as long as it's running or wasn't closed
  }
}

void MainWindow::onEditorContentChanged() {
  if (m_updatingEditor)
    return;

  // Static guard to prevent recursive/re-entrant calls during heavy text
  // manipulation
  static bool isProcessing = false;
  if (isProcessing)
    return;
  isProcessing = true;

  // Check for keywords/commands - look at the last line after Enter press
  QString text = m_editor->content();
  if (text.endsWith('\n')) {
    // Get the last complete line (before the final newline)
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    if (!lines.isEmpty()) {
      QString lastLine = lines.last().trimmed();

      // Skip if this line was already processed as a keyword
      // (prevents infinite loop when highlighter triggers textChanged)
      static QString lastProcessedKeyword;
      static int lastLineCount = 0;

      if (lastLine == lastProcessedKeyword && lines.count() == lastLineCount) {
        // Same keyword, same line count - skip to prevent re-entrancy
        saveCurrentNoteContent();
        isProcessing = false;
        return;
      }

      // Check for Timer mode commands (without /)
      Note currentNote = m_noteManager->currentNote();
      if (currentNote.mode() == NoteMode::Timer && m_timerWidget) {
        QString cmd = lastLine.toLower().trimmed();
        bool timerCmdProcessed = false;

        if (cmd == "stopwatch") {
          m_timerWidget->setType(TimerType::Stopwatch);
          m_timerWidget->start();
          timerCmdProcessed = true;
          qDebug() << "Timer: Started stopwatch";
        } else if (cmd.startsWith("countdown")) {
          // Parse time: countdown 5 or countdown 30s or countdown 2m
          m_timerWidget->setType(TimerType::Countdown);
          // Extract time from command
          QRegularExpression re("countdown\\s+(\\d+)\\s*(s|m|min)?",
                                QRegularExpression::CaseInsensitiveOption);
          QRegularExpressionMatch match = re.match(cmd);
          if (match.hasMatch()) {
            int value = match.captured(1).toInt();
            QString unit = match.captured(2).toLower();
            if (unit == "s") {
              // Seconds - use setCountdownSeconds directly
              if (value > 0) {
                m_timerWidget->setCountdownSeconds(value);
              }
            } else {
              // Minutes (default)
              if (value > 0) {
                m_timerWidget->setCountdownMinutes(value);
              }
            }
          }
          m_timerWidget->start();
          timerCmdProcessed = true;
          qDebug() << "Timer: Started countdown";
        } else if (cmd == "pomodoro") {
          m_timerWidget->setType(TimerType::Pomodoro);
          m_timerWidget->start();
          timerCmdProcessed = true;
          qDebug() << "Timer: Started pomodoro";
        }

        if (timerCmdProcessed) {
          lastProcessedKeyword = lastLine;
          lastLineCount = lines.count();
          saveCurrentNoteContent();
          isProcessing = false;
          return;
        }
      }

      // Check for command (with / prefix) OR keyword (without /)
      if (m_slashCommand->isCommand(lastLine)) {
        qDebug() << "onEditorContentChanged: Found command/keyword:"
                 << lastLine;

        // Set guard BEFORE execute to prevent re-entrancy
        m_updatingEditor = true;
        lastProcessedKeyword = lastLine;
        lastLineCount = lines.count();

        // Execute the command but keep the keyword visible as section header
        m_slashCommand->execute(lastLine);

        m_updatingEditor = false;
        // Don't remove the keyword - it stays as a section marker
        isProcessing = false;
        return;
      }
    }
  }

  saveCurrentNoteContent();
  isProcessing = false;
}

void MainWindow::saveCurrentNoteContent() {
  if (m_noteManager->currentIndex() >= 0) {
    Note current = m_noteManager->currentNote();

    // NEVER save content for locked notes - the encrypted content must be
    // preserved
    if (current.isLocked()) {
      return;
    }

    // CRITICAL: Remove inline tutorial text before saving
    // This prevents tutorial placeholder text from being persisted
    m_editor->hideTutorial();

    m_noteManager->updateNoteContent(current.id(), m_editor->content());
  }
}

void MainWindow::toggleVisibility() {
  qDebug() << "toggleVisibility called, isVisible:" << isVisible()
           << "state:" << m_state;
  if (isVisible()) {
    qDebug() << "Hiding window...";
    hideWindow();
  } else {
    qDebug() << "Showing window...";
    showAndFocus();
  }
}

void MainWindow::showAndFocus() {
  qDebug() << "showAndFocus: called";
  m_autoPastedThisSession = false;

  // If "Open with new note" is enabled, create a new note each time
  if (Settings::instance()->openWithNewNote()) {
    m_noteManager->createNote();
    // Update editor with new note
    onCurrentNoteChanged(m_noteManager->currentIndex());
  }

  // Position window on the screen where the mouse cursor is
  QPoint cursorPos = QCursor::pos();
  QScreen *cursorScreen = QGuiApplication::screenAt(cursorPos);
  if (cursorScreen) {
    QRect screenGeom = cursorScreen->availableGeometry();
    // Center on that screen
    int x = screenGeom.x() + (screenGeom.width() - width()) / 2;
    int y = screenGeom.y() + (screenGeom.height() - height()) / 2;
    move(x, y);
    qDebug() << "showAndFocus: Positioned on screen" << cursorScreen->name()
             << "at" << x << y;
  }

  show();
  raise();
  activateWindow();
  m_editor->setFocus();

  // Perform auto-paste if enabled (delay slightly to ensure editor is ready)
  QTimer::singleShot(100, this, &MainWindow::performAutoPaste);
}

void MainWindow::hideWindow() {
  saveCurrentNoteContent();
  m_noteManager->saveAll();
  hide();
}

void MainWindow::exportNote() {
  // Show export format dialog
  QStringList formats = {tr("Text (.txt)"), tr("Markdown (.md)"),
                         tr("PDF (.pdf)"), tr("CSV (.csv)"),
                         tr("Export All Notes (ZIP)")};

  bool ok;
  QString format = QInputDialog::getItem(this, tr("Export Format"),
                                         tr("Select export format:"), formats,
                                         0, false, &ok);
  if (!ok || format.isEmpty())
    return;

  QString content = m_editor->content();

  if (format.contains("txt")) {
    Export::exportToTxt(content, this);
  } else if (format.contains("md")) {
    Export::exportToMarkdown(content, this);
  } else if (format.contains("pdf")) {
    Export::exportToPdf(content, this);
  } else if (format.contains("csv")) {
    Export::exportToCsv(content, this);
  } else if (format.contains("ZIP")) {
    // Export all notes
    QStringList titles;
    QStringList contents;
    for (int i = 0; i < m_noteManager->noteCount(); ++i) {
      Note note = m_noteManager->noteAt(i);
      titles << note.title();
      contents << note.content();
    }
    Export::exportAllToZip(titles, contents, this);
  }
}

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  setState(Visible);
}

void MainWindow::hideEvent(QHideEvent *event) {
  QMainWindow::hideEvent(event);
  setState(Hidden);
}

void MainWindow::focusInEvent(QFocusEvent *event) {
  QMainWindow::focusInEvent(event);
  if (isVisible()) {
    setState(VisibleFocused);
  }
}

void MainWindow::focusOutEvent(QFocusEvent *event) {
  QMainWindow::focusOutEvent(event);
  // Auto-hide when clicking outside the window
  if (isVisible() && event->reason() != Qt::PopupFocusReason) {
    hideWindow();
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  event->ignore();
  hideWindow();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    hideWindow();
    event->accept();
    return;
  }

  QMainWindow::keyPressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton && m_titleBar) {
    QPoint pos = event->pos();
    QRect titleRect = m_titleBar->geometry();

    if (titleRect.contains(pos)) {
      QWidget *child = childAt(pos);

      // Allow drag unless clicking on interactive widgets
      bool onInteractive = false;
      if (child) {
        onInteractive = qobject_cast<QPushButton *>(child) ||
                        qobject_cast<QComboBox *>(child) ||
                        child->inherits("QAbstractButton") ||
                        child->inherits("QComboBox") ||
                        child->inherits("PageSelector");
      }

      if (!onInteractive) {
        // ═══════════════════════════════════════════════════════════════════
        // Cross-Desktop Compatibility: Suppress focus-out during drag
        // On Wayland (especially GNOME), startSystemMove() transfers focus
        // to the compositor, triggering focusChanged. Without this flag,
        // the window would auto-hide during drag.
        // ═══════════════════════════════════════════════════════════════════
        m_suppressFocusOut = true;

        // Use Qt 5.15+ startSystemMove for Wayland compatibility
        if (windowHandle()) {
          windowHandle()->startSystemMove();
        }
        event->accept();
        return;
      }
    }
  }
  QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  // No longer needed with startSystemMove
  QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: Reset focus suppression after drag
  // This restores normal auto-hide behavior after window move completes.
  // Use a short delay to allow the compositor to return focus properly.
  // ═══════════════════════════════════════════════════════════════════════════
  if (m_suppressFocusOut) {
    QTimer::singleShot(100, this, [this]() { m_suppressFocusOut = false; });
  }
  QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);

  // Update size grip position (overlay at bottom-right corner)
  if (m_sizeGrip && centralWidget()) {
    int x = centralWidget()->width() - m_sizeGrip->width() - 4;
    int y = centralWidget()->height() - m_sizeGrip->height() - 4;
    m_sizeGrip->move(x, y);
    m_sizeGrip->raise();
  }
}
void MainWindow::setState(State newState) {
  if (m_state == newState) {
    return;
  }

  State oldState = m_state;
  m_state = newState;

  if (newState == VisibleFocused && oldState == Hidden) {
    performAutoPaste();
  }
}

void MainWindow::performAutoPaste() {
  qDebug() << "performAutoPaste: called";

  // Check if auto-paste is enabled in settings
  bool enabled = Settings::instance()->autoPasteEnabled();
  qDebug() << "performAutoPaste: enabled=" << enabled;
  if (!enabled) {
    return;
  }

  qDebug() << "performAutoPaste: m_autoPastedThisSession="
           << m_autoPastedThisSession;
  if (m_autoPastedThisSession) {
    return;
  }

  QString clipboardText = m_clipboardManager->getTextOnce();
  qDebug() << "performAutoPaste: clipboard text=" << clipboardText;

  if (clipboardText.isEmpty()) {
    return;
  }

  // Check if we should create a new note for clipboard content
  if (Settings::instance()->autoPasteCreateNewNote()) {
    m_noteManager->createNote();
    onCurrentNoteChanged(m_noteManager->currentIndex());
    m_editor->setContent(clipboardText);
  } else {
    // Append to current note if empty, otherwise append
    if (m_editor->content().isEmpty()) {
      m_editor->setContent(clipboardText);
    } else {
      m_editor->appendContent("\n" + clipboardText);
    }
  }

  m_autoPastedThisSession = true;
  qDebug() << "performAutoPaste: SUCCESS - pasted clipboard";
}

void MainWindow::checkForSlashCommand() {
  QString text = m_editor->content().trimmed();

  qDebug() << "checkForSlashCommand: text=" << text
           << "isCommand=" << m_slashCommand->isCommand(text);

  // Check if the entire content is just a command (with or without /)
  if (m_slashCommand->isCommand(text)) {
    qDebug() << "checkForSlashCommand: Executing command:" << text;
    if (m_slashCommand->execute(text)) {
      // Clear the command from editor
      m_editor->setContent("");
      saveCurrentNoteContent();
    }
  }
}

void MainWindow::openSettingsDialog() {
  // Suppress focus-out during settings dialog
  m_suppressFocusOut = true;

  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: Standalone Settings Dialog
  // Using nullptr as parent makes the dialog a separate window, allowing users
  // to see the main app while changing theme settings (live preview).
  // ═══════════════════════════════════════════════════════════════════════════
  SettingsDialog *dialog = new SettingsDialog(nullptr);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
  dialog->setWindowModality(Qt::NonModal); // Allow interaction with main window

  // Position the dialog near the main window
  QPoint mainCenter = geometry().center();
  dialog->move(mainCenter.x() + 50, mainCenter.y() - dialog->height() / 2);

  connect(dialog, &SettingsDialog::themeChanged, this, &MainWindow::applyTheme);
  connect(dialog, &SettingsDialog::themeChanged, m_editor,
          &NoteEditor::applyFontSettings);
  connect(dialog, &SettingsDialog::transparencyChanged, this,
          &MainWindow::applyTheme); // Re-apply theme with new transparency
  connect(dialog, &SettingsDialog::fontChanged, m_editor,
          &NoteEditor::applyFontSettings);

  // When dialog closes, restore focus handling
  connect(dialog, &QDialog::finished, this,
          [this]() { m_suppressFocusOut = false; });

  dialog->show();
  dialog->raise();
  dialog->activateWindow();
}

void MainWindow::applyTheme() {
  Settings *s = Settings::instance();
  ThemeManager &tm = ThemeManager::instance();
  tm.setCurrentTheme(s->themeName());
  tm.setDarkMode(s->darkMode());
  tm.setTransparency(s->transparency()); // Pass transparency to ThemeManager

  QString styleSheet = tm.generateStyleSheet();
  setStyleSheet(styleSheet);

  // Apply theme to ALL dialogs (QMessageBox, QInputDialog, etc)
  // This ensures consistency across the application
  ThemeColors c = tm.currentTheme().colors(s->darkMode());
  QString dialogStyle = QString(R"(
    QMessageBox, QInputDialog, QDialog {
      background-color: %1;
      color: %2;
      border: 1px solid %3;
      border-radius: 8px;
    }
    QMessageBox QLabel, QInputDialog QLabel {
      color: %2;
      background: transparent;
    }
    QMessageBox QPushButton, QInputDialog QPushButton {
      background-color: %4;
      color: %2;
      border: 1px solid %3;
      border-radius: 6px;
      padding: 6px 16px;
      min-width: 60px;
    }
    QMessageBox QPushButton:hover, QInputDialog QPushButton:hover {
      background-color: %5;
      border-color: %5;
    }
    QMessageBox QPushButton:pressed, QInputDialog QPushButton:pressed {
      background-color: %3;
    }
    QInputDialog QLineEdit {
      background-color: %4;
      color: %2;
      border: 1px solid %3;
      border-radius: 4px;
      padding: 6px;
    }
    QInputDialog QLineEdit:focus {
      border-color: %5;
    }
  )")
                            .arg(c.background.name())
                            .arg(c.foreground.name())
                            .arg(c.border.name())
                            .arg(c.secondary.name())
                            .arg(c.accent.name());

  qApp->setStyleSheet(dialogStyle);

  qDebug() << "MainWindow: Applied theme" << s->themeName()
           << (s->darkMode() ? "(dark)" : "(light)")
           << "transparency:" << s->transparency() << "%";
}

void MainWindow::toggleAlwaysOnTop() {
  m_alwaysOnTop = !m_alwaysOnTop;

  // Try KWin DBus scripting first (works on KDE Wayland)
  if (tryKWinKeepAbove(m_alwaysOnTop)) {
    qDebug() << "MainWindow: KWin DBus keepAbove toggled to" << m_alwaysOnTop;
    updatePinButtonState();
    return;
  }

#ifdef HAVE_KWINDOWSYSTEM
  // Use KWindowSystem for X11
  if (windowHandle()) {
    WId winId = windowHandle()->winId();
    if (m_alwaysOnTop) {
      KX11Extras::setState(winId, NET::KeepAbove);
    } else {
      KX11Extras::clearState(winId, NET::KeepAbove);
    }
    qDebug() << "MainWindow: KX11Extras keepAbove toggled to" << m_alwaysOnTop;
  }
#else
  // Fallback - toggle WindowStaysOnTopHint (may require window recreation)
  Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::Tool;
  if (m_alwaysOnTop) {
    flags |= Qt::WindowStaysOnTopHint;
  }
  setWindowFlags(flags);
  setAttribute(Qt::WA_TranslucentBackground);
  show();
  qDebug() << "MainWindow: Fallback WindowStaysOnTopHint toggled to"
           << m_alwaysOnTop;
#endif

  updatePinButtonState();
}

bool MainWindow::tryKWinKeepAbove(bool enable) {
  // Check if KWin is available via DBus
  QDBusInterface kwinScripting("org.kde.KWin", "/Scripting",
                               "org.kde.kwin.Scripting");
  if (!kwinScripting.isValid()) {
    qDebug() << "MainWindow: KWin DBus not available";
    return false;
  }

  // Create a temporary script with the correct keepAbove value
  // Plasma 6: Use activeWindow and also raise the window
  QString scriptContent =
      QString("// LinNote KWin Script - Toggle Keep Above\n"
              "var activeWin = workspace.activeWindow;\n"
              "if (activeWin) {\n"
              "    activeWin.keepAbove = %1;\n"
              "    if (%1) {\n"
              "        workspace.raiseWindow(activeWin);\n"
              "    }\n"
              "    print('LinNote KWin Script: Set keepAbove=%1, raised=' + %1 "
              "+ ' on: ' + activeWin.caption);\n"
              "} else {\n"
              "    print('LinNote KWin Script: No active window found');\n"
              "}\n")
          .arg(enable ? "true" : "false");

  // Write script to temp file
  QString tempDir =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  QString scriptPath = tempDir + "/linnote_keepabove.js";

  QFile file(scriptPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "MainWindow: Failed to write KWin script";
    return false;
  }
  file.write(scriptContent.toUtf8());
  file.close();

  // Unload previous script if exists
  kwinScripting.call("unloadScript", "linnote_keepabove");

  // Load and run the script
  QDBusReply<int> reply =
      kwinScripting.call("loadScript", scriptPath, "linnote_keepabove");
  if (!reply.isValid()) {
    qDebug() << "MainWindow: KWin loadScript failed:"
             << reply.error().message();
    return false;
  }

  // Start the script
  kwinScripting.call("start");

  // Clean up: unload after a short delay to let it run
  QTimer::singleShot(500, [&kwinScripting]() {
    QDBusInterface kwin("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting");
    kwin.call("unloadScript", "linnote_keepabove");
  });

  return true;
}

void MainWindow::updatePinButtonState() {
  if (!m_pinButton)
    return;

  // Common styles
  QString normalStyle = R"(
    QPushButton {
      background-color: transparent;
      border: none;
      border-radius: 4px;
      padding: 2px;
    }
    QPushButton:hover { background-color: rgba(128, 128, 128, 0.2); }
  )";

  QString pinnedStyle = R"(
    QPushButton {
      background-color: rgba(243, 139, 168, 0.4);
      border: none;
      border-radius: 4px;
      padding: 2px;
    }
    QPushButton:hover { background-color: rgba(243, 139, 168, 0.6); }
  )";

  if (m_alwaysOnTop) {
    m_pinButton->setText(QStringLiteral("📍")); // Pinned emoji
    m_pinButton->setStyleSheet(pinnedStyle);

    // ═══════════════════════════════════════════════════════════════════════
    // Cross-Desktop Compatibility: Professional DE-Aware Tooltips
    // KDE: True always-on-top (z-order control via KWin)
    // GNOME/Others: Stay Visible mode (prevents auto-hide, no z-order control)
    // ═══════════════════════════════════════════════════════════════════════
    if (LinNote::supportsAlwaysOnTop()) {
      // KDE Plasma: Full always-on-top support via KWin DBus scripting
      m_pinButton->setToolTip(tr("Always on Top: ON (Ctrl+T)"));
    } else {
      // GNOME and others: Stay Visible mode (prevents auto-hide)
      // This is a professional alternative, not a "workaround"
      m_pinButton->setToolTip(
          tr("Stay Visible: ON (Ctrl+T)\n"
             "Window won't auto-hide when clicking outside.\n"
             "(True always-on-top is not available on %1)")
              .arg(LinNote::desktopEnvironmentName()));
    }
  } else {
    m_pinButton->setText(QStringLiteral("📌")); // Unpinned emoji
    m_pinButton->setStyleSheet(normalStyle);

    if (LinNote::supportsAlwaysOnTop()) {
      m_pinButton->setToolTip(tr("Always on Top: OFF (Ctrl+T)"));
    } else {
      m_pinButton->setToolTip(tr("Stay Visible: OFF (Ctrl+T)"));
    }
  }
}

void MainWindow::updateToolbarVisibility() {
  if (m_titleBar) {
    if (Settings::instance()->toolbarAutoHide()) {
      m_titleBar->setVisible(false); // Completely hide
    } else {
      m_titleBar->setVisible(true);
      m_titleBar->setFixedHeight(36);
    }
  }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
  // ═══════════════════════════════════════════════════════════════════════════
  // Cross-Desktop Compatibility: Suppress focus-out during resize
  // On Wayland (especially GNOME), QSizeGrip interaction may transfer focus
  // to the compositor, triggering auto-hide. We suppress this during resize.
  // ═══════════════════════════════════════════════════════════════════════════
  if (watched == m_sizeGrip) {
    if (event->type() == QEvent::MouseButtonPress) {
      m_suppressFocusOut = true;
    } else if (event->type() == QEvent::MouseButtonRelease) {
      // Delay restoration to allow compositor to return focus
      QTimer::singleShot(100, this, [this]() { m_suppressFocusOut = false; });
    }
  }

  // Auto-hide: show on enter main window top area, hide after delay
  if (Settings::instance()->toolbarAutoHide() && m_titleBar) {
    if (event->type() == QEvent::MouseMove) {
      QMouseEvent *me = static_cast<QMouseEvent *>(event);
      // Check if mouse is in top 10px of window
      if (me->pos().y() < 10 && !m_titleBar->isVisible()) {
        m_titleBar->setVisible(true);
        m_titleBar->setFixedHeight(36);
        // Auto-hide after 3 seconds
        QTimer::singleShot(3000, this, [this]() {
          if (Settings::instance()->toolbarAutoHide() && m_titleBar) {
            // Only hide if mouse is not in toolbar area
            QPoint globalMousePos = QCursor::pos();
            QPoint localPos = m_titleBar->mapFromGlobal(globalMousePos);
            if (!m_titleBar->rect().contains(localPos)) {
              m_titleBar->setVisible(false);
            }
          }
        });
      }
    }
  }
  return QMainWindow::eventFilter(watched, event);
}

void MainWindow::startAutoLockTimer() {
  m_autoLockTimer->stop(); // Reset any existing timer

  int minutes = Settings::instance()->autoLockMinutes();

  // Special values: 0 = never, -1 = until restart (no timer needed)
  if (minutes <= 0) {
    qDebug() << "Auto-lock timer: disabled (minutes =" << minutes << ")";
    return;
  }

  int msecs = minutes * 60 * 1000;
  m_autoLockTimer->start(msecs);
  qDebug() << "Auto-lock timer: started for" << minutes << "minutes";
}

void MainWindow::onAutoLockTimeout() {
  qDebug() << "Auto-lock timer: timeout! Locking all session-unlocked notes";

  // Clear all session unlocks
  m_noteManager->clearSessionUnlocks();

  // If current note was session unlocked, show locked message
  Note current = m_noteManager->currentNote();
  if (current.isLocked()) {
    m_updatingEditor = true;
    m_editor->setContent(
        tr("🔒 This note is locked.\n\nClick the lock button to unlock."));
    m_editor->setReadOnly(true);
    m_updatingEditor = false;
  }

  // Update UI
  m_pageSelector->updateLockButton();
  m_pageSelector->updateList();
}

void MainWindow::captureOcr() {
  qDebug() << "Starting OCR capture...";

  // Check if Tesseract is available
  if (!OcrHelper::isAvailable()) {
    QMessageBox::warning(
        this, tr("OCR"),
        tr("Tesseract is not installed.\n\n"
           "Install with:\n\n"
           "Ubuntu/Debian:\n"
           "  sudo apt install tesseract-ocr tesseract-ocr-eng\n\n"
           "Fedora:\n"
           "  sudo dnf install tesseract tesseract-langpack-eng\n\n"
           "Arch Linux:\n"
           "  sudo pacman -S tesseract tesseract-data-eng\n\n"
           "openSUSE:\n"
           "  sudo zypper install tesseract-ocr "
           "tesseract-ocr-traineddata-english"));
    return;
  }

  // Get OCR language from settings
  QString lang = Settings::instance()->ocrLanguage();
  if (lang.isEmpty()) {
    lang = "eng";
  }

  // Create OCR helper and start capture
  OcrHelper *ocr = new OcrHelper(this);

  connect(ocr, &OcrHelper::textExtracted, this,
          [this, ocr](const QString &text) {
            if (!text.isEmpty()) {
              showAndFocus();

              // Check if we should create a new note
              if (Settings::instance()->ocrCreateNewNote()) {
                m_noteManager->createNote(); // Create new note
                m_pageSelector->updateList();
                m_editor->setContent(text);
              } else {
                // Append to current note
                m_editor->insertPlainText(text);
              }
              qDebug() << "OCR text inserted:" << text.left(50) << "...";
            }
            ocr->deleteLater();
          });

  connect(ocr, &OcrHelper::errorOccurred, this,
          [this, ocr](const QString &error) {
            QMessageBox::warning(this, tr("OCR Error"), error);
            ocr->deleteLater();
          });

  ocr->captureAndExtract(lang);
}

void MainWindow::createNewNoteWithText(const QString &text) {
  qDebug() << "createNewNoteWithText called with:" << text;
  m_noteManager->createNote();
  onCurrentNoteChanged(m_noteManager->currentIndex());
  if (!text.isEmpty()) {
    m_editor->setContent(text);
  }
}

void MainWindow::searchNotes(const QString &query) {
  qDebug() << "searchNotes called with:" << query;
  if (!query.isEmpty()) {
    m_searchBar->setSearchText(query);
    m_searchBar->showSearch();
  }
}

void MainWindow::shortenSelectedUrl() {
  // Check if link shortening is enabled
  if (!Settings::instance()->linkAutoShortenEnabled()) {
    QMessageBox::information(
        this, tr("URL Shortener"),
        tr("Link shortening is disabled.\n"
           "Enable it in Settings → Editor → Enable link shortening"));
    return;
  }

  QTextCursor cursor = m_editor->textCursor();
  QString selectedText = cursor.selectedText().trimmed();

  if (selectedText.isEmpty()) {
    return;
  }

  // Check if it looks like a URL
  if (!selectedText.startsWith("http://") &&
      !selectedText.startsWith("https://") &&
      !selectedText.startsWith("www.")) {
    QMessageBox::information(this, tr("URL Shortener"),
                             tr("Please select a valid URL"));
    return;
  }

  // Add http:// if needed
  QString url = selectedText;
  if (url.startsWith("www.")) {
    url = "https://" + url;
  }

  // Check if URL is already shortened
  static QRegularExpression shortUrlPattern(
      R"(https?://(is\.gd|v\.gd|tinyurl\.com|bit\.ly|t\.co|goo\.gl|ow\.ly|buff\.ly)/)",
      QRegularExpression::CaseInsensitiveOption);
  if (shortUrlPattern.match(url).hasMatch()) {
    QMessageBox::information(this, tr("URL Shortener"),
                             tr("This URL is already shortened!"));
    return;
  }

  // Get service from settings
  QString serviceName = Settings::instance()->urlShortenerService();
  UrlShortener::Service service = UrlShortener::stringToService(serviceName);

  auto *shortener = new UrlShortener(this);

  connect(shortener, &UrlShortener::shortened, this,
          [this, shortener, cursor](const QString &shortUrl) {
            // Replace selected text with short URL
            QTextCursor c = cursor;
            c.insertText(shortUrl);
            shortener->deleteLater();
            qDebug() << "URL shortened to:" << shortUrl;
          });

  connect(shortener, &UrlShortener::error, this,
          [this, shortener](const QString &msg) {
            QMessageBox::warning(this, tr("URL Shortener"), msg);
            shortener->deleteLater();
          });

  shortener->shortenUrl(url, service);
}

void MainWindow::deleteAllNotes() {
  qDebug() << "MainWindow: Deleting all notes...";
  m_noteManager->deleteAllNotes();
  // Update editor with fresh note content
  m_updatingEditor = true;
  m_editor->setPlainText(m_noteManager->currentNote().content());
  m_updatingEditor = false;
  m_editor->setFocus();
}

void MainWindow::onTimerFinished() {
  qDebug() << "Timer finished! Showing confetti and notification.";

  // Show confetti animation (safely)
  try {
    if (!m_confetti) {
      m_confetti = new ConfettiWidget(centralWidget());
    }
    m_confetti->start();
  } catch (...) {
    qDebug() << "Confetti failed, skipping";
  }

  // Show system notification (Linux desktop notification via D-Bus)
  QDBusInterface notify(
      "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
      "org.freedesktop.Notifications", QDBusConnection::sessionBus());

  if (notify.isValid()) {
    QVariantList args;
    args << "LinNote";                                 // app_name
    args << (uint)0;                                   // replaces_id
    args << "";                                        // app_icon
    args << tr("⏰ Timer Finished!");                  // summary
    args << tr("Your countdown timer has completed."); // body
    args << QStringList();                             // actions
    args << QVariantMap();                             // hints
    args << 5000;                                      // timeout (5 seconds)

    notify.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
  }

  // Play system alert sound
  QApplication::beep();

  // Also flash the window title bar
  QApplication::alert(this, 3000);
}

void MainWindow::ensureTimerWidget() {
  if (m_timerWidget) {
    m_timerWidget->show();
    return;
  }

  m_timerWidget = new TimerWidget(this);
  // Insert timer widget above the editor
  QVBoxLayout *mainLayout =
      qobject_cast<QVBoxLayout *>(centralWidget()->layout());
  if (mainLayout) {
    // Find editor and insert timer before it
    for (int i = 0; i < mainLayout->count(); ++i) {
      if (mainLayout->itemAt(i)->widget() == m_editor) {
        mainLayout->insertWidget(i, m_timerWidget);
        break;
      }
    }
  }
  m_timerWidget->show();

  // Connect timer finished signal to confetti/notification
  connect(m_timerWidget, &TimerWidget::finished, this,
          &MainWindow::onTimerFinished, Qt::UniqueConnection);
}

#ifdef HAVE_KGLOBALACCEL
void MainWindow::registerKdeGlobalShortcut() {
  // Clean old shortcuts when updating (not on first registration)
  if (m_toggleAction) {
    KGlobalAccel::cleanComponent(QStringLiteral("LinNote"));
    KGlobalAccel::cleanComponent(QStringLiteral("linnote.desktop"));
  }

  // Create toggle action once, then reuse it
  if (!m_toggleAction) {
    m_toggleAction = new QAction(this);
    m_toggleAction->setObjectName(QStringLiteral("Toggle"));
    m_toggleAction->setText(tr("Toggle LinNote"));
    connect(m_toggleAction, &QAction::triggered, this,
            &MainWindow::toggleVisibility);
  }

  // Get hotkey from settings and convert to QKeySequence
  QString hotkey = Settings::instance()->globalHotkey();
  QString qtHotkey = hotkey;
  qtHotkey.replace("<Super>", "Meta+");
  qtHotkey.replace("<Control>", "Ctrl+");
  qtHotkey.replace("<Alt>", "Alt+");
  qtHotkey.replace("<Shift>", "Shift+");
  qtHotkey.replace("<", "").replace(">", "");
  // Capitalize last character
  if (!qtHotkey.isEmpty() && qtHotkey != "F12") {
    qtHotkey =
        qtHotkey.left(qtHotkey.length() - 1) + qtHotkey.right(1).toUpper();
  }

  QKeySequence keySeq(qtHotkey);
  m_toggleAction->setShortcut(keySeq);

  // Set both default AND active shortcut with NoAutoloading to force it
  QList<QKeySequence> shortcuts = QList<QKeySequence>() << keySeq;
  KGlobalAccel::self()->setDefaultShortcut(m_toggleAction, shortcuts);
  KGlobalAccel::self()->setShortcut(m_toggleAction, shortcuts,
                                    KGlobalAccel::NoAutoloading);

  qDebug() << "KDE global shortcut updated to:" << qtHotkey;
}
#endif
