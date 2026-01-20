#ifndef LINNOTE_THEME_H
#define LINNOTE_THEME_H

#include <QColor>
#include <QList>
#include <QString>

/**
 * @brief Theme color palette
 */
struct ThemeColors {
  QColor background;
  QColor foreground;
  QColor accent;
  QColor secondary;
  QColor border;
  QColor selection;
  QColor comment;
  QColor highlight;

  // Syntax colors
  QColor keyword;
  QColor number;
  QColor string;
  QColor operator_;
  QColor result;
};

/**
 * @brief Theme definition with light and dark variants
 */
class Theme {
public:
  QString name;
  QList<QColor> swatchColors; // Preview colors for theme card
  ThemeColors lightColors;
  ThemeColors darkColors;

  Theme() = default;
  Theme(const QString &name, const QList<QColor> &swatches,
        const ThemeColors &light, const ThemeColors &dark)
      : name(name), swatchColors(swatches), lightColors(light),
        darkColors(dark) {}

  ThemeColors colors(bool isDark) const {
    return isDark ? darkColors : lightColors;
  }

  QString styleSheet(bool isDark, int transparency = 100) const;
};

/**
 * @brief Theme manager - holds all available themes
 */
class ThemeManager {
public:
  static ThemeManager &instance();

  QList<Theme> themes() const { return m_themes; }
  Theme theme(const QString &name) const;
  Theme currentTheme() const;
  bool isDarkMode() const { return m_darkMode; }

  void setCurrentTheme(const QString &name);
  void setDarkMode(bool dark);
  void setTransparency(int value);

  QString generateStyleSheet() const;

private:
  ThemeManager();
  void initBuiltInThemes();

  QList<Theme> m_themes;
  QString m_currentThemeName;
  bool m_darkMode;
  int m_transparency = 100;
};

#endif // LINNOTE_THEME_H
