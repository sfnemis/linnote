#ifndef LINNOTE_SETTINGSDIALOG_H
#define LINNOTE_SETTINGSDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFontDatabase>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

class UpdateChecker;

/**
 * @brief Settings dialog for LinNote preferences
 * Modern 2-panel sidebar layout
 */
class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = nullptr);

  enum Category {
    Theme,
    Editor,
    Backup,
    Security,
    Shortcuts,
    Calculator,
    Timer,
    About
  };

private slots:
  void onCategoryChanged(int row);
  void onAutoMathToggled(bool checked);
  void onCurrencyToggled(bool checked);
  void onCurrencyChanged(int index);
  void onAutoPasteToggled(bool checked);
  void onOpenWithNewNoteToggled(bool checked);
  void onApiKeyChanged();
  void onProviderChanged(int index);
  void onTestApiClicked();
  void onRefreshRatesClicked();
  void onRefreshIntervalChanged(int index);
  void onThemeChanged(int index);
  void onDarkModeToggled(bool checked);
  void onTransparencyChanged(int value);
  void onAutoCheckboxToggled(bool checked);
  void onPomodoroWorkLabelChanged();
  void onPomodoroBreakLabelChanged();
  void onPomodoroWorkMinutesChanged(int value);
  void onPomodoroBreakMinutesChanged(int value);
  void onHotkeyChanged(const QKeySequence &keySequence);
  void onSetMasterPasswordClicked();
  void onRemoveMasterPasswordClicked();
  void onUseRecoveryKeyClicked();
  void onAutoLockChanged(int index);
  void onFontFamilyChanged(const QString &family);
  void onFontSizeChanged(int size);
  void onBackupEnabledToggled(bool enabled);
  void onBackupPathChanged(const QString &path);
  void onBackupBrowseClicked();
  void onBackupIntervalChanged(int hours);
  void onBackupRetentionChanged(int count);
  void onBackupNowClicked();
  void onStartOnBootToggled(bool checked);
  void onCheckForUpdatesClicked();
  void onUpdateFound(const QString &version, const QString &changelog);
  void onNoUpdateFound();
  void onUpdateCheckFailed(const QString &error);
  void updateDynamicStyles();

private:
  void setupUi();
  void loadSettings();
  void updateFontPreview();
  void updateLastRefreshLabel();
  void updateProviderHint();
  bool isAutostartEnabled() const;
  void setAutostartEnabled(bool enabled);

  // Sidebar page creators
  QWidget *createVisualPage();
  QWidget *createEditorPage();
  QWidget *createBackupPage();
  QWidget *createSecurityPage();
  QWidget *createShortcutsPage();
  QWidget *createCalculatorPage();
  QWidget *createTimerPage();
  QWidget *createOcrPage();
  QWidget *createKeywordsPage();
  QWidget *createAboutPage();

  // Layout components
  QListWidget *m_sidebar;
  QStackedWidget *m_pages;

  // Theme page
  QComboBox *m_themeCombo;
  QCheckBox *m_darkModeCheck;
  QSlider *m_transparencySlider;
  QComboBox *m_displayModeCombo;
  QCheckBox *m_startOnBootCheck;

  // Editor page
  QComboBox *m_fontCombo;
  QSpinBox *m_fontSizeSpin;
  QLabel *m_fontPreview;
  QCheckBox *m_autoPasteCheck;
  QCheckBox *m_openWithNewNoteCheck;
  QCheckBox *m_autoCheckboxCheck;
  QCheckBox *m_markdownLivePreviewCheck;
  QCheckBox *m_autoPasteCreateNewNoteCheck;

  // Backup page
  QCheckBox *m_backupEnabledCheck;
  QLineEdit *m_backupPathEdit;
  QPushButton *m_backupBrowseBtn;
  QSpinBox *m_backupIntervalSpin;
  QSpinBox *m_backupRetentionSpin;
  QPushButton *m_backupNowBtn;

  // Security page
  QPushButton *m_setMasterPasswordBtn;
  QPushButton *m_removeMasterPasswordBtn;
  QPushButton *m_useRecoveryKeyBtn;
  QLabel *m_masterPasswordStatus;
  QComboBox *m_autoLockCombo;

  // Shortcuts page
  QKeySequenceEdit *m_hotkeyEdit;
  QMap<QString, QKeySequenceEdit *> m_shortcutEdits;
  QPushButton *m_resetShortcutsBtn;

  // Calculator page
  QCheckBox *m_autoMathCheck;
  QCheckBox *m_currencyCheck;
  QComboBox *m_currencyCombo;
  QComboBox *m_providerCombo;
  QLineEdit *m_apiKeyEdit;
  QLineEdit *m_cryptoApiKeyEdit;
  QPushButton *m_testApiBtn;
  QLabel *m_apiStatusLabel;
  QPushButton *m_testCryptoApiBtn;
  QLabel *m_cryptoApiStatusLabel;
  QComboBox *m_refreshIntervalCombo;
  QLabel *m_lastRefreshLabel;
  QLabel *m_providerHintLabel;
  QComboBox *m_urlShortenerCombo;

  // Timer page
  QLineEdit *m_pomodoroWorkLabelEdit;
  QLineEdit *m_pomodoroBreakLabelEdit;
  QSpinBox *m_pomodoroWorkMinutesSpin;
  QSpinBox *m_pomodoroBreakMinutesSpin;
  QCheckBox *m_timerAutoStartCheck;

  // About page - Update checker
  QPushButton *m_checkUpdatesBtn;
  QLabel *m_versionLabel;
  QLabel *m_updateStatusLabel;
  UpdateChecker *m_updateChecker;

signals:
  void themeChanged();
  void transparencyChanged(int value);
  void backupSettingsChanged();
  void fontChanged();
};

#endif // LINNOTE_SETTINGSDIALOG_H
