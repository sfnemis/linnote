#include "CommandPopup.h"
#include "core/Settings.h"
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QScreen>

// Define keyword colors (Catppuccin Mocha palette for dark, Latte for light)
static QMap<QString, QPair<QString, QString>> getKeywordColors() {
  // First = dark mode color, Second = light mode color
  return {
      {"list", {"#a6e3a1", "#40a02b"}},     // Green
      {"calc", {"#89b4fa", "#1e66f5"}},     // Blue
      {"sum", {"#f5c2e7", "#ea76cb"}},      // Pink
      {"avg", {"#cba6f7", "#8839ef"}},      // Mauve
      {"count", {"#fab387", "#fe640b"}},    // Peach
      {"code", {"#f9e2af", "#df8e1d"}},     // Yellow
      {"markdown", {"#cba6f7", "#8839ef"}}, // Mauve
      {"paste", {"#94e2d5", "#179299"}},    // Teal
      {"timer", {"#f38ba8", "#d20f39"}},    // Red
      {"plain", {"#a6adc8", "#6c6f85"}},    // Overlay
      {"settings", {"#b4befe", "#7287fd"}}  // Lavender
  };
}

CommandPopup::CommandPopup(QWidget *parent)
    : QFrame(parent), m_list(nullptr), m_editor(nullptr) {
  setupUi();
  applyStyle();

  // Complete command list with all modes and useful commands
  m_allCommands = {{"list", "Checklist mode - Todo list with checkboxes", "1"},
                   {"calc", "Calculator - Math expressions", "2"},
                   {"sum", "Sum all numbers in text", "3"},
                   {"avg", "Average of numbers in text", "4"},
                   {"count", "Count words, chars, sentences", "5"},
                   {"paste", "AutoPaste - Collect copied items", "6"},
                   {"code", "Code mode - Syntax highlighting", "7"},
                   {"markdown", "Markdown - Headers, bold, italic", "8"},
                   {"timer", "Timer - Stopwatch, Countdown, Pomodoro", "9"},
                   {"plain", "Plain text mode - No formatting", ""},
                   {"settings", "Open settings dialog", "0"}};

  updateFilter();

  // Update style when theme changes
  connect(Settings::instance(), &Settings::settingsChanged, this,
          &CommandPopup::applyStyle);
}

void CommandPopup::setupUi() {
  setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(6, 6, 6, 6);
  layout->setSpacing(2);

  m_list = new QListWidget(this);
  m_list->setFocusPolicy(Qt::NoFocus);
  m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  layout->addWidget(m_list);

  connect(m_list, &QListWidget::itemClicked, this,
          [this](QListWidgetItem *item) {
            QString cmd = item->data(Qt::UserRole).toString();
            emit commandSelected(cmd);
            hide();
          });
}

void CommandPopup::applyStyle() {
  bool isDark = Settings::instance()->darkMode();

  QString bgColor = isDark ? "#1e1e2e" : "#eff1f5";
  QString borderColor = isDark ? "#45475a" : "#ccd0da";
  QString hoverBg = isDark ? "#313244" : "#e6e9ef";
  QString scrollbarColor = isDark ? "#45475a" : "#bcc0cc";

  setStyleSheet(QString(R"(
    CommandPopup {
      background-color: %1;
      border: 1px solid %2;
      border-radius: 10px;
    }
    QListWidget {
      background-color: transparent;
      border: none;
      outline: none;
      font-family: 'JetBrains Mono', 'Fira Code', monospace;
      font-size: 12px;
    }
    QListWidget::item {
      padding: 6px 10px;
      border-radius: 6px;
      margin: 2px 0;
    }
    QListWidget::item:selected {
      background-color: %3;
    }
    QListWidget::item:hover {
      background-color: %3;
    }
    QScrollBar:vertical {
      background: transparent;
      width: 8px;
    }
    QScrollBar::handle:vertical {
      background: %4;
      border-radius: 4px;
    }
  )")
                    .arg(bgColor, borderColor, hoverBg, scrollbarColor));
}

void CommandPopup::setCommands(const QList<CommandItem> &commands) {
  m_allCommands = commands;
  updateFilter();
}

void CommandPopup::filterCommands(const QString &filter) {
  m_filter = filter.toLower();
  updateFilter();
}

