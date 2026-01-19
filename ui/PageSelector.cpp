#include "PageSelector.h"
#include "MainWindow.h"
#include "PasswordDialog.h"
#include "core/NoteManager.h"
#include "core/Settings.h"
#include "storage/Crypto.h"
#include <QCursor>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QStyle>

// Helper: Create themed icon with proper color for current palette
static QIcon createThemedIcon(const QString &type, bool isDark) {
  QPixmap pix(24, 24);
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setRenderHint(QPainter::Antialiasing);

  // Use appropriate color based on theme
  QColor color = isDark ? QColor("#cdd6f4") : QColor("#1e1e2e");
  p.setPen(QPen(color, 2.0));
  p.setBrush(Qt::NoBrush);

  if (type == "first") {
    // |◀ - bar + left arrow
    p.drawLine(6, 6, 6, 18);
    QPolygon arrow;
    arrow << QPoint(18, 6) << QPoint(10, 12) << QPoint(18, 18);
    p.drawPolyline(arrow);
  } else if (type == "prev") {
    // ◀ - left arrow
    QPolygon arrow;
    arrow << QPoint(16, 6) << QPoint(8, 12) << QPoint(16, 18);
    p.drawPolyline(arrow);
  } else if (type == "next") {
    // ▶ - right arrow
    QPolygon arrow;
    arrow << QPoint(8, 6) << QPoint(16, 12) << QPoint(8, 18);
    p.drawPolyline(arrow);
  } else if (type == "last") {
    // ▶| - right arrow + bar
    QPolygon arrow;
    arrow << QPoint(6, 6) << QPoint(14, 12) << QPoint(6, 18);
    p.drawPolyline(arrow);
    p.drawLine(18, 6, 18, 18);
  } else if (type == "add") {
    // + plus sign
    p.drawLine(12, 6, 12, 18);
    p.drawLine(6, 12, 18, 12);
  } else if (type == "delete") {
    // Trash icon
    p.drawRect(7, 8, 10, 12);   // bin body
    p.drawLine(5, 8, 19, 8);    // lid line
    p.drawLine(10, 6, 14, 6);   // handle
    p.drawLine(10, 11, 10, 17); // lines inside
    p.drawLine(14, 11, 14, 17);
  } else if (type == "rename") {
    // Pencil icon
    p.drawLine(6, 18, 18, 6); // pencil body
    p.drawLine(6, 18, 8, 16); // tip
    p.drawLine(16, 8, 18, 6); // eraser
  } else if (type == "lock") {
    // 🔒 Padlock closed
    p.setBrush(color);
    p.drawRoundedRect(7, 11, 10, 9, 2, 2); // body
    p.setBrush(Qt::NoBrush);
    QPainterPath arc;
    arc.moveTo(9, 11);
    arc.lineTo(9, 8);
    arc.cubicTo(9, 5, 15, 5, 15, 8);
    arc.lineTo(15, 11);
    p.drawPath(arc);
  } else if (type == "unlock") {
    // 🔓 Padlock open
    p.setBrush(color);
    p.drawRoundedRect(7, 11, 10, 9, 2, 2); // body
    p.setBrush(Qt::NoBrush);
    QPainterPath arc;
    arc.moveTo(9, 11);
    arc.lineTo(9, 8);
    arc.cubicTo(9, 5, 15, 5, 15, 8);
    arc.lineTo(15, 6); // open shackle
    p.drawPath(arc);
  }

  p.end();
  return QIcon(pix);
}

PageSelector::PageSelector(NoteManager *manager, QWidget *parent)
    : QWidget(parent), m_manager(manager), m_combo(nullptr),
      m_firstBtn(nullptr), m_prevBtn(nullptr), m_nextBtn(nullptr),
      m_lastBtn(nullptr), m_addButton(nullptr), m_deleteButton(nullptr),
      m_lockBtn(nullptr), m_updating(false) {
  setAttribute(Qt::WA_TranslucentBackground);
  setupUi();

  // Connect to manager signals
  connect(m_manager, &NoteManager::noteCreated, this,
          &PageSelector::onNoteCreated);
  connect(m_manager, &NoteManager::noteDeleted, this,
          &PageSelector::onNoteDeleted);
  connect(m_manager, &NoteManager::currentNoteChanged, this,
          &PageSelector::onCurrentNoteChanged);
  connect(m_manager, &NoteManager::notesLoaded, this,
          &PageSelector::updateList);
  // Update list when content changes (for smart titles)
  connect(m_manager, &NoteManager::noteContentChanged, this,
          [this](const QString &) { updateList(); });

  // Update icons when theme changes
  connect(Settings::instance(), &Settings::settingsChanged, this,
          &PageSelector::updateIcons);

  // Initial population
  updateList();
}

