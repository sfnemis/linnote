#include "UrlShortener.h"
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

UrlShortener::UrlShortener(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)) {}

void UrlShortener::shortenUrl(const QString &url, Service service) {
  m_originalUrl = url;

  QString apiUrl;
  switch (service) {
  case IsGd:
    apiUrl = QString("https://is.gd/create.php?format=simple&url=%1")
                 .arg(QUrl::toPercentEncoding(url).constData());
    break;
  case TinyUrl:
    apiUrl = QString("https://tinyurl.com/api-create.php?url=%1")
                 .arg(QUrl::toPercentEncoding(url).constData());
    break;
  case Vgd:
    apiUrl = QString("https://v.gd/create.php?format=simple&url=%1")
                 .arg(QUrl::toPercentEncoding(url).constData());
    break;
  }

  QNetworkRequest request;
  request.setUrl(QUrl(apiUrl));
  QNetworkReply *reply = m_manager->get(request);
  connect(reply, &QNetworkReply::finished, this,
          &UrlShortener::onReplyFinished);
}

void UrlShortener::onReplyFinished() {
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  if (!reply) {
    return;
  }

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    emit error(tr("Network error: %1").arg(reply->errorString()));
    return;
  }

  QString shortUrl = QString::fromUtf8(reply->readAll()).trimmed();

  if (shortUrl.isEmpty() || !shortUrl.startsWith("http")) {
    emit error(tr("Invalid response from shortening service"));
    return;
  }

  emit shortened(shortUrl);
}

QString UrlShortener::serviceToString(Service service) {
  switch (service) {
  case IsGd:
    return "is.gd";
  case TinyUrl:
    return "TinyURL";
  case Vgd:
    return "v.gd";
  }
  return "is.gd";
}

UrlShortener::Service UrlShortener::stringToService(const QString &name) {
  if (name == "TinyURL")
    return TinyUrl;
  if (name == "v.gd")
    return Vgd;
  return IsGd;
}
