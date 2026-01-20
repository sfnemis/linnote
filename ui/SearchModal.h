#ifndef LINNOTE_SEARCHMODAL_H
#define LINNOTE_SEARCHMODAL_H

#include <QDateTime>
#include <QFrame>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class NoteManager;

/**
 * @brief Antinote-style search overlay (embedded in MainWindow, not a separate
 * dialog)
 */
class SearchModal : public QFrame {
  Q_OBJECT

public:
  explicit SearchModal(NoteManager *noteManager, QWidget *parent = nullptr);
  ~SearchModal() override = default;

  void showAndFocus();
  void hideModal();
  void updateTheme(); // Call when theme changes

signals:
  void noteSelected(int noteIndex);
  void dismissed();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onSearchTextChanged(const QString &text);
  void onItemClicked(QListWidgetItem *item);
  void onItemDoubleClicked(QListWidgetItem *item);

private:
  void setupUI();
  void applyTheme();
  void populateNotes();
  void filterNotes(const QString &filter);
  QString truncateText(const QString &text, int maxLength) const;
  QString formatDate(const QDateTime &date) const;

  NoteManager *m_noteManager;
  QLineEdit *m_searchEdit;
  QListWidget *m_noteList;
  QPushButton *m_cancelBtn;
};

#endif // LINNOTE_SEARCHMODAL_H
