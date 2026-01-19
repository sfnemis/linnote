#include "SearchBar.h"
#include "core/NoteManager.h"
#include "core/Settings.h"
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent), m_editor(nullptr), m_noteManager(nullptr),
      m_currentResultIndex(-1), m_replaceVisible(false), m_caseSensitive(false),
      m_useRegex(false) {
  setupUI();
  hide();

  // Update style when theme changes
  connect(Settings::instance(), &Settings::settingsChanged, this,
          &SearchBar::applyButtonStyles);
}

void SearchBar::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(16, 12, 16, 12);
  mainLayout->setSpacing(8);

  // === Search Row - Antinote Style ===
  QFrame *searchFrame = new QFrame();
  searchFrame->setObjectName("searchFrame");
  QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
  searchLayout->setContentsMargins(16, 12, 16, 12);
  searchLayout->setSpacing(12);

  // Search icon
  auto *searchIcon = new QLabel("🔍");
  searchIcon->setStyleSheet("font-size: 16px; background: transparent;");
  searchLayout->addWidget(searchIcon);

  // Search input - larger, prominent
  m_searchEdit = new QLineEdit();
  m_searchEdit->setPlaceholderText(tr("Search all notes..."));
  m_searchEdit->setMinimumHeight(36);
  connect(m_searchEdit, &QLineEdit::textChanged, this,
          &SearchBar::onSearchTextChanged);
  connect(m_searchEdit, &QLineEdit::returnPressed, this, &SearchBar::findNext);
  searchLayout->addWidget(m_searchEdit, 1);

  // Match count - subtle
  m_countLabel = new QLabel("");
  m_countLabel->setStyleSheet("color: #6c7086; font-size: 12px;");
  m_countLabel->setMinimumWidth(70);
  searchLayout->addWidget(m_countLabel);

  // Cancel button - text style like Antinote
  m_closeBtn = new QPushButton(tr("Cancel"));
  m_closeBtn->setObjectName("searchCancelBtn");
  m_closeBtn->setFlat(true);
  connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
    clear();
    hide();
    emit closeRequested();
  });
  searchLayout->addWidget(m_closeBtn);

  mainLayout->addWidget(searchFrame);

  // === Navigation Row - compact ===
  auto *navRow = new QHBoxLayout();
  navRow->setSpacing(8);

  // Options
  m_caseCheck = new QCheckBox(tr("Aa"));
  m_caseCheck->setToolTip(tr("Case Sensitive"));
  connect(m_caseCheck, &QCheckBox::toggled, this, [this](bool checked) {
    m_caseSensitive = checked;
    updateHighlights();
  });
  navRow->addWidget(m_caseCheck);

  m_regexCheck = new QCheckBox(tr(".*"));
  m_regexCheck->setToolTip(tr("Regular Expression"));
  connect(m_regexCheck, &QCheckBox::toggled, this, [this](bool checked) {
    m_useRegex = checked;
    updateHighlights();
  });
  navRow->addWidget(m_regexCheck);

  navRow->addStretch();

  // Navigation buttons
  m_prevBtn = new QPushButton("↑");
  m_prevBtn->setFixedSize(32, 28);
  m_prevBtn->setToolTip(tr("Previous (Shift+Enter)"));
  connect(m_prevBtn, &QPushButton::clicked, this, &SearchBar::findPrevious);
  navRow->addWidget(m_prevBtn);

  m_nextBtn = new QPushButton("↓");
  m_nextBtn->setFixedSize(32, 28);
  m_nextBtn->setToolTip(tr("Next (Enter)"));
  connect(m_nextBtn, &QPushButton::clicked, this, &SearchBar::findNext);
  navRow->addWidget(m_nextBtn);

  mainLayout->addLayout(navRow);

  // === Replace Row ===
  m_replaceRow = new QWidget();
  auto *replaceLayout = new QHBoxLayout(m_replaceRow);
  replaceLayout->setContentsMargins(0, 0, 0, 0);
  replaceLayout->setSpacing(8);

  auto *replaceIcon = new QLabel("↻");
  replaceIcon->setStyleSheet("font-size: 14px;");
  replaceLayout->addWidget(replaceIcon);

  m_replaceEdit = new QLineEdit();
  m_replaceEdit->setPlaceholderText(tr("Replace with..."));
  m_replaceEdit->setMinimumHeight(32);
  replaceLayout->addWidget(m_replaceEdit, 1);

  m_replaceBtn = new QPushButton(tr("Replace"));
  m_replaceBtn->setFixedHeight(32);
  connect(m_replaceBtn, &QPushButton::clicked, this, &SearchBar::replaceOne);
  replaceLayout->addWidget(m_replaceBtn);

  m_replaceAllBtn = new QPushButton(tr("All"));
  m_replaceAllBtn->setFixedHeight(32);
  connect(m_replaceAllBtn, &QPushButton::clicked, this, &SearchBar::replaceAll);
  replaceLayout->addWidget(m_replaceAllBtn);

  m_replaceRow->hide();
  mainLayout->addWidget(m_replaceRow);

  setObjectName("searchBar");
  applyButtonStyles();
}

