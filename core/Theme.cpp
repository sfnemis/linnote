#include "Theme.h"
#include <QApplication>

// ============================================================================
// Theme StyleSheet Generation
// ============================================================================

QString Theme::styleSheet(bool isDark, int transparency) const {
  ThemeColors c = colors(isDark);

  QString bg = c.background.name();
  QString fg = c.foreground.name();
  QString accent = c.accent.name();
  QString secondary = c.secondary.name();
  QString border = c.border.name();
  QString selection = c.selection.name();
  QString comment = c.comment.name();
  QString highlight = c.highlight.name();

  // RGBA for transparency (Wayland compatible)
  int alpha = qBound(25, (transparency * 255) / 100, 255); // Min 10% opacity
  QString bgRgba = QString("rgba(%1, %2, %3, %4)")
                       .arg(c.background.red())
                       .arg(c.background.green())
                       .arg(c.background.blue())
                       .arg(alpha);
  QString highlightRgba = QString("rgba(%1, %2, %3, %4)")
                              .arg(c.highlight.red())
                              .arg(c.highlight.green())
                              .arg(c.highlight.blue())
                              .arg(qBound(100, alpha + 30, 255));

  return QString(R"(
    /* === Global === */
    QWidget {
      background-color: transparent;
      color: %2;
    }
    
    /* === Main Window === */
    QMainWindow {
      background: transparent;
    }
    
    /* === Container === */
    #container {
      background-color: %9;
      border-radius: 12px;
      border-top: 1px solid %5;
      border-left: 1px solid %5;
      border-right: 1px solid %5;
      border-bottom: none;
    }
    
    /* === Title Bar === */
    #titleBar {
      background-color: %9;
      border-top-left-radius: 12px;
      border-top-right-radius: 12px;
      border-bottom: 1px solid %5;
    }
    
    #titleLabel {
      color: %2;
      font-weight: bold;
      font-size: 13px;
    }
    
    /* === Title Bar Buttons === */
    #closeBtn, #minimizeBtn, #exportBtn, #settingsBtn, #pinBtn {
      background: transparent;
      border: none;
      color: %7;
      font-size: 14px;
      padding: 4px 8px;
      border-radius: 4px;
    }
    #closeBtn:hover {
      background-color: #f38ba8;
      color: %8;
    }
    #minimizeBtn:hover, #exportBtn:hover, #settingsBtn:hover, #pinBtn:hover {
      background-color: %5;
      color: %2;
    }
    
    /* === Dialog === */
    QDialog {
      background-color: %1;
      border: 1px solid %5;
      border-radius: 12px;
    }
    
    /* === Text Editor === */
    QPlainTextEdit, QTextEdit {
      background-color: %9;
      color: %2;
      border: none;
      selection-background-color: %6;
      selection-color: %2;
      padding: 12px;
      border-bottom-left-radius: 12px;
      border-bottom-right-radius: 12px;
    }
    QPlainTextEdit QWidget, QTextEdit QWidget {
      background: transparent;
    }
    QAbstractScrollArea::corner {
      background: transparent;
    }
    
    /* === Labels === */
    QLabel {
      background-color: transparent;
      color: %2;
    }
    
    /* === Push Buttons === */
    QPushButton {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 6px;
      padding: 8px 16px;
      font-weight: 500;
    }
    QPushButton:hover {
      background-color: %3;
      border-color: %3;
    }
    QPushButton:pressed {
      background-color: %5;
    }
    
    /* === Group Box === */
    QGroupBox {
      background-color: %4;
      border: 1px solid %5;
      border-radius: 8px;
      margin-top: 12px;
      padding: 12px;
      padding-top: 20px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 12px;
      padding: 0 6px;
      color: %3;
      font-weight: bold;
    }
    
    /* === Line Edit === */
    QLineEdit {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 6px;
      padding: 6px 10px;
    }
    QLineEdit:focus {
      border-color: %3;
    }
    
    /* === Combo Box === */
    QComboBox {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 6px;
      padding: 6px 10px;
      min-width: 80px;
    }
    QComboBox::drop-down {
      border: none;
      width: 20px;
    }
    QComboBox QAbstractItemView {
      background-color: %1;
      color: %2;
      selection-background-color: %6;
      border: 1px solid %5;
      border-radius: 6px;
    }
    
    /* === Check Box - Modern Toggle Style === */
    QCheckBox {
      color: %2;
      spacing: 10px;
      padding: 4px;
    }
    QCheckBox::indicator {
      width: 36px;
      height: 20px;
      border-radius: 10px;
      border: none;
      background: %5;
    }
    QCheckBox::indicator:checked {
      background: %3;
    }
    QCheckBox::indicator:hover {
      background: %7;
    }
    QCheckBox::indicator:checked:hover {
      background: %3;
    }
    
    /* === Spin Box === */
    QSpinBox {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 6px;
      padding: 4px 8px;
    }
    
    /* === Scroll Bar (Minimal) === */
    QScrollBar:vertical {
      background: transparent;
      width: 6px;
      margin: 2px 0;
    }
    QScrollBar::handle:vertical {
      background: rgba(128, 128, 128, 0.3);
      border-radius: 3px;
      min-height: 30px;
    }
    QScrollBar::handle:vertical:hover {
      background: rgba(128, 128, 128, 0.6);
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
      background: transparent;
      height: 0px;
    }
    QScrollBar:horizontal {
      background: transparent;
      height: 6px;
      margin: 0 2px;
    }
    QScrollBar::handle:horizontal {
      background: rgba(128, 128, 128, 0.3);
      border-radius: 3px;
      min-width: 30px;
    }
    QScrollBar::handle:horizontal:hover {
      background: rgba(128, 128, 128, 0.6);
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
      background: transparent;
      width: 0px;
    }
    
    /* === Timer Widget (Horizontal Bar) === */
    #timerStatusLabel {
      font-size: 14px;
      font-weight: 500;
      color: %2;
      background: transparent;
      padding: 4px 8px;
    }
    
    #timerDisplay {
      font-size: 24px;
      font-weight: 700;
      font-family: 'JetBrains Mono', 'SF Mono', 'Fira Code', monospace;
      color: %3;
      background: transparent;
      padding: 4px 12px;
    }
    
    #timerSessionLabel {
      font-size: 13px;
      font-weight: 500;
      color: %7;
      background: transparent;
      padding: 4px;
    }
    
    #timerSpinBox {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 4px;
      padding: 4px 8px;
      font-size: 13px;
    }
    
    #timerStartBtn {
      background-color: %3;
      color: %1;
      border: none;
      border-radius: 6px;
      padding: 6px 12px;
      font-weight: 600;
      font-size: 13px;
    }
    #timerStartBtn:hover {
      background-color: %10;
      color: %2;
    }
    
    #timerResetBtn {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 6px;
      padding: 6px 12px;
      font-size: 13px;
    }
    #timerResetBtn:hover {
      background-color: %5;
    }
    
    #timerCloseBtn {
      background-color: rgba(243, 139, 168, 0.15);
      color: #f38ba8;
      border: 1px solid rgba(243, 139, 168, 0.3);
      border-radius: 6px;
      font-size: 14px;
      font-weight: bold;
      padding: 0px;
      min-width: 32px;
      min-height: 32px;
    }
    #timerCloseBtn:hover {
      color: #ffffff;
      background-color: #f38ba8;
      border-color: #f38ba8;
    }

    /* ========== Search Bar ========== */
    #searchBar {
      background-color: %1;
      border-top: 1px solid %5;
      padding: 4px 8px;
    }
    #searchBar QLineEdit {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 4px;
      padding: 4px 8px;
    }
    #searchBar QLineEdit:focus {
      border-color: %3;
    }
    #searchBar QPushButton {
      background-color: %4;
      color: %2;
      border: 1px solid %5;
      border-radius: 4px;
      font-weight: bold;
      font-size: 13px;
      padding: 4px 8px;
      min-width: 28px;
      min-height: 24px;
    }
    #searchBar QPushButton:hover {
      background-color: %6;
      border-color: %3;
    }
    #searchBar QPushButton:pressed {
      background-color: %5;
    }
    #searchBarCloseBtn {
      background-color: rgba(243, 139, 168, 0.15);
      color: #f38ba8;
      border: 1px solid rgba(243, 139, 168, 0.3);
    }
    #searchBarCloseBtn:hover {
      background-color: #f38ba8;
      color: %1;
      border-color: #f38ba8;
    }
  )")
      .arg(bg, fg, accent, secondary, border, selection, comment, highlight)
      .arg(bgRgba)
      .arg(highlightRgba);
}

