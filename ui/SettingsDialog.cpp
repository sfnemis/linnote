#include "SettingsDialog.h"
#include "MainWindow.h"
#include "PasswordDialog.h"
#include "core/CurrencyConverter.h"
#include "core/OcrHelper.h"
#include "core/Settings.h"
#include "core/UpdateChecker.h"
#include "storage/BackupManager.h"
#include "storage/Crypto.h"
#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

// Forward declarations
static QString generateRecoveryKey();
static void showRecoveryKeyDialog(QWidget *parent, const QString &title,
                                  const QString &message,
                                  const QString &recoveryKey);

// Helper: Convert Qt QKeySequence to XDG Portal format
static QString qtToPortalHotkey(const QKeySequence &seq) {
  if (seq.isEmpty())
    return "<Super><Control>n";

  QString str = seq.toString(QKeySequence::PortableText);
  str.replace("Ctrl+", "<Control>");
  str.replace("Alt+", "<Alt>");
  str.replace("Shift+", "<Shift>");
  str.replace("Meta+", "<Super>");
  if (!str.isEmpty() && str.back().isUpper()) {
    str = str.left(str.length() - 1) + str.back().toLower();
  }
  return str;
}

// Helper: Convert XDG Portal format to Qt QKeySequence
static QKeySequence portalToQtHotkey(const QString &portal) {
  QString str = portal;
  str.replace("<Control>", "Ctrl+");
  str.replace("<Alt>", "Alt+");
  str.replace("<Shift>", "Shift+");
  str.replace("<Super>", "Meta+");
  if (!str.isEmpty() && str.back().isLower()) {
    str = str.left(str.length() - 1) + str.back().toUpper();
  }
  return QKeySequence(str);
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("LinNote Settings"));
  setupUi();
  loadSettings();
  updateDynamicStyles();
  setMinimumSize(560, 520);
  resize(600, 550);

  // Connect to settings changes for dynamic theme updates
  connect(Settings::instance(), &Settings::settingsChanged, this,
          &SettingsDialog::updateDynamicStyles);
}

void SettingsDialog::setupUi() {
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // === SIDEBAR ===
  m_sidebar = new QListWidget(this);
  m_sidebar->setFixedWidth(140);
  m_sidebar->setIconSize(QSize(20, 20));
  m_sidebar->setSpacing(4);
  m_sidebar->setFrameShape(QFrame::NoFrame);
  // Styling is handled by updateDynamicStyles() for dark/light mode support
  m_sidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  m_sidebar->addItem("🎨 Visual");
  m_sidebar->addItem("✏️ Editor");
  m_sidebar->addItem("💾 Backup");
  m_sidebar->addItem("🔒 Security");
  m_sidebar->addItem("⌨️ Shortcuts");
  m_sidebar->addItem("🧮 Calculator");
  m_sidebar->addItem("⏱️ Timer");
  m_sidebar->addItem("📷 OCR");
  m_sidebar->addItem("🏷️ Keywords");
  m_sidebar->addItem("ℹ️ About");

  connect(m_sidebar, &QListWidget::currentRowChanged, this,
          &SettingsDialog::onCategoryChanged);

  mainLayout->addWidget(m_sidebar);

  // === PAGES ===
  m_pages = new QStackedWidget(this);
  // No hardcoded background - inherit from app theme

  m_pages->addWidget(createVisualPage());
  m_pages->addWidget(createEditorPage());
  m_pages->addWidget(createBackupPage());
  m_pages->addWidget(createSecurityPage());
  m_pages->addWidget(createShortcutsPage());
  m_pages->addWidget(createCalculatorPage());
  m_pages->addWidget(createTimerPage());
  m_pages->addWidget(createOcrPage());
  m_pages->addWidget(createKeywordsPage());
  m_pages->addWidget(createAboutPage());

  mainLayout->addWidget(m_pages, 1);

  m_sidebar->setCurrentRow(0);
}

void SettingsDialog::onCategoryChanged(int row) {
  m_pages->setCurrentIndex(row);
}

void SettingsDialog::updateDynamicStyles() {
  bool isDark = Settings::instance()->darkMode();

  // Sidebar dynamic styling
  QString textColor = isDark ? "#cdd6f4" : "#2e3436";
  QString hoverBg = isDark ? "rgba(255,255,255,0.08)" : "rgba(0,0,0,0.08)";
  QString hoverBorder = isDark ? "rgba(255,255,255,0.1)" : "rgba(0,0,0,0.1)";

  m_sidebar->setStyleSheet(
      QString("QListWidget { border: none; padding: 8px; "
              "border-right: 1px solid palette(mid); }"
              "QListWidget::item { padding: 10px 8px; border-radius: 8px; "
              "margin: 1px 4px; border: 1px solid transparent; color: %1; }"
              "QListWidget::item:hover { background: %2; "
              "border: 1px solid %3; color: %1; }"
              "QListWidget::item:selected { background-color: #3584e4; "
              "color: #ffffff; font-weight: bold; "
              "border: 1px solid #3584e4; }")
          .arg(textColor, hoverBg, hoverBorder));
}

// === THEME PAGE ===
QWidget *SettingsDialog::createVisualPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(16);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("🎨 Theme"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  layout->addWidget(title);

  // Color scheme
  QHBoxLayout *row1 = new QHBoxLayout();
  row1->addWidget(new QLabel(tr("Color scheme:")));
  m_themeCombo = new QComboBox();
  m_themeCombo->addItems({"Catppuccin", "Tokyo Drift", "Vancouver", "Totoro",
                          "Mononoke", "Piccolo", "Nord", "Dracula", "Gruvbox",
                          "Solarized", "One Dark", "Monokai", "Breeze",
                          "Adwaita"});
  connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::onThemeChanged);
  row1->addWidget(m_themeCombo);
  row1->addStretch();
  layout->addLayout(row1);

  // Dark mode
  m_darkModeCheck = new QCheckBox(tr("Dark mode"));
  connect(m_darkModeCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onDarkModeToggled);
  layout->addWidget(m_darkModeCheck);

  // Transparency (apply on release to avoid stutter)
  QHBoxLayout *row2 = new QHBoxLayout();
  row2->addWidget(new QLabel(tr("Transparency:")));
  m_transparencySlider = new QSlider(Qt::Horizontal);
  m_transparencySlider->setRange(10, 100);
  m_transparencySlider->setValue(100);

  // Label updates immediately (cheap operation)
  QLabel *transLabel = new QLabel("100%");
  connect(m_transparencySlider, &QSlider::valueChanged,
          [transLabel](int v) { transLabel->setText(QString("%1%").arg(v)); });

  // Actual transparency change only on release (expensive operation)
  connect(m_transparencySlider, &QSlider::sliderReleased, this, [this]() {
    int value = m_transparencySlider->value();
    Settings::instance()->setTransparency(value);
    emit transparencyChanged(value);
  });

  row2->addWidget(m_transparencySlider);
  row2->addWidget(transLabel);
  layout->addLayout(row2);

  // Display mode (Tray Only, Dock Only, Both)
  QHBoxLayout *displayRow = new QHBoxLayout();
  displayRow->addWidget(new QLabel(tr("Display mode:")));
  m_displayModeCombo = new QComboBox();
  m_displayModeCombo->addItem(tr("Tray Only"), Settings::TrayOnly);
  m_displayModeCombo->addItem(tr("Dock Only"), Settings::DockOnly);
  m_displayModeCombo->addItem(tr("Both"), Settings::Both);
  connect(m_displayModeCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [](int index) {
            Settings::instance()->setDisplayMode(
                static_cast<Settings::DisplayMode>(index));
          });
  displayRow->addWidget(m_displayModeCombo);
  displayRow->addStretch();
  layout->addLayout(displayRow);

  // Start on boot
  m_startOnBootCheck = new QCheckBox(tr("Start LinNote when computer starts"));
  m_startOnBootCheck->setChecked(isAutostartEnabled());
  connect(m_startOnBootCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onStartOnBootToggled);
  layout->addWidget(m_startOnBootCheck);

  // Restart notice
  QLabel *restartNote =
      new QLabel(tr("⚠️ Display mode change requires restart"));
  restartNote->setStyleSheet("font-size: 11px; color: #888;");
  layout->addWidget(restartNote);

  layout->addStretch();
  return page;
}

