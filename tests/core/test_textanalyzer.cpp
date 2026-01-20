#include "core/TextAnalyzer.h"
#include <QTest>

class TestTextAnalyzer : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Number extraction tests
  void testExtractNumbers();
  void testExtractNumbers_data();

  // Sum tests
  void testSum();
  void testSumCurrency();

  // Average tests
  void testAvg();

  // Min/Max tests
  void testMinMax();

  // Text statistics
  void testAnalyze();

  // Formatting tests
  void testFormatSum();
  void testFormatAvg();
  void testFormatCount();

  // Edge cases
  void testEmptyText();
  void testNoNumbers();
  void testNegativeNumbers();

private:
  TextAnalyzer *m_analyzer;
};

void TestTextAnalyzer::initTestCase() { m_analyzer = new TextAnalyzer(); }

void TestTextAnalyzer::cleanupTestCase() { delete m_analyzer; }

// ============ Number Extraction ============

void TestTextAnalyzer::testExtractNumbers_data() {
  QTest::addColumn<QString>("text");
  QTest::addColumn<int>("expectedCount");

  QTest::newRow("simple numbers") << "1 2 3" << 3;
  QTest::newRow("with text") << "I have 5 apples and 3 oranges" << 2;
  QTest::newRow("decimals") << "3.14 and 2.71" << 2;
  QTest::newRow("currency $") << "$25 and $100" << 2;
  QTest::newRow("currency €") << "€50" << 1;
  QTest::newRow("with thousands") << "1,000 and 2,500" << 2;
  QTest::newRow("negative") << "-5 and -10" << 2;
}

void TestTextAnalyzer::testExtractNumbers() {
  QFETCH(QString, text);
  QFETCH(int, expectedCount);

  QList<double> numbers = m_analyzer->extractNumbers(text);
  QCOMPARE(numbers.size(), expectedCount);
}

// ============ Sum Tests ============

void TestTextAnalyzer::testSum() {
  double result = m_analyzer->sum("1 2 3 4 5");
  QCOMPARE(result, 15.0);

  result = m_analyzer->sum("10.5 + 4.5");
  QCOMPARE(result, 15.0);
}

void TestTextAnalyzer::testSumCurrency() {
  double result = m_analyzer->sum("$25 + $50 + $25");
  QCOMPARE(result, 100.0);
}

// ============ Average Tests ============

void TestTextAnalyzer::testAvg() {
  double result = m_analyzer->avg("10 20 30");
  QCOMPARE(result, 20.0);

  result = m_analyzer->avg("1 2 3 4 5");
  QCOMPARE(result, 3.0);
}

// ============ Min/Max Tests ============

void TestTextAnalyzer::testMinMax() {
  double minVal = m_analyzer->min("5 1 10 3");
  QCOMPARE(minVal, 1.0);

  double maxVal = m_analyzer->max("5 1 10 3");
  QCOMPARE(maxVal, 10.0);
}

// ============ Text Statistics ============

void TestTextAnalyzer::testAnalyze() {
  QString text = "Hello world. This is a test.";
  TextAnalyzer::TextStats stats = m_analyzer->analyze(text);

  QVERIFY(stats.words >= 6);
  QVERIFY(stats.sentences >= 2);
  QVERIFY(stats.characters > 0);
}

// ============ Format Tests ============

void TestTextAnalyzer::testFormatSum() {
  QString result = m_analyzer->formatSum("10 20 30");
  QVERIFY(!result.isEmpty());
  QVERIFY(result.contains("60"));
}

void TestTextAnalyzer::testFormatAvg() {
  QString result = m_analyzer->formatAvg("10 20 30");
  QVERIFY(!result.isEmpty());
  QVERIFY(result.contains("20"));
}

void TestTextAnalyzer::testFormatCount() {
  QString result = m_analyzer->formatCount("apple\nbanana\norange");
  QVERIFY(!result.isEmpty());
}

// ============ Edge Cases ============

void TestTextAnalyzer::testEmptyText() {
  double sum = m_analyzer->sum("");
  QCOMPARE(sum, 0.0);

  double avg = m_analyzer->avg("");
  QVERIFY(qIsNaN(avg) || avg == 0.0); // Division by zero handling
}

void TestTextAnalyzer::testNoNumbers() {
  double sum = m_analyzer->sum("no numbers here");
  QCOMPARE(sum, 0.0);
}

void TestTextAnalyzer::testNegativeNumbers() {
  double sum = m_analyzer->sum("-5 + 10 - 3");
  QCOMPARE(sum, 2.0);
}

QTEST_MAIN(TestTextAnalyzer)
#include "test_textanalyzer.moc"
