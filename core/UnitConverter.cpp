#include "UnitConverter.h"
#include <QRegularExpression>
#include <QtMath>

UnitConverter *UnitConverter::s_instance = nullptr;

UnitConverter *UnitConverter::instance() {
  if (!s_instance) {
    s_instance = new UnitConverter();
  }
  return s_instance;
}

UnitConverter::UnitConverter() { initUnits(); }

void UnitConverter::initUnits() {
  // ===== LENGTH (base: meter) =====
  QMap<QString, UnitDef> length;
  length["mm"] = {"millimeter", 0.001, 1000, false, 0};
  length["millimeter"] = {"millimeter", 0.001, 1000, false, 0};
  length["millimeters"] = {"millimeter", 0.001, 1000, false, 0};
  length["cm"] = {"centimeter", 0.01, 100, false, 0};
  length["centimeter"] = {"centimeter", 0.01, 100, false, 0};
  length["centimeters"] = {"centimeter", 0.01, 100, false, 0};
  length["m"] = {"meter", 1.0, 1.0, false, 0};
  length["meter"] = {"meter", 1.0, 1.0, false, 0};
  length["meters"] = {"meter", 1.0, 1.0, false, 0};
  length["km"] = {"kilometer", 1000.0, 0.001, false, 0};
  length["kilometer"] = {"kilometer", 1000.0, 0.001, false, 0};
  length["kilometers"] = {"kilometer", 1000.0, 0.001, false, 0};
  length["inch"] = {"inch", 0.0254, 39.3701, false, 0};
  length["inches"] = {"inch", 0.0254, 39.3701, false, 0};
  length["in"] = {"inch", 0.0254, 39.3701, false, 0};
  length["foot"] = {"foot", 0.3048, 3.28084, false, 0};
  length["feet"] = {"foot", 0.3048, 3.28084, false, 0};
  length["ft"] = {"foot", 0.3048, 3.28084, false, 0};
  length["yard"] = {"yard", 0.9144, 1.09361, false, 0};
  length["yards"] = {"yard", 0.9144, 1.09361, false, 0};
  length["yd"] = {"yard", 0.9144, 1.09361, false, 0};
  length["mile"] = {"mile", 1609.34, 0.000621371, false, 0};
  length["miles"] = {"mile", 1609.34, 0.000621371, false, 0};
  length["mi"] = {"mile", 1609.34, 0.000621371, false, 0};
  length["nm"] = {"nautical mile", 1852.0, 0.000539957, false, 0};
  length["nauticalmile"] = {"nautical mile", 1852.0, 0.000539957, false, 0};
  m_units["length"] = length;

  // ===== MASS (base: kilogram) =====
  QMap<QString, UnitDef> mass;
  mass["mg"] = {"milligram", 0.000001, 1000000, false, 0};
  mass["milligram"] = {"milligram", 0.000001, 1000000, false, 0};
  mass["milligrams"] = {"milligram", 0.000001, 1000000, false, 0};
  mass["g"] = {"gram", 0.001, 1000, false, 0};
  mass["gram"] = {"gram", 0.001, 1000, false, 0};
  mass["grams"] = {"gram", 0.001, 1000, false, 0};
  mass["kg"] = {"kilogram", 1.0, 1.0, false, 0};
  mass["kilogram"] = {"kilogram", 1.0, 1.0, false, 0};
  mass["kilograms"] = {"kilogram", 1.0, 1.0, false, 0};
  mass["ton"] = {"metric ton", 1000.0, 0.001, false, 0};
  mass["tons"] = {"metric ton", 1000.0, 0.001, false, 0};
  mass["tonne"] = {"metric ton", 1000.0, 0.001, false, 0};
  mass["oz"] = {"ounce", 0.0283495, 35.274, false, 0};
  mass["ounce"] = {"ounce", 0.0283495, 35.274, false, 0};
  mass["ounces"] = {"ounce", 0.0283495, 35.274, false, 0};
  mass["lb"] = {"pound", 0.453592, 2.20462, false, 0};
  mass["lbs"] = {"pound", 0.453592, 2.20462, false, 0};
  mass["pound"] = {"pound", 0.453592, 2.20462, false, 0};
  mass["pounds"] = {"pound", 0.453592, 2.20462, false, 0};
  mass["stone"] = {"stone", 6.35029, 0.157473, false, 0};
  mass["stones"] = {"stone", 6.35029, 0.157473, false, 0};
  m_units["mass"] = mass;

  // ===== TEMPERATURE (base: kelvin) =====
  QMap<QString, UnitDef> temp;
  temp["c"] = {"celsius", 0, 0, true, 273.15};
  temp["celsius"] = {"celsius", 0, 0, true, 273.15};
  temp["°c"] = {"celsius", 0, 0, true, 273.15};
  temp["f"] = {"fahrenheit", 0, 0, true, 0}; // Special handling
  temp["fahrenheit"] = {"fahrenheit", 0, 0, true, 0};
  temp["°f"] = {"fahrenheit", 0, 0, true, 0};
  temp["k"] = {"kelvin", 1, 1, false, 0};
  temp["kelvin"] = {"kelvin", 1, 1, false, 0};
  m_units["temperature"] = temp;

  // ===== COMPUTING STORAGE (base: byte) =====
  QMap<QString, UnitDef> storage;
  storage["bit"] = {"bit", 0.125, 8, false, 0};
  storage["bits"] = {"bit", 0.125, 8, false, 0};
  storage["byte"] = {"byte", 1.0, 1.0, false, 0};
  storage["bytes"] = {"byte", 1.0, 1.0, false, 0};
  storage["b"] = {"byte", 1.0, 1.0, false, 0};
  storage["kb"] = {"kilobyte", 1024.0, 1.0 / 1024.0, false, 0};
  storage["kilobyte"] = {"kilobyte", 1024.0, 1.0 / 1024.0, false, 0};
  storage["kilobytes"] = {"kilobyte", 1024.0, 1.0 / 1024.0, false, 0};
  storage["mb"] = {"megabyte", 1048576.0, 1.0 / 1048576.0, false, 0};
  storage["megabyte"] = {"megabyte", 1048576.0, 1.0 / 1048576.0, false, 0};
  storage["megabytes"] = {"megabyte", 1048576.0, 1.0 / 1048576.0, false, 0};
  storage["gb"] = {"gigabyte", 1073741824.0, 1.0 / 1073741824.0, false, 0};
  storage["gigabyte"] = {"gigabyte", 1073741824.0, 1.0 / 1073741824.0, false,
                         0};
  storage["gigabytes"] = {"gigabyte", 1073741824.0, 1.0 / 1073741824.0, false,
                          0};
  storage["tb"] = {"terabyte", 1099511627776.0, 1.0 / 1099511627776.0, false,
                   0};
  storage["terabyte"] = {"terabyte", 1099511627776.0, 1.0 / 1099511627776.0,
                         false, 0};
  storage["terabytes"] = {"terabyte", 1099511627776.0, 1.0 / 1099511627776.0,
                          false, 0};
  storage["pb"] = {"petabyte", 1125899906842624.0, 1.0 / 1125899906842624.0,
                   false, 0};
  storage["petabyte"] = {"petabyte", 1125899906842624.0,
                         1.0 / 1125899906842624.0, false, 0};
  storage["petabytes"] = {"petabyte", 1125899906842624.0,
                          1.0 / 1125899906842624.0, false, 0};
  m_units["storage"] = storage;

  // ===== VOLUME (base: liter) =====
  QMap<QString, UnitDef> volume;
  volume["ml"] = {"milliliter", 0.001, 1000, false, 0};
  volume["milliliter"] = {"milliliter", 0.001, 1000, false, 0};
  volume["milliliters"] = {"milliliter", 0.001, 1000, false, 0};
  volume["l"] = {"liter", 1.0, 1.0, false, 0};
  volume["liter"] = {"liter", 1.0, 1.0, false, 0};
  volume["liters"] = {"liter", 1.0, 1.0, false, 0};
  volume["litre"] = {"liter", 1.0, 1.0, false, 0};
  volume["litres"] = {"liter", 1.0, 1.0, false, 0};
  volume["gal"] = {"gallon", 3.78541, 0.264172, false, 0};
  volume["gallon"] = {"gallon", 3.78541, 0.264172, false, 0};
  volume["gallons"] = {"gallon", 3.78541, 0.264172, false, 0};
  volume["cup"] = {"cup", 0.236588, 4.22675, false, 0};
  volume["cups"] = {"cup", 0.236588, 4.22675, false, 0};
  volume["pint"] = {"pint", 0.473176, 2.11338, false, 0};
  volume["pints"] = {"pint", 0.473176, 2.11338, false, 0};
  volume["pt"] = {"pint", 0.473176, 2.11338, false, 0};
  volume["floz"] = {"fluid ounce", 0.0295735, 33.814, false, 0};
  volume["fl oz"] = {"fluid ounce", 0.0295735, 33.814, false, 0};
  m_units["volume"] = volume;

  // ===== AREA (base: square meter) =====
  QMap<QString, UnitDef> area;
  area["sqm"] = {"square meter", 1.0, 1.0, false, 0};
  area["m2"] = {"square meter", 1.0, 1.0, false, 0};
  area["m²"] = {"square meter", 1.0, 1.0, false, 0};
  area["sqkm"] = {"square kilometer", 1000000.0, 0.000001, false, 0};
  area["km2"] = {"square kilometer", 1000000.0, 0.000001, false, 0};
  area["km²"] = {"square kilometer", 1000000.0, 0.000001, false, 0};
  area["sqft"] = {"square foot", 0.092903, 10.7639, false, 0};
  area["ft2"] = {"square foot", 0.092903, 10.7639, false, 0};
  area["ft²"] = {"square foot", 0.092903, 10.7639, false, 0};
  area["acre"] = {"acre", 4046.86, 0.000247105, false, 0};
  area["acres"] = {"acre", 4046.86, 0.000247105, false, 0};
  area["hectare"] = {"hectare", 10000.0, 0.0001, false, 0};
  area["hectares"] = {"hectare", 10000.0, 0.0001, false, 0};
  area["ha"] = {"hectare", 10000.0, 0.0001, false, 0};
  m_units["area"] = area;

  // ===== SPEED (base: m/s) =====
  QMap<QString, UnitDef> speed;
  speed["m/s"] = {"meters per second", 1.0, 1.0, false, 0};
  speed["mps"] = {"meters per second", 1.0, 1.0, false, 0};
  speed["km/h"] = {"kilometers per hour", 0.277778, 3.6, false, 0};
  speed["kmh"] = {"kilometers per hour", 0.277778, 3.6, false, 0};
  speed["kph"] = {"kilometers per hour", 0.277778, 3.6, false, 0};
  speed["mph"] = {"miles per hour", 0.44704, 2.23694, false, 0};
  speed["knot"] = {"knot", 0.514444, 1.94384, false, 0};
  speed["knots"] = {"knot", 0.514444, 1.94384, false, 0};
  speed["mach"] = {"mach", 343.0, 0.00291545, false, 0};
  m_units["speed"] = speed;

  // ===== TIME (base: second) =====
  QMap<QString, UnitDef> time;
  time["ms"] = {"millisecond", 0.001, 1000, false, 0};
  time["millisecond"] = {"millisecond", 0.001, 1000, false, 0};
  time["milliseconds"] = {"millisecond", 0.001, 1000, false, 0};
  time["s"] = {"second", 1.0, 1.0, false, 0};
  time["sec"] = {"second", 1.0, 1.0, false, 0};
  time["second"] = {"second", 1.0, 1.0, false, 0};
  time["seconds"] = {"second", 1.0, 1.0, false, 0};
  time["min"] = {"minute", 60.0, 1.0 / 60.0, false, 0};
  time["minute"] = {"minute", 60.0, 1.0 / 60.0, false, 0};
  time["minutes"] = {"minute", 60.0, 1.0 / 60.0, false, 0};
  time["hour"] = {"hour", 3600.0, 1.0 / 3600.0, false, 0};
  time["hours"] = {"hour", 3600.0, 1.0 / 3600.0, false, 0};
  time["hr"] = {"hour", 3600.0, 1.0 / 3600.0, false, 0};
  time["day"] = {"day", 86400.0, 1.0 / 86400.0, false, 0};
  time["days"] = {"day", 86400.0, 1.0 / 86400.0, false, 0};
  time["week"] = {"week", 604800.0, 1.0 / 604800.0, false, 0};
  time["weeks"] = {"week", 604800.0, 1.0 / 604800.0, false, 0};
  time["month"] = {"month", 2629746.0, 1.0 / 2629746.0, false,
                   0}; // Average month
  time["months"] = {"month", 2629746.0, 1.0 / 2629746.0, false, 0};
  time["year"] = {"year", 31556952.0, 1.0 / 31556952.0, false,
                  0}; // Average year
  time["years"] = {"year", 31556952.0, 1.0 / 31556952.0, false, 0};
  time["yr"] = {"year", 31556952.0, 1.0 / 31556952.0, false, 0};
  m_units["time"] = time;

  // Build alias->category map
  for (auto it = m_units.constBegin(); it != m_units.constEnd(); ++it) {
    QString category = it.key();
    for (auto uit = it.value().constBegin(); uit != it.value().constEnd();
         ++uit) {
      m_aliasToCategory[uit.key().toLower()] = category;
    }
  }
}

