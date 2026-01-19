#ifndef LINNOTE_COMMANDPOPUP_H
#define LINNOTE_COMMANDPOPUP_H

#include <QFrame>
#include <QListWidget>
#include <QVBoxLayout>

class QPlainTextEdit;

/**
 * @brief Popup widget for slash commands
 *
 * Shows available commands when user types '/'
 * Allows keyboard navigation and selection
 */
class CommandPopup : public QFrame {
  Q_OBJECT

public:
  struct CommandItem {
    QString keyword;
    QString description;
    QString shortcut; // Keyboard shortcut (optional)
  };

  explicit CommandPopup(QWidget *parent = nullptr);

  void setCommands(const QList<CommandItem> &commands);
  void filterCommands(const QString &filter);
  void show(const QPoint &pos, QPlainTextEdit *editor);

  // Navigation
  void moveUp();
  void moveDown();
  bool hasSelection() const;
  QString selectedCommand() const;

signals:
  void commandSelected(const QString &command);
  void dismissed();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

private:
  void setupUi();
  void applyStyle();
  void updateFilter();

  QListWidget *m_list;
  QList<CommandItem> m_allCommands;
  QString m_filter;
  QPlainTextEdit *m_editor;
};

#endif // LINNOTE_COMMANDPOPUP_H
