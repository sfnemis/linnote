#include "CurrencyConverter.h"
#include "Settings.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>

CurrencyConverter *CurrencyConverter::instance() {
  static CurrencyConverter instance;
  return &instance;
}

CurrencyConverter::CurrencyConverter(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)),
      m_baseCurrency("USD"), m_provider("frankfurter"), m_ratesLoaded(false) {
  // Frankfurter.dev is free and doesn't require API key
  m_apiKey = "";

  loadCachedRates();

  // If no cached rates, use fallback hardcoded rates
  if (m_rates.isEmpty()) {
    // Fiat currencies (approximate rates to USD)
    m_rates["USD"] = 1.0;
    m_rates["EUR"] = 0.92;
    m_rates["TRY"] = 35.5;
    m_rates["GBP"] = 0.79;
    m_rates["JPY"] = 157.0;
    m_rates["CNY"] = 7.3;
    m_rates["RUB"] = 95.0;
    m_rates["AUD"] = 1.57;
    m_rates["CAD"] = 1.44;
    m_rates["CHF"] = 0.88;
    m_rates["INR"] = 84.0;
    m_rates["KRW"] = 1450.0;
    m_rates["BRL"] = 6.2;
    m_rates["MXN"] = 20.5;
    m_rates["PLN"] = 4.0;
    m_rates["SEK"] = 11.0;
    m_rates["NOK"] = 11.2;
    m_rates["DKK"] = 7.0;
    m_rates["SGD"] = 1.36;
    m_rates["HKD"] = 7.8;
    m_rates["NZD"] = 1.78;
    m_rates["ZAR"] = 18.5;
    m_rates["THB"] = 35.0;
    m_rates["AED"] = 3.67;
    m_rates["SAR"] = 3.75;

    // Crypto currencies (approximate USD values)
    m_rates["BTC"] = 0.000023; // ~$43k
    m_rates["ETH"] = 0.00043;  // ~$2.3k
    m_rates["BNB"] = 0.0033;   // ~$300
    m_rates["XRP"] = 1.6;      // ~$0.62
    m_rates["ADA"] = 1.7;      // ~$0.58
    m_rates["SOL"] = 0.0095;   // ~$105
    m_rates["DOGE"] = 12.5;    // ~$0.08
    m_rates["DOT"] = 0.125;    // ~$8
    m_rates["MATIC"] = 1.1;    // ~$0.9
    m_rates["LTC"] = 0.014;    // ~$71
    m_rates["USDT"] = 1.0;     // Stablecoin
    m_rates["USDC"] = 1.0;     // Stablecoin
    m_rates["TRX"] = 5.0;      // Tron ~$0.20

    // Additional fiat currencies
    m_rates["ALL"] = 95.0; // Albanian Lek

    m_ratesLoaded = true;
  }

  // Auto-refresh rates on startup (1 second delay to let app initialize)
  QTimer::singleShot(1000, this, &CurrencyConverter::refreshRates);
}

void CurrencyConverter::setApiKey(const QString &key) { m_apiKey = key; }

void CurrencyConverter::setProvider(const QString &provider) {
  m_provider = provider;
}

double CurrencyConverter::convert(double amount, const QString &from,
                                  const QString &to) {
  QString fromUpper = from.toUpper();
  QString toUpper = to.toUpper();

  if (!m_rates.contains(fromUpper) || !m_rates.contains(toUpper)) {
    qDebug() << "CurrencyConverter: Unknown currency" << fromUpper << "or"
             << toUpper;
    return -1;
  }

  // Convert through base currency (USD)
  double fromRate = m_rates[fromUpper];
  double toRate = m_rates[toUpper];

  // amount in FROM -> USD -> TO
  double inBase = amount / fromRate; // to USD
  double result = inBase * toRate;   // to target

  return result;
}