void PageSelector::updateIcons() {
  bool isDark = Settings::instance()->darkMode();

  // Use programmatically drawn themed icons
  m_firstBtn->setIcon(createThemedIcon("first", isDark));
  m_prevBtn->setIcon(createThemedIcon("prev", isDark));
  m_nextBtn->setIcon(createThemedIcon("next", isDark));
  m_lastBtn->setIcon(createThemedIcon("last", isDark));
  m_addButton->setIcon(createThemedIcon("add", isDark));
  m_deleteButton->setIcon(createThemedIcon("delete", isDark));
  m_renameButton->setIcon(createThemedIcon("rename", isDark));
  updateLockButton(); // Lock button has its own icon logic
}

void PageSelector::setupUi() {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(2);

  // Common button style - transparent with hover effect
  QString btnStyle = R"(
    QPushButton {
      background-color: transparent;
      border: none;
      border-radius: 4px;
      padding: 2px;
    }
    QPushButton:hover { background-color: rgba(128, 128, 128, 0.2); }
    QPushButton:disabled { opacity: 0.4; }
  )";

  // First page button
  m_firstBtn = new QPushButton();
  m_firstBtn->setFixedSize(26, 26);
  m_firstBtn->setToolTip(tr("First Page (Ctrl+Home)"));
  m_firstBtn->setStyleSheet(btnStyle);
  connect(m_firstBtn, &QPushButton::clicked, this,
          &PageSelector::goToFirstPage);
  layout->addWidget(m_firstBtn);

  // Previous button
  m_prevBtn = new QPushButton();
  m_prevBtn->setFixedSize(26, 26);
  m_prevBtn->setToolTip(tr("Previous Page"));
  m_prevBtn->setStyleSheet(btnStyle);
  connect(m_prevBtn, &QPushButton::clicked, this,
          &PageSelector::goToPreviousPage);
  layout->addWidget(m_prevBtn);

  // Page dropdown
  m_combo = new QComboBox();
  m_combo->setMinimumWidth(80);
  m_combo->setMaximumWidth(120);
  m_combo->setContextMenuPolicy(Qt::CustomContextMenu);
  m_combo->setStyleSheet(R"(
    QComboBox {
      background-color: transparent;
      border: 1px solid rgba(128, 128, 128, 0.3);
      border-radius: 4px;
      padding: 2px 6px;
    }
    QComboBox:hover {
      background-color: rgba(128, 128, 128, 0.2);
    }
    QComboBox::drop-down {
      border: none;
      width: 16px;
    }
  )");
  connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &PageSelector::onComboIndexChanged);
  connect(m_combo, &QComboBox::customContextMenuRequested, this,
          &PageSelector::showContextMenu);
  layout->addWidget(m_combo);

  // Next button
  m_nextBtn = new QPushButton();
  m_nextBtn->setFixedSize(26, 26);
  m_nextBtn->setToolTip(tr("Next Page"));
  m_nextBtn->setStyleSheet(btnStyle);
  connect(m_nextBtn, &QPushButton::clicked, this, &PageSelector::goToNextPage);
  layout->addWidget(m_nextBtn);

  // Last page button
  m_lastBtn = new QPushButton();
  m_lastBtn->setFixedSize(26, 26);
  m_lastBtn->setToolTip(tr("Last Page (Ctrl+End)"));
  m_lastBtn->setStyleSheet(btnStyle);
  connect(m_lastBtn, &QPushButton::clicked, this, &PageSelector::goToLastPage);
  layout->addWidget(m_lastBtn);

  layout->addSpacing(4);

  // Add button
  m_addButton = new QPushButton();
  m_addButton->setFixedSize(26, 26);
  m_addButton->setToolTip(tr("New Page (Ctrl+N)"));
  m_addButton->setStyleSheet(btnStyle);
  connect(m_addButton, &QPushButton::clicked, this,
          [this]() { m_manager->createNote(); });
  layout->addWidget(m_addButton);

  // Delete button
  m_deleteButton = new QPushButton();
  m_deleteButton->setFixedSize(26, 26);
  m_deleteButton->setToolTip(tr("Delete Page (Ctrl+Shift+D)"));
  m_deleteButton->setStyleSheet(btnStyle);
  connect(m_deleteButton, &QPushButton::clicked, this,
          &PageSelector::deleteCurrentPage);
  layout->addWidget(m_deleteButton);

  // Rename button
  m_renameButton = new QPushButton();
  m_renameButton->setFixedSize(26, 26);
  m_renameButton->setToolTip(tr("Rename Page"));
  m_renameButton->setStyleSheet(btnStyle);
  connect(m_renameButton, &QPushButton::clicked, this,
          &PageSelector::renameCurrentPage);
  layout->addWidget(m_renameButton);

  layout->addSpacing(2);

  // Lock button - icon will be set by updateLockButton()
  m_lockBtn = new QPushButton();
  m_lockBtn->setFixedSize(26, 26);
  m_lockBtn->setToolTip(tr("Lock/Unlock Page"));
  m_lockBtn->setStyleSheet(btnStyle);
  connect(m_lockBtn, &QPushButton::clicked, this, &PageSelector::toggleLock);
  layout->addWidget(m_lockBtn);

  // Set initial icons based on current theme
  updateIcons();

  // Expiry button - emoji with popup menu
  QString expiryBtnStyle =
      btnStyle + "QPushButton::menu-indicator { width: 0; height: 0; }";
  m_expiryBtn = new QPushButton("⏳");
  m_expiryBtn->setFixedSize(26, 26);
  m_expiryBtn->setToolTip(tr("Set Note Expiry"));
  m_expiryBtn->setStyleSheet(expiryBtnStyle);
  // Popup menu for expiry options with hover highlight (theme-aware)
  QMenu *expiryMenu = new QMenu(m_expiryBtn);

  // Lambda to update menu style based on current theme
  auto updateMenuStyle = [expiryMenu]() {
    bool isDark = Settings::instance()->darkMode();
    QString menuStyle = QString(R"(
      QMenu {
        background-color: %1;
        border: 1px solid %2;
        border-radius: 4px;
        padding: 4px;
      }
      QMenu::item {
        padding: 6px 20px;
        border-radius: 3px;
        color: %3;
      }
      QMenu::item:selected {
        background-color: %4;
        color: %5;
      }
      QMenu::separator {
        height: 1px;
        background: %2;
        margin: 4px 8px;
      }
    )")
                            .arg(isDark ? "#313244" : "#ffffff", // background
                                 isDark ? "#45475a" : "#ccd0da", // border
                                 isDark ? "#cdd6f4" : "#4c4f69", // text
                                 isDark ? "#585b70" : "#dce0e8", // hover bg
                                 isDark ? "#cdd6f4" : "#4c4f69"  // hover text
                            );
    expiryMenu->setStyleSheet(menuStyle);
  };

  // Update style when menu is about to show (dynamic theme support)
  connect(expiryMenu, &QMenu::aboutToShow, this, updateMenuStyle);
  // Also set initial style
  updateMenuStyle();

  QAction *exp1h = expiryMenu->addAction(tr("⏱️ 1 Hour"));
  connect(exp1h, &QAction::triggered, this, [this]() { setExpiry(1); });
  QAction *exp24h = expiryMenu->addAction(tr("📅 24 Hours"));
  connect(exp24h, &QAction::triggered, this, [this]() { setExpiry(24); });
  QAction *exp7d = expiryMenu->addAction(tr("📆 7 Days"));
  connect(exp7d, &QAction::triggered, this, [this]() { setExpiry(24 * 7); });
  expiryMenu->addSeparator();
  QAction *expNever = expiryMenu->addAction(tr("❌ Remove Expiry"));
  connect(expNever, &QAction::triggered, this, [this]() {
    if (m_manager->currentIndex() >= 0) {
      m_manager->clearNoteExpiry(m_manager->currentNote().id());
      updateList();
    }
  });
  m_expiryBtn->setMenu(expiryMenu);
  layout->addWidget(m_expiryBtn);
}

