#include "FirstRunDialog.h"
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

FirstRunDialog::FirstRunDialog(QWidget *parent)
    : QDialog(parent), m_encryptionEnabled(false) {
  setWindowTitle(tr("LinNote Setup"));
  setMinimumSize(560, 520);
  resize(560, 580);
  setupUi();

  // Position at screen center
  QScreen *screen = QApplication::primaryScreen();
  if (screen) {
    QRect screenGeom = screen->availableGeometry();
    move(screenGeom.center() - rect().center());
  }
}

void FirstRunDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Pages container
  m_pages = new QStackedWidget();
  createWelcomePage();
  createPreferencesPage();
  createSecurityPage();
  createReadyPage();
  mainLayout->addWidget(m_pages, 1);

  // Navigation bar - use platform colors
  QWidget *navBar = new QWidget();
  QHBoxLayout *navLayout = new QHBoxLayout(navBar);
  navLayout->setContentsMargins(20, 12, 20, 12);

  m_pageIndicator = new QLabel();
  navLayout->addWidget(m_pageIndicator);

  navLayout->addStretch();

  m_prevBtn = new QPushButton(tr("← Back"));
  connect(m_prevBtn, &QPushButton::clicked, this, &FirstRunDialog::prevPage);
  navLayout->addWidget(m_prevBtn);

  m_nextBtn = new QPushButton(tr("Next →"));
  connect(m_nextBtn, &QPushButton::clicked, this, &FirstRunDialog::nextPage);
  navLayout->addWidget(m_nextBtn);

  mainLayout->addWidget(navBar);

  updateNavButtons();
}