// ============================================================================
// ThemeManager Singleton
// ============================================================================

ThemeManager &ThemeManager::instance() {
  static ThemeManager instance;
  return instance;
}

ThemeManager::ThemeManager()
    : m_darkMode(true), m_currentThemeName("Catppuccin") {
  initBuiltInThemes();
}

void ThemeManager::initBuiltInThemes() {
  // === Catppuccin (Default) ===
  m_themes.append(
      Theme("Catppuccin",
            {QColor("#f38ba8"), QColor("#fab387"), QColor("#f9e2af"),
             QColor("#a6e3a1"), QColor("#89b4fa"), QColor("#cba6f7"),
             QColor("#f5c2e7"), QColor("#94e2d5")},
            // Light (Latte)
            ThemeColors{QColor("#eff1f5"), QColor("#4c4f69"), QColor("#1e66f5"),
                        QColor("#dce0e8"), QColor("#bcc0cc"), QColor("#acb0be"),
                        QColor("#9ca0b0"), QColor("#e6e9ef"), QColor("#8839ef"),
                        QColor("#fe640b"), QColor("#40a02b"), QColor("#1e66f5"),
                        QColor("#179299")},
            // Dark (Mocha)
            ThemeColors{QColor("#1e1e2e"), QColor("#cdd6f4"), QColor("#89b4fa"),
                        QColor("#313244"), QColor("#45475a"), QColor("#585b70"),
                        QColor("#6c7086"), QColor("#181825"), QColor("#cba6f7"),
                        QColor("#fab387"), QColor("#a6e3a1"), QColor("#89b4fa"),
                        QColor("#a6e3a1")}));

  // === Tokyo Drift ===
  m_themes.append(
      Theme("Tokyo Drift",
            {QColor("#1a1b26"), QColor("#7aa2f7"), QColor("#bb9af7"),
             QColor("#9ece6a"), QColor("#f7768e"), QColor("#ff9e64"),
             QColor("#73daca"), QColor("#e0af68")},
            // Light
            ThemeColors{QColor("#d5d6db"), QColor("#343b58"), QColor("#34548a"),
                        QColor("#9699a3"), QColor("#8c8fa8"), QColor("#9699a3"),
                        QColor("#9699a3"), QColor("#e9e9ec"), QColor("#5a4a78"),
                        QColor("#965027"), QColor("#485e30"), QColor("#34548a"),
                        QColor("#166775")},
            // Dark
            ThemeColors{QColor("#1a1b26"), QColor("#a9b1d6"), QColor("#7aa2f7"),
                        QColor("#24283b"), QColor("#414868"), QColor("#33467c"),
                        QColor("#565f89"), QColor("#16161e"), QColor("#bb9af7"),
                        QColor("#ff9e64"), QColor("#9ece6a"), QColor("#7aa2f7"),
                        QColor("#73daca")}));

  // === Vancouver ===
  m_themes.append(
      Theme("Vancouver",
            {QColor("#1a4480"), QColor("#2e8b57"), QColor("#228b22"),
             QColor("#32cd32"), QColor("#ffd700"), QColor("#ff6347"),
             QColor("#ff69b4"), QColor("#dda0dd")},
            // Light
            ThemeColors{QColor("#f0f5f9"), QColor("#1a4480"), QColor("#2e8b57"),
                        QColor("#e1e8ed"), QColor("#c5d0db"), QColor("#b8c9d8"),
                        QColor("#8fa4b8"), QColor("#f5f8fb"), QColor("#1a4480"),
                        QColor("#d35400"), QColor("#27ae60"), QColor("#2980b9"),
                        QColor("#16a085")},
            // Dark
            ThemeColors{QColor("#0d1b2a"), QColor("#e0e1dd"), QColor("#4ea8de"),
                        QColor("#1b263b"), QColor("#415a77"), QColor("#336699"),
                        QColor("#778da9"), QColor("#0a141f"), QColor("#7ec8e3"),
                        QColor("#f4a261"), QColor("#2a9d8f"), QColor("#4ea8de"),
                        QColor("#52b788")}));

  // === Totoro ===
  m_themes.append(
      Theme("Totoro",
            {QColor("#4a6741"), QColor("#6b8e23"), QColor("#8fbc8f"),
             QColor("#98fb98"), QColor("#ffa07a"), QColor("#ff6b6b"),
             QColor("#dda0dd"), QColor("#b0e0e6")},
            // Light
            ThemeColors{QColor("#f5f5dc"), QColor("#3d5a3d"), QColor("#6b8e23"),
                        QColor("#e8e8d0"), QColor("#d4d4b8"), QColor("#c5c5a0"),
                        QColor("#8b8b6e"), QColor("#fafae6"), QColor("#228b22"),
                        QColor("#cd853f"), QColor("#6b8e23"), QColor("#4682b4"),
                        QColor("#2e8b57")},
            // Dark
            ThemeColors{QColor("#1a2e1a"), QColor("#d4edda"), QColor("#8fbc8f"),
                        QColor("#243524"), QColor("#3d5a3d"), QColor("#4a6741"),
                        QColor("#6b8b6b"), QColor("#121f12"), QColor("#98fb98"),
                        QColor("#ffa07a"), QColor("#90ee90"), QColor("#87ceeb"),
                        QColor("#7fffd4")}));

  // === Mononoke ===
  m_themes.append(
      Theme("Mononoke",
            {QColor("#2d5a3f"), QColor("#4a7c59"), QColor("#8cc084"),
             QColor("#ffa500"), QColor("#ff4500"), QColor("#cd5c5c"),
             QColor("#deb887"), QColor("#bc8f8f")},
            // Light
            ThemeColors{QColor("#f5f0e6"), QColor("#2d3a2e"), QColor("#4a7c59"),
                        QColor("#e6dfd0"), QColor("#d4c8b0"), QColor("#c4b89a"),
                        QColor("#8c8572"), QColor("#faf5eb"), QColor("#2d5a3f"),
                        QColor("#cc7000"), QColor("#4a7c59"), QColor("#8b4513"),
                        QColor("#228b22")},
            // Dark
            ThemeColors{QColor("#1a1f1a"), QColor("#d4dbd5"), QColor("#8cc084"),
                        QColor("#252b25"), QColor("#3d4a3d"), QColor("#4a5a4a"),
                        QColor("#6b7b6b"), QColor("#12160f"), QColor("#a8d8a8"),
                        QColor("#ffa500"), QColor("#90ee90"), QColor("#deb887"),
                        QColor("#98fb98")}));

  // === Piccolo ===
  m_themes.append(
      Theme("Piccolo",
            {QColor("#6a0dad"), QColor("#9932cc"), QColor("#ba55d3"),
             QColor("#ee82ee"), QColor("#ff69b4"), QColor("#ff1493"),
             QColor("#00ff7f"), QColor("#7fff00")},
            // Light
            ThemeColors{QColor("#faf5ff"), QColor("#4a1259"), QColor("#9932cc"),
                        QColor("#ede0f5"), QColor("#dcc5e8"), QColor("#cba9db"),
                        QColor("#9f7fb8"), QColor("#fcf8ff"), QColor("#6a0dad"),
                        QColor("#c71585"), QColor("#228b22"), QColor("#4169e1"),
                        QColor("#2e8b57")},
            // Dark
            ThemeColors{QColor("#1a0d1f"), QColor("#e8d5f0"), QColor("#ba55d3"),
                        QColor("#2d1a35"), QColor("#4a2a55"), QColor("#5c3a68"),
                        QColor("#7a5a8a"), QColor("#120812"), QColor("#da70d6"),
                        QColor("#ff69b4"), QColor("#98fb98"), QColor("#87cefa"),
                        QColor("#7fff00")}));

  // === Nord ===
  m_themes.append(
      Theme("Nord",
            {QColor("#5e81ac"), QColor("#81a1c1"), QColor("#88c0d0"),
             QColor("#8fbcbb"), QColor("#a3be8c"), QColor("#ebcb8b"),
             QColor("#d08770"), QColor("#bf616a")},
            // Light (Snow Storm)
            ThemeColors{QColor("#eceff4"), QColor("#2e3440"), QColor("#5e81ac"),
                        QColor("#e5e9f0"), QColor("#d8dee9"), QColor("#c5cdd9"),
                        QColor("#7b88a1"), QColor("#f0f4f8"), QColor("#5e81ac"),
                        QColor("#d08770"), QColor("#a3be8c"), QColor("#81a1c1"),
                        QColor("#88c0d0")},
            // Dark (Polar Night)
            ThemeColors{QColor("#2e3440"), QColor("#eceff4"), QColor("#88c0d0"),
                        QColor("#3b4252"), QColor("#434c5e"), QColor("#4c566a"),
                        QColor("#616e88"), QColor("#242933"), QColor("#81a1c1"),
                        QColor("#d08770"), QColor("#a3be8c"), QColor("#88c0d0"),
                        QColor("#8fbcbb")}));

  // === Dracula ===
  m_themes.append(
      Theme("Dracula",
            {QColor("#ff79c6"), QColor("#bd93f9"), QColor("#8be9fd"),
             QColor("#50fa7b"), QColor("#f1fa8c"), QColor("#ffb86c"),
             QColor("#ff5555"), QColor("#6272a4")},
            // Light
            ThemeColors{QColor("#f8f8f2"), QColor("#282a36"), QColor("#bd93f9"),
                        QColor("#e8e8e0"), QColor("#d0d0c8"), QColor("#c0c0b8"),
                        QColor("#909088"), QColor("#fcfcf6"), QColor("#8b5cf6"),
                        QColor("#f97316"), QColor("#22c55e"), QColor("#3b82f6"),
                        QColor("#14b8a6")},
            // Dark
            ThemeColors{QColor("#282a36"), QColor("#f8f8f2"), QColor("#bd93f9"),
                        QColor("#44475a"), QColor("#6272a4"), QColor("#7280a8"),
                        QColor("#6272a4"), QColor("#1e1f29"), QColor("#ff79c6"),
                        QColor("#ffb86c"), QColor("#50fa7b"), QColor("#8be9fd"),
                        QColor("#50fa7b")}));

  // === Gruvbox ===
  m_themes.append(
      Theme("Gruvbox",
            {QColor("#cc241d"), QColor("#d65d0e"), QColor("#d79921"),
             QColor("#98971a"), QColor("#689d6a"), QColor("#458588"),
             QColor("#b16286"), QColor("#a89984")},
            // Light
            ThemeColors{QColor("#fbf1c7"), QColor("#3c3836"), QColor("#d65d0e"),
                        QColor("#ebdbb2"), QColor("#d5c4a1"), QColor("#bdae93"),
                        QColor("#7c6f64"), QColor("#f9f5d7"), QColor("#9d0006"),
                        QColor("#af3a03"), QColor("#79740e"), QColor("#076678"),
                        QColor("#427b58")},
            // Dark
            ThemeColors{QColor("#282828"), QColor("#ebdbb2"), QColor("#fe8019"),
                        QColor("#3c3836"), QColor("#504945"), QColor("#665c54"),
                        QColor("#928374"), QColor("#1d2021"), QColor("#fb4934"),
                        QColor("#fe8019"), QColor("#b8bb26"), QColor("#83a598"),
                        QColor("#8ec07c")}));

  // === Solarized ===
  m_themes.append(
      Theme("Solarized",
            {QColor("#b58900"), QColor("#cb4b16"), QColor("#dc322f"),
             QColor("#d33682"), QColor("#6c71c4"), QColor("#268bd2"),
             QColor("#2aa198"), QColor("#859900")},
            // Light
            ThemeColors{QColor("#fdf6e3"), QColor("#657b83"), QColor("#268bd2"),
                        QColor("#eee8d5"), QColor("#93a1a1"), QColor("#839496"),
                        QColor("#657b83"), QColor("#fffbeb"), QColor("#6c71c4"),
                        QColor("#cb4b16"), QColor("#859900"), QColor("#268bd2"),
                        QColor("#2aa198")},
            // Dark
            ThemeColors{QColor("#002b36"), QColor("#839496"), QColor("#268bd2"),
                        QColor("#073642"), QColor("#586e75"), QColor("#657b83"),
                        QColor("#657b83"), QColor("#001e26"), QColor("#b58900"),
                        QColor("#cb4b16"), QColor("#859900"), QColor("#2aa198"),
                        QColor("#2aa198")}));

  // === One Dark ===
  m_themes.append(
      Theme("One Dark",
            {QColor("#e06c75"), QColor("#d19a66"), QColor("#e5c07b"),
             QColor("#98c379"), QColor("#56b6c2"), QColor("#61afef"),
             QColor("#c678dd"), QColor("#abb2bf")},
            // Light (One Light)
            ThemeColors{QColor("#fafafa"), QColor("#383a42"), QColor("#4078f2"),
                        QColor("#f0f0f0"), QColor("#d4d4d4"), QColor("#c0c0c0"),
                        QColor("#a0a1a7"), QColor("#ffffff"), QColor("#a626a4"),
                        QColor("#986801"), QColor("#50a14f"), QColor("#4078f2"),
                        QColor("#0184bc")},
            // Dark
            ThemeColors{QColor("#282c34"), QColor("#abb2bf"), QColor("#61afef"),
                        QColor("#2c323c"), QColor("#3e4451"), QColor("#4b5263"),
                        QColor("#5c6370"), QColor("#21252b"), QColor("#c678dd"),
                        QColor("#d19a66"), QColor("#98c379"), QColor("#61afef"),
                        QColor("#56b6c2")}));

  // === Monokai ===
  m_themes.append(
      Theme("Monokai",
            {QColor("#f92672"), QColor("#fd971f"), QColor("#e6db74"),
             QColor("#a6e22e"), QColor("#66d9ef"), QColor("#ae81ff"),
             QColor("#f8f8f2"), QColor("#75715e")},
            // Light
            ThemeColors{QColor("#fafafa"), QColor("#49483e"), QColor("#f92672"),
                        QColor("#f0f0f0"), QColor("#d8d8d0"), QColor("#c8c8c0"),
                        QColor("#90908a"), QColor("#ffffff"), QColor("#9c1c6b"),
                        QColor("#b87514"), QColor("#629c1c"), QColor("#1c7a9c"),
                        QColor("#1c9c7a")},
            // Dark
            ThemeColors{QColor("#272822"), QColor("#f8f8f2"), QColor("#f92672"),
                        QColor("#3e3d32"), QColor("#49483e"), QColor("#75715e"),
                        QColor("#75715e"), QColor("#1e1f1c"), QColor("#f92672"),
                        QColor("#fd971f"), QColor("#a6e22e"), QColor("#66d9ef"),
                        QColor("#a6e22e")}));

  // === KDE Breeze ===
  m_themes.append(
      Theme("Breeze",
            {QColor("#3daee9"), QColor("#27ae60"), QColor("#f67400"),
             QColor("#da4453"), QColor("#8e44ad"), QColor("#2ecc71"),
             QColor("#1abc9c"), QColor("#f39c12")},
            // Light
            ThemeColors{QColor("#eff0f1"), QColor("#232629"), QColor("#3daee9"),
                        QColor("#fcfcfc"), QColor("#bdc3c7"), QColor("#93cee9"),
                        QColor("#7f8c8d"), QColor("#ffffff"), QColor("#8e44ad"),
                        QColor("#f67400"), QColor("#27ae60"), QColor("#3daee9"),
                        QColor("#1abc9c")},
            // Dark
            ThemeColors{QColor("#232629"), QColor("#eff0f1"), QColor("#3daee9"),
                        QColor("#31363b"), QColor("#4d4d4d"), QColor("#3daee9"),
                        QColor("#7f8c8d"), QColor("#1b1e20"), QColor("#8e44ad"),
                        QColor("#f67400"), QColor("#27ae60"), QColor("#3daee9"),
                        QColor("#1abc9c")}));

  // === Adwaita ===
  m_themes.append(
      Theme("Adwaita",
            {QColor("#3584e4"), QColor("#33d17a"), QColor("#f6d32d"),
             QColor("#ff7800"), QColor("#e01b24"), QColor("#9141ac"),
             QColor("#1c71d8"), QColor("#77767b")},
            // Light
            ThemeColors{QColor("#fafafa"), QColor("#3d3846"), QColor("#1c71d8"),
                        QColor("#f6f5f4"), QColor("#deddda"), QColor("#99c1f1"),
                        QColor("#77767b"), QColor("#ffffff"), QColor("#613583"),
                        QColor("#e66100"), QColor("#26a269"), QColor("#1c71d8"),
                        QColor("#1a5fb4")},
            // Dark
            ThemeColors{QColor("#242424"), QColor("#ffffff"), QColor("#3584e4"),
                        QColor("#303030"), QColor("#464646"), QColor("#3584e4"),
                        QColor("#9a9996"), QColor("#1e1e1e"), QColor("#c061cb"),
                        QColor("#ff7800"), QColor("#33d17a"), QColor("#62a0ea"),
                        QColor("#57e389")}));

  m_currentThemeName = "Catppuccin";
}

Theme ThemeManager::theme(const QString &name) const {
  for (const Theme &t : m_themes) {
    if (t.name == name)
      return t;
  }
  return m_themes.isEmpty() ? Theme() : m_themes.first();
}

Theme ThemeManager::currentTheme() const { return theme(m_currentThemeName); }

void ThemeManager::setCurrentTheme(const QString &name) {
  m_currentThemeName = name;
}

void ThemeManager::setDarkMode(bool dark) { m_darkMode = dark; }

void ThemeManager::setTransparency(int value) { m_transparency = value; }

QString ThemeManager::generateStyleSheet() const {
  return currentTheme().styleSheet(m_darkMode, m_transparency);
}
