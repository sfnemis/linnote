#ifndef LINNOTE_SEARCHBAR_H
#define LINNOTE_SEARCHBAR_H

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QPlainTextEdit;
class QLabel;
class NoteManager;

/**
 * @brief Search result with page index and position
 */
struct SearchResult {
  int pageIndex;
  int position;
  int length;
};

/**
 * @brief Inline search/replace bar for NoteEditor with cross-page search
 */
class SearchBar : public QWidget {
  Q_OBJECT

public:
  explicit SearchBar(QWidget *parent = nullptr);
  ~SearchBar() override = default;

  // Attach to editor and note manager
  void setEditor(QPlainTextEdit *editor);
  void setNoteManager(NoteManager *noteManager);

  // Show modes
  void showSearch();
  void showReplace();

  // Focus search input
  void focusSearch();

  // Set search text
  void setSearchText(const QString &text);

  // Clear and hide
  void clear();

signals:
  void closeRequested();
  void goToPage(int pageIndex);

public slots:
  void findNext();
  void findPrevious();
  void replaceOne();
  void replaceAll();

protected:
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void onSearchTextChanged(const QString &text);
  void updateHighlights();

private:
  void setupUI();
  void applyButtonStyles();
  void clearHighlights();
  void searchAllPages();
  void selectResult(int index);

  QPlainTextEdit *m_editor;
  NoteManager *m_noteManager;
  QLineEdit *m_searchEdit;
  QLineEdit *m_replaceEdit;
  QPushButton *m_prevBtn;
  QPushButton *m_nextBtn;
  QPushButton *m_replaceBtn;
  QPushButton *m_replaceAllBtn;
  QPushButton *m_closeBtn;
  QLabel *m_countLabel;
  QWidget *m_replaceRow;
  QCheckBox *m_caseCheck;
  QCheckBox *m_regexCheck;

  QList<SearchResult> m_results;
  int m_currentResultIndex;
  bool m_replaceVisible;
  bool m_caseSensitive;
  bool m_useRegex;
};

#endif // LINNOTE_SEARCHBAR_H
