#include "core/UnitConverter.h"
#include <QTest>

class TestUnitConverter : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Length conversions
  void testKilometersToMiles();
  void testMilesToKilometers();
  void testInchesToCentimeters();
  void testFeetToMeters();

  // Weight conversions
  void testKilogramsToPounds();
  void testPoundsToKilograms();
  void testOuncesToGrams();

  // Temperature conversions
  void testCelsiusToFahrenheit();
  void testFahrenheitToCelsius();
  void testCelsiusToKelvin();
  void testKelvinToCelsius();

  // Volume conversions
  void testLitersToGallons();
  void testGallonsToLiters();
  void testMillilitersToOunces();

  // Data conversions
  void testGigabytesToMegabytes();
  void testKilobytesToMegabytes();

  // Speed conversions
  void testMphToKmh();
  void testKmhToMph();

  // Time conversions
  void testHoursToMinutes();
  void testSecondsToHours();

  // Parsing tests
  void testIsConversion();
  void testIsConversion_data();

  // Edge cases
  void testZeroValue();
  void testNegativeValue();
  void testVeryLargeValue();
  void testInvalidUnit();
  void testSameUnit();

  // Categories
  void testCategories();
  void testUnitsInCategory();

private:
  UnitConverter *m_converter;
};

void TestUnitConverter::initTestCase() {
  m_converter = UnitConverter::instance();
}

void TestUnitConverter::cleanupTestCase() {
  // Singleton, don't delete
}

// ============ Length Conversions ============

void TestUnitConverter::testKilometersToMiles() {
  QString result = m_converter->convert("10 km to miles");
  QVERIFY(result.contains("6.21") || result.contains("6.2"));
}

void TestUnitConverter::testMilesToKilometers() {
  QString result = m_converter->convert("10 miles to km");
  QVERIFY(result.contains("16.09") || result.contains("16.1"));
}

void TestUnitConverter::testInchesToCentimeters() {
  QString result = m_converter->convert("1 inch to cm");
  QVERIFY(result.contains("2.54"));
}

void TestUnitConverter::testFeetToMeters() {
  QString result = m_converter->convert("10 feet to meters");
  QVERIFY(result.contains("3.04") || result.contains("3.05"));
}

// ============ Weight Conversions ============

void TestUnitConverter::testKilogramsToPounds() {
  QString result = m_converter->convert("1 kg to lb");
  QVERIFY(result.contains("2.2"));
}

void TestUnitConverter::testPoundsToKilograms() {
  QString result = m_converter->convert("10 lb to kg");
  QVERIFY(result.contains("4.53") || result.contains("4.5"));
}

void TestUnitConverter::testOuncesToGrams() {
  QString result = m_converter->convert("16 oz to g");
  QVERIFY(result.contains("453") || result.contains("454"));
}

// ============ Temperature Conversions ============

void TestUnitConverter::testCelsiusToFahrenheit() {
  QString result = m_converter->convert("0 C to F");
  QVERIFY(result.contains("32"));

  result = m_converter->convert("100 C to F");
  QVERIFY(result.contains("212"));
}

void TestUnitConverter::testFahrenheitToCelsius() {
  QString result = m_converter->convert("32 F to C");
  QVERIFY(result.contains("0"));

  result = m_converter->convert("212 F to C");
  QVERIFY(result.contains("100"));
}

void TestUnitConverter::testCelsiusToKelvin() {
  QString result = m_converter->convert("0 C to K");
  QVERIFY(result.contains("273"));
}

void TestUnitConverter::testKelvinToCelsius() {
  QString result = m_converter->convert("273 K to C");
  // Should be close to 0
  QVERIFY(result.contains("0") || result.contains("-0"));
}

// ============ Volume Conversions ============

void TestUnitConverter::testLitersToGallons() {
  QString result = m_converter->convert("1 liter to gallons");
  QVERIFY(result.contains("0.26") || result.contains("0.264"));
}

void TestUnitConverter::testGallonsToLiters() {
  QString result = m_converter->convert("1 gallon to liters");
  QVERIFY(result.contains("3.78") || result.contains("3.79"));
}

void TestUnitConverter::testMillilitersToOunces() {
  QString result = m_converter->convert("500 ml to fl oz");
  QVERIFY(!result.isEmpty());
}

// ============ Data Conversions ============

void TestUnitConverter::testGigabytesToMegabytes() {
  QString result = m_converter->convert("1 GB to MB");
  QVERIFY(result.contains("1024") || result.contains("1000"));
}

void TestUnitConverter::testKilobytesToMegabytes() {
  QString result = m_converter->convert("1024 KB to MB");
  QVERIFY(result.contains("1"));
}

// ============ Speed Conversions ============

void TestUnitConverter::testMphToKmh() {
  QString result = m_converter->convert("60 mph to kmh");
  QVERIFY(result.contains("96") || result.contains("97"));
}

void TestUnitConverter::testKmhToMph() {
  QString result = m_converter->convert("100 kmh to mph");
  QVERIFY(result.contains("62"));
}

// ============ Time Conversions ============

void TestUnitConverter::testHoursToMinutes() {
  QString result = m_converter->convert("2 hours to minutes");
  QVERIFY(result.contains("120"));
}

void TestUnitConverter::testSecondsToHours() {
  QString result = m_converter->convert("3600 seconds to hours");
  QVERIFY(result.contains("1"));
}

// ============ Parsing Tests ============

void TestUnitConverter::testIsConversion_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("expected");

  QTest::newRow("km to miles") << "10 km to miles" << true;
  QTest::newRow("with in") << "5 feet in meters" << true;
  QTest::newRow("plain text") << "hello world" << false;
  QTest::newRow("math expression") << "2+2" << false;
  QTest::newRow("single number") << "100" << false;
}

void TestUnitConverter::testIsConversion() {
  QFETCH(QString, input);
  QFETCH(bool, expected);

  QCOMPARE(m_converter->isConversion(input), expected);
}

// ============ Edge Cases ============

void TestUnitConverter::testZeroValue() {
  QString result = m_converter->convert("0 km to miles");
  QVERIFY(result.contains("0"));
}

void TestUnitConverter::testNegativeValue() {
  QString result = m_converter->convert("-10 C to F");
  QVERIFY(result.contains("14")); // -10C = 14F
}

void TestUnitConverter::testVeryLargeValue() {
  QString result = m_converter->convert("1000000 km to miles");
  QVERIFY(!result.isEmpty());
}

void TestUnitConverter::testInvalidUnit() {
  QString result = m_converter->convert("10 xyz to abc");
  // Should either be empty or contain error
  QVERIFY(result.isEmpty() || result.contains("Error") ||
          result.contains("Unknown"));
}

void TestUnitConverter::testSameUnit() {
  QString result = m_converter->convert("10 km to km");
  QVERIFY(result.contains("10"));
}

// ============ Categories ============

void TestUnitConverter::testCategories() {
  QStringList categories = m_converter->categories();
  QVERIFY(!categories.isEmpty());
  QVERIFY(categories.contains("Length") || categories.contains("length"));
}

void TestUnitConverter::testUnitsInCategory() {
  QStringList lengthUnits = m_converter->unitsInCategory("Length");
  QVERIFY(!lengthUnits.isEmpty());
}

QTEST_MAIN(TestUnitConverter)
#include "test_unitconv.moc"
