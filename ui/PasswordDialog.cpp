#include "PasswordDialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

PasswordDialog::PasswordDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode), m_confirmEdit(nullptr) {
  setupUI();
}

QString PasswordDialog::password() const { return m_passwordEdit->text(); }

QString PasswordDialog::confirmPassword() const {
  return m_confirmEdit ? m_confirmEdit->text() : QString();
}

void PasswordDialog::setupUI() {
  setWindowTitle(m_mode == SetPassword            ? tr("Set Password")
                 : m_mode == SetMasterPassword    ? tr("Set Master Password")
                 : m_mode == VerifyPassword       ? tr("Enter Password")
                 : m_mode == VerifyMasterPassword ? tr("Enter Master Password")
                                                  : tr("Remove Password"));
  setMinimumWidth(300);
  setModal(true);

  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);

  QString titleText;
  switch (m_mode) {
  case SetPassword:
    titleText = tr("🔒 Set a password to lock this note");
    break;
  case SetMasterPassword:
    titleText = tr("🔐 Set your new master password");
    break;
  case VerifyPassword:
    titleText = tr("🔐 This note is locked. Enter password:");
    break;
  case VerifyMasterPassword:
    titleText = tr("🔐 Enter your master password:");
    break;
  case RemovePassword:
    titleText = tr("🔓 Enter current password to unlock:");
    break;
  }
  auto *titleLabel = new QLabel(titleText);
  titleLabel->setWordWrap(true);
  layout->addWidget(titleLabel);

  // Password input
  m_passwordEdit = new QLineEdit();
  m_passwordEdit->setEchoMode(QLineEdit::Password);
  m_passwordEdit->setPlaceholderText(tr("Password"));
  connect(m_passwordEdit, &QLineEdit::returnPressed, this,
          &PasswordDialog::onOkClicked);
  layout->addWidget(m_passwordEdit);

  // Confirm password (only for SetPassword and SetMasterPassword modes)
  if (m_mode == SetPassword || m_mode == SetMasterPassword) {
    m_confirmEdit = new QLineEdit();
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setPlaceholderText(tr("Confirm Password"));
    connect(m_confirmEdit, &QLineEdit::returnPressed, this,
            &PasswordDialog::onOkClicked);
    layout->addWidget(m_confirmEdit);
  }

  // Buttons
  auto *buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();

  m_cancelBtn = new QPushButton(tr("Cancel"));
  connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  buttonLayout->addWidget(m_cancelBtn);

  m_okBtn = new QPushButton(m_mode == SetPassword         ? tr("Lock")
                            : m_mode == SetMasterPassword ? tr("Save")
                            : m_mode == RemovePassword    ? tr("Unlock")
                                                          : tr("OK"));
  m_okBtn->setDefault(true);
  connect(m_okBtn, &QPushButton::clicked, this, &PasswordDialog::onOkClicked);
  buttonLayout->addWidget(m_okBtn);

  layout->addLayout(buttonLayout);

  // Note: Uses application-level theme stylesheet (qApp->setStyleSheet)
  // No hardcoded colors here - theme is applied globally
}

void PasswordDialog::onOkClicked() {
  if (m_passwordEdit->text().isEmpty()) {
    QMessageBox::warning(this, tr("Error"), tr("Password cannot be empty."));
    return;
  }

  if (m_mode == SetPassword || m_mode == SetMasterPassword) {
    if (m_passwordEdit->text().length() < 4) {
      QMessageBox::warning(this, tr("Error"),
                           tr("Password must be at least 4 characters."));
      return;
    }
    if (m_passwordEdit->text() != m_confirmEdit->text()) {
      QMessageBox::warning(this, tr("Error"), tr("Passwords do not match."));
      m_confirmEdit->clear();
      m_confirmEdit->setFocus();
      return;
    }
  }

  accept();
}