bool CurrencyConverter::parseAndConvert(const QString &expression,
                                        double &result, QString &fromCurrency,
                                        QString &toCurrency) {
  // Currency symbol mapping
  static const QMap<QString, QString> symbolToCode = {
      {"$", "USD"}, {"€", "EUR"}, {"₺", "TRY"}, {"£", "GBP"},
      {"¥", "JPY"}, {"₽", "RUB"}, {"₿", "BTC"}, {"Ξ", "ETH"}};

  QString expr = expression.trimmed();

  // Replace symbols with codes for easier parsing
  for (auto it = symbolToCode.constBegin(); it != symbolToCode.constEnd();
       ++it) {
    expr.replace(it.key(), it.value());
  }

  // Pattern 1: "100 USD to TRY" or "100USD TRY" or "100 usd -> try"
  // Support 2-5 char symbols for crypto (BTC, ETH, USDT, MATIC, etc)
  static QRegularExpression fullPattern(
      R"((\d+(?:[.,]\d+)?)\s*([A-Za-z]{2,5})\s*(?:to|->|=>|>)?\s*([A-Za-z]{2,5}))",
      QRegularExpression::CaseInsensitiveOption);

  // Pattern 2: "100 USD" (no target - use base currency)
  static QRegularExpression shortPattern(
      R"((\d+(?:[.,]\d+)?)\s*([A-Za-z]{2,5})$)",
      QRegularExpression::CaseInsensitiveOption);

  QRegularExpressionMatch match = fullPattern.match(expr);
  if (match.hasMatch()) {
    QString amountStr = match.captured(1).replace(',', '.');
    fromCurrency = match.captured(2).toUpper();
    toCurrency = match.captured(3).toUpper();

    bool ok;
    double amount = amountStr.toDouble(&ok);
    if (!ok)
      return false;

    double converted = convert(amount, fromCurrency, toCurrency);
    if (converted < 0)
      return false;

    result = converted;
    return true;
  }

  // Try short pattern: "100 USD" -> convert to base currency
  match = shortPattern.match(expr);
  if (match.hasMatch()) {
    QString amountStr = match.captured(1).replace(',', '.');
    fromCurrency = match.captured(2).toUpper();
    toCurrency = Settings::instance()->baseCurrency().toUpper();

    // Don't convert if from and to are the same
    if (fromCurrency == toCurrency)
      return false;

    bool ok;
    double amount = amountStr.toDouble(&ok);
    if (!ok)
      return false;

    double converted = convert(amount, fromCurrency, toCurrency);
    if (converted < 0)
      return false;

    result = converted;
    return true;
  }

  return false;
}

void CurrencyConverter::refreshRates() {
  // Frankfurter.dev doesn't require API key
  if (m_provider != "frankfurter" && m_apiKey.isEmpty()) {
    qDebug() << "CurrencyConverter: No API key set, using cached rates";
    // Still fetch crypto rates (CoinGecko is free)
    refreshCryptoRates();
    return;
  }

  QString url = buildApiUrl();
  if (url.isEmpty()) {
    emit error("Unknown provider: " + m_provider);
    refreshCryptoRates(); // Still try crypto
    return;
  }

  QNetworkRequest request(url);

  // CoinAPI requires API key in header
  if (m_provider == "coinapi") {
    request.setRawHeader("X-CoinAPI-Key", m_apiKey.toUtf8());
  }

  QNetworkReply *reply = m_networkManager->get(request);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << "CurrencyConverter: Network error" << reply->errorString();
      emit error(reply->errorString());
    } else {
      parseApiResponse(reply->readAll());
    }

    // Always fetch crypto rates alongside fiat rates
    refreshCryptoRates();
  });
}

QString CurrencyConverter::buildApiUrl() const {
  // Provider-specific API URLs
  if (m_provider == "frankfurter") {
    // Frankfurter.dev - free, no API key required
    return QString("https://api.frankfurter.dev/v1/latest?base=USD");
  } else if (m_provider == "openexchangerates") {
    return QString("https://openexchangerates.org/api/latest.json?app_id=%1")
        .arg(m_apiKey);
  } else if (m_provider == "exchangerate.host") {
    return QString("https://api.exchangerate.host/live?access_key=%1")
        .arg(m_apiKey);
  } else if (m_provider == "alphavantage") {
    // Alpha Vantage requires different endpoint per pair, use EUR/USD as base
    return QString("https://www.alphavantage.co/query?function=FX_DAILY&from_"
                   "symbol=EUR&to_symbol=USD&apikey=%1")
        .arg(m_apiKey);
  } else if (m_provider == "twelvedata") {
    return QString("https://api.twelvedata.com/exchange_rate?symbol=EUR/"
                   "USD&apikey=%1")
        .arg(m_apiKey);
  } else if (m_provider == "coinapi") {
    return QString("https://rest.coinapi.io/v1/exchangerate/USD");
  } else if (m_provider == "fixer") {
    return QString("https://data.fixer.io/api/latest?access_key=%1")
        .arg(m_apiKey);
  } else if (m_provider == "currencylayer") {
    return QString("https://api.currencylayer.com/live?access_key=%1")
        .arg(m_apiKey);
  }
  return QString();
}