void CommandPopup::updateFilter() {
  m_list->clear();

  bool isDark = Settings::instance()->darkMode();
  auto keywordColors = getKeywordColors();
  QString defaultColor = isDark ? "#6c7086" : "#5c5f77";

  int index = 1;
  for (const CommandItem &cmd : m_allCommands) {
    if (m_filter.isEmpty() || cmd.keyword.toLower().contains(m_filter)) {
      auto *item = new QListWidgetItem();

      // Get keyword color based on theme (use default gray if not found)
      QString keywordColor = defaultColor;
      if (keywordColors.contains(cmd.keyword)) {
        auto colorPair = keywordColors[cmd.keyword];
        keywordColor = isDark ? colorPair.first : colorPair.second;
      }

      // Format with colors: "N  keyword  description"
      QString displayNum = QString::number(index == 10 ? 0 : index);

      // Create HTML formatted text (won't work with QListWidgetItem directly,
      // so use simple approach)
      QString text = QString("%1  %2  %3")
                         .arg(displayNum)
                         .arg(cmd.keyword, -10)
                         .arg(cmd.description);

      item->setText(text);
      item->setData(Qt::UserRole, cmd.keyword);

      // Set foreground color for the keyword
      item->setForeground(QColor(keywordColor));

      m_list->addItem(item);
      index++;
    }
  }

  // Select first item
  if (m_list->count() > 0) {
    m_list->setCurrentRow(0);
  }

  // Calculate size - fit all items
  int itemHeight = 26;
  int contentHeight = m_list->count() * itemHeight + 16;
  int maxHeight = 300;
  int popupWidth = 320;

  setFixedSize(popupWidth, qMin(contentHeight, maxHeight));
}

void CommandPopup::show(const QPoint &pos, QPlainTextEdit *editor) {
  m_editor = editor;
  m_filter.clear();
  updateFilter();

  // Calculate position - ensure popup stays within screen bounds
  QPoint adjustedPos = pos;

  // Get screen geometry
  QScreen *screen = QGuiApplication::screenAt(pos);
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }
  QRect screenRect = screen->availableGeometry();

  // Adjust X if popup would go off right edge
  if (adjustedPos.x() + width() > screenRect.right()) {
    adjustedPos.setX(screenRect.right() - width() - 10);
  }

  // Adjust Y if popup would go off bottom edge
  if (adjustedPos.y() + height() > screenRect.bottom()) {
    // Show above cursor instead
    adjustedPos.setY(pos.y() - height() - 20);
  }

  // Ensure X is not negative
  if (adjustedPos.x() < screenRect.left()) {
    adjustedPos.setX(screenRect.left() + 10);
  }

  move(adjustedPos);
  QFrame::show();
  raise();
  setFocus();
}

void CommandPopup::moveUp() {
  int row = m_list->currentRow();
  if (row > 0) {
    m_list->setCurrentRow(row - 1);
  }
}

void CommandPopup::moveDown() {
  int row = m_list->currentRow();
  if (row < m_list->count() - 1) {
    m_list->setCurrentRow(row + 1);
  }
}

bool CommandPopup::hasSelection() const {
  return m_list->currentItem() != nullptr;
}

QString CommandPopup::selectedCommand() const {
  if (m_list->currentItem()) {
    return m_list->currentItem()->data(Qt::UserRole).toString();
  }
  return QString();
}

void CommandPopup::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Up:
    moveUp();
    break;
  case Qt::Key_Down:
    moveDown();
    break;
  case Qt::Key_Return:
  case Qt::Key_Enter:
    if (hasSelection()) {
      emit commandSelected(selectedCommand());
      hide();
    }
    break;
  case Qt::Key_Escape:
    emit dismissed();
    hide();
    break;
  case Qt::Key_1:
  case Qt::Key_2:
  case Qt::Key_3:
  case Qt::Key_4:
  case Qt::Key_5:
  case Qt::Key_6:
  case Qt::Key_7:
  case Qt::Key_8:
  case Qt::Key_9:
  case Qt::Key_0: {
    // Number shortcuts
    int num = event->key() - Qt::Key_0;
    if (num == 0)
      num = 10; // 0 -> 10th item
    if (num <= m_list->count()) {
      m_list->setCurrentRow(num - 1);
      emit commandSelected(selectedCommand());
      hide();
    }
    break;
  }
  default:
    // Filter by typed character
    if (event->text().length() == 1 && event->text()[0].isLetter()) {
      m_filter += event->text().toLower();
      updateFilter();
    } else if (event->key() == Qt::Key_Backspace && !m_filter.isEmpty()) {
      m_filter.chop(1);
      updateFilter();
    }
    break;
  }
}

void CommandPopup::focusOutEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  emit dismissed();
  hide();
}
