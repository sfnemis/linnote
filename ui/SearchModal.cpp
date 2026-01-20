#include "SearchModal.h"
#include "core/NoteManager.h"
#include "core/Theme.h"
#include <QApplication>
#include <QLabel>
#include <QScrollBar>

SearchModal::SearchModal(NoteManager *noteManager, QWidget *parent)
    : QFrame(parent), m_noteManager(noteManager), m_searchEdit(nullptr),
      m_noteList(nullptr), m_cancelBtn(nullptr) {
  setupUI();
  applyTheme();
  hide(); // Start hidden
}

void SearchModal::setupUI() {
  setObjectName("searchModal");

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // === Search Header ===
  QWidget *headerWidget = new QWidget();
  headerWidget->setObjectName("searchHeader");
  QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
  headerLayout->setContentsMargins(16, 12, 16, 12);
  headerLayout->setSpacing(12);

  // Search icon
  QLabel *searchIcon = new QLabel("🔍");
  searchIcon->setObjectName("searchIcon");
  headerLayout->addWidget(searchIcon);

  // Search input
  m_searchEdit = new QLineEdit();
  m_searchEdit->setPlaceholderText(tr("Search all notes"));
  m_searchEdit->setObjectName("searchInput");
  connect(m_searchEdit, &QLineEdit::textChanged, this,
          &SearchModal::onSearchTextChanged);
  headerLayout->addWidget(m_searchEdit, 1);

  // Cancel button
  m_cancelBtn = new QPushButton(tr("Cancel"));
  m_cancelBtn->setObjectName("cancelBtn");
  m_cancelBtn->setFlat(true);
  connect(m_cancelBtn, &QPushButton::clicked, this, &SearchModal::hideModal);
  headerLayout->addWidget(m_cancelBtn);

  mainLayout->addWidget(headerWidget);

  // === Note List ===
  m_noteList = new QListWidget();
  m_noteList->setObjectName("noteList");
  m_noteList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_noteList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_noteList->setSpacing(2);
  connect(m_noteList, &QListWidget::itemClicked, this,
          &SearchModal::onItemClicked);
  connect(m_noteList, &QListWidget::itemDoubleClicked, this,
          &SearchModal::onItemDoubleClicked);
  mainLayout->addWidget(m_noteList, 1);

  // Install event filter to catch clicks outside
  if (parent()) {
    parent()->installEventFilter(this);
  }
}

void SearchModal::applyTheme() {
  // Get theme colors from ThemeManager
  ThemeManager &tm = ThemeManager::instance();
  Theme theme = tm.currentTheme();
  ThemeColors colors = theme.colors(tm.isDarkMode());

  QString bg = colors.background.name();
  QString fg = colors.foreground.name();
  QString border = colors.border.name();
  QString selection = colors.selection.name();
  QString accent = colors.accent.name();
  QString secondary = colors.secondary.name();
  QString comment = colors.comment.name();

  setStyleSheet(QString(R"(
    #searchModal {
      background-color: %1;
      border: 1px solid %2;
      border-radius: 12px;
    }
    #searchHeader {
      background-color: %1;
      border-bottom: 1px solid %2;
      border-top-left-radius: 12px;
      border-top-right-radius: 12px;
    }
    #searchIcon {
      font-size: 16px;
      background: transparent;
    }
    #searchInput {
      background-color: transparent;
      border: none;
      color: %3;
      font-size: 16px;
      padding: 8px;
    }
    #cancelBtn {
      color: %4;
      font-size: 14px;
      padding: 8px 16px;
      border: none;
      background: transparent;
    }
    #cancelBtn:hover {
      color: %5;
    }
    #noteList {
      background-color: %1;
      border: none;
      outline: none;
      border-bottom-left-radius: 12px;
      border-bottom-right-radius: 12px;
    }
    #noteList::item {
      background-color: transparent;
      border-bottom: 1px solid %2;
      padding: 12px 16px;
    }
    #noteList::item:selected {
      background-color: %6;
    }
    #noteList::item:hover {
      background-color: %6;
    }
    QScrollBar:vertical {
      background: %1;
      width: 8px;
      border-radius: 4px;
    }
    QScrollBar::handle:vertical {
      background: %2;
      border-radius: 4px;
      min-height: 30px;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
      height: 0;
    }
  )")
                    .arg(bg)          // %1
                    .arg(border)      // %2
                    .arg(fg)          // %3
                    .arg(accent)      // %4
                    .arg(secondary)   // %5
                    .arg(selection)); // %6
}

void SearchModal::updateTheme() { applyTheme(); }

void SearchModal::showAndFocus() {
  applyTheme(); // Refresh theme
  populateNotes();
  m_searchEdit->clear();

  // Position in center of parent
  if (parentWidget()) {
    int w = qMin(600, parentWidget()->width() - 40);
    int h = qMin(500, parentWidget()->height() - 60);
    int x = (parentWidget()->width() - w) / 2;
    int y = 30; // Top margin
    setGeometry(x, y, w, h);
  }

  show();
  raise();
  m_searchEdit->setFocus();
}

void SearchModal::hideModal() {
  hide();
  emit dismissed();
}

