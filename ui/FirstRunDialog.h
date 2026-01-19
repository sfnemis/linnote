#ifndef LINNOTE_FIRSTRUNDIALOG_H
#define LINNOTE_FIRSTRUNDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>

/**
 * @brief Setup Wizard dialog (4 pages)
 *
 * Page 1: Welcome & Feature overview
 * Page 2: Preferences (Theme, Dark Mode, Currency)
 * Page 3: Security (Master Password)
 * Page 4: Ready to start
 */
class FirstRunDialog : public QDialog {
  Q_OBJECT

public:
  explicit FirstRunDialog(QWidget *parent = nullptr);

  // Results
  bool encryptionEnabled() const;
  QString password() const;
  QString recoveryKey() const;
  QString selectedTheme() const;
  bool darkModeEnabled() const;
  QString selectedCurrency() const;
  bool autoPasteEnabled() const;
  bool startOnBootEnabled() const;
  int selectedDisplayMode() const;
  QString selectedHotkey() const;

private slots:
  void nextPage();
  void prevPage();
  void onEncryptionToggled(bool enabled);
  void onPasswordChanged();
  void onFinish();

private:
  void setupUi();
  void createWelcomePage();
  void createPreferencesPage();
  void createSecurityPage();
  void createReadyPage();
  void updateNavButtons();
  QString generateRecoveryKey();

  // Pages
  QStackedWidget *m_pages;
  QWidget *m_welcomePage;
  QWidget *m_prefsPage;
  QWidget *m_securityPage;
  QWidget *m_readyPage;

  // Navigation
  QPushButton *m_prevBtn;
  QPushButton *m_nextBtn;
  QLabel *m_pageIndicator;

  // Preferences
  QComboBox *m_themeCombo;
  QCheckBox *m_darkModeCheck;
  QComboBox *m_currencyCombo;
  QComboBox *m_displayModeCombo;
  QComboBox *m_hotkeyCombo;
  QCheckBox *m_autoPasteCheck;
  QCheckBox *m_startOnBootCheck;

  // Security
  QPushButton *m_encryptYesBtn;
  QPushButton *m_encryptNoBtn;
  bool m_encryptionEnabled;
  QWidget *m_passwordSection;
  QLineEdit *m_passwordEdit;
  QLineEdit *m_confirmEdit;
  QLabel *m_passwordMatchLabel;
  QWidget *m_recoverySection;
  QLineEdit *m_recoveryKeyEdit;
  QString m_recoveryKey;
};

#endif // LINNOTE_FIRSTRUNDIALOG_H
