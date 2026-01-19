#include "OcrHelper.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QScreen>
#include <QTemporaryFile>

OcrHelper::OcrHelper(QObject *parent) : QObject(parent) {}

bool OcrHelper::isAvailable() {
  QProcess process;
  process.start("tesseract", {"--version"});
  process.waitForFinished(3000);
  return process.exitCode() == 0;
}

bool OcrHelper::isLanguageAvailable(const QString &langCode) {
  QStringList available = getAvailableLanguages();
  return available.contains(langCode);
}

QStringList OcrHelper::getAvailableLanguages() {
  QStringList languages;

  QProcess process;
  process.start("tesseract", {"--list-langs"});
  process.waitForFinished(3000);

  if (process.exitCode() == 0) {
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    // Skip first line (header)
    for (int i = 1; i < lines.size(); ++i) {
      QString lang = lines[i].trimmed();
      if (!lang.isEmpty()) {
        languages.append(lang);
      }
    }
  }

  // Fallback if detection fails
  if (languages.isEmpty()) {
    languages << "eng" << "tur" << "deu" << "fra" << "spa" << "ita" << "por"
              << "rus" << "chi_sim" << "jpn" << "kor" << "ara";
  }

  return languages;
}

QString OcrHelper::extractText(const QImage &image, const QString &language) {
  if (image.isNull()) {
    emit errorOccurred(tr("No image provided"));
    return QString();
  }

  if (!isAvailable()) {
    emit errorOccurred(
        tr("Tesseract is not installed.\n\nInstall with:\nsudo apt install "
           "tesseract-ocr tesseract-ocr-") +
        language);
    return QString();
  }

  // Save image to temp file
  QTemporaryFile tempFile(QDir::tempPath() + "/linnote_ocr_XXXXXX.png");
  if (!tempFile.open()) {
    emit errorOccurred(tr("Failed to create temporary file"));
    return QString();
  }

  image.save(&tempFile, "PNG");
  tempFile.close();

  QString result = runTesseract(tempFile.fileName(), language);

  return result;
}

void OcrHelper::captureAndExtract(const QString &language) {
  // Use KDE screenshot tool for area selection
  QProcess process;
  QString tempPath = QDir::tempPath() + "/linnote_screenshot.png";

  // Try spectacle (KDE) first
  process.start("spectacle", {"-a", "-f", tempPath});
  process.waitForFinished(30000); // 30s timeout for user selection

  if (process.exitCode() != 0) {
    // Try KDE Spectacle
    process.start("spectacle", {"-r", "-b", "-n", "-o", tempPath});
    process.waitForFinished(30000);
  }

  if (process.exitCode() != 0) {
    // Try generic scrot
    process.start("scrot", {"-s", tempPath});
    process.waitForFinished(30000);
  }

  if (process.exitCode() != 0) {
    emit errorOccurred(
        tr("Screenshot failed. Please install spectacle or spectacle."));
    return;
  }

  // Load captured image
  QImage image(tempPath);
  if (image.isNull()) {
    emit errorOccurred(tr("Failed to load screenshot"));
    return;
  }

  QString text = extractText(image, language);
  if (!text.isEmpty()) {
    emit textExtracted(text);
  }

  // Clean up
  QFile::remove(tempPath);
}

QString OcrHelper::runTesseract(const QString &imagePath,
                                const QString &language) {
  QProcess process;
  process.start("tesseract", {imagePath, "stdout", "-l", language});
  process.waitForFinished(30000);

  if (process.exitCode() != 0) {
    QString error = QString::fromUtf8(process.readAllStandardError());
    emit errorOccurred(tr("OCR failed: ") + error);
    return QString();
  }

  QString result = QString::fromUtf8(process.readAllStandardOutput());
  return result.trimmed();
}