void FirstRunDialog::createWelcomePage() {
  m_welcomePage = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(m_welcomePage);
  layout->setContentsMargins(30, 20, 30, 20);
  layout->setSpacing(12);

  // Logo - centered at top
  QLabel *logoLabel = new QLabel();
  QPixmap logoPix(":/app/icons/app/linnote-128.png");
  if (!logoPix.isNull()) {
    logoLabel->setPixmap(
        logoPix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
  logoLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(logoLabel);

  // Title
  QLabel *title = new QLabel(tr("Welcome to LinNote"));
  title->setStyleSheet("font-size: 22px; font-weight: bold;");
  title->setAlignment(Qt::AlignCenter);
  layout->addWidget(title);

  QLabel *subtitle = new QLabel(tr("Your powerful Linux scratchpad"));
  subtitle->setAlignment(Qt::AlignCenter);
  layout->addWidget(subtitle);

  layout->addSpacing(8);

  // Features in a scroll area
  QScrollArea *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);

  QWidget *featuresWidget = new QWidget();
  QVBoxLayout *featuresLayout = new QVBoxLayout(featuresWidget);
  featuresLayout->setContentsMargins(0, 0, 0, 0);
  featuresLayout->setSpacing(8);

  // Feature labels - plain text, no HTML issues
  QLabel *f1 = new QLabel(tr(
      "Mode Keywords\n"
      "Type at line start: calc, list, code, timer, markdown, paste, plain"));
  f1->setWordWrap(true);
  featuresLayout->addWidget(f1);

  QLabel *f2 = new QLabel(tr("Slash Commands\n"
                             "Press / anywhere to open quick command menu"));
  f2->setWordWrap(true);
  featuresLayout->addWidget(f2);

  QLabel *f3 = new QLabel(tr("Text Analysis\n"
                             "sum - Sum all numbers\n"
                             "avg - Average of numbers\n"
                             "count - Character & word count"));
  f3->setWordWrap(true);
  featuresLayout->addWidget(f3);

  QLabel *f4 = new QLabel(tr("Smart Calculations\n"
                             "End lines with = for instant results\n"
                             "Currency: 100 USD to EUR\n"
                             "Units: 50 km to miles"));
  f4->setWordWrap(true);
  featuresLayout->addWidget(f4);

  QLabel *f5 = new QLabel(tr("Power Features\n"
                             "Ctrl+Shift+O - Screen capture (OCR)\n"
                             "Ctrl+Shift+L - Shorten selected URL"));
  f5->setWordWrap(true);
  featuresLayout->addWidget(f5);

  featuresLayout->addStretch();
  scroll->setWidget(featuresWidget);
  layout->addWidget(scroll, 1);

  m_pages->addWidget(m_welcomePage);
}

void FirstRunDialog::createPreferencesPage() {
  m_prefsPage = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(m_prefsPage);
  layout->setContentsMargins(30, 30, 30, 20);
  layout->setSpacing(16);

  QLabel *title = new QLabel(tr("Preferences"));
  title->setStyleSheet("font-size: 22px; font-weight: bold;");
  layout->addWidget(title);

  QLabel *desc = new QLabel(tr("Set your preferred defaults"));
  layout->addWidget(desc);

  layout->addSpacing(8);

  // Theme selection - REAL themes from ThemeManager
  QLabel *themeLabel = new QLabel(tr("Color Theme:"));
  layout->addWidget(themeLabel);

  m_themeCombo = new QComboBox();
  // Same themes as in Theme.cpp / SettingsDialog
  m_themeCombo->addItems({"Catppuccin", "Tokyo Drift", "Vancouver", "Totoro",
                          "Mononoke", "Piccolo", "Nord", "Dracula", "Gruvbox",
                          "Solarized", "One Dark", "Monokai", "Breeze",
                          "Adwaita"});
  m_themeCombo->setCurrentText("Catppuccin");
  layout->addWidget(m_themeCombo);

  // Dark Mode checkbox
  m_darkModeCheck = new QCheckBox(tr("Dark Mode"));
  m_darkModeCheck->setChecked(true); // Default dark
  layout->addWidget(m_darkModeCheck);

  layout->addSpacing(8);

  // Display Mode selection
  QLabel *displayLabel = new QLabel(tr("Display Mode:"));
  layout->addWidget(displayLabel);

  m_displayModeCombo = new QComboBox();
  m_displayModeCombo->addItem(tr("Tray Icon Only"), 0);
  m_displayModeCombo->addItem(tr("Dock Icon Only"), 1);
  m_displayModeCombo->addItem(tr("Both (Tray + Dock)"), 2);
  m_displayModeCombo->setCurrentIndex(2); // Default: Both
  layout->addWidget(m_displayModeCombo);

  layout->addSpacing(8);

  // Currency selection
  QLabel *currLabel = new QLabel(tr("Default Currency:"));
  layout->addWidget(currLabel);

  m_currencyCombo = new QComboBox();
  // Fiat currencies - same as SettingsDialog
  m_currencyCombo->addItems({"USD", "EUR", "GBP", "JPY", "TRY", "CAD", "AUD",
                             "CHF", "CNY", "NZD", "HKD", "ALL", "INR", "KRW",
                             "BRL", "MXN", "PLN", "SEK", "NOK", "DKK", "SGD",
                             "ZAR", "THB", "AED", "SAR", "RUB"});
  // Crypto currencies
  m_currencyCombo->addItems({"BTC", "ETH", "USDT", "USDC", "XRP", "BNB", "SOL",
                             "ADA", "DOGE", "TRX"});
  m_currencyCombo->setCurrentText("USD");
  layout->addWidget(m_currencyCombo);

  layout->addSpacing(8);

  // Global Hotkey selection
  QLabel *hotkeyLabel = new QLabel(tr("Global Hotkey (Toggle Window):"));
  layout->addWidget(hotkeyLabel);

  m_hotkeyCombo = new QComboBox();
  m_hotkeyCombo->addItem(tr("Meta+Ctrl+N"), "<Super><Control>n");
  m_hotkeyCombo->addItem(tr("Ctrl+Alt+N"), "<Control><Alt>n");
  m_hotkeyCombo->addItem(tr("Meta+Shift+N"), "<Super><Shift>n");
  m_hotkeyCombo->addItem(tr("Ctrl+Alt+L"), "<Control><Alt>l");
  m_hotkeyCombo->addItem(tr("Meta+Space"), "<Super>space");
  m_hotkeyCombo->addItem(tr("F12"), "F12");
  m_hotkeyCombo->setCurrentIndex(0); // Default: Meta+Ctrl+N
  layout->addWidget(m_hotkeyCombo);

  layout->addSpacing(8);

  // Auto-paste checkbox - default OFF
  m_autoPasteCheck = new QCheckBox(tr("Auto-paste clipboard on open"));
  m_autoPasteCheck->setChecked(false); // Default OFF
  layout->addWidget(m_autoPasteCheck);

  // Start on boot checkbox - default ON
  m_startOnBootCheck = new QCheckBox(tr("Start LinNote when computer starts"));
  m_startOnBootCheck->setChecked(true); // Default ON for scratchpad
  layout->addWidget(m_startOnBootCheck);

  layout->addStretch();

  m_pages->addWidget(m_prefsPage);
}

void FirstRunDialog::createSecurityPage() {
  m_securityPage = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(m_securityPage);
  layout->setContentsMargins(30, 30, 30, 20);
  layout->setSpacing(12);

  QLabel *title = new QLabel(tr("Security"));
  title->setStyleSheet("font-size: 22px; font-weight: bold;");
  layout->addWidget(title);

  QLabel *desc = new QLabel(
      tr("Would you like to encrypt your notes with a Master Password?\n\n"
         "If enabled, all notes will be encrypted at rest. "
         "A recovery key will be provided in case you forget your password."));
  desc->setWordWrap(true);
  layout->addWidget(desc);

  // Choice buttons - colored styling
  QHBoxLayout *choiceLayout = new QHBoxLayout();
  m_encryptYesBtn = new QPushButton(tr("🔒 Yes, encrypt my notes"));
  m_encryptNoBtn = new QPushButton(tr("❌ No, keep unencrypted"));
  m_encryptYesBtn->setCheckable(true);
  m_encryptNoBtn->setCheckable(true);
  // Neither selected initially - force user to choose
  m_encryptYesBtn->setChecked(false);
  m_encryptNoBtn->setChecked(false);

  // Green for encrypt (recommended)
  m_encryptYesBtn->setStyleSheet(
      "QPushButton { background-color: #2d7d46; color: white; padding: 10px "
      "16px; "
      "border-radius: 6px; font-weight: bold; } "
      "QPushButton:hover { background-color: #38a55c; } "
      "QPushButton:checked { background-color: #1b5e2e; border: 2px solid "
      "#4caf50; }");

  // Red for no encryption
  m_encryptNoBtn->setStyleSheet(
      "QPushButton { background-color: #c62828; color: white; padding: 10px "
      "16px; "
      "border-radius: 6px; font-weight: bold; } "
      "QPushButton:hover { background-color: #e53935; } "
      "QPushButton:checked { background-color: #8b0000; border: 2px solid "
      "#ff5252; }");

  connect(m_encryptYesBtn, &QPushButton::clicked, this, [this]() {
    m_encryptYesBtn->setChecked(true);
    m_encryptNoBtn->setChecked(false);
    onEncryptionToggled(true);
    updateNavButtons(); // Enable Next button
  });
  connect(m_encryptNoBtn, &QPushButton::clicked, this, [this]() {
    m_encryptYesBtn->setChecked(false);
    m_encryptNoBtn->setChecked(true);
    onEncryptionToggled(false);
    updateNavButtons(); // Enable Next button
  });

  choiceLayout->addWidget(m_encryptYesBtn);
  choiceLayout->addWidget(m_encryptNoBtn);
  layout->addLayout(choiceLayout);

  // Password section
  m_passwordSection = new QWidget();
  QVBoxLayout *passLayout = new QVBoxLayout(m_passwordSection);
  passLayout->setContentsMargins(0, 8, 0, 0);
  passLayout->setSpacing(6);

  QLabel *passLabel = new QLabel(tr("Master Password:"));
  passLayout->addWidget(passLabel);

  m_passwordEdit = new QLineEdit();
  m_passwordEdit->setEchoMode(QLineEdit::Password);
  m_passwordEdit->setPlaceholderText(tr("Enter master password"));
  passLayout->addWidget(m_passwordEdit);

  QLabel *confirmLabel = new QLabel(tr("Confirm Password:"));
  passLayout->addWidget(confirmLabel);

  m_confirmEdit = new QLineEdit();
  m_confirmEdit->setEchoMode(QLineEdit::Password);
  m_confirmEdit->setPlaceholderText(tr("Confirm password"));
  passLayout->addWidget(m_confirmEdit);

  m_passwordMatchLabel = new QLabel();
  passLayout->addWidget(m_passwordMatchLabel);

  connect(m_passwordEdit, &QLineEdit::textChanged, this,
          &FirstRunDialog::onPasswordChanged);
  connect(m_confirmEdit, &QLineEdit::textChanged, this,
          &FirstRunDialog::onPasswordChanged);

  m_passwordSection->setVisible(false);
  layout->addWidget(m_passwordSection);

  // Recovery key section
  m_recoverySection = new QWidget();
  QVBoxLayout *recLayout = new QVBoxLayout(m_recoverySection);
  recLayout->setContentsMargins(0, 8, 0, 0);
  recLayout->setSpacing(6);

  QLabel *recLabel = new QLabel(tr("Recovery Key (SAVE THIS!):"));
  recLabel->setStyleSheet("font-weight: bold; color: #e67700;");
  recLayout->addWidget(recLabel);

  m_recoveryKeyEdit = new QLineEdit();
  m_recoveryKeyEdit->setReadOnly(true);
  m_recoveryKeyEdit->setStyleSheet("font-family: monospace; font-size: 13px;");
  recLayout->addWidget(m_recoveryKeyEdit);

  QPushButton *copyBtn = new QPushButton(tr("Copy to Clipboard"));
  connect(copyBtn, &QPushButton::clicked, this, [this]() {
    QApplication::clipboard()->setText(m_recoveryKeyEdit->text());
    QMessageBox::information(this, tr("Copied"),
                             tr("Recovery key copied to clipboard!"));
  });
  recLayout->addWidget(copyBtn);

  m_recoverySection->setVisible(false);
  layout->addWidget(m_recoverySection);

  layout->addStretch();

  m_pages->addWidget(m_securityPage);
}

void FirstRunDialog::createReadyPage() {
  m_readyPage = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(m_readyPage);
  layout->setContentsMargins(30, 50, 30, 30);
  layout->setSpacing(16);

  QLabel *title = new QLabel(tr("You're All Set!"));
  title->setStyleSheet("font-size: 26px; font-weight: bold;");
  title->setAlignment(Qt::AlignCenter);
  layout->addWidget(title);

  QLabel *desc = new QLabel(tr("LinNote is ready to use.\n\n"
                               "Quick reminders:\n"
                               "• Type keywords to change modes\n"
                               "• Press / for command menu\n"
                               "• Ctrl+N for new page\n"
                               "• Esc to hide window"));
  desc->setAlignment(Qt::AlignCenter);
  layout->addWidget(desc);

  layout->addStretch();

  m_pages->addWidget(m_readyPage);
}

void FirstRunDialog::updateNavButtons() {
  int current = m_pages->currentIndex();
  int total = m_pages->count();

  m_prevBtn->setVisible(current > 0);
  m_pageIndicator->setText(tr("Step %1 of %2").arg(current + 1).arg(total));

  if (current == total - 1) {
    m_nextBtn->setText(tr("Start LinNote"));
  } else {
    m_nextBtn->setText(tr("Next →"));
  }

  // Security page (index 2): disable Next until user makes a choice
  if (current == 2) {
    bool hasSelection =
        m_encryptYesBtn->isChecked() || m_encryptNoBtn->isChecked();
    m_nextBtn->setEnabled(hasSelection);
    if (!hasSelection) {
      m_nextBtn->setToolTip(tr("Please choose an encryption option"));
    } else {
      m_nextBtn->setToolTip("");
    }
  } else {
    m_nextBtn->setEnabled(true);
    m_nextBtn->setToolTip("");
  }
}

void FirstRunDialog::nextPage() {
  int current = m_pages->currentIndex();

  // Validation for security page
  if (current == 2 && m_encryptionEnabled) {
    if (m_passwordEdit->text().isEmpty() ||
        m_passwordEdit->text() != m_confirmEdit->text()) {
      QMessageBox::warning(this, tr("Error"),
                           tr("Please enter matching passwords."));
      return;
    }
  }

  if (current < m_pages->count() - 1) {
    m_pages->setCurrentIndex(current + 1);
    updateNavButtons();
  } else {
    onFinish();
  }
}

void FirstRunDialog::prevPage() {
  int current = m_pages->currentIndex();
  if (current > 0) {
    m_pages->setCurrentIndex(current - 1);
    updateNavButtons();
  }
}

void FirstRunDialog::onEncryptionToggled(bool enabled) {
  m_encryptionEnabled = enabled;
  m_passwordSection->setVisible(enabled);

  if (enabled) {
    m_recoveryKey = generateRecoveryKey();
    m_recoveryKeyEdit->setText(m_recoveryKey);
    m_recoverySection->setVisible(true);
    m_passwordEdit->setFocus();
  } else {
    m_recoverySection->setVisible(false);
    m_recoveryKey.clear();
  }

  onPasswordChanged();
}

void FirstRunDialog::onPasswordChanged() {
  if (!m_encryptionEnabled) {
    m_passwordMatchLabel->clear();
    return;
  }

  QString pass = m_passwordEdit->text();
  QString confirm = m_confirmEdit->text();

  if (pass.isEmpty()) {
    m_passwordMatchLabel->setText(tr("Password cannot be empty"));
    m_passwordMatchLabel->setStyleSheet("color: #c0392b;");
  } else if (pass.length() < 4) {
    m_passwordMatchLabel->setText(tr("Password too short (min 4 characters)"));
    m_passwordMatchLabel->setStyleSheet("color: #c0392b;");
  } else if (pass != confirm) {
    m_passwordMatchLabel->setText(tr("Passwords do not match"));
    m_passwordMatchLabel->setStyleSheet("color: #c0392b;");
  } else {
    m_passwordMatchLabel->setText(tr("Passwords match"));
    m_passwordMatchLabel->setStyleSheet("color: #27ae60;");
  }
}

void FirstRunDialog::onFinish() {
  if (m_encryptionEnabled) {
    int ret = QMessageBox::question(
        this, tr("Recovery Key"),
        tr("Have you saved your recovery key?\n\n"
           "You will NOT be able to recover your notes without it!"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) {
      return;
    }
  }

  accept();
}

QString FirstRunDialog::generateRecoveryKey() {
  const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  QString key;
  for (int i = 0; i < 24; i++) {
    if (i > 0 && i % 4 == 0)
      key += "-";
    key += chars.at(QRandomGenerator::global()->bounded(chars.length()));
  }
  return key;
}

bool FirstRunDialog::encryptionEnabled() const { return m_encryptionEnabled; }
QString FirstRunDialog::password() const { return m_passwordEdit->text(); }
QString FirstRunDialog::recoveryKey() const { return m_recoveryKey; }
QString FirstRunDialog::selectedTheme() const {
  return m_themeCombo->currentText();
}
bool FirstRunDialog::darkModeEnabled() const {
  return m_darkModeCheck->isChecked();
}
QString FirstRunDialog::selectedCurrency() const {
  return m_currencyCombo->currentText();
}
bool FirstRunDialog::autoPasteEnabled() const {
  return m_autoPasteCheck->isChecked();
}
bool FirstRunDialog::startOnBootEnabled() const {
  return m_startOnBootCheck->isChecked();
}
int FirstRunDialog::selectedDisplayMode() const {
  return m_displayModeCombo->currentData().toInt();
}

QString FirstRunDialog::selectedHotkey() const {
  return m_hotkeyCombo->currentData().toString();
}