// === EDITOR PAGE ===
QWidget *SettingsDialog::createEditorPage() {
  // Outer container with scroll
  QWidget *page = new QWidget();
  QVBoxLayout *outerLayout = new QVBoxLayout(page);
  outerLayout->setContentsMargins(0, 0, 0, 0);

  QScrollArea *scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QWidget *content = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(content);
  layout->setSpacing(10);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("✏️ Editor"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  layout->addWidget(title);

  // Font family
  QHBoxLayout *fontRow = new QHBoxLayout();
  fontRow->addWidget(new QLabel(tr("Font:")));
  m_fontCombo = new QComboBox();
  QFontDatabase fontDb;
  m_fontCombo->addItems(fontDb.families());
  connect(m_fontCombo, &QComboBox::currentTextChanged, this,
          &SettingsDialog::onFontFamilyChanged);
  fontRow->addWidget(m_fontCombo);

  m_fontSizeSpin = new QSpinBox();
  m_fontSizeSpin->setRange(8, 32);
  m_fontSizeSpin->setSuffix(" pt");
  connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingsDialog::onFontSizeChanged);
  fontRow->addWidget(m_fontSizeSpin);
  fontRow->addStretch();
  layout->addLayout(fontRow);

  // Font preview - compact with word wrap
  m_fontPreview = new QLabel(tr("Abc 123"));
  m_fontPreview->setWordWrap(true);
  m_fontPreview->setMaximumHeight(60);
  m_fontPreview->setStyleSheet(
      "background: palette(base); color: palette(text); "
      "padding: 8px; border-radius: 6px;");
  layout->addWidget(m_fontPreview);

  // Note Title Mode
  auto *titleRow = new QHBoxLayout();
  titleRow->addWidget(new QLabel(tr("Note Title:")));
  auto *titleModeCombo = new QComboBox();
  titleModeCombo->addItem(tr("🧠 Smart (auto-detect)"), 0);
  titleModeCombo->addItem(tr("📅 Date & Time"), 1);
  titleModeCombo->addItem(tr("✏️ First Line"), 2);
  titleModeCombo->setCurrentIndex(Settings::instance()->noteTitleMode());
  connect(titleModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [](int index) { Settings::instance()->setNoteTitleMode(index); });
  titleRow->addWidget(titleModeCombo);
  titleRow->addStretch();
  layout->addLayout(titleRow);

  // Default Mode (new notes open in this mode)
  auto *modeRow = new QHBoxLayout();
  modeRow->addWidget(new QLabel(tr("Default Mode:")));
  auto *defaultModeCombo = new QComboBox();
  // NoteMode enum: PlainText=0, Checklist=1, Code=2, Math=3, Timer=4,
  // Markdown=5
  defaultModeCombo->addItem(tr("📝 Plain Text"), 0);
  defaultModeCombo->addItem(tr("✅ Checklist"), 1);
  defaultModeCombo->addItem(tr("💻 Code"), 2);
  defaultModeCombo->addItem(tr("🧮 Calc"), 3); // Math mode (incl. currency)
  defaultModeCombo->addItem(tr("⏱️ Timer"), 4);
  defaultModeCombo->addItem(tr("📑 Markdown"), 5);
  defaultModeCombo->setCurrentIndex(Settings::instance()->defaultMode());
  connect(defaultModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [](int index) { Settings::instance()->setDefaultMode(index); });
  modeRow->addWidget(defaultModeCombo);
  modeRow->addStretch();
  layout->addLayout(modeRow);

  // Behavior toggles
  m_autoPasteCheck = new QCheckBox(tr("Auto-paste clipboard on open"));
  connect(m_autoPasteCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onAutoPasteToggled);
  layout->addWidget(m_autoPasteCheck);

  m_autoPasteCreateNewNoteCheck =
      new QCheckBox(tr("  ↳ Create new note for paste"));
  m_autoPasteCreateNewNoteCheck->setChecked(
      Settings::instance()->autoPasteCreateNewNote());
  connect(m_autoPasteCreateNewNoteCheck, &QCheckBox::toggled, this,
          [](bool checked) {
            Settings::instance()->setAutoPasteCreateNewNote(checked);
          });
  layout->addWidget(m_autoPasteCreateNewNoteCheck);

  m_openWithNewNoteCheck = new QCheckBox(tr("Open with new note"));
  connect(m_openWithNewNoteCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onOpenWithNewNoteToggled);
  layout->addWidget(m_openWithNewNoteCheck);

  m_autoCheckboxCheck = new QCheckBox(tr("Auto-checkbox in list mode"));
  connect(m_autoCheckboxCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onAutoCheckboxToggled);
  layout->addWidget(m_autoCheckboxCheck);

  // === Paste Cleanup Section ===
  auto *pasteHeader = new QLabel(tr("📋 Paste Cleanup"));
  pasteHeader->setStyleSheet("font-weight: bold; margin-top: 16px;");
  layout->addWidget(pasteHeader);

  auto *pasteDesc = new QLabel(tr("Auto-clean pasted content:"));
  pasteDesc->setStyleSheet("color: #aaa; font-size: 11px;");
  layout->addWidget(pasteDesc);

  Settings *s = Settings::instance();

  auto *removeSpacesCheck = new QCheckBox(tr("Remove leading spaces/tabs"));
  removeSpacesCheck->setChecked(s->pasteRemoveLeadingSpaces());
  connect(removeSpacesCheck, &QCheckBox::toggled, [](bool checked) {
    Settings::instance()->setPasteRemoveLeadingSpaces(checked);
  });
  layout->addWidget(removeSpacesCheck);

  auto *removeNumbersCheck =
      new QCheckBox(tr("Remove leading numbers (1. 2.)"));
  removeNumbersCheck->setChecked(s->pasteRemoveNumbers());
  connect(removeNumbersCheck, &QCheckBox::toggled, [](bool checked) {
    Settings::instance()->setPasteRemoveNumbers(checked);
  });
  layout->addWidget(removeNumbersCheck);

  auto *removeBulletsCheck = new QCheckBox(tr("Remove bullets (•, -, *)"));
  removeBulletsCheck->setChecked(s->pasteRemoveBullets());
  connect(removeBulletsCheck, &QCheckBox::toggled, [](bool checked) {
    Settings::instance()->setPasteRemoveBullets(checked);
  });
  layout->addWidget(removeBulletsCheck);

  auto *removeMarkdownCheck = new QCheckBox(tr("Remove markdown formatting"));
  removeMarkdownCheck->setChecked(s->pasteRemoveMarkdown());
  connect(removeMarkdownCheck, &QCheckBox::toggled, [](bool checked) {
    Settings::instance()->setPasteRemoveMarkdown(checked);
  });
  layout->addWidget(removeMarkdownCheck);

  auto *removeEmptyCheck = new QCheckBox(tr("Remove empty lines"));
  removeEmptyCheck->setChecked(s->pasteRemoveEmptyLines());
  connect(removeEmptyCheck, &QCheckBox::toggled, [](bool checked) {
    Settings::instance()->setPasteRemoveEmptyLines(checked);
  });
  layout->addWidget(removeEmptyCheck);

  // URL Shortener service
  auto *shortenerLabel = new QLabel(tr("URL Shortener:"));
  layout->addWidget(shortenerLabel);

  m_urlShortenerCombo = new QComboBox();
  m_urlShortenerCombo->addItems({"is.gd", "TinyURL", "v.gd"});
  m_urlShortenerCombo->setCurrentText(
      Settings::instance()->urlShortenerService());
  connect(m_urlShortenerCombo, &QComboBox::currentTextChanged, this,
          [](const QString &text) {
            Settings::instance()->setUrlShortenerService(text);
          });
  layout->addWidget(m_urlShortenerCombo);

  auto *linkAutoShortenCheck =
      new QCheckBox(tr("Enable link shortening (Ctrl+Shift+L)"));
  linkAutoShortenCheck->setChecked(
      Settings::instance()->linkAutoShortenEnabled());
  connect(linkAutoShortenCheck, &QCheckBox::toggled, [](bool checked) {
    Settings::instance()->setLinkAutoShortenEnabled(checked);
  });
  layout->addWidget(linkAutoShortenCheck);

  layout->addStretch();

  // Connect scroll area
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}

