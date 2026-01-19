#ifndef LINNOTE_MODESELECTOR_H
#define LINNOTE_MODESELECTOR_H

#include "core/NoteMode.h"
#include <QComboBox>
#include <QWidget>

class NoteManager;

/**
 * @brief Mode selector widget for title bar
 *
 * Shows a dropdown to select note mode:
 * - 📝 Plain Text
 * - ☑ Checklist
 * - </> Code
 * - ∑ Math
 */
class ModeSelector : public QWidget {
  Q_OBJECT

public:
  explicit ModeSelector(NoteManager *manager, QWidget *parent = nullptr);

  void setCurrentMode(NoteMode mode);
  NoteMode currentMode() const;

signals:
  void modeChanged(NoteMode mode);

private slots:
  void onComboIndexChanged(int index);
  void onCurrentNoteChanged(int index);
  void updateStyle();

private:
  void setupUi();
  void populateModes();

  NoteManager *m_manager;
  QComboBox *m_combo;
  bool m_updating;
};

#endif // LINNOTE_MODESELECTOR_H