void CurrencyConverter::parseApiResponse(const QByteArray &data) {
  QJsonDocument doc = QJsonDocument::fromJson(data);

  if (!doc.isObject()) {
    emit error("Invalid API response");
    return;
  }

  QJsonObject json = doc.object();
  QJsonObject rates;

  // Provider-specific parsing
  if (m_provider == "frankfurter" || m_provider == "openexchangerates" ||
      m_provider == "fixer") {
    // These providers use same format: {"rates": {...}}
    if (json.contains("rates")) {
      rates = json["rates"].toObject();
    }
  } else if (m_provider == "exchangerate.host") {
    if (json.contains("quotes")) {
      // exchangerate.host returns "USDEUR", "USDGBP" etc
      QJsonObject quotes = json["quotes"].toObject();
      for (auto it = quotes.begin(); it != quotes.end(); ++it) {
        QString key = it.key();
        if (key.startsWith("USD")) {
          rates[key.mid(3)] = it.value().toDouble();
        }
      }
    }
  } else if (m_provider == "currencylayer") {
    if (json.contains("quotes")) {
      QJsonObject quotes = json["quotes"].toObject();
      for (auto it = quotes.begin(); it != quotes.end(); ++it) {
        QString key = it.key();
        if (key.startsWith("USD")) {
          rates[key.mid(3)] = it.value().toDouble();
        }
      }
    }
  } else if (m_provider == "coinapi") {
    if (json.contains("rates")) {
      QJsonArray ratesArr = json["rates"].toArray();
      for (const auto &val : ratesArr) {
        QJsonObject obj = val.toObject();
        rates[obj["asset_id_quote"].toString()] = obj["rate"].toDouble();
      }
    }
  } else if (json.contains("data")) {
    rates = json["data"].toObject();
  }

  if (rates.isEmpty()) {
    emit error("No rates found in API response");
    return;
  }

  m_rates.clear();
  m_rates["USD"] = 1.0; // Base

  for (auto it = rates.begin(); it != rates.end(); ++it) {
    m_rates[it.key()] = it.value().toDouble();
  }

  m_ratesLoaded = true;
  saveCachedRates();

  qDebug() << "CurrencyConverter: Updated" << m_rates.size() << "rates from"
           << m_provider;
  emit ratesUpdated();
}

bool CurrencyConverter::hasRates() const {
  return m_ratesLoaded && !m_rates.isEmpty();
}

QStringList CurrencyConverter::supportedCurrencies() const {
  return m_rates.keys();
}

void CurrencyConverter::loadCachedRates() {
  QString path = cachePath();
  QFile file(path);

  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (!doc.isObject())
    return;

  QJsonObject json = doc.object();
  m_baseCurrency = json["base"].toString("USD");

  QJsonObject rates = json["rates"].toObject();
  for (auto it = rates.begin(); it != rates.end(); ++it) {
    m_rates[it.key()] = it.value().toDouble();
  }

  m_ratesLoaded = true;
  qDebug() << "CurrencyConverter: Loaded" << m_rates.size() << "cached rates";
}

void CurrencyConverter::saveCachedRates() {
  QString path = cachePath();
  QFile file(path);

  if (!file.open(QIODevice::WriteOnly)) {
    return;
  }

  QJsonObject json;
  json["base"] = m_baseCurrency;

  QJsonObject rates;
  for (auto it = m_rates.begin(); it != m_rates.end(); ++it) {
    rates[it.key()] = it.value();
  }
  json["rates"] = rates;

  file.write(QJsonDocument(json).toJson());
  file.close();
}

QString CurrencyConverter::cachePath() const {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(dataPath);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  return dataPath + "/currency_rates.json";
}
void CurrencyConverter::refreshCryptoRates() {
  // Get crypto API key from settings
  QString cryptoApiKey = Settings::instance()->cryptoApiKey();

  // FreeCryptoAPI only supports single symbol per request
  static const QStringList cryptoSymbols = {"BTC", "ETH", "USDT", "USDC", "BNB",
                                            "XRP", "ADA", "DOGE", "SOL",  "TRX",
                                            "DOT", "LTC", "MATIC"};

  if (!cryptoApiKey.isEmpty()) {
    // Try FreeCryptoAPI first for each symbol
    for (const QString &symbol : cryptoSymbols) {
      QString url =
          QString("https://api.freecryptoapi.com/v1/getData?symbol=%1")
              .arg(symbol);

      QNetworkRequest request(url);
      request.setRawHeader("Accept", "application/json");
      request.setRawHeader("Authorization",
                           QString("Bearer %1").arg(cryptoApiKey).toUtf8());

      QNetworkReply *reply = m_networkManager->get(request);

      connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
          fetchCoinGeckoRate(symbol); // Fallback to CoinGecko
          return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject()) {
          fetchCoinGeckoRate(symbol);
          return;
        }

        QJsonObject json = doc.object();

        if (json["status"].toString() == "success" &&
            json.contains("symbols") && json["symbols"].isArray()) {
          QJsonArray symbols = json["symbols"].toArray();
          if (!symbols.isEmpty()) {
            QJsonObject coinData = symbols[0].toObject();
            double usdPrice = coinData["last"].toString().toDouble();
            if (usdPrice > 0) {
              m_rates[symbol] = 1.0 / usdPrice;
              qDebug() << "CurrencyConverter:" << symbol << "=" << usdPrice
                       << "USD (FreeCryptoAPI)";
              saveCachedRates();
              emit ratesUpdated();
              return;
            }
          }
        }
        // FreeCryptoAPI doesn't have this symbol, try CoinGecko
        fetchCoinGeckoRate(symbol);
      });
    }
  } else {
    // No FreeCryptoAPI key, use CoinGecko for all
    fetchAllCoinGeckoRates();
  }
}

