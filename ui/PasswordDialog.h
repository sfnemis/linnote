#ifndef LINNOTE_PASSWORDDIALOG_H
#define LINNOTE_PASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

/**
 * @brief Dialog for setting/verifying note password
 */
class PasswordDialog : public QDialog {
  Q_OBJECT

public:
  enum Mode {
    SetPassword,
    VerifyPassword,
    RemovePassword,
    SetMasterPassword,
    VerifyMasterPassword
  };

  explicit PasswordDialog(Mode mode, QWidget *parent = nullptr);

  QString password() const;
  QString confirmPassword() const;

private slots:
  void onOkClicked();

private:
  void setupUI();

  Mode m_mode;
  QLineEdit *m_passwordEdit;
  QLineEdit *m_confirmEdit;
  QPushButton *m_okBtn;
  QPushButton *m_cancelBtn;
};

#endif // LINNOTE_PASSWORDDIALOG_H
