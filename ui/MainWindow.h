#ifndef LINNOTE_MAINWINDOW_H
#define LINNOTE_MAINWINDOW_H

#include "NoteEditor.h"
#include "integration/ClipboardManager.h"
#include <QMainWindow>
#include <QPushButton>

class NoteManager;
class PageSelector;
class SlashCommand;
class TimerWidget;
class SearchBar;
class QSizeGrip;
class BackupManager;

/**
 * @brief Main application window with frameless design
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  enum State { Hidden, Visible, VisibleFocused };

  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  State state() const;
  void setShortcutManager(class PortalShortcuts *shortcuts);
  NoteManager *noteManager() const;
  void setUpdatingEditor(bool updating) { m_updatingEditor = updating; }

public slots:
  void toggleVisibility();
  void showAndFocus();
  void hideWindow();
  void exportNote();
  void toggleAlwaysOnTop();
  void captureOcr();
  void shortenSelectedUrl();
  void deleteAllNotes();
  void openSettingsDialog();

  // URL Scheme support
  void createNewNoteWithText(const QString &text);
  void searchNotes(const QString &query);

protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
  void onCurrentNoteChanged(int index);
  void onNoteModeChanged(const QString &id, NoteMode mode);
  void onEditorContentChanged();
  void checkForSlashCommand();
  void applyTheme();
  void onAutoLockTimeout();

private:
  void setupUi();
  void setupShortcuts();
  void applyStyle();
  void setState(State newState);
  void performAutoPaste();
  void saveCurrentNoteContent();
  void updatePinButtonState();
  void updateToolbarVisibility();
  bool tryKWinKeepAbove(bool enable); // KDE Wayland DBus method
  void ensureTimerWidget();           // Creates timer widget if not exists
#ifdef HAVE_KGLOBALACCEL
  void registerKdeGlobalShortcut(); // Register with KDE global shortcuts
#endif

  NoteEditor *m_editor;
  ClipboardManager *m_clipboardManager;
  NoteManager *m_noteManager;
  PageSelector *m_pageSelector;
  SlashCommand *m_slashCommand;
  State m_state;
  bool m_autoPastedThisSession;
  bool m_updatingEditor;

  // Frameless window dragging
  QWidget *m_titleBar;
  QSizeGrip *m_sizeGrip;
  bool m_dragging;
  QPoint m_dragPosition;

  // Always on top
  QPushButton *m_pinButton;
  QPushButton *m_autoHideButton;
  bool m_alwaysOnTop;

  // Timer widget
  TimerWidget *m_timerWidget;
  class ConfettiWidget *m_confetti;

  // Temporarily suppress focus-out hiding (during dialogs)
  bool m_suppressFocusOut;

  // Search bar (inline) and modal (full search)
  SearchBar *m_searchBar;
  class SearchModal *m_searchModal;

  // Auto-lock timer
  QTimer *m_autoLockTimer;

  // Auto-backup manager
  BackupManager *m_backupManager;

#ifdef HAVE_KGLOBALACCEL
  // KDE global shortcut action
  QAction *m_toggleAction = nullptr;
#endif

public:
  void setSuppressFocusOut(bool suppress) { m_suppressFocusOut = suppress; }
  void startAutoLockTimer();

private slots:
  void onTimerFinished();
};

#endif // LINNOTE_MAINWINDOW_H