void SearchBar::applyButtonStyles() {
  bool isDark = Settings::instance()->darkMode();

  // Theme-aware color palette
  QString bgColor = isDark ? "#1e1e2e" : "#fafafa";
  QString borderColor = isDark ? "#45475a" : "#ccd0da";
  QString frameBg =
      isDark ? "rgba(49, 50, 68, 0.5)" : "rgba(220, 224, 232, 0.5)";
  QString textColor = isDark ? "#cdd6f4" : "#4c4f69";
  QString placeholderColor = isDark ? "#6c7086" : "#9ca0b0";
  QString accentColor = isDark ? "#89b4fa" : "#1e66f5";
  QString accentHover = isDark ? "#b4befe" : "#4c7cff";
  QString btnBg = isDark ? "#313244" : "#e6e9ef";
  QString btnHover = isDark ? "#45475a" : "#dce0e8";
  QString checkActiveColor = isDark ? "#a6e3a1" : "#40a02b";

  // Modern theme-aware styling
  setStyleSheet(QString(R"(
    #searchBar {
      background-color: %1;
      border: 1px solid %2;
      border-radius: 12px;
    }
    #searchFrame {
      background-color: %3;
      border: 1px solid %2;
      border-radius: 8px;
    }
    #searchFrame QLineEdit {
      background: transparent;
      border: none;
      color: %4;
      font-size: 15px;
      padding: 6px;
    }
    #searchFrame QLineEdit::placeholder {
      color: %5;
    }
    #searchCancelBtn {
      color: %6;
      font-size: 13px;
      padding: 6px 12px;
      border: none;
      background: transparent;
    }
    #searchCancelBtn:hover {
      color: %7;
    }
    QPushButton {
      background-color: %8;
      color: %4;
      border: 1px solid %2;
      border-radius: 6px;
      padding: 4px 12px;
    }
    QPushButton:hover {
      background-color: %9;
    }
    QCheckBox {
      color: %5;
      spacing: 4px;
    }
    QCheckBox:checked {
      color: %10;
    }
    QLineEdit {
      background-color: %3;
      border: 1px solid %2;
      border-radius: 6px;
      color: %4;
      padding: 6px 10px;
    }
  )")
                    .arg(bgColor, borderColor, frameBg, textColor,
                         placeholderColor, accentColor, accentHover, btnBg,
                         btnHover, checkActiveColor));
}

void SearchBar::setEditor(QPlainTextEdit *editor) { m_editor = editor; }

void SearchBar::setNoteManager(NoteManager *noteManager) {
  m_noteManager = noteManager;
}

void SearchBar::showSearch() {
  m_replaceVisible = false;
  m_replaceRow->hide();
  show();
  focusSearch();
}

void SearchBar::showReplace() {
  m_replaceVisible = true;
  m_replaceRow->show();
  show();
  focusSearch();
}

void SearchBar::focusSearch() {
  m_searchEdit->setFocus();
  m_searchEdit->selectAll();
}

void SearchBar::setSearchText(const QString &text) {
  m_searchEdit->setText(text);
  updateHighlights();
}

void SearchBar::clear() {
  m_searchEdit->clear();
  m_replaceEdit->clear();
  clearHighlights();
  m_results.clear();
  m_currentResultIndex = -1;
  m_countLabel->clear();
}

void SearchBar::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    clear();
    hide();
    emit closeRequested();
    return;
  }

  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    if (event->modifiers() & Qt::ShiftModifier) {
      findPrevious();
    } else {
      findNext();
    }
    return;
  }

  QWidget::keyPressEvent(event);
}

void SearchBar::onSearchTextChanged(const QString &text) {
  Q_UNUSED(text);
  updateHighlights();
}

void SearchBar::updateHighlights() {
  clearHighlights();
  m_results.clear();
  m_currentResultIndex = -1;

  if (m_searchEdit->text().isEmpty()) {
    m_countLabel->clear();
    return;
  }

  searchAllPages();

  if (m_results.isEmpty()) {
    m_countLabel->setText(tr("No matches"));
    m_searchEdit->setStyleSheet("background-color: rgba(243, 139, 168, 0.3);");
  } else {
    m_currentResultIndex = 0;
    selectResult(0);
    m_searchEdit->setStyleSheet("");
  }
}

