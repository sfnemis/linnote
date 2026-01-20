#include "Settings.h"
#include "integration/DesktopHelper.h"
#include "storage/SqliteStorage.h"
#include "version.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>

// === VERSION INFO (from generated version.h) ===
QString Settings::appVersion() { return QString(LINNOTE_VERSION); }
int Settings::versionMajor() { return LINNOTE_VERSION_MAJOR; }
int Settings::versionMinor() { return LINNOTE_VERSION_MINOR; }
int Settings::versionPatch() { return LINNOTE_VERSION_PATCH; }
QString Settings::appName() { return QString(LINNOTE_APP_NAME); }
QString Settings::githubRepo() { return QString(LINNOTE_GITHUB_REPO); }
QString Settings::githubUrl() { return QString(LINNOTE_GITHUB_URL); }

Settings *Settings::instance() {
  static Settings instance;
  return &instance;
}

Settings::Settings(QObject *parent)
    : QObject(parent), m_autoMathEnabled(true),
      m_currencyConversionEnabled(true), m_baseCurrency("USD"),
      m_currencyApiKey(""), m_cryptoApiKey("gz2pvqoerci2x921124g"),
      m_currencyProvider("frankfurter"),
      m_refreshIntervalMinutes(360), // 6 hours default
      m_autoPasteEnabled(true), m_openWithNewNote(false),
      m_themeName("Catppuccin"), m_darkMode(true), m_transparency(100),
      m_autoCheckboxEnabled(true), m_pomodoroWorkLabel("Work"),
      m_pomodoroBreakLabel("Break"), m_pomodoroWorkMinutes(25),
      m_pomodoroBreakMinutes(5), m_timerAutoStartStopwatch(false),
      m_fontFamily("Inter"), m_fontSize(13),
      m_globalHotkey("<Super><Control>n"), m_firstRunComplete(false),
      m_encryptionEnabled(false), m_autoLockMinutes(0), m_backupEnabled(false),
      m_backupIntervalHours(3), m_backupRetentionCount(12),
      // Checkpoint 3 defaults
      m_linkAutoShortenEnabled(true), m_soundEnabled(true), m_soundVolume(75),
      m_pasteRemoveLeadingSpaces(false), m_pasteRemoveNumbers(false),
      m_pasteRemoveBullets(false), m_pasteRemoveMarkdown(false),
      m_pasteRemoveEmptyLines(false), m_keepIndentationOnEnter(true),
      m_skipKeywordsOnCopy(true), m_skipTriggersOnCopy(true), m_defaultMode(0),
      m_defaultCodeLanguage("javascript"), m_noteTitleMode(0),
      m_displayMode(Both), m_toolbarAutoHide(false),
      m_onboardingCompleted(false), m_examplesShown(false) {
  m_shortcuts = defaultShortcuts();
  load();
}

bool Settings::autoMathEnabled() const { return m_autoMathEnabled; }