void CurrencyConverter::fetchCoinGeckoRate(const QString &symbol) {
  // Map symbol to CoinGecko ID
  static const QMap<QString, QString> symbolToId = {
      {"BTC", "bitcoin"},        {"ETH", "ethereum"},    {"USDT", "tether"},
      {"USDC", "usd-coin"},      {"BNB", "binancecoin"}, {"XRP", "ripple"},
      {"ADA", "cardano"},        {"DOGE", "dogecoin"},   {"SOL", "solana"},
      {"TRX", "tron"},           {"DOT", "polkadot"},    {"LTC", "litecoin"},
      {"MATIC", "matic-network"}};

  if (!symbolToId.contains(symbol))
    return;

  QString coinId = symbolToId[symbol];
  QString url = QString("https://api.coingecko.com/api/v3/simple/"
                        "price?ids=%1&vs_currencies=usd")
                    .arg(coinId);

  QNetworkRequest request(url);
  request.setRawHeader("Accept", "application/json");

  QNetworkReply *reply = m_networkManager->get(request);

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, symbol, coinId]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError)
              return;

            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (!doc.isObject())
              return;

            QJsonObject json = doc.object();
            if (json.contains(coinId)) {
              QJsonObject coinData = json[coinId].toObject();
              if (coinData.contains("usd")) {
                double usdPrice = coinData["usd"].toDouble();
                if (usdPrice > 0) {
                  m_rates[symbol] = 1.0 / usdPrice;
                  qDebug() << "CurrencyConverter:" << symbol << "=" << usdPrice
                           << "USD (CoinGecko fallback)";
                  saveCachedRates();
                  emit ratesUpdated();
                }
              }
            }
          });
}

void CurrencyConverter::fetchAllCoinGeckoRates() {
  QString url = "https://api.coingecko.com/api/v3/simple/price?"
                "ids=bitcoin,ethereum,tether,usd-coin,binancecoin,ripple,"
                "cardano,dogecoin,solana,tron,polkadot,litecoin,matic-network"
                "&vs_currencies=usd";

  QNetworkRequest request(url);
  request.setRawHeader("Accept", "application/json");

  QNetworkReply *reply = m_networkManager->get(request);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
      return;

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject())
      return;

    QJsonObject json = doc.object();

    static const QMap<QString, QString> idToSymbol = {
        {"bitcoin", "BTC"},        {"ethereum", "ETH"},    {"tether", "USDT"},
        {"usd-coin", "USDC"},      {"binancecoin", "BNB"}, {"ripple", "XRP"},
        {"cardano", "ADA"},        {"dogecoin", "DOGE"},   {"solana", "SOL"},
        {"tron", "TRX"},           {"polkadot", "DOT"},    {"litecoin", "LTC"},
        {"matic-network", "MATIC"}};

    int updated = 0;
    for (auto it = idToSymbol.constBegin(); it != idToSymbol.constEnd(); ++it) {
      if (json.contains(it.key())) {
        QJsonObject coinData = json[it.key()].toObject();
        if (coinData.contains("usd")) {
          double usdPrice = coinData["usd"].toDouble();
          if (usdPrice > 0) {
            m_rates[it.value()] = 1.0 / usdPrice;
            updated++;
          }
        }
      }
    }

    if (updated > 0) {
      qDebug() << "CurrencyConverter: Updated" << updated
               << "crypto rates (CoinGecko)";
      saveCachedRates();
      emit ratesUpdated();
    }
  });
}