bool UnitConverter::isConversion(const QString &line) const {
  ParsedConversion parsed = parse(line);
  return parsed.valid;
}

UnitConverter::ParsedConversion
UnitConverter::parse(const QString &line) const {
  ParsedConversion result = {0, "", "", false};

  // Pattern: <number> <unit> to <unit>
  // Also supports: <number><unit> to <unit>, <number> <unit> in <unit>
  // Unit can include letters, digits (for m2, km2), and special chars (°, ², /)
  // Pattern supports multi-word units like "fl oz"
  static QRegularExpression pattern(
      R"(^\s*(-?[\d.,]+)\s*([a-zA-Z°²/0-9]+(?:\s+[a-zA-Z°²/0-9]+)?)\s+(?:to|in|as)\s+([a-zA-Z°²/0-9]+(?:\s+[a-zA-Z°²/0-9]+)?)\s*$)",
      QRegularExpression::CaseInsensitiveOption);

  QRegularExpressionMatch match = pattern.match(line.trimmed());
  if (!match.hasMatch()) {
    return result;
  }

  QString valueStr = match.captured(1);
  valueStr.replace(',', '.');
  bool ok;
  double value = valueStr.toDouble(&ok);
  if (!ok) {
    return result;
  }

  QString fromUnit = match.captured(2).toLower();
  QString toUnit = match.captured(3).toLower();

  // Check if both units exist
  if (!m_aliasToCategory.contains(fromUnit) ||
      !m_aliasToCategory.contains(toUnit)) {
    return result;
  }

  // Check if both units are in the same category
  if (m_aliasToCategory[fromUnit] != m_aliasToCategory[toUnit]) {
    return result;
  }

  result.value = value;
  result.fromUnit = fromUnit;
  result.toUnit = toUnit;
  result.valid = true;
  return result;
}

