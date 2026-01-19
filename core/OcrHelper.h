#ifndef LINNOTE_OCRHELPER_H
#define LINNOTE_OCRHELPER_H

#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief Helper class for OCR (Optical Character Recognition)
 *
 * Uses Tesseract OCR engine to extract text from images.
 * Supports screen area selection and clipboard images.
 */
class OcrHelper : public QObject {
  Q_OBJECT

public:
  explicit OcrHelper(QObject *parent = nullptr);

  // Check if Tesseract is installed
  static bool isAvailable();

  // Check if specific language is installed
  static bool isLanguageAvailable(const QString &langCode);

  // Get list of available languages
  static QStringList getAvailableLanguages();

  // Extract text from image
  QString extractText(const QImage &image, const QString &language = "eng");

  // Capture screen area and extract text
  void captureAndExtract(const QString &language = "eng");

signals:
  void textExtracted(const QString &text);
  void errorOccurred(const QString &error);

private:
  QString runTesseract(const QString &imagePath, const QString &language);
};

#endif // LINNOTE_OCRHELPER_H