// === BACKUP PAGE ===
QWidget *SettingsDialog::createBackupPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(12);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("💾 Backup"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  layout->addWidget(title);

  m_backupEnabledCheck = new QCheckBox(tr("Enable automatic backups"));
  connect(m_backupEnabledCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onBackupEnabledToggled);
  layout->addWidget(m_backupEnabledCheck);

  // Folder
  QHBoxLayout *folderRow = new QHBoxLayout();
  folderRow->addWidget(new QLabel(tr("Folder:")));
  m_backupPathEdit = new QLineEdit();
  m_backupPathEdit->setReadOnly(true);
  folderRow->addWidget(m_backupPathEdit);
  m_backupBrowseBtn = new QPushButton(tr("Browse..."));
  connect(m_backupBrowseBtn, &QPushButton::clicked, this,
          &SettingsDialog::onBackupBrowseClicked);
  folderRow->addWidget(m_backupBrowseBtn);
  layout->addLayout(folderRow);

  // Interval
  QHBoxLayout *intervalRow = new QHBoxLayout();
  intervalRow->addWidget(new QLabel(tr("Backup every:")));
  m_backupIntervalSpin = new QSpinBox();
  m_backupIntervalSpin->setRange(1, 24);
  m_backupIntervalSpin->setSuffix(tr(" hours"));
  connect(m_backupIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &SettingsDialog::onBackupIntervalChanged);
  intervalRow->addWidget(m_backupIntervalSpin);
  intervalRow->addStretch();
  layout->addLayout(intervalRow);

  // Retention
  QHBoxLayout *retentionRow = new QHBoxLayout();
  retentionRow->addWidget(new QLabel(tr("Keep last:")));
  m_backupRetentionSpin = new QSpinBox();
  m_backupRetentionSpin->setRange(1, 100);
  m_backupRetentionSpin->setSuffix(tr(" backups"));
  connect(m_backupRetentionSpin, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &SettingsDialog::onBackupRetentionChanged);
  retentionRow->addWidget(m_backupRetentionSpin);
  retentionRow->addStretch();
  layout->addLayout(retentionRow);

  // Backup Now
  m_backupNowBtn = new QPushButton(tr("📦 Backup Now"));
  connect(m_backupNowBtn, &QPushButton::clicked, this,
          &SettingsDialog::onBackupNowClicked);
  layout->addWidget(m_backupNowBtn);

  layout->addSpacing(20);

  // Danger Zone
  QLabel *dangerLabel = new QLabel(tr("⚠️ Danger Zone"));
  dangerLabel->setStyleSheet(
      "font-size: 14px; font-weight: bold; color: #f38ba8;");
  layout->addWidget(dangerLabel);

  QPushButton *deleteAllBtn = new QPushButton(tr("🗑️ Delete All Notes"));
  deleteAllBtn->setStyleSheet(
      "QPushButton { background-color: #f38ba8; color: #1e1e2e; "
      "font-weight: bold; padding: 8px 16px; border-radius: 4px; }"
      "QPushButton:hover { background-color: #eba0ac; }");
  connect(deleteAllBtn, &QPushButton::clicked, this, [this]() {
    // Strong confirmation: require typing "DELETE"
    bool ok;
    QString confirm = QInputDialog::getText(
        this, tr("Confirm Delete All Notes"),
        tr("This will permanently delete ALL your notes!\n\n"
           "Type DELETE to confirm:"),
        QLineEdit::Normal, QString(), &ok);

    if (ok && confirm.toUpper() == "DELETE") {
      // Find MainWindow and call deleteAllNotes
      MainWindow *mainWin = qobject_cast<MainWindow *>(parentWidget());
      if (!mainWin) {
        mainWin = qobject_cast<MainWindow *>(window()->parentWidget());
      }
      if (mainWin) {
        // Close settings dialog first
        accept();
        // Directly call deleteAllNotes
        mainWin->deleteAllNotes();
      }
    } else if (ok) {
      QMessageBox::warning(this, tr("Cancelled"),
                           tr("You must type DELETE to confirm."));
    }
  });
  layout->addWidget(deleteAllBtn);

  layout->addStretch();
  return page;
}

// === SECURITY PAGE ===
QWidget *SettingsDialog::createSecurityPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(12);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("🔒 Security"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  layout->addWidget(title);

  // Master password status
  m_masterPasswordStatus = new QLabel();
  layout->addWidget(m_masterPasswordStatus);

  // Buttons
  QHBoxLayout *btnRow = new QHBoxLayout();
  m_setMasterPasswordBtn = new QPushButton(tr("Set Master Password"));
  connect(m_setMasterPasswordBtn, &QPushButton::clicked, this,
          &SettingsDialog::onSetMasterPasswordClicked);
  btnRow->addWidget(m_setMasterPasswordBtn);

  m_removeMasterPasswordBtn = new QPushButton(tr("Remove"));
  connect(m_removeMasterPasswordBtn, &QPushButton::clicked, this,
          &SettingsDialog::onRemoveMasterPasswordClicked);
  btnRow->addWidget(m_removeMasterPasswordBtn);
  btnRow->addStretch();
  layout->addLayout(btnRow);

  m_useRecoveryKeyBtn = new QPushButton(tr("🔑 Use Recovery Key"));
  connect(m_useRecoveryKeyBtn, &QPushButton::clicked, this,
          &SettingsDialog::onUseRecoveryKeyClicked);
  layout->addWidget(m_useRecoveryKeyBtn);

  // Auto-lock for encrypted notes
  QHBoxLayout *lockRow = new QHBoxLayout();
  lockRow->addWidget(new QLabel(tr("Auto-Lock For Notes:")));
  m_autoLockCombo = new QComboBox();
  m_autoLockCombo->addItem(tr("Never"), 0);
  m_autoLockCombo->addItem(tr("Until Restart"), -1);
  m_autoLockCombo->addItem(tr("1 minute"), 1);
  m_autoLockCombo->addItem(tr("5 minutes"), 5);
  m_autoLockCombo->addItem(tr("15 minutes"), 15);
  m_autoLockCombo->addItem(tr("30 minutes"), 30);
  connect(m_autoLockCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::onAutoLockChanged);
  lockRow->addWidget(m_autoLockCombo);
  lockRow->addStretch();
  layout->addLayout(lockRow);

  layout->addStretch();
  return page;
}

// === SHORTCUTS PAGE ===
QWidget *SettingsDialog::createShortcutsPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *mainLayout = new QVBoxLayout(page);
  mainLayout->setSpacing(12);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("⌨️ Shortcuts"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  mainLayout->addWidget(title);

  // Global hotkey section
  QHBoxLayout *hotkeyRow = new QHBoxLayout();
  hotkeyRow->addWidget(new QLabel(tr("Global Hotkey:")));
  m_hotkeyEdit = new QKeySequenceEdit();
  connect(m_hotkeyEdit, &QKeySequenceEdit::keySequenceChanged, this,
          &SettingsDialog::onHotkeyChanged);
  hotkeyRow->addWidget(m_hotkeyEdit);
  hotkeyRow->addStretch();
  mainLayout->addLayout(hotkeyRow);

  // Scroll area for shortcuts
  QScrollArea *scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);

  QWidget *scrollContent = new QWidget();
  QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
  scrollLayout->setSpacing(8);

  // Search box for shortcuts - platform native styling
  QLineEdit *searchBox = new QLineEdit();
  searchBox->setPlaceholderText(tr("🔍 Filter..."));
  searchBox->setClearButtonEnabled(true);
  searchBox->setMaximumWidth(200);
  mainLayout->addWidget(searchBox);

  // Store all rows for filtering
  QList<QWidget *> shortcutRows;
  QList<QLabel *> categoryHeaders;

  // Helper lambda to add shortcut row
  auto addShortcut = [&](const QString &id, const QString &label) {
    QHBoxLayout *row = new QHBoxLayout();
    QLabel *lbl = new QLabel(label);
    lbl->setMinimumWidth(120);
    row->addWidget(lbl);

    QKeySequenceEdit *edit = new QKeySequenceEdit();
    edit->setKeySequence(Settings::instance()->shortcut(id));
    connect(edit, &QKeySequenceEdit::keySequenceChanged, this,
            [this, id](const QKeySequence &seq) {
              Settings::instance()->setShortcut(id, seq);
            });
    row->addWidget(edit);
    row->addStretch();
    m_shortcutEdits[id] = edit;

    // Create container widget for the row
    QWidget *rowWidget = new QWidget();
    rowWidget->setLayout(row);
    rowWidget->setProperty("searchText", label.toLower());
    scrollLayout->addWidget(rowWidget);
    shortcutRows.append(rowWidget);
  };

  // Helper lambda to add category header
  auto addCategory = [&](const QString &name) {
    QLabel *header = new QLabel(name);
    header->setStyleSheet(
        "font-weight: bold; margin-top: 8px; color: palette(link);");
    header->setProperty("searchText", name.toLower());
    scrollLayout->addWidget(header);
    categoryHeaders.append(header);
  };

  // === Categories ===
  addCategory(tr("📄 Page Operations"));
  addShortcut("newPage", tr("New Page"));
  addShortcut("deletePage", tr("Delete Page"));

  addCategory(tr("🔀 Navigation"));
  addShortcut("nextPage", tr("Next Page"));
  addShortcut("prevPage", tr("Previous Page"));
  addShortcut("firstPage", tr("First Page"));
  addShortcut("lastPage", tr("Last Page"));

  addCategory(tr("✏️ Editing"));
  addShortcut("find", tr("Find"));
  addShortcut("replace", tr("Find & Replace"));
  addShortcut("toggleLock", tr("Lock/Unlock Page"));

  addCategory(tr("📁 File"));
  addShortcut("export", tr("Export"));

  addCategory(tr("🪟 Window"));
  addShortcut("alwaysOnTop", tr("Always on Top"));
  addShortcut("hide", tr("Hide Window"));

  addCategory(tr("📷 OCR"));
  addShortcut("ocr", tr("Capture OCR"));

  addCategory(tr("🔗 URL"));
  addShortcut("shortenUrl", tr("Shorten URL"));

  // Connect search box to filter shortcuts
  connect(searchBox, &QLineEdit::textChanged, this,
          [shortcutRows, categoryHeaders](const QString &text) {
            QString searchText = text.toLower().trimmed();
            for (QWidget *row : shortcutRows) {
              QString rowText = row->property("searchText").toString();
              row->setVisible(searchText.isEmpty() ||
                              rowText.contains(searchText));
            }
            // Hide category headers if all their items are hidden or if
            // searching
            for (QLabel *header : categoryHeaders) {
              header->setVisible(searchText.isEmpty());
            }
          });

  scrollLayout->addStretch();
  scrollArea->setWidget(scrollContent);
  mainLayout->addWidget(scrollArea, 1);

  // Reset to defaults button
  m_resetShortcutsBtn = new QPushButton(tr("Reset All to Defaults"));
  connect(m_resetShortcutsBtn, &QPushButton::clicked, this, [this]() {
    Settings::instance()->resetShortcutsToDefaults();
    // Update all edits
    for (auto it = m_shortcutEdits.constBegin();
         it != m_shortcutEdits.constEnd(); ++it) {
      it.value()->setKeySequence(Settings::instance()->shortcut(it.key()));
    }
  });
  mainLayout->addWidget(m_resetShortcutsBtn);

  return page;
}