void PageSelector::updateList() {
  m_updating = true;

  m_combo->clear();

  const QList<Note> &notes = m_manager->notes();
  int titleMode = Settings::instance()->noteTitleMode();

  for (int i = 0; i < notes.size(); ++i) {
    QString title;
    const Note &note = notes.at(i);

    // Check title mode: 0=Smart, 1=DateTime, 2=FirstLine
    switch (titleMode) {
    case 1: // DateTime only
      title = note.created().toString("ddd hh:mm");
      break;
    case 2: { // FirstLine only
      QString content = note.content().trimmed();
      if (content.isEmpty()) {
        title = note.created().toString("ddd hh:mm");
      } else {
        title = content.split('\n').first().trimmed();
        if (title.length() > 18)
          title = title.left(16) + "…";
      }
      break;
    }
    default: // 0 = Smart (use Note::smartTitle)
      title = note.smartTitle();
      break;
    }

    // Add lock icon for encrypted notes
    if (note.isLocked()) {
      title = "🔒 " + title;
    }
    m_combo->addItem(title, note.id());
  }

  m_combo->setCurrentIndex(m_manager->currentIndex());

  m_updating = false;
  updateNavigationButtons();
  updateLockButton();
  updateExpiryButton();
}

void PageSelector::setCurrentIndex(int index) {
  if (!m_updating) {
    m_updating = true;
    m_combo->setCurrentIndex(index);
    m_updating = false;
    updateNavigationButtons();
  }
}