void SearchBar::searchAllPages() {
  if (!m_noteManager)
    return;

  QString searchText = m_searchEdit->text();
  if (searchText.isEmpty())
    return;

  // Determine case sensitivity
  Qt::CaseSensitivity cs =
      m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

  // Search in all pages
  int noteCount = m_noteManager->noteCount();
  for (int pageIdx = 0; pageIdx < noteCount; ++pageIdx) {
    QString content = m_noteManager->noteContentAt(pageIdx);

    if (m_useRegex) {
      // Regex search
      QRegularExpression::PatternOptions opts =
          QRegularExpression::NoPatternOption;
      if (!m_caseSensitive) {
        opts |= QRegularExpression::CaseInsensitiveOption;
      }
      QRegularExpression regex(searchText, opts);
      if (!regex.isValid()) {
        m_countLabel->setText(tr("Invalid regex"));
        m_searchEdit->setStyleSheet(
            "background-color: rgba(243, 139, 168, 0.3);");
        return;
      }
      QRegularExpressionMatchIterator it = regex.globalMatch(content);
      while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        SearchResult result;
        result.pageIndex = pageIdx;
        result.position = match.capturedStart();
        result.length = match.capturedLength();
        m_results.append(result);
      }
    } else {
      // Plain text search
      int pos = 0;
      while ((pos = content.indexOf(searchText, pos, cs)) != -1) {
        SearchResult result;
        result.pageIndex = pageIdx;
        result.position = pos;
        result.length = searchText.length();
        m_results.append(result);
        pos += searchText.length();
      }
    }
  }

  if (!m_results.isEmpty()) {
    int currentPage = m_noteManager->currentIndex();
    // Find first result on current page, or first result overall
    int startIdx = 0;
    for (int i = 0; i < m_results.size(); ++i) {
      if (m_results[i].pageIndex == currentPage) {
        startIdx = i;
        break;
      }
    }
    m_currentResultIndex = startIdx;
    m_countLabel->setText(
        QString("%1/%2").arg(startIdx + 1).arg(m_results.size()));
  }
}

void SearchBar::clearHighlights() { m_searchEdit->setStyleSheet(""); }

void SearchBar::selectResult(int index) {
  if (m_results.isEmpty() || index < 0 || index >= m_results.size()) {
    return;
  }

  m_currentResultIndex = index;
  const SearchResult &result = m_results[index];

  // Switch page if needed
  if (m_noteManager && m_noteManager->currentIndex() != result.pageIndex) {
    m_noteManager->goToNote(result.pageIndex);
    emit goToPage(result.pageIndex);
  }

  // Select text in editor
  if (m_editor) {
    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(result.position);
    cursor.setPosition(result.position + result.length,
                       QTextCursor::KeepAnchor);
    m_editor->setTextCursor(cursor);
    m_editor->centerCursor();
  }

  // Update count
  m_countLabel->setText(QString("%1/%2").arg(index + 1).arg(m_results.size()));
}

void SearchBar::findNext() {
  if (m_results.isEmpty()) {
    updateHighlights();
    return;
  }

  int nextIndex = (m_currentResultIndex + 1) % m_results.size();
  selectResult(nextIndex);
}

void SearchBar::findPrevious() {
  if (m_results.isEmpty()) {
    updateHighlights();
    return;
  }

  int prevIndex = m_currentResultIndex - 1;
  if (prevIndex < 0) {
    prevIndex = m_results.size() - 1;
  }
  selectResult(prevIndex);
}

void SearchBar::replaceOne() {
  if (!m_editor || m_results.isEmpty() || m_currentResultIndex < 0) {
    return;
  }

  QTextCursor cursor = m_editor->textCursor();
  if (cursor.hasSelection()) {
    cursor.insertText(m_replaceEdit->text());
  }

  // Re-search after replace
  updateHighlights();
  if (!m_results.isEmpty()) {
    int nextIndex = m_currentResultIndex;
    if (nextIndex >= m_results.size()) {
      nextIndex = 0;
    }
    selectResult(nextIndex);
  }
}

void SearchBar::replaceAll() {
  if (!m_editor || !m_noteManager || m_searchEdit->text().isEmpty()) {
    return;
  }

  QString searchText = m_searchEdit->text();
  QString replaceText = m_replaceEdit->text();
  int totalCount = 0;

  // Determine case sensitivity
  Qt::CaseSensitivity cs =
      m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

  // Replace in all pages
  int noteCount = m_noteManager->noteCount();
  for (int pageIdx = 0; pageIdx < noteCount; ++pageIdx) {
    QString content = m_noteManager->noteContentAt(pageIdx);

    if (m_useRegex) {
      // Regex replace
      QRegularExpression::PatternOptions opts =
          QRegularExpression::NoPatternOption;
      if (!m_caseSensitive) {
        opts |= QRegularExpression::CaseInsensitiveOption;
      }
      QRegularExpression regex(searchText, opts);
      if (!regex.isValid()) {
        m_countLabel->setText(tr("Invalid regex"));
        return;
      }
      int count = 0;
      QString newContent = content;
      newContent.replace(regex, replaceText);
      // Count matches
      QRegularExpressionMatchIterator it = regex.globalMatch(content);
      while (it.hasNext()) {
        it.next();
        count++;
      }
      if (count > 0) {
        m_noteManager->setNoteContentAt(pageIdx, newContent);
        totalCount += count;
      }
    } else {
      // Plain text replace
      int count = content.count(searchText, cs);
      if (count > 0) {
        content.replace(searchText, replaceText, cs);
        m_noteManager->setNoteContentAt(pageIdx, content);
        totalCount += count;
      }
    }
  }

  // Refresh current editor
  if (m_editor) {
    int currentIdx = m_noteManager->currentIndex();
    m_editor->setPlainText(m_noteManager->noteContentAt(currentIdx));
  }

  // Update highlights
  updateHighlights();
  m_countLabel->setText(tr("%1 replaced").arg(totalCount));
}
