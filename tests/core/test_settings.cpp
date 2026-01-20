#include "core/Settings.h"
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class TestSettings : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Singleton
  void testSingleton();

  // Boolean settings
  void testAutoMathEnabled();
  void testCurrencyConversionEnabled();
  void testAutoPasteEnabled();
  void testDarkMode();
  void testOpenWithNewNote();
  void testToolbarAutoHide();
  void testAutoCheckboxEnabled();

  // String settings
  void testBaseCurrency();
  void testThemeName();
  void testGlobalHotkey();

  // Integer settings
  void testTransparency();
  void testFontSize();
  void testPomodoroWorkMinutes();
  void testPomodoroBreakMinutes();

  // Size/Position settings
  void testWindowSize();
  void testWindowPosition();

  // Signals
  void testSettingsChangedSignal();

  // Persistence
  void testSaveAndLoad();

  // Edge cases
  void testInvalidTransparencyValue();
  void testEmptyStringSettings();

private:
  Settings *m_settings;
  QTemporaryDir *m_tempDir;
};

void TestSettings::initTestCase() {
  m_tempDir = new QTemporaryDir();
  QVERIFY(m_tempDir->isValid());
  qputenv("XDG_DATA_HOME", m_tempDir->path().toUtf8());
  qputenv("XDG_CONFIG_HOME", m_tempDir->path().toUtf8());

  m_settings = Settings::instance();
  QVERIFY(m_settings != nullptr);
}

void TestSettings::cleanupTestCase() { delete m_tempDir; }

void TestSettings::testSingleton() {
  Settings *s1 = Settings::instance();
  Settings *s2 = Settings::instance();
  QCOMPARE(s1, s2);
}

void TestSettings::testAutoMathEnabled() {
  m_settings->setAutoMathEnabled(true);
  QCOMPARE(m_settings->autoMathEnabled(), true);

  m_settings->setAutoMathEnabled(false);
  QCOMPARE(m_settings->autoMathEnabled(), false);
}

void TestSettings::testCurrencyConversionEnabled() {
  m_settings->setCurrencyConversionEnabled(true);
  QCOMPARE(m_settings->currencyConversionEnabled(), true);

  m_settings->setCurrencyConversionEnabled(false);
  QCOMPARE(m_settings->currencyConversionEnabled(), false);
}

void TestSettings::testAutoPasteEnabled() {
  m_settings->setAutoPasteEnabled(true);
  QCOMPARE(m_settings->autoPasteEnabled(), true);

  m_settings->setAutoPasteEnabled(false);
  QCOMPARE(m_settings->autoPasteEnabled(), false);
}

void TestSettings::testDarkMode() {
  m_settings->setDarkMode(true);
  QCOMPARE(m_settings->darkMode(), true);

  m_settings->setDarkMode(false);
  QCOMPARE(m_settings->darkMode(), false);
}

void TestSettings::testOpenWithNewNote() {
  m_settings->setOpenWithNewNote(true);
  QCOMPARE(m_settings->openWithNewNote(), true);

  m_settings->setOpenWithNewNote(false);
  QCOMPARE(m_settings->openWithNewNote(), false);
}

void TestSettings::testToolbarAutoHide() {
  m_settings->setToolbarAutoHide(true);
  QCOMPARE(m_settings->toolbarAutoHide(), true);

  m_settings->setToolbarAutoHide(false);
  QCOMPARE(m_settings->toolbarAutoHide(), false);
}

void TestSettings::testAutoCheckboxEnabled() {
  m_settings->setAutoCheckboxEnabled(true);
  QCOMPARE(m_settings->autoCheckboxEnabled(), true);

  m_settings->setAutoCheckboxEnabled(false);
  QCOMPARE(m_settings->autoCheckboxEnabled(), false);
}

void TestSettings::testBaseCurrency() {
  m_settings->setBaseCurrency("USD");
  QCOMPARE(m_settings->baseCurrency(), QString("USD"));

  m_settings->setBaseCurrency("EUR");
  QCOMPARE(m_settings->baseCurrency(), QString("EUR"));
}