bool SearchModal::eventFilter(QObject *obj, QEvent *event) {
  // Close modal on click outside
  if (event->type() == QEvent::MouseButtonPress && isVisible()) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (!geometry().contains(mouseEvent->pos())) {
      hideModal();
      return true;
    }
  }
  return QFrame::eventFilter(obj, event);
}

void SearchModal::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    hideModal();
    return;
  }
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    if (m_noteList->currentItem()) {
      int index = m_noteList->currentItem()->data(Qt::UserRole).toInt();
      emit noteSelected(index);
      hideModal();
    }
    return;
  }
  if (event->key() == Qt::Key_Down) {
    int row = m_noteList->currentRow();
    if (row < m_noteList->count() - 1) {
      m_noteList->setCurrentRow(row + 1);
    }
    return;
  }
  if (event->key() == Qt::Key_Up) {
    int row = m_noteList->currentRow();
    if (row > 0) {
      m_noteList->setCurrentRow(row - 1);
    }
    return;
  }
  QFrame::keyPressEvent(event);
}

void SearchModal::onSearchTextChanged(const QString &text) {
  filterNotes(text);
}

void SearchModal::onItemClicked(QListWidgetItem *item) {
  m_noteList->setCurrentItem(item);
}

void SearchModal::onItemDoubleClicked(QListWidgetItem *item) {
  int index = item->data(Qt::UserRole).toInt();
  emit noteSelected(index);
  hideModal();
}

void SearchModal::populateNotes() { filterNotes(QString()); }

void SearchModal::filterNotes(const QString &filter) {
  m_noteList->clear();

  if (!m_noteManager)
    return;

  // Get theme colors for item labels
  ThemeManager &tm = ThemeManager::instance();
  Theme theme = tm.currentTheme();
  ThemeColors themeColors = theme.colors(tm.isDarkMode());

  QString textColor = themeColors.foreground.name();
  QString subtextColor = themeColors.comment.name();
  QString greenColor = themeColors.result.name();

  QString lowerFilter = filter.toLower().trimmed();
  int noteCount = m_noteManager->noteCount();

  for (int i = 0; i < noteCount; ++i) {
    QString content = m_noteManager->noteContentAt(i);
    Note note = m_noteManager->noteAt(i);

    // Filter check
    if (!lowerFilter.isEmpty()) {
      if (!content.toLower().contains(lowerFilter)) {
        continue;
      }
    }

    // Create list item with custom widget
    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::UserRole, i);

    // Create custom widget for the item
    QWidget *itemWidget = new QWidget();
    QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
    itemLayout->setContentsMargins(0, 4, 0, 4);
    itemLayout->setSpacing(4);

    // Mode indicator + Title (first line)
    QHBoxLayout *titleRow = new QHBoxLayout();
    titleRow->setSpacing(8);

    // Mode indicator
    QString modeName = noteModeName(note.mode()).toLower();
    if (note.mode() != NoteMode::PlainText) {
      QLabel *modeLabel = new QLabel(modeName);
      modeLabel->setStyleSheet(
          QString("color: %1; font-size: 11px; font-weight: bold; background: "
                  "transparent;")
              .arg(greenColor));
      titleRow->addWidget(modeLabel);
    }

    // Title (first line or "[Empty Note]")
    QString firstLine = content.section('\n', 0, 0).trimmed();
    if (firstLine.isEmpty()) {
      firstLine = tr("[Empty Note]");
    }
    QLabel *titleLabel = new QLabel(truncateText(firstLine, 60));
    titleLabel->setStyleSheet(
        QString("color: %1; font-size: 13px; font-weight: bold; background: "
                "transparent;")
            .arg(textColor));
    titleRow->addWidget(titleLabel, 1);
    itemLayout->addLayout(titleRow);

    // Content preview (second line onwards)
    QString preview = content.section('\n', 1).trimmed();
    preview = preview.replace('\n', " | ");
    if (!preview.isEmpty()) {
      QLabel *previewLabel = new QLabel(truncateText(preview, 100));
      previewLabel->setStyleSheet(
          QString("color: %1; font-size: 12px; background: transparent;")
              .arg(subtextColor));
      previewLabel->setWordWrap(true);
      itemLayout->addWidget(previewLabel);
    }

    // Date
    QLabel *dateLabel = new QLabel(formatDate(note.modified()));
    dateLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; background: transparent;")
            .arg(subtextColor));
    itemLayout->addWidget(dateLabel);

    item->setSizeHint(itemWidget->sizeHint() + QSize(0, 20));
    m_noteList->addItem(item);
    m_noteList->setItemWidget(item, itemWidget);
  }

  // Select first item
  if (m_noteList->count() > 0) {
    m_noteList->setCurrentRow(0);
  }
}

QString SearchModal::truncateText(const QString &text, int maxLength) const {
  if (text.length() <= maxLength) {
    return text;
  }
  return text.left(maxLength - 3) + "...";
}

QString SearchModal::formatDate(const QDateTime &date) const {
  QDate today = QDate::currentDate();
  QDate noteDate = date.date();

  if (noteDate == today) {
    return date.toString("h:mm AP");
  } else if (noteDate == today.addDays(-1)) {
    return tr("Yesterday");
  } else if (noteDate.year() == today.year()) {
    return date.toString("MMMM d");
  } else {
    return date.toString("MMMM d, yyyy");
  }
}
