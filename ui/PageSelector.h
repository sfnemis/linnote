#ifndef LINNOTE_PAGESELECTOR_H
#define LINNOTE_PAGESELECTOR_H

#include <QComboBox>
#include <QPushButton>
#include <QWidget>

class NoteManager;

/**
 * @brief Page selector widget for title bar
 *
 * Shows a dropdown of all notes with:
 * - Navigation buttons (first/last)
 * - Current page selection
 * - Add/Delete/Rename buttons
 */
class PageSelector : public QWidget {
  Q_OBJECT

public:
  explicit PageSelector(NoteManager *manager, QWidget *parent = nullptr);

  void updateList();
  void setCurrentIndex(int index);

signals:
  void addPageRequested();

public slots:
  void goToFirstPage();
  void goToLastPage();
  void goToPreviousPage();
  void goToNextPage();
  void deleteCurrentPage();
  void renameCurrentPage();
  void lockCurrentPage();
  void unlockCurrentPage();
  void toggleLock();
  void changePasswordCurrentPage();
  void updateLockButton();

private slots:
  void onComboIndexChanged(int index);
  void onNoteCreated();
  void onNoteDeleted(const QString &id);
  void onCurrentNoteChanged(int index);
  void updateIcons();

private:
  void setupUi();
  void updateNavigationButtons();
  void showContextMenu(const QPoint &pos);
  void setExpiry(int hours);
  void updateExpiryButton();

  NoteManager *m_manager;
  QComboBox *m_combo;
  QPushButton *m_firstBtn;
  QPushButton *m_prevBtn;
  QPushButton *m_nextBtn;
  QPushButton *m_lastBtn;
  QPushButton *m_addButton;
  QPushButton *m_renameButton;
  QPushButton *m_deleteButton;
  QPushButton *m_lockBtn;
  QPushButton *m_expiryBtn;
  bool m_updating;
};

#endif // LINNOTE_PAGESELECTOR_H
