#ifndef LINNOTE_OCRHELPER_H
#define LINNOTE_OCRHELPER_H

#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief Helper class for OCR (Optical Character Recognition)
 *
 * Uses desktop-specific screenshot capture:
 * - KDE: spectacle (direct area selection)
 * - GNOME: XDG Portal (native integration)
 */
class OcrHelper : public QObject {
  Q_OBJECT

public:
  explicit OcrHelper(QObject *parent = nullptr);

  static bool isAvailable();
  static bool isLanguageAvailable(const QString &langCode);
  static QStringList getAvailableLanguages();

  QString extractText(const QImage &image, const QString &language = "eng");
  void captureAndExtract(const QString &language = "eng");

signals:
  void textExtracted(const QString &text);
  void errorOccurred(const QString &error);

private slots:
  void onPortalResponse(uint response, const QVariantMap &results);

private:
  void captureViaSpectacle();
  void captureViaPortal();
  void processScreenshot(const QString &imagePath);
  QString runTesseract(const QString &imagePath, const QString &language);

  QString m_language;
  QString m_requestToken;
};

#endif // LINNOTE_OCRHELPER_H