void PageSelector::updateNavigationButtons() {
  int current = m_manager->currentIndex();
  int count = m_manager->noteCount();

  m_firstBtn->setEnabled(current > 0);
  m_prevBtn->setEnabled(current > 0);
  m_nextBtn->setEnabled(current < count - 1);
  m_lastBtn->setEnabled(current < count - 1);
  m_deleteButton->setEnabled(true); // Always allow delete - creates new if last
  updateLockButton();
}

void PageSelector::updateLockButton() {
  Note current = m_manager->currentNote();

  // Session unlocked notes should show as unlocked (open lock icon)
  bool effectivelyLocked =
      current.isLocked() && !m_manager->isSessionUnlocked(current.id());

  QString btnStyle = R"(
    QPushButton {
      background-color: transparent;
      border: none;
      border-radius: 4px;
      padding: 2px;
    }
    QPushButton:hover { background-color: rgba(128, 128, 128, 0.2); }
    QPushButton::menu-indicator { width: 0; height: 0; }
  )";

  if (effectivelyLocked) {
    m_lockBtn->setIcon(
        createThemedIcon("lock", Settings::instance()->darkMode()));
    m_lockBtn->setToolTip(tr("Locked - Click to manage"));
    // Highlight locked state with subtle background
    m_lockBtn->setStyleSheet(R"(
      QPushButton {
        background-color: rgba(249, 226, 175, 0.3);
        border: none;
        border-radius: 4px;
        padding: 2px;
      }
      QPushButton:hover { background-color: rgba(249, 226, 175, 0.5); }
      QPushButton::menu-indicator { width: 0; height: 0; }
    )");
  } else {
    m_lockBtn->setIcon(
        createThemedIcon("unlock", Settings::instance()->darkMode()));
    m_lockBtn->setToolTip(tr("Lock / Expiry"));
    m_lockBtn->setStyleSheet(btnStyle);
  }

  // Update expiry indicator if note has expiry
  if (current.hasExpiry()) {
    QString expiryStr = current.expiresAt().toString("dd MMM hh:mm");
    m_lockBtn->setToolTip(tr("Lock / Expiry\nExpires: %1").arg(expiryStr));
  }
}