QString UnitConverter::convert(const QString &line) const {
  ParsedConversion parsed = parse(line);
  if (!parsed.valid) {
    return QString();
  }

  QString category = m_aliasToCategory[parsed.fromUnit];
  const QMap<QString, UnitDef> &units = m_units[category];
  const UnitDef &fromDef = units[parsed.fromUnit];
  const UnitDef &toDef = units[parsed.toUnit];

  double result;

  // Special handling for temperature
  if (category == "temperature") {
    double kelvin;
    // Convert to Kelvin first
    if (fromDef.canonical == "celsius") {
      kelvin = celsiusToKelvin(parsed.value);
    } else if (fromDef.canonical == "fahrenheit") {
      kelvin = fahrenheitToKelvin(parsed.value);
    } else {
      kelvin = parsed.value; // Already Kelvin
    }

    // Convert from Kelvin to target
    if (toDef.canonical == "celsius") {
      result = kelvinToCelsius(kelvin);
    } else if (toDef.canonical == "fahrenheit") {
      result = kelvinToFahrenheit(kelvin);
    } else {
      result = kelvin;
    }
  } else {
    // Standard conversion: value -> base unit -> target unit
    double baseValue = parsed.value * fromDef.toBase;
    result = baseValue * toDef.fromBase;
  }

  // Format result nicely
  QString resultStr;
  if (qAbs(result) >= 1000000 || (qAbs(result) < 0.001 && result != 0)) {
    resultStr = QString::number(result, 'g', 6);
  } else if (result == qFloor(result)) {
    resultStr = QString::number(static_cast<long long>(result));
  } else {
    resultStr = QString::number(result, 'f', 4)
                    .remove(QRegularExpression("0+$"))
                    .remove(QRegularExpression("\\.$"));
  }

  // Add target unit to result (e.g., "1000 meter")
  return resultStr + " " + toDef.canonical;
}

double UnitConverter::celsiusToKelvin(double c) const { return c + 273.15; }

double UnitConverter::kelvinToCelsius(double k) const { return k - 273.15; }

double UnitConverter::fahrenheitToKelvin(double f) const {
  return (f - 32.0) * 5.0 / 9.0 + 273.15;
}

double UnitConverter::kelvinToFahrenheit(double k) const {
  return (k - 273.15) * 9.0 / 5.0 + 32.0;
}

QStringList UnitConverter::categories() const { return m_units.keys(); }

QStringList UnitConverter::unitsInCategory(const QString &category) const {
  // Case-insensitive category lookup
  QString lowerCategory = category.toLower();
  if (!m_units.contains(lowerCategory)) {
    return QStringList();
  }
  return m_units[lowerCategory].keys();
}
