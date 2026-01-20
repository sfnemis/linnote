#include "ModeSelector.h"
#include "core/NoteManager.h"
#include "core/Settings.h"
#include <QHBoxLayout>

ModeSelector::ModeSelector(NoteManager *manager, QWidget *parent)
    : QWidget(parent), m_manager(manager), m_combo(nullptr), m_updating(false) {
  setupUi();
  populateModes();

  // Connect to manager
  connect(m_manager, &NoteManager::currentNoteChanged, this,
          &ModeSelector::onCurrentNoteChanged);

  // Connect to settings for theme changes
  connect(Settings::instance(), &Settings::settingsChanged, this,
          &ModeSelector::updateStyle);

  // Set initial mode
  if (m_manager->noteCount() > 0) {
    setCurrentMode(m_manager->currentNote().mode());
  }
}

void ModeSelector::updateStyle() {
  bool isDark = Settings::instance()->darkMode();

  QString bgColor = isDark ? "#313244" : "#e6e9ef";
  QString textColor = isDark ? "#cdd6f4" : "#4c4f69";
  QString hoverBg = isDark ? "#45475a" : "#ccd0da";
  QString dropdownBg = isDark ? "#1e1e2e" : "#eff1f5";
  QString borderColor = isDark ? "#313244" : "#ccd0da";
  QString arrowColor = isDark ? "#6c7086" : "#5c5f77";

  m_combo->setStyleSheet(QString(R"(
        QComboBox {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 12px;
        }
        QComboBox:hover {
            background-color: %3;
        }
        QComboBox::drop-down {
            border: none;
            width: 16px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 3px solid transparent;
            border-right: 3px solid transparent;
            border-top: 4px solid %6;
            margin-right: 4px;
        }
        QComboBox QAbstractItemView {
            background-color: %4;
            color: %2;
            selection-background-color: %3;
            border: 1px solid %5;
            border-radius: 4px;
        }
    )")
                             .arg(bgColor, textColor, hoverBg, dropdownBg,
                                  borderColor, arrowColor));
}

void ModeSelector::setupUi() {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_combo = new QComboBox();
  m_combo->setMinimumWidth(80);
  m_combo->setMaximumWidth(100);

  // Apply initial style
  updateStyle();

  connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ModeSelector::onComboIndexChanged);

  layout->addWidget(m_combo);
}

void ModeSelector::populateModes() {
  m_updating = true;
  m_combo->clear();

  // Add all modes
  m_combo->addItem(noteModeIcon(NoteMode::PlainText) + " " +
                       noteModeName(NoteMode::PlainText),
                   static_cast<int>(NoteMode::PlainText));
  m_combo->addItem(noteModeIcon(NoteMode::Checklist) + " " +
                       noteModeName(NoteMode::Checklist),
                   static_cast<int>(NoteMode::Checklist));
  m_combo->addItem(noteModeIcon(NoteMode::Code) + " " +
                       noteModeName(NoteMode::Code),
                   static_cast<int>(NoteMode::Code));
  m_combo->addItem(noteModeIcon(NoteMode::Math) + " " +
                       noteModeName(NoteMode::Math),
                   static_cast<int>(NoteMode::Math));

  m_updating = false;
}

void ModeSelector::setCurrentMode(NoteMode mode) {
  m_updating = true;
  for (int i = 0; i < m_combo->count(); ++i) {
    if (m_combo->itemData(i).toInt() == static_cast<int>(mode)) {
      m_combo->setCurrentIndex(i);
      break;
    }
  }
  m_updating = false;
}

NoteMode ModeSelector::currentMode() const {
  return static_cast<NoteMode>(m_combo->currentData().toInt());
}

void ModeSelector::onComboIndexChanged(int index) {
  if (m_updating || index < 0)
    return;

  NoteMode mode = static_cast<NoteMode>(m_combo->itemData(index).toInt());

  // Update current note's mode
  if (m_manager->currentIndex() >= 0) {
    Note note = m_manager->currentNote();
    m_manager->updateNoteMode(note.id(), mode);
  }

  emit modeChanged(mode);
}

void ModeSelector::onCurrentNoteChanged(int index) {
  if (index >= 0) {
    setCurrentMode(m_manager->noteAt(index).mode());
  }
}
