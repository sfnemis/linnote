#include "OcrHelper.h"
#include "../integration/DesktopHelper.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>
#include <QUuid>

OcrHelper::OcrHelper(QObject *parent) : QObject(parent) {}

bool OcrHelper::isAvailable() {
  QProcess process;
  process.start("tesseract", {"--version"});
  process.waitForFinished(3000);
  return process.exitCode() == 0;
}

bool OcrHelper::isLanguageAvailable(const QString &langCode) {
  return getAvailableLanguages().contains(langCode);
}

QStringList OcrHelper::getAvailableLanguages() {
  QStringList languages;
  QProcess process;
  process.start("tesseract", {"--list-langs"});
  process.waitForFinished(3000);

  if (process.exitCode() == 0) {
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (int i = 1; i < lines.size(); ++i) {
      QString lang = lines[i].trimmed();
      if (!lang.isEmpty()) {
        languages.append(lang);
      }
    }
  }

  if (languages.isEmpty()) {
    languages << "eng" << "tur";
  }
  return languages;
}

QString OcrHelper::extractText(const QImage &image, const QString &language) {
  if (image.isNull()) {
    emit errorOccurred(tr("No image provided"));
    return QString();
  }

  if (!isAvailable()) {
    emit errorOccurred(tr("Tesseract not installed"));
    return QString();
  }

  QString inputPath = QDir::tempPath() + "/linnote_ocr_input.png";
  if (!image.save(inputPath, "PNG")) {
    emit errorOccurred(tr("Failed to save image"));
    return QString();
  }

  QString result = runTesseract(inputPath, language);
  QFile::remove(inputPath);
  return result;
}

void OcrHelper::captureAndExtract(const QString &language) {
  m_language = language;

  // ═══════════════════════════════════════════════════════════════════════════
  // Desktop-specific screenshot capture:
  // - KDE: Use spectacle directly (immediate area selection)
  // - GNOME/Others: Use XDG Portal (native integration)
  // ═══════════════════════════════════════════════════════════════════════════

  if (LinNote::detectDesktopEnvironment() == LinNote::DesktopEnvironment::KDE) {
    captureViaSpectacle();
  } else {
    captureViaPortal();
  }
}

void OcrHelper::captureViaSpectacle() {
  qDebug() << "OCR: Using spectacle for KDE...";

  QString tempPath = QDir::tempPath() + "/linnote_screenshot.png";
  QFile::remove(tempPath);

  // spectacle -r = region, -b = no decorations, -n = no notification, -o =
  // output
  QProcess process;
  process.start("spectacle", {"-r", "-b", "-n", "-o", tempPath});
  process.waitForFinished(60000); // 60 second timeout for user selection

  if (process.exitCode() == 0 && QFile::exists(tempPath) &&
      QFileInfo(tempPath).size() > 100) {
    processScreenshot(tempPath);
  } else {
    emit errorOccurred(tr("Screenshot cancelled or failed"));
  }
}

void OcrHelper::captureViaPortal() {
  qDebug() << "OCR: Using XDG Portal for GNOME...";

  QDBusInterface portalInterface(
      "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
      "org.freedesktop.portal.Screenshot", QDBusConnection::sessionBus());

  if (!portalInterface.isValid()) {
    emit errorOccurred(tr("Screenshot portal not available"));
    return;
  }

  QString uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
  m_requestToken = QStringLiteral("linnote_ocr_") + uniqueId;

  QString senderName = QDBusConnection::sessionBus().baseService();
  QString senderPath = senderName;
  senderPath.remove(0, 1);
  senderPath.replace('.', '_');

  QString responsePath =
      QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
          .arg(senderPath, m_requestToken);

  QDBusConnection::sessionBus().connect(
      "org.freedesktop.portal.Desktop", responsePath,
      "org.freedesktop.portal.Request", "Response", this,
      SLOT(onPortalResponse(uint, QVariantMap)));

  QVariantMap options;
  options["handle_token"] = m_requestToken;
  options["interactive"] = true;

  QDBusMessage reply = portalInterface.call("Screenshot", QString(), options);

  if (reply.type() == QDBusMessage::ErrorMessage) {
    emit errorOccurred(tr("Screenshot request failed"));
  }
}

void OcrHelper::onPortalResponse(uint response, const QVariantMap &results) {
  if (response != 0) {
    emit errorOccurred(tr("Screenshot was cancelled"));
    return;
  }

  QString uri = results.value("uri").toString();
  if (uri.isEmpty()) {
    emit errorOccurred(tr("No screenshot received"));
    return;
  }

  QUrl url(uri);
  QString imagePath = url.toLocalFile();

  if (imagePath.isEmpty() || !QFile::exists(imagePath)) {
    emit errorOccurred(tr("Screenshot file not found"));
    return;
  }

  processScreenshot(imagePath);
}

void OcrHelper::processScreenshot(const QString &imagePath) {
  qDebug() << "OCR: Processing" << imagePath;

  QImage image(imagePath);
  if (image.isNull()) {
    emit errorOccurred(tr("Failed to load screenshot"));
    return;
  }

  QString text = extractText(image, m_language);

  if (!text.isEmpty()) {
    emit textExtracted(text);
  } else {
    emit errorOccurred(tr("No text found in image"));
  }

  QFile::remove(imagePath);
}

QString OcrHelper::runTesseract(const QString &imagePath,
                                const QString &language) {
  QProcess process;
  process.start("tesseract", {imagePath, "stdout", "-l", language});

  if (!process.waitForFinished(30000)) {
    emit errorOccurred(tr("OCR timed out"));
    return QString();
  }

  if (process.exitCode() != 0) {
    QString error = QString::fromUtf8(process.readAllStandardError());
    emit errorOccurred(tr("OCR failed: ") + error);
    return QString();
  }

  return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}