// === CALCULATOR PAGE ===
QWidget *SettingsDialog::createCalculatorPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(12);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("🧮 Calculator"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  layout->addWidget(title);

  m_autoMathCheck = new QCheckBox(tr("Enable math calculations (= suffix)"));
  connect(m_autoMathCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onAutoMathToggled);
  layout->addWidget(m_autoMathCheck);

  m_currencyCheck = new QCheckBox(tr("Enable currency conversion"));
  connect(m_currencyCheck, &QCheckBox::toggled, this,
          &SettingsDialog::onCurrencyToggled);
  layout->addWidget(m_currencyCheck);

  // Base currency
  QHBoxLayout *currRow = new QHBoxLayout();
  currRow->addWidget(new QLabel(tr("Base currency:")));
  m_currencyCombo = new QComboBox();
  // Fiat currencies
  m_currencyCombo->addItems({"USD", "EUR", "GBP", "JPY", "TRY", "CAD", "AUD",
                             "CHF", "CNY", "NZD", "HKD", "ALL", "INR", "KRW",
                             "BRL", "MXN", "PLN", "SEK", "NOK", "DKK", "SGD",
                             "ZAR", "THB", "AED", "SAR", "RUB"});
  // Crypto currencies
  m_currencyCombo->addItems({"BTC", "ETH", "USDT", "USDC", "XRP", "BNB", "SOL",
                             "ADA", "DOGE", "TRX"});
  connect(m_currencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::onCurrencyChanged);
  currRow->addWidget(m_currencyCombo);
  currRow->addStretch();
  layout->addLayout(currRow);

  // Provider
  QHBoxLayout *provRow = new QHBoxLayout();
  provRow->addWidget(new QLabel(tr("Provider:")));
  m_providerCombo = new QComboBox();
  m_providerCombo->addItem("Frankfurter (Free)", "frankfurter");
  m_providerCombo->addItem("OpenExchangeRates", "openexchangerates");
  m_providerCombo->addItem("Exchangerate.host", "exchangerate.host");
  m_providerCombo->addItem("Fixer", "fixer");
  m_providerCombo->addItem("Currencylayer", "currencylayer");
  m_providerCombo->addItem("CoinAPI", "coinapi");
  m_providerCombo->addItem("Alpha Vantage", "alphavantage");
  m_providerCombo->addItem("Twelve Data", "twelvedata");
  connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::onProviderChanged);
  provRow->addWidget(m_providerCombo);
  provRow->addStretch();
  layout->addLayout(provRow);

  // Provider hint label (dynamic, shows API key registration link)
  m_providerHintLabel = new QLabel();
  m_providerHintLabel->setStyleSheet("font-size: 11px; color: #6c7086;");
  m_providerHintLabel->setOpenExternalLinks(true);
  m_providerHintLabel->setTextFormat(Qt::RichText);
  layout->addWidget(m_providerHintLabel);
  updateProviderHint(); // Set initial hint

  // API Key
  QHBoxLayout *apiRow = new QHBoxLayout();
  apiRow->addWidget(new QLabel(tr("API Key:")));
  m_apiKeyEdit = new QLineEdit();
  m_apiKeyEdit->setEchoMode(QLineEdit::Password);
  connect(m_apiKeyEdit, &QLineEdit::editingFinished, this,
          &SettingsDialog::onApiKeyChanged);
  apiRow->addWidget(m_apiKeyEdit);
  layout->addLayout(apiRow);

  // Crypto API Key (FreeCryptoAPI)
  QHBoxLayout *cryptoApiRow = new QHBoxLayout();
  cryptoApiRow->addWidget(new QLabel(tr("Crypto API Key:")));
  m_cryptoApiKeyEdit = new QLineEdit();
  m_cryptoApiKeyEdit->setEchoMode(QLineEdit::Password);
  m_cryptoApiKeyEdit->setPlaceholderText(tr("FreeCryptoAPI.com key"));
  connect(m_cryptoApiKeyEdit, &QLineEdit::editingFinished, this, [this]() {
    Settings::instance()->setCryptoApiKey(m_cryptoApiKeyEdit->text());
  });
  cryptoApiRow->addWidget(m_cryptoApiKeyEdit);
  layout->addLayout(cryptoApiRow);

  QLabel *cryptoHint = new QLabel(
      tr("🔑 <a href='https://www.freecryptoapi.com/' style='color: #89b4fa;'>"
         "Get free API key from FreeCryptoAPI.com</a>"));
  cryptoHint->setStyleSheet("font-size: 11px;");
  cryptoHint->setOpenExternalLinks(true);
  cryptoHint->setTextFormat(Qt::RichText);
  layout->addWidget(cryptoHint);

  // Refresh interval
  QHBoxLayout *refRow = new QHBoxLayout();
  refRow->addWidget(new QLabel(tr("Refresh:")));
  m_refreshIntervalCombo = new QComboBox();
  m_refreshIntervalCombo->addItem(tr("Every hour"), 60);
  m_refreshIntervalCombo->addItem(tr("Every 6 hours"), 360);
  m_refreshIntervalCombo->addItem(tr("Daily"), 1440);
  connect(m_refreshIntervalCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &SettingsDialog::onRefreshIntervalChanged);
  refRow->addWidget(m_refreshIntervalCombo);

  QPushButton *refreshBtn = new QPushButton(tr("Refresh Now"));
  connect(refreshBtn, &QPushButton::clicked, this,
          &SettingsDialog::onRefreshRatesClicked);
  refRow->addWidget(refreshBtn);
  refRow->addStretch();
  layout->addLayout(refRow);

  // Last refresh timestamp
  m_lastRefreshLabel = new QLabel();
  m_lastRefreshLabel->setStyleSheet(
      "font-size: 10px; color: gray; font-style: italic;");
  updateLastRefreshLabel();
  layout->addWidget(m_lastRefreshLabel);

  // Test API button
  QHBoxLayout *testRow = new QHBoxLayout();
  m_testApiBtn = new QPushButton(tr("Test API"));
  m_testApiBtn->setToolTip(tr("Test if API key works with selected provider"));
  connect(m_testApiBtn, &QPushButton::clicked, this,
          &SettingsDialog::onTestApiClicked);
  testRow->addWidget(m_testApiBtn);

  m_apiStatusLabel = new QLabel();
  m_apiStatusLabel->setStyleSheet("font-style: italic;");
  testRow->addWidget(m_apiStatusLabel);
  testRow->addStretch();
  layout->addLayout(testRow);

  // Crypto API Test button
  QHBoxLayout *cryptoTestRow = new QHBoxLayout();
  m_testCryptoApiBtn = new QPushButton(tr("Test Crypto API"));
  m_testCryptoApiBtn->setToolTip(tr("Test if FreeCryptoAPI key works"));
  connect(m_testCryptoApiBtn, &QPushButton::clicked, this, [this]() {
    QString key = Settings::instance()->cryptoApiKey();
    if (key.isEmpty()) {
      m_cryptoApiStatusLabel->setText(tr("⚠️ No crypto API key set"));
      m_cryptoApiStatusLabel->setStyleSheet("color: orange;");
      return;
    }

    m_cryptoApiStatusLabel->setText(tr("Testing..."));
    m_cryptoApiStatusLabel->setStyleSheet("color: gray;");

    // Test FreeCryptoAPI with Bearer auth
    QNetworkRequest request(
        QUrl("https://api.freecryptoapi.com/v1/getData?symbol=BTC"));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(key).toUtf8());

    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    QNetworkReply *reply = mgr->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, mgr]() {
      reply->deleteLater();
      mgr->deleteLater();

      if (reply->error() != QNetworkReply::NoError) {
        m_cryptoApiStatusLabel->setText(tr("❌ %1").arg(reply->errorString()));
        m_cryptoApiStatusLabel->setStyleSheet("color: red;");
        return;
      }

      QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
      if (doc.isObject() && doc.object()["status"].toString() == "success") {
        m_cryptoApiStatusLabel->setText(tr("✅ FreeCryptoAPI working!"));
        m_cryptoApiStatusLabel->setStyleSheet("color: green;");
      } else {
        QString err = doc.object()["error"].toString();
        m_cryptoApiStatusLabel->setText(
            tr("❌ %1").arg(err.isEmpty() ? "Invalid response" : err));
        m_cryptoApiStatusLabel->setStyleSheet("color: red;");
      }
    });
  });
  cryptoTestRow->addWidget(m_testCryptoApiBtn);

  m_cryptoApiStatusLabel = new QLabel();
  m_cryptoApiStatusLabel->setStyleSheet("font-style: italic;");
  cryptoTestRow->addWidget(m_cryptoApiStatusLabel);
  cryptoTestRow->addStretch();
  layout->addLayout(cryptoTestRow);

  layout->addStretch();
  return page;
}

