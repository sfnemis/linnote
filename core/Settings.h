#ifndef LINNOTE_SETTINGS_H
#define LINNOTE_SETTINGS_H

#include <QDateTime>
#include <QKeySequence>
#include <QMap>
#include <QObject>
#include <QPoint>
#include <QSize>
#include <QString>

/**
 * @brief Application settings manager
 *
 * Stores user preferences:
 * - Auto math evaluation
 * - Currency conversion + API key + auto-refresh
 * - AutoPaste toggle
 */
class Settings : public QObject {
  Q_OBJECT

public:
  // Display mode for tray/dock visibility
  enum DisplayMode { TrayOnly, DockOnly, Both };
  Q_ENUM(DisplayMode)

  static Settings *instance();

  // Version info (from generated version.h)
  static QString appVersion();
  static int versionMajor();
  static int versionMinor();
  static int versionPatch();
  static QString appName();
  static QString githubRepo();
  static QString githubUrl();

  // Math settings
  bool autoMathEnabled() const;
  void setAutoMathEnabled(bool enabled);

  // Currency settings
  bool currencyConversionEnabled() const;
  void setCurrencyConversionEnabled(bool enabled);
  QString baseCurrency() const;
  void setBaseCurrency(const QString &currency);
  QString currencyApiKey() const;
  void setCurrencyApiKey(const QString &key);
  QString cryptoApiKey() const;
  void setCryptoApiKey(const QString &key);
  QString currencyProvider() const;
  void setCurrencyProvider(const QString &provider);

  // Auto-refresh interval in minutes (0 = disabled)
  int refreshIntervalMinutes() const;
  void setRefreshIntervalMinutes(int minutes);

  // Last currency refresh timestamp
  QDateTime lastCurrencyRefresh() const;
  void setLastCurrencyRefresh(const QDateTime &dt);

  // AutoPaste
  bool autoPasteEnabled() const;
  void setAutoPasteEnabled(bool enabled);

  // Window geometry
  QSize windowSize() const;
  void setWindowSize(const QSize &size);
  QPoint windowPosition() const;
  void setWindowPosition(const QPoint &pos);

  // Toggle behavior: open with new note or last note
  bool openWithNewNote() const;
  void setOpenWithNewNote(bool enabled);

  // Theme settings
  QString themeName() const;
  void setThemeName(const QString &name);
  bool darkMode() const;
  void setDarkMode(bool dark);
  int transparency() const;
  void setTransparency(int value);
  DisplayMode displayMode() const;
  void setDisplayMode(DisplayMode mode);

  // Toolbar auto-hide
  bool toolbarAutoHide() const;
  void setToolbarAutoHide(bool enabled);

  // List mode settings
  bool autoCheckboxEnabled() const;
  void setAutoCheckboxEnabled(bool enabled);

  // Pomodoro settings
  QString pomodoroWorkLabel() const;
  void setPomodoroWorkLabel(const QString &label);
  QString pomodoroBreakLabel() const;
  void setPomodoroBreakLabel(const QString &label);
  int pomodoroWorkMinutes() const;
  void setPomodoroWorkMinutes(int minutes);
  int pomodoroBreakMinutes() const;
  void setPomodoroBreakMinutes(int minutes);

  // Timer settings
  bool timerAutoStartStopwatch() const;
  void setTimerAutoStartStopwatch(bool enabled);

  // Font settings
  QString fontFamily() const;
  void setFontFamily(const QString &family);
  int fontSize() const;
  void setFontSize(int size);

  // Global hotkey (XDG Portal format, e.g. "<Super><Control>n")
  QString globalHotkey() const;
  void setGlobalHotkey(const QString &hotkey);

  // Master Password (optional - for SQLite encryption)
  QString masterPasswordHash() const;
  void setMasterPasswordHash(const QString &hash);
  bool hasMasterPassword() const;

  // Recovery Key (for password reset)
  QString recoveryKey() const;
  void setRecoveryKey(const QString &key);

  // First run check
  bool isFirstRun() const;
  void setFirstRunComplete();

  // Encryption enabled
  bool encryptionEnabled() const;
  void setEncryptionEnabled(bool enabled);

  // Auto-Lock Timer (0 = never, values in minutes)
  int autoLockMinutes() const;
  void setAutoLockMinutes(int minutes);

  // Auto-Backup settings
  bool backupEnabled() const;
  void setBackupEnabled(bool enabled);
  QString backupPath() const;
  void setBackupPath(const QString &path);
  int backupIntervalHours() const;
  void setBackupIntervalHours(int hours);
  int backupRetentionCount() const;
  void setBackupRetentionCount(int count);

  // Keyboard shortcuts
  QKeySequence shortcut(const QString &id) const;
  void setShortcut(const QString &id, const QKeySequence &seq);
  QMap<QString, QKeySequence> allShortcuts() const;
  void resetShortcutsToDefaults();
  static QMap<QString, QKeySequence> defaultShortcuts();

  // OCR settings
  QString ocrLanguage() const;
  void setOcrLanguage(const QString &lang);
  bool ocrCreateNewNote() const;
  void setOcrCreateNewNote(bool createNew);

  // Auto-paste settings
  bool autoPasteCreateNewNote() const;
  void setAutoPasteCreateNewNote(bool createNew);