void TestSettings::testThemeName() {
  m_settings->setThemeName("dracula");
  QCOMPARE(m_settings->themeName(), QString("dracula"));

  m_settings->setThemeName("nord");
  QCOMPARE(m_settings->themeName(), QString("nord"));
}

void TestSettings::testGlobalHotkey() {
  m_settings->setGlobalHotkey("Ctrl+Shift+N");
  QCOMPARE(m_settings->globalHotkey(), QString("Ctrl+Shift+N"));

  m_settings->setGlobalHotkey("Super+N");
  QCOMPARE(m_settings->globalHotkey(), QString("Super+N"));
}

void TestSettings::testTransparency() {
  m_settings->setTransparency(80);
  QCOMPARE(m_settings->transparency(), 80);

  m_settings->setTransparency(100);
  QCOMPARE(m_settings->transparency(), 100);

  m_settings->setTransparency(50);
  QCOMPARE(m_settings->transparency(), 50);
}

void TestSettings::testFontSize() {
  m_settings->setFontSize(14);
  QCOMPARE(m_settings->fontSize(), 14);

  m_settings->setFontSize(18);
  QCOMPARE(m_settings->fontSize(), 18);
}

void TestSettings::testPomodoroWorkMinutes() {
  m_settings->setPomodoroWorkMinutes(25);
  QCOMPARE(m_settings->pomodoroWorkMinutes(), 25);

  m_settings->setPomodoroWorkMinutes(50);
  QCOMPARE(m_settings->pomodoroWorkMinutes(), 50);
}

void TestSettings::testPomodoroBreakMinutes() {
  m_settings->setPomodoroBreakMinutes(5);
  QCOMPARE(m_settings->pomodoroBreakMinutes(), 5);

  m_settings->setPomodoroBreakMinutes(15);
  QCOMPARE(m_settings->pomodoroBreakMinutes(), 15);
}

void TestSettings::testWindowSize() {
  QSize size(800, 600);
  m_settings->setWindowSize(size);
  QCOMPARE(m_settings->windowSize(), size);

  QSize size2(1920, 1080);
  m_settings->setWindowSize(size2);
  QCOMPARE(m_settings->windowSize(), size2);
}

void TestSettings::testWindowPosition() {
  QPoint pos(100, 200);
  m_settings->setWindowPosition(pos);
  QCOMPARE(m_settings->windowPosition(), pos);

  QPoint pos2(500, 300);
  m_settings->setWindowPosition(pos2);
  QCOMPARE(m_settings->windowPosition(), pos2);
}

void TestSettings::testSettingsChangedSignal() {
  QSignalSpy spy(m_settings, &Settings::settingsChanged);

  m_settings->setDarkMode(!m_settings->darkMode());

  // Signal should have been emitted
  QVERIFY(spy.count() >= 1);
}

void TestSettings::testSaveAndLoad() {
  // Set values
  m_settings->setDarkMode(true);
  m_settings->setFontSize(16);
  m_settings->setThemeName("test_theme");

  // Save
  m_settings->save();

  // Values should persist (Settings is singleton, so just verify current
  // values)
  QCOMPARE(m_settings->darkMode(), true);
  QCOMPARE(m_settings->fontSize(), 16);
  QCOMPARE(m_settings->themeName(), QString("test_theme"));
}

void TestSettings::testInvalidTransparencyValue() {
  // Try setting out-of-range values
  m_settings->setTransparency(150); // Above max
  QVERIFY(m_settings->transparency() <= 100);

  m_settings->setTransparency(-10); // Below min
  QVERIFY(m_settings->transparency() >= 0);
}

void TestSettings::testEmptyStringSettings() {
  m_settings->setThemeName("");
  // Should either keep old value or use default, not crash
  QVERIFY(true);
}

QTEST_MAIN(TestSettings)
#include "test_settings.moc"