void PageSelector::toggleLock() {
  Note current = m_manager->currentNote();

  // Check if note is session unlocked (temporarily unlocked in this session)
  // If session unlocked, clicking lock should lock it again
  if (m_manager->isSessionUnlocked(current.id())) {
    lockCurrentPage();
    return;
  }

  if (current.isLocked()) {
    unlockCurrentPage();
  } else {
    lockCurrentPage();
  }
}

void PageSelector::goToFirstPage() {
  if (m_manager->noteCount() > 0) {
    m_manager->setCurrentIndex(0);
  }
}

void PageSelector::goToLastPage() {
  int count = m_manager->noteCount();
  if (count > 0) {
    m_manager->setCurrentIndex(count - 1);
  }
}

void PageSelector::goToPreviousPage() {
  int current = m_manager->currentIndex();
  if (current > 0) {
    m_manager->setCurrentIndex(current - 1);
  }
}

void PageSelector::goToNextPage() {
  int current = m_manager->currentIndex();
  if (current < m_manager->noteCount() - 1) {
    m_manager->setCurrentIndex(current + 1);
  }
}

void PageSelector::deleteCurrentPage() {
  bool isLastNote = (m_manager->noteCount() <= 1);

  Note current = m_manager->currentNote();

  // Cannot delete locked notes
  // Cannot delete locked notes - but offer Force Delete option
  if (current.isLocked()) {
    MainWindow *mainWin = qobject_cast<MainWindow *>(window());
    if (mainWin) {
      mainWin->setSuppressFocusOut(true);
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Delete Locked Note"));
    msgBox.setText(tr("🔒 This note is locked."));
    msgBox.setInformativeText(tr("You can unlock it first, or force delete "
                                 "(content will be lost permanently)."));
    QPushButton *forceDeleteBtn =
        msgBox.addButton(tr("Force Delete"), QMessageBox::DestructiveRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    msgBox.setDefaultButton(
        qobject_cast<QPushButton *>(msgBox.buttons().last()));

    msgBox.exec();

    if (mainWin) {
      mainWin->setSuppressFocusOut(false);
    }

    if (msgBox.clickedButton() == forceDeleteBtn) {
      // User confirmed force delete
      m_manager->deleteNote(current.id());
      updateList();
      updateNavigationButtons();
    }
    return;
  }

  QString title = current.title();
  if (title.isEmpty()) {
    title = QString("Page %1").arg(m_manager->currentIndex() + 1);
  }

  // Suppress focus-out during dialog
  MainWindow *mainWin = qobject_cast<MainWindow *>(window());
  if (mainWin) {
    mainWin->setSuppressFocusOut(true);
  }

  // Confirmation dialog
  QMessageBox::StandardButton reply = QMessageBox::question(
      this, tr("Delete Page"),
      tr("Are you sure you want to delete '%1'?\n\nThis cannot be undone.")
          .arg(title),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  // Re-enable focus-out
  if (mainWin) {
    mainWin->setSuppressFocusOut(false);
  }

  if (reply == QMessageBox::Yes) {
    m_manager->deleteNote(current.id());
    // If that was the last note, create a new empty one
    if (m_manager->noteCount() == 0) {
      m_manager->createNote();
    }
  }
}

void PageSelector::onComboIndexChanged(int index) {
  if (!m_updating && index >= 0) {
    m_manager->setCurrentIndex(index);
    updateNavigationButtons();
  }
}

void PageSelector::onNoteCreated() { updateList(); }

void PageSelector::onNoteDeleted(const QString & /*id*/) { updateList(); }

void PageSelector::onCurrentNoteChanged(int index) { setCurrentIndex(index); }

void PageSelector::renameCurrentPage() {
  Note current = m_manager->currentNote();
  QString currentTitle = current.smartTitle();

  bool ok;
  QString newTitle = QInputDialog::getText(
      this, tr("Rename Page"), tr("Enter new name for this page:"),
      QLineEdit::Normal, currentTitle, &ok);

  if (ok && !newTitle.isEmpty() && newTitle != currentTitle) {
    m_manager->renameNote(current.id(), newTitle);
    updateList();
  }
}

void PageSelector::lockCurrentPage() {
  Note current = m_manager->currentNote();

  // If note is session unlocked, just clear the session unlock (re-lock)
  if (m_manager->isSessionUnlocked(current.id())) {
    m_manager->sessionLock(current.id());

    // Update editor to show locked message
    MainWindow *mainWin = qobject_cast<MainWindow *>(window());
    if (mainWin) {
      mainWin->setUpdatingEditor(true);
      QPlainTextEdit *editor = mainWin->findChild<QPlainTextEdit *>();
      if (editor) {
        editor->setPlainText(
            tr("🔒 This note is locked.\n\nClick the lock button to unlock."));
        editor->setReadOnly(true);
      }
      mainWin->setUpdatingEditor(false);
    }

    updateList();
    updateLockButton();
    return;
  }

  if (current.isLocked()) {
    QMessageBox::information(this, tr("Already Locked"),
                             tr("This page is already locked."));
    return;
  }

  // Suppress focus-out during dialog
  MainWindow *mainWin = qobject_cast<MainWindow *>(window());
  if (mainWin) {
    mainWin->setSuppressFocusOut(true);
  }

  PasswordDialog dialog(PasswordDialog::SetPassword, this);
  // Position dialog at cursor for multi-monitor support
  QPoint cursorPos = QCursor::pos();
  dialog.move(cursorPos.x() - 150, cursorPos.y() - 100);
  if (dialog.exec() == QDialog::Accepted) {
    QString password = dialog.password();
    QString hash = Crypto::hashPassword(password);

    // ENCRYPT the content
    QString originalContent = current.content();
    qDebug() << "Original content to encrypt:" << originalContent.left(50);
    QString encryptedContent = Crypto::encrypt(originalContent, password);
    qDebug() << "Encrypted content:" << encryptedContent.left(80);

    // Save encrypted content and hash
    m_manager->updateNoteContent(current.id(), encryptedContent);
    m_manager->setNotePasswordHash(current.id(), hash);

    // Clear session unlock if it was unlocked in this session
    m_manager->sessionLock(current.id());

    // Clear editor - show locked message (prevent saving back to NoteManager)
    if (mainWin) {
      mainWin->setUpdatingEditor(true); // Prevent saving back
      QPlainTextEdit *editor = mainWin->findChild<QPlainTextEdit *>();
      if (editor) {
        editor->setPlainText(
            tr("🔒 This note is locked.\n\nClick the lock button to unlock."));
        editor->setReadOnly(true);
      }
      mainWin->setUpdatingEditor(false);
    }

    updateList();
    updateLockButton();
  }

  // Re-enable focus-out
  if (mainWin) {
    mainWin->setSuppressFocusOut(false);
  }
}

void PageSelector::unlockCurrentPage() {
  Note current = m_manager->currentNote();

  if (!current.isLocked()) {
    QMessageBox::information(this, tr("Not Locked"),
                             tr("This page is not locked."));
    return;
  }

  // Suppress focus-out during dialog
  MainWindow *mainWin = qobject_cast<MainWindow *>(window());
  if (mainWin) {
    mainWin->setSuppressFocusOut(true);
  }

  PasswordDialog dialog(PasswordDialog::RemovePassword, this);
  // Position dialog at cursor for multi-monitor support
  QPoint cursorPos = QCursor::pos();
  dialog.move(cursorPos.x() - 150, cursorPos.y() - 100);
  if (dialog.exec() == QDialog::Accepted) {
    QString password = dialog.password();

    // Verify password (note password only - master password is for SQLite
    // encryption)
    if (!Crypto::verifyPassword(password, current.passwordHash())) {
      QMessageBox::warning(this, tr("Wrong Password"),
                           tr("The password is incorrect."));
      if (mainWin) {
        mainWin->setSuppressFocusOut(false);
      }
      return;
    }

    // DECRYPT the content
    QString encryptedContent = current.content();
    fprintf(stderr, "[UNLOCK] Encrypted content: %s\n",
            encryptedContent.toStdString().c_str());
    fflush(stderr);

    QString decryptedContent = Crypto::decrypt(encryptedContent, password);
    fprintf(stderr, "[UNLOCK] Decrypted content len: %lld\n",
            (long long)decryptedContent.length());
    fflush(stderr);

    if (decryptedContent.isEmpty() && !encryptedContent.isEmpty()) {
      QMessageBox::warning(this, tr("Decryption Failed"),
                           tr("Could not decrypt the content."));
      if (mainWin) {
        mainWin->setSuppressFocusOut(false);
      }
      return;
    }

    // SESSION UNLOCK - don't modify storage, just cache decrypted content
    m_manager->sessionUnlock(current.id());
    m_manager->setDecryptedContent(current.id(), decryptedContent);

    // Update editor directly - show decrypted content (prevent auto-save
    // override)
    if (mainWin) {
      mainWin->setUpdatingEditor(true); // Prevent saving back
      QPlainTextEdit *editor = mainWin->findChild<QPlainTextEdit *>();
      if (editor) {
        editor->setPlainText(decryptedContent);
        editor->setReadOnly(false);
      }
      mainWin->setUpdatingEditor(false);
    }

    updateList();
    updateLockButton();

    // Start auto-lock timer if configured
    if (mainWin) {
      mainWin->startAutoLockTimer();
    }
  }

  // Re-enable focus-out
  if (mainWin) {
    mainWin->setSuppressFocusOut(false);
  }
}

void PageSelector::changePasswordCurrentPage() {
  Note current = m_manager->currentNote();

  if (!current.isLocked()) {
    QMessageBox::information(
        this, tr("Not Locked"),
        tr("This page is not locked. Lock it first to set a password."));
    return;
  }

  // Suppress focus-out during dialog
  MainWindow *mainWin = qobject_cast<MainWindow *>(window());
  if (mainWin) {
    mainWin->setSuppressFocusOut(true);
  }

  // First verify current password
  PasswordDialog verifyDialog(PasswordDialog::RemovePassword, this);
  verifyDialog.setWindowTitle(tr("Verify Current Password"));
  QPoint cursorPos = QCursor::pos();
  verifyDialog.move(cursorPos.x() - 150, cursorPos.y() - 100);

  if (verifyDialog.exec() != QDialog::Accepted) {
    if (mainWin) {
      mainWin->setSuppressFocusOut(false);
    }
    return;
  }

  QString oldPassword = verifyDialog.password();

  // Verify password (note password only)
  if (!Crypto::verifyPassword(oldPassword, current.passwordHash())) {
    QMessageBox::warning(this, tr("Wrong Password"),
                         tr("The current password is incorrect."));
    if (mainWin) {
      mainWin->setSuppressFocusOut(false);
    }
    return;
  }

  // Now ask for new password
  PasswordDialog newDialog(PasswordDialog::SetPassword, this);
  newDialog.setWindowTitle(tr("Set New Password"));
  newDialog.move(cursorPos.x() - 150, cursorPos.y() - 100);

  if (newDialog.exec() == QDialog::Accepted) {
    QString newPassword = newDialog.password();

    // Decrypt content with old password
    QString encryptedContent = current.content();
    QString decryptedContent = Crypto::decrypt(encryptedContent, oldPassword);

    // Re-encrypt with new password
    QString newEncryptedContent =
        Crypto::encrypt(decryptedContent, newPassword);
    QString newHash = Crypto::hashPassword(newPassword);

    // Update storage
    m_manager->updateNoteContent(current.id(), newEncryptedContent);
    m_manager->setNotePasswordHash(current.id(), newHash);

    QMessageBox::information(this, tr("Password Changed"),
                             tr("The password has been changed successfully."));
  }

  if (mainWin) {
    mainWin->setSuppressFocusOut(false);
  }
}

void PageSelector::showContextMenu(const QPoint &pos) {
  if (m_manager->noteCount() == 0)
    return;

  QMenu menu(this);
  Note note = m_manager->currentNote();

  // Rename action
  QAction *renameAction = menu.addAction(tr("📝 Rename..."));
  connect(renameAction, &QAction::triggered, this,
          &PageSelector::renameCurrentPage);

  menu.addSeparator();

  // Lock/Unlock action
  if (note.isLocked()) {
    QAction *unlockAction = menu.addAction(tr("🔓 Unlock"));
    connect(unlockAction, &QAction::triggered, this,
            &PageSelector::unlockCurrentPage);
    QAction *changePwAction = menu.addAction(tr("🔑 Change Password..."));
    connect(changePwAction, &QAction::triggered, this,
            &PageSelector::changePasswordCurrentPage);
  } else {
    QAction *lockAction = menu.addAction(tr("🔒 Lock with Password..."));
    connect(lockAction, &QAction::triggered, this,
            &PageSelector::lockCurrentPage);
  }

  menu.addSeparator();

  // Expiry submenu
  QMenu *expiryMenu = menu.addMenu(tr("⏰ Set Expiry"));
  if (note.hasExpiry()) {
    QString expiryStr = note.expiresAt().toString("dd MMM hh:mm");
    expiryMenu->addAction(tr("Expires: %1").arg(expiryStr))->setEnabled(false);
    expiryMenu->addSeparator();
  }
  QAction *exp1h = expiryMenu->addAction(tr("1 Hour"));
  connect(exp1h, &QAction::triggered, this, [this]() { setExpiry(1); });
  QAction *exp24h = expiryMenu->addAction(tr("24 Hours"));
  connect(exp24h, &QAction::triggered, this, [this]() { setExpiry(24); });
  QAction *exp7d = expiryMenu->addAction(tr("7 Days"));
  connect(exp7d, &QAction::triggered, this, [this]() { setExpiry(24 * 7); });
  expiryMenu->addSeparator();
  QAction *expNever = expiryMenu->addAction(tr("Never (Remove)"));
  connect(expNever, &QAction::triggered, this, [this]() {
    if (m_manager->currentIndex() >= 0) {
      m_manager->clearNoteExpiry(m_manager->currentNote().id());
      updateList();
    }
  });

  menu.addSeparator();

  // Delete action - always available
  QAction *deleteAction = menu.addAction(tr("🗑️ Delete"));
  connect(deleteAction, &QAction::triggered, this,
          &PageSelector::deleteCurrentPage);

  menu.exec(m_combo->mapToGlobal(pos));
}

void PageSelector::setExpiry(int hours) {
  if (m_manager->currentIndex() < 0)
    return;

  QDateTime expiresAt = QDateTime::currentDateTime().addSecs(hours * 3600);
  m_manager->setNoteExpiry(m_manager->currentNote().id(), expiresAt);
  updateList();
}

void PageSelector::updateExpiryButton() {
  Note current = m_manager->currentNote();

  QString normalStyle = R"(
    QPushButton {
      background-color: transparent;
      border: none;
      border-radius: 4px;
      padding: 2px;
    }
    QPushButton:hover { background-color: rgba(128, 128, 128, 0.2); }
    QPushButton::menu-indicator { width: 0; height: 0; }
  )";

  if (current.hasExpiry()) {
    // Calculate remaining time
    QDateTime now = QDateTime::currentDateTime();
    QDateTime expiry = current.expiresAt();
    qint64 secsRemaining = now.secsTo(expiry);

    QString remaining;
    if (secsRemaining <= 0) {
      remaining = tr("Expired!");
    } else if (secsRemaining < 3600) {
      remaining = tr("%1 min").arg(secsRemaining / 60);
    } else if (secsRemaining < 86400) {
      remaining = tr("%1h %2m")
                      .arg(secsRemaining / 3600)
                      .arg((secsRemaining % 3600) / 60);
    } else {
      remaining = tr("%1 days").arg(secsRemaining / 86400);
    }

    m_expiryBtn->setToolTip(
        tr("⏳ Expires in: %1\nClick to change").arg(remaining));

    // Highlight button with orange/yellow background
    m_expiryBtn->setStyleSheet(R"(
      QPushButton {
        background-color: rgba(250, 179, 135, 0.4);
        border: none;
        border-radius: 4px;
        padding: 2px;
      }
      QPushButton:hover { background-color: rgba(250, 179, 135, 0.6); }
      QPushButton::menu-indicator { width: 0; height: 0; }
    )");
  } else {
    m_expiryBtn->setToolTip(tr("Set Note Expiry"));
    m_expiryBtn->setStyleSheet(normalStyle);
  }
}