// === TIMER PAGE ===
QWidget *SettingsDialog::createTimerPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(12);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel(tr("⏱️ Timer"));
  title->setStyleSheet("font-size: 16px; font-weight: bold;");
  layout->addWidget(title);

  // Pomodoro labels
  QHBoxLayout *workLabelRow = new QHBoxLayout();
  workLabelRow->addWidget(new QLabel(tr("Work label:")));
  m_pomodoroWorkLabelEdit = new QLineEdit();
  connect(m_pomodoroWorkLabelEdit, &QLineEdit::editingFinished, this,
          &SettingsDialog::onPomodoroWorkLabelChanged);
  workLabelRow->addWidget(m_pomodoroWorkLabelEdit);
  layout->addLayout(workLabelRow);

  QHBoxLayout *breakLabelRow = new QHBoxLayout();
  breakLabelRow->addWidget(new QLabel(tr("Break label:")));
  m_pomodoroBreakLabelEdit = new QLineEdit();
  connect(m_pomodoroBreakLabelEdit, &QLineEdit::editingFinished, this,
          &SettingsDialog::onPomodoroBreakLabelChanged);
  breakLabelRow->addWidget(m_pomodoroBreakLabelEdit);
  layout->addLayout(breakLabelRow);

  // Durations
  QHBoxLayout *workRow = new QHBoxLayout();
  workRow->addWidget(new QLabel(tr("Work duration:")));
  m_pomodoroWorkMinutesSpin = new QSpinBox();
  m_pomodoroWorkMinutesSpin->setRange(1, 120);
  m_pomodoroWorkMinutesSpin->setSuffix(tr(" min"));
  connect(m_pomodoroWorkMinutesSpin,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingsDialog::onPomodoroWorkMinutesChanged);
  workRow->addWidget(m_pomodoroWorkMinutesSpin);
  workRow->addStretch();
  layout->addLayout(workRow);

  QHBoxLayout *breakRow = new QHBoxLayout();
  breakRow->addWidget(new QLabel(tr("Break duration:")));
  m_pomodoroBreakMinutesSpin = new QSpinBox();
  m_pomodoroBreakMinutesSpin->setRange(1, 60);
  m_pomodoroBreakMinutesSpin->setSuffix(tr(" min"));
  connect(m_pomodoroBreakMinutesSpin,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingsDialog::onPomodoroBreakMinutesChanged);
  breakRow->addWidget(m_pomodoroBreakMinutesSpin);
  breakRow->addStretch();
  layout->addLayout(breakRow);

  // Auto-start stopwatch
  layout->addSpacing(10);
  m_timerAutoStartCheck =
      new QCheckBox(tr("Auto-start stopwatch when typing 'timer' + Enter"));
  m_timerAutoStartCheck->setToolTip(
      tr("When enabled, typing just 'timer' and pressing Enter will "
         "automatically start a stopwatch."));
  connect(m_timerAutoStartCheck, &QCheckBox::toggled, this, [](bool checked) {
    Settings::instance()->setTimerAutoStartStopwatch(checked);
  });
  layout->addWidget(m_timerAutoStartCheck);

  layout->addStretch();
  return page;
}

// === OCR PAGE ===
QWidget *SettingsDialog::createOcrPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(12);

  // OCR Language
  QGroupBox *langGroup = new QGroupBox(tr("OCR Language"));
  QVBoxLayout *langLayout = new QVBoxLayout(langGroup);

  QHBoxLayout *langRow = new QHBoxLayout();
  langRow->addWidget(new QLabel(tr("Language:")));
  QComboBox *langCombo = new QComboBox();
  langCombo->addItem("English", "eng");
  langCombo->addItem("Türkçe", "tur");
  langCombo->addItem("Deutsch", "deu");
  langCombo->addItem("Français", "fra");
  langCombo->addItem("Español", "spa");
  langCombo->addItem("Italiano", "ita");
  langCombo->addItem("Nederlands", "nld");
  langCombo->addItem("Português", "por");
  langCombo->addItem("Русский", "rus");
  langCombo->addItem("中文 (Simplified)", "chi_sim");
  langCombo->addItem("日本語", "jpn");
  langCombo->addItem("한국어", "kor");
  langCombo->addItem("العربية", "ara");

  // Set current language from settings
  Settings *s = Settings::instance();
  QString currentLang = s->ocrLanguage();
  if (currentLang.isEmpty())
    currentLang = "eng";
  for (int i = 0; i < langCombo->count(); ++i) {
    if (langCombo->itemData(i).toString() == currentLang) {
      langCombo->setCurrentIndex(i);
      break;
    }
  }

  connect(langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this, langCombo](int index) {
            QString langCode = langCombo->itemData(index).toString();
            Settings::instance()->setOcrLanguage(langCode);

            // Check if language is installed
            if (OcrHelper::isAvailable() &&
                !OcrHelper::isLanguageAvailable(langCode)) {
              QString langName = langCombo->itemText(index);
              QMessageBox::warning(
                  this, tr("OCR Language"),
                  tr("The language '%1' (%2) is not installed.\n\n"
                     "Install with:\n\n"
                     "Ubuntu/Debian:\n"
                     "  sudo apt install tesseract-ocr-%3\n\n"
                     "Fedora:\n"
                     "  sudo dnf install tesseract-langpack-%3\n\n"
                     "Arch Linux:\n"
                     "  sudo pacman -S tesseract-data-%3\n\n"
                     "openSUSE:\n"
                     "  sudo zypper install tesseract-ocr-traineddata-%3")
                      .arg(langName, langCode, langCode));
            }
          });
  langRow->addWidget(langCombo);
  langRow->addStretch();
  langLayout->addLayout(langRow);
  layout->addWidget(langGroup);

  // OCR Behavior
  QGroupBox *behaviorGroup = new QGroupBox(tr("OCR Behavior"));
  QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);

  QCheckBox *newNoteCheck =
      new QCheckBox(tr("Create new note for each OCR scan"));
  newNoteCheck->setChecked(s->ocrCreateNewNote());
  connect(newNoteCheck, &QCheckBox::toggled, this, [](bool checked) {
    Settings::instance()->setOcrCreateNewNote(checked);
  });
  behaviorLayout->addWidget(newNoteCheck);

  QLabel *hint = new QLabel(
      tr("When disabled, OCR text will be appended to the current note."));
  hint->setStyleSheet("color: gray; font-size: 11px;");
  hint->setWordWrap(true);
  behaviorLayout->addWidget(hint);

  layout->addWidget(behaviorGroup);

  layout->addStretch();
  return page;
}

