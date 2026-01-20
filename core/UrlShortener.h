#ifndef LINNOTE_URLSHORTENER_H
#define LINNOTE_URLSHORTENER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

/**
 * @brief URL shortening service wrapper
 * Supports: is.gd, TinyURL, v.gd
 */
class UrlShortener : public QObject {
  Q_OBJECT

public:
  enum Service { IsGd, TinyUrl, Vgd };

  explicit UrlShortener(QObject *parent = nullptr);

  void shortenUrl(const QString &url, Service service = IsGd);

  static QString serviceToString(Service service);
  static Service stringToService(const QString &name);

signals:
  void shortened(const QString &shortUrl);
  void error(const QString &message);

private slots:
  void onReplyFinished();

private:
  QNetworkAccessManager *m_manager;
  QString m_originalUrl;
};

#endif // LINNOTE_URLSHORTENER_H