  // Markdown settings
  bool markdownLivePreview() const;
  void setMarkdownLivePreview(bool enabled);

  // URL Shortener
  QString urlShortenerService() const;
  void setUrlShortenerService(const QString &service);
  bool linkAutoShortenEnabled() const;
  void setLinkAutoShortenEnabled(bool enabled);

  // Sound settings
  bool soundEnabled() const;
  void setSoundEnabled(bool enabled);
  int soundVolume() const;
  void setSoundVolume(int volume);

  // Paste behavior
  bool pasteRemoveLeadingSpaces() const;
  void setPasteRemoveLeadingSpaces(bool enabled);
  bool pasteRemoveNumbers() const;
  void setPasteRemoveNumbers(bool enabled);
  bool pasteRemoveBullets() const;
  void setPasteRemoveBullets(bool enabled);
  bool pasteRemoveMarkdown() const;
  void setPasteRemoveMarkdown(bool enabled);
  bool pasteRemoveEmptyLines() const;
  void setPasteRemoveEmptyLines(bool enabled);

  // Editor behavior
  bool keepIndentationOnEnter() const;
  void setKeepIndentationOnEnter(bool enabled);
  bool skipKeywordsOnCopy() const;
  void setSkipKeywordsOnCopy(bool enabled);
  bool skipTriggersOnCopy() const;
  void setSkipTriggersOnCopy(bool enabled);

  // Default mode and language
  int defaultMode() const;
  void setDefaultMode(int mode);
  QString defaultCodeLanguage() const;
  void setDefaultCodeLanguage(const QString &lang);

  // Note title mode: 0=Smart (content-based), 1=DateTime, 2=FirstLine
  int noteTitleMode() const;
  void setNoteTitleMode(int mode);

  // Onboarding tour
  bool onboardingCompleted() const;
  void setOnboardingCompleted(bool completed);

  // Example notes shown
  bool examplesShown() const;
  void setExamplesShown(bool shown);

  // Keyword aliases
  QMap<QString, QString> keywordAliases() const;
  void setKeywordAlias(const QString &original, const QString &alias);
  void setKeywordAliases(const QMap<QString, QString> &aliases);

  // Persistence
  void save();
  void load();

signals:
  void settingsChanged();
  void apiKeyChanged(const QString &key);
  void refreshIntervalChanged(int minutes);
  void transparencyChanged(int value);
  void shortcutsChanged();

private:
  explicit Settings(QObject *parent = nullptr);
  QString settingsPath() const;
  void updateKdeGlobalShortcut(const QString &hotkey);

  bool m_autoMathEnabled;
  bool m_currencyConversionEnabled;
  QString m_baseCurrency;
  QString m_currencyApiKey;
  QString m_cryptoApiKey;
  QString m_currencyProvider;
  int m_refreshIntervalMinutes;
  QDateTime m_lastCurrencyRefresh;
  bool m_autoPasteEnabled;
  QSize m_windowSize;
  QPoint m_windowPosition;
  bool m_openWithNewNote;
  QString m_themeName;
  bool m_darkMode;
  int m_transparency;
  DisplayMode m_displayMode;
  bool m_toolbarAutoHide;

  // List mode
  bool m_autoCheckboxEnabled;

  // Pomodoro
  QString m_pomodoroWorkLabel;
  QString m_pomodoroBreakLabel;
  int m_pomodoroWorkMinutes;
  int m_pomodoroBreakMinutes;

  // Timer
  bool m_timerAutoStartStopwatch;

  // Font settings
  QString m_fontFamily;
  int m_fontSize;

  // Global hotkey
  QString m_globalHotkey;

  // Master Password (optional - for SQLite encryption)
  QString m_masterPasswordHash;
  QString m_recoveryKey;
  bool m_firstRunComplete;
  bool m_encryptionEnabled;

  // Auto-Lock Timer (0 = never)
  int m_autoLockMinutes;

  // Auto-Backup
  bool m_backupEnabled;
  QString m_backupPath;
  int m_backupIntervalHours;
  int m_backupRetentionCount;

  // Keyboard shortcuts
  QMap<QString, QKeySequence> m_shortcuts;

  // OCR
  QString m_ocrLanguage;
  bool m_ocrCreateNewNote;

  // Auto-paste
  bool m_autoPasteCreateNewNote;

  // Markdown
  bool m_markdownLivePreview;

  // URL Shortener
  QString m_urlShortenerService;
  bool m_linkAutoShortenEnabled;

  // Sound settings
  bool m_soundEnabled;
  int m_soundVolume;

  // Paste behavior
  bool m_pasteRemoveLeadingSpaces;
  bool m_pasteRemoveNumbers;
  bool m_pasteRemoveBullets;
  bool m_pasteRemoveMarkdown;
  bool m_pasteRemoveEmptyLines;

  // Editor behavior
  bool m_keepIndentationOnEnter;
  bool m_skipKeywordsOnCopy;
  bool m_skipTriggersOnCopy;

  // Default mode and language
  int m_defaultMode;
  QString m_defaultCodeLanguage;
  int m_noteTitleMode; // 0=Smart, 1=DateTime, 2=FirstLine

  // Keyword aliases
  QMap<QString, QString> m_keywordAliases;

  // Onboarding tour
  bool m_onboardingCompleted;
  bool m_examplesShown;
};

#endif // LINNOTE_SETTINGS_H