// === ABOUT PAGE ===
QWidget *SettingsDialog::createAboutPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(8);
  layout->setContentsMargins(20, 15, 20, 15);

  // Header with logo
  QHBoxLayout *headerLayout = new QHBoxLayout();

  QLabel *logoLabel = new QLabel();
  QPixmap logoPix(":/app/icons/app/linnote-128.png");
  if (!logoPix.isNull()) {
    logoLabel->setPixmap(
        logoPix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
  headerLayout->addWidget(logoLabel);

  QVBoxLayout *titleLayout = new QVBoxLayout();
  titleLayout->setSpacing(2);

  QLabel *appName = new QLabel(tr("LinNote"));
  appName->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: palette(link);");
  titleLayout->addWidget(appName);

  QLabel *version = new QLabel(tr("Version %1").arg(Settings::appVersion()));
  version->setStyleSheet("font-size: 12px; color: palette(mid);");
  titleLayout->addWidget(version);
  m_versionLabel = version;

  QLabel *tagline = new QLabel(tr("Professional Linux Scratchpad"));
  tagline->setStyleSheet("font-size: 11px; font-style: italic;");
  titleLayout->addWidget(tagline);

  headerLayout->addLayout(titleLayout);
  headerLayout->addStretch();
  layout->addLayout(headerLayout);

  // Description
  QLabel *desc = new QLabel(tr("A powerful, feature-rich note-taking "
                               "application designed for Linux power users. "
                               "Combines smart calculations, OCR, timers, and "
                               "30+ professional features."));
  desc->setWordWrap(true);
  desc->setStyleSheet("margin: 10px 0; line-height: 1.4;");
  layout->addWidget(desc);

  // Features section - modern clean layout
  QLabel *featuresTitle = new QLabel(tr("✨ Features"));
  featuresTitle->setStyleSheet(
      "font-size: 14px; font-weight: bold; color: "
      "palette(link); margin-top: 8px; margin-bottom: 4px;");
  layout->addWidget(featuresTitle);

  // Two-column feature grid - clean modern spacing
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(8);
  grid->setContentsMargins(5, 0, 5, 0);

  QStringList features = {"📝 Multi-page notes",
                          "🧮 Smart calculations",
                          "📊 sum / avg / count",
                          "💱 Currency conversion",
                          "📐 Unit conversion",
                          "⏱️ Pomodoro timer",
                          "⏲️ Stopwatch & countdown",
                          "📷 OCR screen capture",
                          "🔗 URL shortening",
                          "✅ Checklists",
                          "🔒 Note encryption",
                          "🎨 14+ themes",
                          "🌙 Dark mode",
                          "📤 Export (TXT/MD/HTML)",
                          "🔍 Full-text search",
                          "⌨️ Keyboard shortcuts",
                          "💾 Auto-save",
                          "📦 Backup system",
                          "🖥️ Wayland native",
                          "🐧 KDE Plasma support"};

  for (int i = 0; i < features.size(); ++i) {
    QLabel *f = new QLabel(features[i]);
    f->setStyleSheet("font-size: 13px;");
    grid->addWidget(f, i / 2, i % 2);
  }
  layout->addLayout(grid);

  // Footer
  QLabel *credits =
      new QLabel(tr("<center>Made with ❤️ for Linux<br>"
                    "<span style='font-size: 10px; color: gray;'>© 2025 "
                    "sfnemis • MIT License</span></center>"));
  credits->setTextFormat(Qt::RichText);
  credits->setStyleSheet("margin-top: 10px;");
  layout->addWidget(credits);

  layout->addStretch();

  // Update check section
  QHBoxLayout *updateRow = new QHBoxLayout();
  m_checkUpdatesBtn = new QPushButton(tr("🔄 Check for Updates"));
  connect(m_checkUpdatesBtn, &QPushButton::clicked, this,
          &SettingsDialog::onCheckForUpdatesClicked);
  updateRow->addWidget(m_checkUpdatesBtn);

  m_updateStatusLabel = new QLabel();
  m_updateStatusLabel->setStyleSheet("font-size: 11px; color: palette(mid);");
  updateRow->addWidget(m_updateStatusLabel);
  updateRow->addStretch();
  layout->addLayout(updateRow);

  // GitHub link
  QLabel *githubLink = new QLabel(
      tr("<a href='%1'>View on GitHub</a>").arg(Settings::githubUrl()));
  githubLink->setOpenExternalLinks(true);
  githubLink->setStyleSheet("font-size: 11px;");
  layout->addWidget(githubLink);

  // Initialize update checker
  m_updateChecker = new UpdateChecker(this);
  connect(m_updateChecker, &UpdateChecker::updateFound, this,
          &SettingsDialog::onUpdateFound);
  connect(m_updateChecker, &UpdateChecker::noUpdateFound, this,
          &SettingsDialog::onNoUpdateFound);
  connect(m_updateChecker, &UpdateChecker::updateCheckFailed, this,
          &SettingsDialog::onUpdateCheckFailed);

  return page;
}

void SettingsDialog::loadSettings() {
  Settings *s = Settings::instance();

  // Theme
  m_themeCombo->setCurrentText(s->themeName());
  m_darkModeCheck->setChecked(s->darkMode());
  m_transparencySlider->setValue(s->transparency());
  m_displayModeCombo->setCurrentIndex(static_cast<int>(s->displayMode()));

  // Editor
  m_fontCombo->setCurrentText(s->fontFamily());
  m_fontSizeSpin->setValue(s->fontSize());
  m_autoPasteCheck->setChecked(s->autoPasteEnabled());
  m_openWithNewNoteCheck->setChecked(s->openWithNewNote());
  m_autoCheckboxCheck->setChecked(s->autoCheckboxEnabled());
  updateFontPreview();

  // Backup
  m_backupEnabledCheck->setChecked(s->backupEnabled());
  m_backupPathEdit->setText(s->backupPath());
  m_backupIntervalSpin->setValue(s->backupIntervalHours());
  m_backupRetentionSpin->setValue(s->backupRetentionCount());

  // Security
  if (s->hasMasterPassword()) {
    m_masterPasswordStatus->setText(tr("✅ Master password is set"));
    m_masterPasswordStatus->setStyleSheet("color: #a6e3a1;");
    m_setMasterPasswordBtn->setText(tr("Change Master Password"));
    m_removeMasterPasswordBtn->setEnabled(true);
  } else {
    m_masterPasswordStatus->setText(tr("❌ No master password set"));
    m_masterPasswordStatus->setStyleSheet("color: #f38ba8;");
    m_setMasterPasswordBtn->setText(tr("Set Master Password"));
    m_removeMasterPasswordBtn->setEnabled(false);
  }

  int autoLockMinutes = s->autoLockMinutes();
  for (int i = 0; i < m_autoLockCombo->count(); ++i) {
    if (m_autoLockCombo->itemData(i).toInt() == autoLockMinutes) {
      m_autoLockCombo->setCurrentIndex(i);
      break;
    }
  }

  // Shortcuts
  m_hotkeyEdit->setKeySequence(portalToQtHotkey(s->globalHotkey()));

  // Calculator
  m_autoMathCheck->setChecked(s->autoMathEnabled());
  m_currencyCheck->setChecked(s->currencyConversionEnabled());
  m_currencyCombo->setCurrentText(s->baseCurrency());
  m_apiKeyEdit->setText(s->currencyApiKey());
  m_cryptoApiKeyEdit->setText(s->cryptoApiKey());

  // Find and set provider
  QString provider = s->currencyProvider();
  for (int i = 0; i < m_providerCombo->count(); ++i) {
    if (m_providerCombo->itemData(i).toString() == provider) {
      m_providerCombo->setCurrentIndex(i);
      break;
    }
  }

  // Timer
  m_pomodoroWorkLabelEdit->setText(s->pomodoroWorkLabel());
  m_pomodoroBreakLabelEdit->setText(s->pomodoroBreakLabel());
  m_pomodoroWorkMinutesSpin->setValue(s->pomodoroWorkMinutes());
  m_pomodoroBreakMinutesSpin->setValue(s->pomodoroBreakMinutes());
  m_timerAutoStartCheck->setChecked(s->timerAutoStartStopwatch());
}

void SettingsDialog::updateFontPreview() {
  QString family = m_fontCombo->currentText();
  int size = m_fontSizeSpin->value();
  if (m_fontPreview) {
    QString style =
        QString("background-color: palette(base); color: palette(text); "
                "padding: 12px; border-radius: 6px; "
                "font-family: '%1'; font-size: %2pt;")
            .arg(family)
            .arg(size);
    m_fontPreview->setStyleSheet(style);
  }
}

// === SLOT IMPLEMENTATIONS ===

void SettingsDialog::onAutoMathToggled(bool checked) {
  Settings::instance()->setAutoMathEnabled(checked);
}

void SettingsDialog::onCurrencyToggled(bool checked) {
  Settings::instance()->setCurrencyConversionEnabled(checked);
  m_currencyCombo->setEnabled(checked);
}

void SettingsDialog::onCurrencyChanged(int index) {
  QString currency = m_currencyCombo->itemText(index);
  Settings::instance()->setBaseCurrency(currency);
}

void SettingsDialog::onAutoPasteToggled(bool checked) {
  Settings::instance()->setAutoPasteEnabled(checked);
}

void SettingsDialog::onOpenWithNewNoteToggled(bool checked) {
  Settings::instance()->setOpenWithNewNote(checked);
}

void SettingsDialog::onApiKeyChanged() {
  Settings::instance()->setCurrencyApiKey(m_apiKeyEdit->text());
}

void SettingsDialog::onProviderChanged(int index) {
  QString provider = m_providerCombo->itemData(index).toString();
  Settings::instance()->setCurrencyProvider(provider);
  CurrencyConverter::instance()->setProvider(provider);
  m_apiStatusLabel->clear(); // Clear status when provider changes
  updateProviderHint();      // Update API key hint link
}

void SettingsDialog::onTestApiClicked() {
  m_testApiBtn->setEnabled(false);
  m_apiStatusLabel->setText(tr("Testing..."));
  m_apiStatusLabel->setStyleSheet("color: palette(text); font-style: italic;");

  // Save current settings to converter
  CurrencyConverter *conv = CurrencyConverter::instance();
  conv->setApiKey(m_apiKeyEdit->text());
  conv->setProvider(m_providerCombo->currentData().toString());

  // Connect to result signals
  QMetaObject::Connection successConn, errorConn;

  successConn = connect(conv, &CurrencyConverter::ratesUpdated, this,
                        [this, &successConn, &errorConn]() {
                          m_testApiBtn->setEnabled(true);
                          m_apiStatusLabel->setText(tr("✓ API works!"));
                          m_apiStatusLabel->setStyleSheet("color: green;");
                        });

  errorConn = connect(conv, &CurrencyConverter::error, this,
                      [this, &successConn, &errorConn](const QString &msg) {
                        m_testApiBtn->setEnabled(true);
                        m_apiStatusLabel->setText(tr("✗ ") + msg);
                        m_apiStatusLabel->setStyleSheet("color: red;");
                      });

  // Refresh rates to test
  conv->refreshRates();

  // Timeout after 10 seconds
  QTimer::singleShot(10000, this, [this, successConn, errorConn]() {
    if (!m_testApiBtn->isEnabled()) {
      m_testApiBtn->setEnabled(true);
      m_apiStatusLabel->setText(tr("✗ Timeout"));
      m_apiStatusLabel->setStyleSheet("color: red;");
      disconnect(successConn);
      disconnect(errorConn);
    }
  });
}

void SettingsDialog::onRefreshRatesClicked() {
  CurrencyConverter::instance()->refreshRates();
  // Save current timestamp to settings
  Settings::instance()->setLastCurrencyRefresh(QDateTime::currentDateTime());
  updateLastRefreshLabel();
}

void SettingsDialog::updateLastRefreshLabel() {
  QDateTime lastRefresh = Settings::instance()->lastCurrencyRefresh();
  if (lastRefresh.isValid()) {
    QString formatted = lastRefresh.toString("dd MMM yyyy hh:mm");
    m_lastRefreshLabel->setText(tr("Last updated: %1").arg(formatted));
  } else {
    m_lastRefreshLabel->setText(tr("Not yet refreshed"));
  }
}

void SettingsDialog::updateProviderHint() {
  QString provider = m_providerCombo->currentData().toString();

  // Provider API registration links
  static const QMap<QString, QPair<QString, QString>> providerInfo = {
      {"frankfurter", {"✅ Free, no API key required", ""}},
      {"openexchangerates",
       {"🔑 Get free API key", "https://openexchangerates.org/signup/free"}},
      {"exchangerate.host",
       {"🔑 Get free API key", "https://exchangerate.host/"}},
      {"fixer",
       {"🔑 Get free API key (100 requests/month)",
        "https://fixer.io/signup/free"}},
      {"currencylayer",
       {"🔑 Get free API key (100 requests/month)",
        "https://currencylayer.com/signup/free"}},
      {"coinapi",
       {"🔑 Get free API key (100 requests/day)",
        "https://www.coinapi.io/pricing?apikey"}},
      {"alphavantage",
       {"🔑 Get free API key (25 requests/day)",
        "https://www.alphavantage.co/support/#api-key"}},
      {"twelvedata",
       {"🔑 Get free API key (800 requests/day)",
        "https://twelvedata.com/pricing"}}};

  if (providerInfo.contains(provider)) {
    auto info = providerInfo[provider];
    if (info.second.isEmpty()) {
      // No link needed (e.g. Frankfurter)
      m_providerHintLabel->setText(info.first);
    } else {
      // Clickable link
      m_providerHintLabel->setText(
          QString("<a href='%1' style='color: #89b4fa;'>%2</a>")
              .arg(info.second, info.first));
    }
  } else {
    m_providerHintLabel->clear();
  }
}

void SettingsDialog::onRefreshIntervalChanged(int index) {
  int minutes = m_refreshIntervalCombo->itemData(index).toInt();
  Settings::instance()->setRefreshIntervalMinutes(minutes);
}

void SettingsDialog::onThemeChanged(int index) {
  Settings::instance()->setThemeName(m_themeCombo->itemText(index));
  emit themeChanged();
}

void SettingsDialog::onDarkModeToggled(bool checked) {
  Settings::instance()->setDarkMode(checked);
  emit themeChanged();
}

void SettingsDialog::onTransparencyChanged(int value) {
  Settings::instance()->setTransparency(value);
  emit transparencyChanged(value);
}

void SettingsDialog::onAutoCheckboxToggled(bool checked) {
  Settings::instance()->setAutoCheckboxEnabled(checked);
}

void SettingsDialog::onStartOnBootToggled(bool checked) {
  setAutostartEnabled(checked);
}

bool SettingsDialog::isAutostartEnabled() const {
  QString autostartPath =
      QDir::homePath() + "/.config/autostart/linnote.desktop";
  return QFile::exists(autostartPath);
}

void SettingsDialog::setAutostartEnabled(bool enabled) {
  QString autostartDir = QDir::homePath() + "/.config/autostart";
  QString autostartPath = autostartDir + "/linnote.desktop";

  if (enabled) {
    // Create autostart directory if it doesn't exist
    QDir().mkpath(autostartDir);

    // Create .desktop file
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
      qDebug() << "Autostart enabled:" << autostartPath;
    }
  } else {
    // Remove autostart file
    if (QFile::exists(autostartPath)) {
      QFile::remove(autostartPath);
      qDebug() << "Autostart disabled:" << autostartPath;
    }
  }
}