void Settings::setAutoMathEnabled(bool enabled) {
  if (m_autoMathEnabled != enabled) {
    m_autoMathEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::currencyConversionEnabled() const {
  return m_currencyConversionEnabled;
}

void Settings::setCurrencyConversionEnabled(bool enabled) {
  if (m_currencyConversionEnabled != enabled) {
    m_currencyConversionEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

QString Settings::baseCurrency() const { return m_baseCurrency; }

void Settings::setBaseCurrency(const QString &currency) {
  if (m_baseCurrency != currency) {
    m_baseCurrency = currency;
    save();
    emit settingsChanged();
  }
}

QString Settings::currencyApiKey() const { return m_currencyApiKey; }

void Settings::setCurrencyApiKey(const QString &key) {
  if (m_currencyApiKey != key) {
    m_currencyApiKey = key;
    save();
    emit settingsChanged();
    emit apiKeyChanged(key);
  }
}

QString Settings::cryptoApiKey() const { return m_cryptoApiKey; }

void Settings::setCryptoApiKey(const QString &key) {
  if (m_cryptoApiKey != key) {
    m_cryptoApiKey = key;
    save();
    emit settingsChanged();
  }
}

QString Settings::currencyProvider() const { return m_currencyProvider; }

void Settings::setCurrencyProvider(const QString &provider) {
  if (m_currencyProvider != provider) {
    m_currencyProvider = provider;
    save();
    emit settingsChanged();
  }
}

int Settings::refreshIntervalMinutes() const {
  return m_refreshIntervalMinutes;
}

void Settings::setRefreshIntervalMinutes(int minutes) {
  if (m_refreshIntervalMinutes != minutes) {
    m_refreshIntervalMinutes = minutes;
    save();
    emit refreshIntervalChanged(minutes);
    emit settingsChanged();
  }
}

QDateTime Settings::lastCurrencyRefresh() const {
  return m_lastCurrencyRefresh;
}

void Settings::setLastCurrencyRefresh(const QDateTime &dt) {
  m_lastCurrencyRefresh = dt;
  save();
}

bool Settings::autoPasteEnabled() const { return m_autoPasteEnabled; }

void Settings::setAutoPasteEnabled(bool enabled) {
  if (m_autoPasteEnabled != enabled) {
    m_autoPasteEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

QSize Settings::windowSize() const { return m_windowSize; }

void Settings::setWindowSize(const QSize &size) {
  if (m_windowSize != size) {
    m_windowSize = size;
    save();
  }
}

QPoint Settings::windowPosition() const { return m_windowPosition; }

void Settings::setWindowPosition(const QPoint &pos) {
  if (m_windowPosition != pos) {
    m_windowPosition = pos;
    save();
  }
}

bool Settings::openWithNewNote() const { return m_openWithNewNote; }

void Settings::setOpenWithNewNote(bool enabled) {
  if (m_openWithNewNote != enabled) {
    m_openWithNewNote = enabled;
    save();
    emit settingsChanged();
  }
}

QString Settings::themeName() const { return m_themeName; }

void Settings::setThemeName(const QString &name) {
  if (m_themeName != name) {
    m_themeName = name;
    save();
    emit settingsChanged();
  }
}

bool Settings::darkMode() const { return m_darkMode; }

void Settings::setDarkMode(bool dark) {
  if (m_darkMode != dark) {
    m_darkMode = dark;
    save();
    emit settingsChanged();
  }
}

int Settings::transparency() const { return m_transparency; }

void Settings::setTransparency(int value) {
  value = qBound(20, value, 100); // Min 20%, max 100%
  if (m_transparency != value) {
    m_transparency = value;
    save();
    emit transparencyChanged(value);
  }
}

Settings::DisplayMode Settings::displayMode() const { return m_displayMode; }

void Settings::setDisplayMode(DisplayMode mode) {
  if (m_displayMode != mode) {
    m_displayMode = mode;
    save();
    emit settingsChanged();
  }
}

bool Settings::toolbarAutoHide() const { return m_toolbarAutoHide; }

void Settings::setToolbarAutoHide(bool enabled) {
  if (m_toolbarAutoHide != enabled) {
    m_toolbarAutoHide = enabled;
    save();
    emit settingsChanged();
  }
}

// List mode settings
bool Settings::autoCheckboxEnabled() const { return m_autoCheckboxEnabled; }

void Settings::setAutoCheckboxEnabled(bool enabled) {
  if (m_autoCheckboxEnabled != enabled) {
    m_autoCheckboxEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

// Pomodoro settings
QString Settings::pomodoroWorkLabel() const { return m_pomodoroWorkLabel; }

void Settings::setPomodoroWorkLabel(const QString &label) {
  if (m_pomodoroWorkLabel != label) {
    m_pomodoroWorkLabel = label;
    save();
    emit settingsChanged();
  }
}

QString Settings::pomodoroBreakLabel() const { return m_pomodoroBreakLabel; }

void Settings::setPomodoroBreakLabel(const QString &label) {
  if (m_pomodoroBreakLabel != label) {
    m_pomodoroBreakLabel = label;
    save();
    emit settingsChanged();
  }
}

int Settings::pomodoroWorkMinutes() const { return m_pomodoroWorkMinutes; }

void Settings::setPomodoroWorkMinutes(int minutes) {
  minutes = qBound(1, minutes, 120);
  if (m_pomodoroWorkMinutes != minutes) {
    m_pomodoroWorkMinutes = minutes;
    save();
    emit settingsChanged();
  }
}

int Settings::pomodoroBreakMinutes() const { return m_pomodoroBreakMinutes; }

void Settings::setPomodoroBreakMinutes(int minutes) {
  minutes = qBound(1, minutes, 60);
  if (m_pomodoroBreakMinutes != minutes) {
    m_pomodoroBreakMinutes = minutes;
    save();
    emit settingsChanged();
  }
}

// Timer settings
bool Settings::timerAutoStartStopwatch() const {
  return m_timerAutoStartStopwatch;
}

void Settings::setTimerAutoStartStopwatch(bool enabled) {
  if (m_timerAutoStartStopwatch != enabled) {
    m_timerAutoStartStopwatch = enabled;
    save();
    emit settingsChanged();
  }
}

// Font settings
QString Settings::fontFamily() const { return m_fontFamily; }

void Settings::setFontFamily(const QString &family) {
  if (m_fontFamily != family) {
    m_fontFamily = family;
    save();
    emit settingsChanged();
  }
}

int Settings::fontSize() const { return m_fontSize; }

void Settings::setFontSize(int size) {
  size = qBound(8, size, 32);
  if (m_fontSize != size) {
    m_fontSize = size;
    save();
    emit settingsChanged();
  }
}

// Global hotkey
QString Settings::globalHotkey() const { return m_globalHotkey; }

void Settings::setGlobalHotkey(const QString &hotkey) {
  if (m_globalHotkey != hotkey) {
    m_globalHotkey = hotkey;
    save();
    emit settingsChanged();

    // Sync to KDE global shortcuts if running under KDE
    updateKdeGlobalShortcut(hotkey);
  }
}

// Master Password
QString Settings::masterPasswordHash() const { return m_masterPasswordHash; }

void Settings::setMasterPasswordHash(const QString &hash) {
  if (m_masterPasswordHash != hash) {
    m_masterPasswordHash = hash;
    save();
    emit settingsChanged();
  }
}

bool Settings::hasMasterPassword() const {
  return !m_masterPasswordHash.isEmpty();
}

// Recovery Key
QString Settings::recoveryKey() const { return m_recoveryKey; }

void Settings::setRecoveryKey(const QString &key) {
  if (m_recoveryKey != key) {
    m_recoveryKey = key;
    save();
    emit settingsChanged();
  }
}

// First Run
bool Settings::isFirstRun() const { return !m_firstRunComplete; }

void Settings::setFirstRunComplete() {
  if (!m_firstRunComplete) {
    m_firstRunComplete = true;
    save();
  }
}

// Encryption Enabled
bool Settings::encryptionEnabled() const { return m_encryptionEnabled; }

void Settings::setEncryptionEnabled(bool enabled) {
  if (m_encryptionEnabled != enabled) {
    m_encryptionEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

// Auto-Lock Timer
int Settings::autoLockMinutes() const { return m_autoLockMinutes; }

void Settings::setAutoLockMinutes(int minutes) {
  if (m_autoLockMinutes != minutes) {
    m_autoLockMinutes = minutes;
    save();
    emit settingsChanged();
  }
}

// Auto-Backup
bool Settings::backupEnabled() const { return m_backupEnabled; }

void Settings::setBackupEnabled(bool enabled) {
  if (m_backupEnabled != enabled) {
    m_backupEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

QString Settings::backupPath() const { return m_backupPath; }

void Settings::setBackupPath(const QString &path) {
  if (m_backupPath != path) {
    m_backupPath = path;
    save();
    emit settingsChanged();
  }
}

int Settings::backupIntervalHours() const { return m_backupIntervalHours; }

void Settings::setBackupIntervalHours(int hours) {
  if (m_backupIntervalHours != hours) {
    m_backupIntervalHours = hours;
    save();
    emit settingsChanged();
  }
}

int Settings::backupRetentionCount() const { return m_backupRetentionCount; }

void Settings::setBackupRetentionCount(int count) {
  if (m_backupRetentionCount != count) {
    m_backupRetentionCount = count;
    save();
    emit settingsChanged();
  }
}

void Settings::save() {
  // Build JSON object with all settings
  QJsonObject json;
  json["autoMathEnabled"] = m_autoMathEnabled;
  json["currencyConversionEnabled"] = m_currencyConversionEnabled;
  json["baseCurrency"] = m_baseCurrency;
  json["currencyApiKey"] = m_currencyApiKey;
  json["cryptoApiKey"] = m_cryptoApiKey;
  json["currencyProvider"] = m_currencyProvider;
  json["refreshIntervalMinutes"] = m_refreshIntervalMinutes;
  json["autoPasteEnabled"] = m_autoPasteEnabled;
  json["windowWidth"] = m_windowSize.width();
  json["windowHeight"] = m_windowSize.height();
  json["windowX"] = m_windowPosition.x();
  json["windowY"] = m_windowPosition.y();
  json["openWithNewNote"] = m_openWithNewNote;
  json["themeName"] = m_themeName;
  json["darkMode"] = m_darkMode;
  json["transparency"] = m_transparency;
  json["displayMode"] = static_cast<int>(m_displayMode);
  json["toolbarAutoHide"] = m_toolbarAutoHide;
  json["autoCheckboxEnabled"] = m_autoCheckboxEnabled;
  json["pomodoroWorkLabel"] = m_pomodoroWorkLabel;
  json["pomodoroBreakLabel"] = m_pomodoroBreakLabel;
  json["pomodoroWorkMinutes"] = m_pomodoroWorkMinutes;
  json["pomodoroBreakMinutes"] = m_pomodoroBreakMinutes;
  json["timerAutoStartStopwatch"] = m_timerAutoStartStopwatch;
  json["fontFamily"] = m_fontFamily;
  json["fontSize"] = m_fontSize;
  json["globalHotkey"] = m_globalHotkey;
  json["masterPasswordHash"] = m_masterPasswordHash;
  json["recoveryKey"] = m_recoveryKey;
  json["firstRunComplete"] = m_firstRunComplete;
  json["encryptionEnabled"] = m_encryptionEnabled;
  json["autoLockMinutes"] = m_autoLockMinutes;
  json["backupEnabled"] = m_backupEnabled;
  json["backupPath"] = m_backupPath;
  json["backupIntervalHours"] = m_backupIntervalHours;
  json["backupRetentionCount"] = m_backupRetentionCount;

  // New settings - Checkpoint 3
  json["linkAutoShortenEnabled"] = m_linkAutoShortenEnabled;
  json["soundEnabled"] = m_soundEnabled;
  json["soundVolume"] = m_soundVolume;
  json["pasteRemoveLeadingSpaces"] = m_pasteRemoveLeadingSpaces;
  json["pasteRemoveNumbers"] = m_pasteRemoveNumbers;
  json["pasteRemoveBullets"] = m_pasteRemoveBullets;
  json["pasteRemoveMarkdown"] = m_pasteRemoveMarkdown;
  json["pasteRemoveEmptyLines"] = m_pasteRemoveEmptyLines;
  json["keepIndentationOnEnter"] = m_keepIndentationOnEnter;
  json["skipKeywordsOnCopy"] = m_skipKeywordsOnCopy;
  json["skipTriggersOnCopy"] = m_skipTriggersOnCopy;
  json["defaultMode"] = m_defaultMode;
  json["defaultCodeLanguage"] = m_defaultCodeLanguage;
  json["noteTitleMode"] = m_noteTitleMode;
  json["onboardingCompleted"] = m_onboardingCompleted;
  json["examplesShown"] = m_examplesShown;

  // Keyword aliases
  QJsonObject aliasesObj;
  for (auto it = m_keywordAliases.constBegin();
       it != m_keywordAliases.constEnd(); ++it) {
    aliasesObj[it.key()] = it.value();
  }
  json["keywordAliases"] = aliasesObj;

  // Save shortcuts
  QJsonObject shortcutsObj;
  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    shortcutsObj[it.key()] = it.value().toString();
  }
  json["shortcuts"] = shortcutsObj;

  // Save to SQLite
  static SqliteStorage *storage = nullptr;
  if (!storage) {
    storage = new SqliteStorage();
  }
  if (storage->saveSettings(json)) {
    qDebug() << "Settings: Saved to SQLite";
  } else {
    qWarning() << "Settings: Failed to save to SQLite";
  }
}

void Settings::load() {
  // Try to load from SQLite first
  static SqliteStorage *storage = nullptr;
  if (!storage) {
    storage = new SqliteStorage();
  }

  QJsonObject json = storage->loadSettings();

  // If no settings in SQLite, try to migrate from JSON file
  if (json.isEmpty()) {
    QString path = settingsPath();
    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
      QJsonParseError error;
      QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
      file.close();

      if (error.error == QJsonParseError::NoError) {
        json = doc.object();
        qDebug() << "Settings: Migrating from JSON to SQLite";

        // Save to SQLite for future loads
        storage->saveSettings(json);

        // Delete old JSON file after successful migration
        QFile::remove(path);
        qDebug() << "Settings: Removed old JSON file after migration";
      }
    }
  }

  // Apply settings from JSON object (defaults if empty)
  m_autoMathEnabled = json["autoMathEnabled"].toBool(true);
  m_currencyConversionEnabled = json["currencyConversionEnabled"].toBool(true);
  m_baseCurrency = json["baseCurrency"].toString("USD");
  m_currencyApiKey = json["currencyApiKey"].toString("");
  m_cryptoApiKey = json["cryptoApiKey"].toString("gz2pvqoerci2x921124g");
  m_currencyProvider = json["currencyProvider"].toString("frankfurter");
  m_refreshIntervalMinutes = json["refreshIntervalMinutes"].toInt(360);
  m_autoPasteEnabled = json["autoPasteEnabled"].toBool(true);
  m_windowSize =
      QSize(json["windowWidth"].toInt(500), json["windowHeight"].toInt(400));
  m_windowPosition =
      QPoint(json["windowX"].toInt(-1), json["windowY"].toInt(-1));
  m_openWithNewNote = json["openWithNewNote"].toBool(false);
  // Set default theme based on desktop environment on first run
  QString defaultTheme = "Catppuccin";
  if (!json.contains("themeName")) {
    LinNote::DesktopEnvironment de = LinNote::detectDesktopEnvironment();
    if (de == LinNote::DesktopEnvironment::KDE) {
      defaultTheme = "Breeze";
    } else if (de == LinNote::DesktopEnvironment::GNOME) {
      defaultTheme = "Adwaita";
    }
  }
  m_themeName = json["themeName"].toString(defaultTheme);
  m_darkMode = json["darkMode"].toBool(true);
  m_transparency = json["transparency"].toInt(100);
  m_displayMode =
      static_cast<DisplayMode>(json["displayMode"].toInt(2)); // 2=Both
  m_toolbarAutoHide = json["toolbarAutoHide"].toBool(false);
  m_autoCheckboxEnabled = json["autoCheckboxEnabled"].toBool(true);
  m_pomodoroWorkLabel = json["pomodoroWorkLabel"].toString("Work");
  m_pomodoroBreakLabel = json["pomodoroBreakLabel"].toString("Break");
  m_pomodoroWorkMinutes = json["pomodoroWorkMinutes"].toInt(25);
  m_pomodoroBreakMinutes = json["pomodoroBreakMinutes"].toInt(5);
  m_timerAutoStartStopwatch = json["timerAutoStartStopwatch"].toBool(false);
  m_fontFamily = json["fontFamily"].toString("Inter");
  m_fontSize = json["fontSize"].toInt(13);
  m_globalHotkey = json["globalHotkey"].toString("<Super><Control>n");
  m_masterPasswordHash = json["masterPasswordHash"].toString();
  m_recoveryKey = json["recoveryKey"].toString();
  m_firstRunComplete = json["firstRunComplete"].toBool(false);
  m_encryptionEnabled = json["encryptionEnabled"].toBool(false);
  m_autoLockMinutes = json["autoLockMinutes"].toInt(0);
  m_backupEnabled = json["backupEnabled"].toBool(false);
  m_backupPath = json["backupPath"].toString();
  m_backupIntervalHours = json["backupIntervalHours"].toInt(3);
  m_backupRetentionCount = json["backupRetentionCount"].toInt(12);

  // New settings - Checkpoint 3
  m_linkAutoShortenEnabled = json["linkAutoShortenEnabled"].toBool(true);
  m_soundEnabled = json["soundEnabled"].toBool(true);
  m_soundVolume = json["soundVolume"].toInt(75);
  m_pasteRemoveLeadingSpaces = json["pasteRemoveLeadingSpaces"].toBool(false);
  m_pasteRemoveNumbers = json["pasteRemoveNumbers"].toBool(false);
  m_pasteRemoveBullets = json["pasteRemoveBullets"].toBool(false);
  m_pasteRemoveMarkdown = json["pasteRemoveMarkdown"].toBool(false);
  m_pasteRemoveEmptyLines = json["pasteRemoveEmptyLines"].toBool(false);
  m_keepIndentationOnEnter = json["keepIndentationOnEnter"].toBool(true);
  m_skipKeywordsOnCopy = json["skipKeywordsOnCopy"].toBool(true);
  m_skipTriggersOnCopy = json["skipTriggersOnCopy"].toBool(true);
  m_defaultMode = json["defaultMode"].toInt(0);
  m_defaultCodeLanguage = json["defaultCodeLanguage"].toString("javascript");
  m_noteTitleMode = json["noteTitleMode"].toInt(0);
  m_onboardingCompleted = json["onboardingCompleted"].toBool(false);
  m_examplesShown = json["examplesShown"].toBool(false);

  // Keyword aliases
  if (json.contains("keywordAliases")) {
    QJsonObject aliasesObj = json["keywordAliases"].toObject();
    for (auto it = aliasesObj.constBegin(); it != aliasesObj.constEnd(); ++it) {
      m_keywordAliases[it.key()] = it.value().toString();
    }
  }

  // Load shortcuts (merge with defaults)
  if (json.contains("shortcuts")) {
    QJsonObject shortcutsObj = json["shortcuts"].toObject();
    for (auto it = shortcutsObj.constBegin(); it != shortcutsObj.constEnd();
         ++it) {
      m_shortcuts[it.key()] = QKeySequence(it.value().toString());
    }
  }

  qDebug() << "Settings: Loaded from SQLite";
}

QString Settings::settingsPath() const {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dataPath.isEmpty())
    return QString();

  QDir dir(dataPath);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  return dataPath + "/settings.json";
}

// === KEYBOARD SHORTCUTS ===

QKeySequence Settings::shortcut(const QString &id) const {
  return m_shortcuts.value(id, defaultShortcuts().value(id));
}

void Settings::setShortcut(const QString &id, const QKeySequence &seq) {
  if (m_shortcuts.value(id) != seq) {
    m_shortcuts[id] = seq;
    save();
    emit shortcutsChanged();
    emit settingsChanged();
  }
}

QMap<QString, QKeySequence> Settings::allShortcuts() const {
  return m_shortcuts;
}

void Settings::resetShortcutsToDefaults() {
  m_shortcuts = defaultShortcuts();
  save();
  emit shortcutsChanged();
  emit settingsChanged();
}

QMap<QString, QKeySequence> Settings::defaultShortcuts() {
  QMap<QString, QKeySequence> defaults;

  // Page operations
  defaults["newPage"] = QKeySequence("Ctrl+N");
  defaults["deletePage"] = QKeySequence("Ctrl+D");

  // Navigation
  defaults["nextPage"] = QKeySequence("Ctrl+Tab");
  defaults["prevPage"] = QKeySequence("Ctrl+Shift+Tab");
  defaults["firstPage"] = QKeySequence("Ctrl+Home");
  defaults["lastPage"] = QKeySequence("Ctrl+End");

  // Quick page access (Ctrl+1 to Ctrl+9)
  for (int i = 1; i <= 9; i++) {
    defaults[QString("goToPage%1").arg(i)] =
        QKeySequence(QString("Ctrl+%1").arg(i));
  }

  // File operations
  defaults["export"] = QKeySequence("Ctrl+E");

  // Window operations
  defaults["alwaysOnTop"] = QKeySequence("Ctrl+T");
  defaults["hide"] = QKeySequence("Esc");

  // Editing
  defaults["find"] = QKeySequence("Ctrl+F");
  defaults["replace"] = QKeySequence("Ctrl+H");
  defaults["toggleLock"] = QKeySequence("Ctrl+L");

  // OCR
  defaults["ocr"] = QKeySequence("Ctrl+Shift+O");

  // URL Shortening
  defaults["shortenUrl"] = QKeySequence("Ctrl+Shift+L");

  return defaults;
}

// OCR settings
QString Settings::ocrLanguage() const { return m_ocrLanguage; }

void Settings::setOcrLanguage(const QString &lang) {
  if (m_ocrLanguage != lang) {
    m_ocrLanguage = lang;
    save();
    emit settingsChanged();
  }
}

bool Settings::ocrCreateNewNote() const { return m_ocrCreateNewNote; }

void Settings::setOcrCreateNewNote(bool createNew) {
  if (m_ocrCreateNewNote != createNew) {
    m_ocrCreateNewNote = createNew;
    save();
    emit settingsChanged();
  }
}

bool Settings::markdownLivePreview() const { return m_markdownLivePreview; }

void Settings::setMarkdownLivePreview(bool enabled) {
  if (m_markdownLivePreview != enabled) {
    m_markdownLivePreview = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::autoPasteCreateNewNote() const {
  return m_autoPasteCreateNewNote;
}

void Settings::setAutoPasteCreateNewNote(bool createNew) {
  if (m_autoPasteCreateNewNote != createNew) {
    m_autoPasteCreateNewNote = createNew;
    save();
    emit settingsChanged();
  }
}

QString Settings::urlShortenerService() const {
  return m_urlShortenerService.isEmpty() ? "is.gd" : m_urlShortenerService;
}

void Settings::setUrlShortenerService(const QString &service) {
  if (m_urlShortenerService != service) {
    m_urlShortenerService = service;
    save();
    emit settingsChanged();
  }
}

// === CHECKPOINT 3 NEW SETTINGS ===

bool Settings::linkAutoShortenEnabled() const {
  return m_linkAutoShortenEnabled;
}

void Settings::setLinkAutoShortenEnabled(bool enabled) {
  if (m_linkAutoShortenEnabled != enabled) {
    m_linkAutoShortenEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::soundEnabled() const { return m_soundEnabled; }

void Settings::setSoundEnabled(bool enabled) {
  if (m_soundEnabled != enabled) {
    m_soundEnabled = enabled;
    save();
    emit settingsChanged();
  }
}

int Settings::soundVolume() const { return m_soundVolume; }

void Settings::setSoundVolume(int volume) {
  if (m_soundVolume != volume) {
    m_soundVolume = qBound(0, volume, 100);
    save();
    emit settingsChanged();
  }
}

bool Settings::pasteRemoveLeadingSpaces() const {
  return m_pasteRemoveLeadingSpaces;
}

void Settings::setPasteRemoveLeadingSpaces(bool enabled) {
  if (m_pasteRemoveLeadingSpaces != enabled) {
    m_pasteRemoveLeadingSpaces = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::pasteRemoveNumbers() const { return m_pasteRemoveNumbers; }

void Settings::setPasteRemoveNumbers(bool enabled) {
  if (m_pasteRemoveNumbers != enabled) {
    m_pasteRemoveNumbers = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::pasteRemoveBullets() const { return m_pasteRemoveBullets; }

void Settings::setPasteRemoveBullets(bool enabled) {
  if (m_pasteRemoveBullets != enabled) {
    m_pasteRemoveBullets = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::pasteRemoveMarkdown() const { return m_pasteRemoveMarkdown; }

void Settings::setPasteRemoveMarkdown(bool enabled) {
  if (m_pasteRemoveMarkdown != enabled) {
    m_pasteRemoveMarkdown = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::pasteRemoveEmptyLines() const { return m_pasteRemoveEmptyLines; }

void Settings::setPasteRemoveEmptyLines(bool enabled) {
  if (m_pasteRemoveEmptyLines != enabled) {
    m_pasteRemoveEmptyLines = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::keepIndentationOnEnter() const {
  return m_keepIndentationOnEnter;
}

void Settings::setKeepIndentationOnEnter(bool enabled) {
  if (m_keepIndentationOnEnter != enabled) {
    m_keepIndentationOnEnter = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::skipKeywordsOnCopy() const { return m_skipKeywordsOnCopy; }

void Settings::setSkipKeywordsOnCopy(bool enabled) {
  if (m_skipKeywordsOnCopy != enabled) {
    m_skipKeywordsOnCopy = enabled;
    save();
    emit settingsChanged();
  }
}

bool Settings::skipTriggersOnCopy() const { return m_skipTriggersOnCopy; }

void Settings::setSkipTriggersOnCopy(bool enabled) {
  if (m_skipTriggersOnCopy != enabled) {
    m_skipTriggersOnCopy = enabled;
    save();
    emit settingsChanged();
  }
}

int Settings::defaultMode() const { return m_defaultMode; }

void Settings::setDefaultMode(int mode) {
  if (m_defaultMode != mode) {
    m_defaultMode = mode;
    save();
    emit settingsChanged();
  }
}

QString Settings::defaultCodeLanguage() const {
  return m_defaultCodeLanguage.isEmpty() ? "javascript" : m_defaultCodeLanguage;
}

void Settings::setDefaultCodeLanguage(const QString &lang) {
  if (m_defaultCodeLanguage != lang) {
    m_defaultCodeLanguage = lang;
    save();
    emit settingsChanged();
  }
}

int Settings::noteTitleMode() const { return m_noteTitleMode; }

void Settings::setNoteTitleMode(int mode) {
  if (m_noteTitleMode != mode) {
    m_noteTitleMode = mode;
    save();
    emit settingsChanged();
  }
}

QMap<QString, QString> Settings::keywordAliases() const {
  return m_keywordAliases;
}

void Settings::setKeywordAlias(const QString &original, const QString &alias) {
  qDebug() << "Settings::setKeywordAlias" << original << "->" << alias;
  if (alias.isEmpty()) {
    m_keywordAliases.remove(original);
  } else {
    m_keywordAliases[original] = alias;
  }
  qDebug() << "Settings: aliases now:" << m_keywordAliases;
  save();
  emit settingsChanged();
}

void Settings::setKeywordAliases(const QMap<QString, QString> &aliases) {
  if (m_keywordAliases != aliases) {
    m_keywordAliases = aliases;
    save();
    emit settingsChanged();
  }
}

bool Settings::onboardingCompleted() const { return m_onboardingCompleted; }

void Settings::setOnboardingCompleted(bool completed) {
  if (m_onboardingCompleted != completed) {
    m_onboardingCompleted = completed;
    save();
  }
}

bool Settings::examplesShown() const { return m_examplesShown; }

void Settings::setExamplesShown(bool shown) {
  if (m_examplesShown != shown) {
    m_examplesShown = shown;
    save();
  }
}

// Update KDE global shortcuts when hotkey changes in Settings
void Settings::updateKdeGlobalShortcut(const QString &hotkey) {
  // Only run under KDE
  QString desktop = qgetenv("XDG_CURRENT_DESKTOP");
  if (!desktop.contains("KDE", Qt::CaseInsensitive)) {
    return;
  }

  // Convert portal format (<Super><Control>n) to Qt key code
  // We need to convert to a native key code for DBus
  int keyCode = 0;

  // Parse modifiers
  QString remaining = hotkey;
  if (remaining.contains("<Super>")) {
    keyCode |= Qt::MetaModifier;
    remaining.remove("<Super>");
  }
  if (remaining.contains("<Control>")) {
    keyCode |= Qt::ControlModifier;
    remaining.remove("<Control>");
  }
  if (remaining.contains("<Alt>")) {
    keyCode |= Qt::AltModifier;
    remaining.remove("<Alt>");
  }
  if (remaining.contains("<Shift>")) {
    keyCode |= Qt::ShiftModifier;
    remaining.remove("<Shift>");
  }

  // Parse key - remaining should be the key name
  remaining = remaining.trimmed().toLower();
  if (remaining == "n")
    keyCode |= Qt::Key_N;
  else if (remaining == "l")
    keyCode |= Qt::Key_L;
  else if (remaining == "space")
    keyCode |= Qt::Key_Space;
  else if (remaining == "f12")
    keyCode = Qt::Key_F12; // F12 alone, no modifiers
  else
    keyCode |= Qt::Key_N; // Default

  // Update kglobalshortcutsrc file AND call qdbus6 to refresh
  QString kdeHotkey = hotkey;
  kdeHotkey.replace("<Super>", "Meta+");
  kdeHotkey.replace("<Control>", "Ctrl+");
  kdeHotkey.replace("<Alt>", "Alt+");
  kdeHotkey.replace("<Shift>", "Shift+");
  kdeHotkey.replace("<", "").replace(">", "");
  if (!kdeHotkey.isEmpty() && kdeHotkey != "F12") {
    kdeHotkey =
        kdeHotkey.left(kdeHotkey.length() - 1) + kdeHotkey.right(1).toUpper();
  }

  // Update config file
  QString configPath =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/kglobalshortcutsrc";
  QFile configFile(configPath);

  // Read current content
  QString content;
  if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    content = configFile.readAll();
    configFile.close();
  }

  // Find and update LinNote shortcut section
  QString sectionName = "[linnote.desktop]";
  int sectionStart = content.indexOf(sectionName);

  if (sectionStart == -1) {
    // Section doesn't exist, append it
    content += "\n" + sectionName + "\n";
    content += "_k_friendly_name=LinNote\n";
    content += "Toggle=" + kdeHotkey + ",none,Toggle LinNote\n";
  } else {
    // Find Toggle= line in this section
    int nextSection = content.indexOf("[", sectionStart + 1);
    if (nextSection == -1)
      nextSection = content.length();

    int toggleStart = content.indexOf("Toggle=", sectionStart);
    if (toggleStart != -1 && toggleStart < nextSection) {
      int lineEnd = content.indexOf("\n", toggleStart);
      QString oldLine = content.mid(toggleStart, lineEnd - toggleStart);
      QString newLine = "Toggle=" + kdeHotkey + ",none,Toggle LinNote";
      content.replace(oldLine, newLine);
    } else {
      // Toggle line doesn't exist, insert it
      content.insert(sectionStart + sectionName.length() + 1,
                     "Toggle=" + kdeHotkey + ",none,Toggle LinNote\n");
    }
  }

  // Write back
  if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    configFile.write(content.toUtf8());
    configFile.close();
  }

  // Use kwriteconfig6 to update kglobalshortcutsrc - this is the reliable
  // method
  QString shortcutValue = kdeHotkey + ",none,Toggle LinNote";

  QProcess::execute("kwriteconfig6", {"--file", "kglobalshortcutsrc", "--group",
                                      "linnote.desktop", "--key",
                                      "_k_friendly_name", "LinNote"});

  QProcess::execute("kwriteconfig6",
                    {"--file", "kglobalshortcutsrc", "--group",
                     "linnote.desktop", "--key", "Toggle", shortcutValue});

  qDebug() << "KDE global shortcut synced to:" << kdeHotkey;
}
