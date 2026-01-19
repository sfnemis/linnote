#ifndef LINNOTE_UNITCONVERTER_H
#define LINNOTE_UNITCONVERTER_H

#include <QMap>
#include <QString>
#include <QStringList>

/**
 * @brief Unit conversion system for LinNote Math mode
 *
 * Supports conversion patterns like:
 *   5 km to mile
 *   100 kg to lb
 *   25 C to F
 *   1 GB to MB
 */
class UnitConverter {
public:
  static UnitConverter *instance();

  // Check if line contains a unit conversion pattern
  bool isConversion(const QString &line) const;

  // Perform conversion, returns result string or empty if failed
  QString convert(const QString &line) const;

  // Get all supported units for a category
  QStringList unitsInCategory(const QString &category) const;

  // Get all category names
  QStringList categories() const;

private:
  UnitConverter();

  void initUnits();

  // Unit definitions: category -> (unit alias -> {canonical name, factor to
  // base})
  struct UnitDef {
    QString canonical; // Canonical name (e.g., "meter")
    double toBase;     // Multiplication factor to convert TO base unit
    double fromBase;   // Multiplication factor to convert FROM base unit
    bool isOffset; // For temperature (needs offset, not just multiplication)
    double offset; // Offset for temperature conversions
  };

  QMap<QString, QMap<QString, UnitDef>>
      m_units;                              // category -> (alias -> UnitDef)
  QMap<QString, QString> m_aliasToCategory; // alias -> category

  // Temperature conversion helpers
  double celsiusToKelvin(double c) const;
  double kelvinToCelsius(double k) const;
  double fahrenheitToKelvin(double f) const;
  double kelvinToFahrenheit(double k) const;

  // Parse input like "5 km to mile" -> {5.0, "km", "mile"}
  struct ParsedConversion {
    double value;
    QString fromUnit;
    QString toUnit;
    bool valid;
  };
  ParsedConversion parse(const QString &line) const;

  static UnitConverter *s_instance;
};

#endif // LINNOTE_UNITCONVERTER_H