void SettingsDialog::onPomodoroWorkLabelChanged() {
  Settings::instance()->setPomodoroWorkLabel(m_pomodoroWorkLabelEdit->text());
}

void SettingsDialog::onPomodoroBreakLabelChanged() {
  Settings::instance()->setPomodoroBreakLabel(m_pomodoroBreakLabelEdit->text());
}

void SettingsDialog::onPomodoroWorkMinutesChanged(int value) {
  Settings::instance()->setPomodoroWorkMinutes(value);
}

void SettingsDialog::onPomodoroBreakMinutesChanged(int value) {
  Settings::instance()->setPomodoroBreakMinutes(value);
}

void SettingsDialog::onHotkeyChanged(const QKeySequence &keySequence) {
  QString portalHotkey = qtToPortalHotkey(keySequence);
  Settings::instance()->setGlobalHotkey(portalHotkey);
}

void SettingsDialog::onSetMasterPasswordClicked() {
  Settings *s = Settings::instance();
  bool isChange = s->hasMasterPassword();

  if (isChange) {
    PasswordDialog verifyDlg(PasswordDialog::VerifyMasterPassword, this);
    if (verifyDlg.exec() != QDialog::Accepted)
      return;

    QString currentPassword = verifyDlg.password();
    if (!Crypto::verifyPassword(currentPassword, s->masterPasswordHash())) {
      QMessageBox::warning(this, tr("Incorrect Password"),
                           tr("The password you entered is incorrect."));
      return;
    }
  }

  PasswordDialog newDlg(PasswordDialog::SetMasterPassword, this);
  if (newDlg.exec() != QDialog::Accepted)
    return;

  QString newPassword = newDlg.password();
  if (newPassword.isEmpty()) {
    QMessageBox::warning(this, tr("Empty Password"),
                         tr("Password cannot be empty."));
    return;
  }

  QString hash = Crypto::hashPassword(newPassword);
  s->setMasterPasswordHash(hash);

  QString recoveryKey = generateRecoveryKey();
  s->setRecoveryKey(Crypto::hashPassword(recoveryKey));

  showRecoveryKeyDialog(this, tr("Recovery Key Generated"),
                        tr("Save this recovery key in a safe place.\n"
                           "You can use it to reset your master password."),
                        recoveryKey);

  loadSettings();
}

void SettingsDialog::onRemoveMasterPasswordClicked() {
  Settings *s = Settings::instance();

  PasswordDialog verifyDlg(PasswordDialog::VerifyMasterPassword, this);
  if (verifyDlg.exec() != QDialog::Accepted)
    return;

  if (!Crypto::verifyPassword(verifyDlg.password(), s->masterPasswordHash())) {
    QMessageBox::warning(this, tr("Incorrect Password"),
                         tr("The password you entered is incorrect."));
    return;
  }

  int result = QMessageBox::question(
      this, tr("Remove Master Password"),
      tr("Are you sure you want to remove the master password?\n"
         "Your notes will no longer be encrypted."),
      QMessageBox::Yes | QMessageBox::No);

  if (result == QMessageBox::Yes) {
    s->setMasterPasswordHash("");
    s->setRecoveryKey("");
    loadSettings();
  }
}

void SettingsDialog::onUseRecoveryKeyClicked() {
  Settings *s = Settings::instance();

  if (!s->hasMasterPassword()) {
    QMessageBox::information(this, tr("No Password Set"),
                             tr("There is no master password to recover."));
    return;
  }

  bool ok;
  QString recoveryKey = QInputDialog::getText(
      this, tr("Recovery Key"), tr("Enter your recovery key:"),
      QLineEdit::Normal, QString(), &ok);
  if (!ok || recoveryKey.isEmpty())
    return;

  if (!Crypto::verifyPassword(recoveryKey, s->recoveryKey())) {
    QMessageBox::warning(this, tr("Invalid Recovery Key"),
                         tr("The recovery key is invalid."));
    return;
  }

  PasswordDialog newDlg(PasswordDialog::SetMasterPassword, this);
  if (newDlg.exec() != QDialog::Accepted)
    return;

  QString newPassword = newDlg.password();
  if (newPassword.isEmpty()) {
    QMessageBox::warning(this, tr("Empty Password"),
                         tr("Password cannot be empty."));
    return;
  }

  s->setMasterPasswordHash(Crypto::hashPassword(newPassword));

  QString newRecoveryKey = generateRecoveryKey();
  s->setRecoveryKey(Crypto::hashPassword(newRecoveryKey));

  showRecoveryKeyDialog(this, tr("New Recovery Key Generated"),
                        tr("Your password has been reset.\n"
                           "Save this new recovery key:"),
                        newRecoveryKey);

  loadSettings();
}

void SettingsDialog::onAutoLockChanged(int index) {
  int minutes = m_autoLockCombo->itemData(index).toInt();
  Settings::instance()->setAutoLockMinutes(minutes);
}

void SettingsDialog::onFontFamilyChanged(const QString &family) {
  Settings::instance()->setFontFamily(family);
  updateFontPreview();
  emit fontChanged();
}

void SettingsDialog::onFontSizeChanged(int size) {
  Settings::instance()->setFontSize(size);
  updateFontPreview();
  emit fontChanged();
}

void SettingsDialog::onBackupEnabledToggled(bool enabled) {
  Settings::instance()->setBackupEnabled(enabled);
  emit backupSettingsChanged();
}

void SettingsDialog::onBackupPathChanged(const QString &path) {
  Settings::instance()->setBackupPath(path);
  emit backupSettingsChanged();
}

void SettingsDialog::onBackupBrowseClicked() {
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Select Backup Folder"),
      m_backupPathEdit->text().isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
          : m_backupPathEdit->text());
  if (!dir.isEmpty()) {
    m_backupPathEdit->setText(dir);
    Settings::instance()->setBackupPath(dir);
    emit backupSettingsChanged();
  }
}

void SettingsDialog::onBackupIntervalChanged(int hours) {
  Settings::instance()->setBackupIntervalHours(hours);
  emit backupSettingsChanged();
}

void SettingsDialog::onBackupRetentionChanged(int count) {
  Settings::instance()->setBackupRetentionCount(count);
  emit backupSettingsChanged();
}

void SettingsDialog::onBackupNowClicked() {
  BackupManager manager;
  if (manager.createBackup()) {
    QMessageBox::information(this, tr("Backup Created"),
                             tr("Backup was created successfully."));
  } else {
    QMessageBox::warning(
        this, tr("Backup Failed"),
        tr("Failed to create backup. Check the backup folder."));
  }
}

// Helper functions
QString generateRecoveryKey() {
  const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  QString key;
  for (int i = 0; i < 24; i++) {
    if (i > 0 && i % 4 == 0)
      key += "-";
    key += chars.at(QRandomGenerator::global()->bounded(chars.length()));
  }
  return key;
}

void showRecoveryKeyDialog(QWidget *parent, const QString &title,
                           const QString &message, const QString &recoveryKey) {
  QDialog dialog(parent);
  dialog.setWindowTitle(title);
  dialog.setModal(true);
  dialog.setFixedWidth(400);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->setSpacing(12);

  QLabel *msgLabel = new QLabel(message);
  msgLabel->setWordWrap(true);
  layout->addWidget(msgLabel);

  QLineEdit *keyEdit = new QLineEdit(recoveryKey);
  keyEdit->setReadOnly(true);
  keyEdit->setStyleSheet(
      "background-color: #313244; padding: 10px; font-family: monospace; "
      "font-size: 14px; font-weight: bold; color: #a6e3a1;");
  keyEdit->selectAll();
  layout->addWidget(keyEdit);

  QLabel *warning =
      new QLabel(QObject::tr("⚠️ Please save this key in a safe place!"));
  warning->setStyleSheet("color: #f9e2af;");
  layout->addWidget(warning);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  QPushButton *copyBtn = new QPushButton(QObject::tr("📋 Copy to Clipboard"));
  copyBtn->setStyleSheet(
      "background-color: #89b4fa; color: #1e1e2e; padding: 8px 16px; "
      "border-radius: 6px; font-weight: bold;");
  QObject::connect(copyBtn, &QPushButton::clicked, [keyEdit, &dialog]() {
    QApplication::clipboard()->setText(keyEdit->text());
    keyEdit->selectAll();
  });
  btnLayout->addWidget(copyBtn);

  QPushButton *okBtn = new QPushButton(QObject::tr("OK"));
  okBtn->setStyleSheet(
      "background-color: #45475a; color: #cdd6f4; padding: 8px 16px; "
      "border-radius: 6px;");
  QObject::connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
  btnLayout->addWidget(okBtn);

  layout->addLayout(btnLayout);
  dialog.exec();
}

// === KEYWORDS PAGE ===
QWidget *SettingsDialog::createKeywordsPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setSpacing(12);
  layout->setContentsMargins(24, 24, 24, 24);

  // Header - no custom colors, use platform defaults
  QLabel *header = new QLabel(tr("🏷️ Keyword Aliases"));
  header->setStyleSheet("font-size: 18px; font-weight: bold;");
  layout->addWidget(header);

  QLabel *desc = new QLabel(tr("Customize slash commands. Type a custom "
                               "keyword to replace the original."));
  desc->setWordWrap(true);
  desc->setStyleSheet("margin-bottom: 12px;");
  layout->addWidget(desc);

  // Keywords with line edits (more modern than table)
  QStringList keywords = {"list", "code", "calc", "timer", "markdown", "ocr"};
  QStringList icons = {"📋", "💻", "🧮", "⏱️", "📝", "📷"};
  Settings *s = Settings::instance();
  QMap<QString, QString> aliases = s->keywordAliases();

  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(8);

  for (int i = 0; i < keywords.size(); ++i) {
    QString kw = keywords[i];

    // Icon + Original keyword label - platform default colors
    QLabel *label = new QLabel(QString("%1  %2").arg(icons[i]).arg(kw));
    label->setStyleSheet("font-size: 13px; padding: 8px 0;");
    grid->addWidget(label, i, 0);

    // Arrow
    QLabel *arrow = new QLabel("→");
    arrow->setAlignment(Qt::AlignCenter);
    grid->addWidget(arrow, i, 1);

    // Custom alias input - platform default styling
    QLineEdit *edit = new QLineEdit();
    edit->setPlaceholderText(tr("Custom keyword..."));
    edit->setText(aliases.value(kw, ""));
    edit->setMaximumWidth(150);
    grid->addWidget(edit, i, 2);

    // Connect signal
    connect(edit, &QLineEdit::textChanged, this, [=](const QString &text) {
      s->setKeywordAlias(kw, text.trimmed());
    });
  }

  layout->addLayout(grid);

  // Info label
  QLabel *info = new QLabel(tr("💡 Leave empty to use the original keyword."));
  info->setStyleSheet("font-size: 11px; margin-top: 16px;");
  layout->addWidget(info);

  layout->addStretch();
  return page;
}

// === UPDATE CHECKER SLOTS ===

void SettingsDialog::onCheckForUpdatesClicked() {
  m_checkUpdatesBtn->setEnabled(false);
  m_checkUpdatesBtn->setText(tr("Checking..."));
  m_updateStatusLabel->setText(tr("Checking for updates..."));
  m_updateChecker->checkForUpdates();
}

void SettingsDialog::onUpdateFound(const QString &version,
                                   const QString &changelog) {
  m_checkUpdatesBtn->setEnabled(true);
  m_checkUpdatesBtn->setText(tr("🔄 Check for Updates"));
  m_updateStatusLabel->setText(tr("✨ Update available: v%1").arg(version));
  m_updateStatusLabel->setStyleSheet(
      "font-size: 11px; color: #a6e3a1; font-weight: bold;");

  // Show update dialog
  QMessageBox msgBox(this);
  msgBox.setWindowTitle(tr("Update Available"));
  msgBox.setText(tr("A new version of LinNote is available!"));
  msgBox.setInformativeText(tr("Current: v%1\nLatest: v%2\n\n%3")
                                .arg(Settings::appVersion())
                                .arg(version)
                                .arg(changelog.left(500)));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.button(QMessageBox::Yes)->setText(tr("Update Now"));
  msgBox.button(QMessageBox::No)->setText(tr("Later"));

  if (msgBox.exec() == QMessageBox::Yes) {
    m_updateStatusLabel->setText(tr("Updating..."));
    m_updateChecker->performUpdate();
  }
}

void SettingsDialog::onNoUpdateFound() {
  m_checkUpdatesBtn->setEnabled(true);
  m_checkUpdatesBtn->setText(tr("🔄 Check for Updates"));
  m_updateStatusLabel->setText(
      tr("✓ You're up to date (v%1)").arg(Settings::appVersion()));
  m_updateStatusLabel->setStyleSheet("font-size: 11px; color: palette(mid);");
}

void SettingsDialog::onUpdateCheckFailed(const QString &error) {
  m_checkUpdatesBtn->setEnabled(true);
  m_checkUpdatesBtn->setText(tr("🔄 Check for Updates"));
  m_updateStatusLabel->setText(tr("⚠ Check failed: %1").arg(error));
  m_updateStatusLabel->setStyleSheet("font-size: 11px; color: #f38ba8;");
}
