#ifndef LINNOTE_CURRENCYCONVERTER_H
#define LINNOTE_CURRENCYCONVERTER_H

#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

/**
 * @brief Currency converter with API integration
 *
 * Uses exchangerate.host or similar free API
 * Caches rates for offline use
 */
class CurrencyConverter : public QObject {
  Q_OBJECT

public:
  static CurrencyConverter *instance();

  /**
   * @brief Set API key (if using paid API)
   */
  void setApiKey(const QString &key);

  /**
   * @brief Set exchange rate provider
   */
  void setProvider(const QString &provider);

  /**
   * @brief Convert amount between currencies
   * @param amount The amount to convert
   * @param from Source currency code (e.g., "USD")
   * @param to Target currency code (e.g., "TRY")
   * @return Converted amount, or -1 if conversion failed
   */
  double convert(double amount, const QString &from, const QString &to);

  /**
   * @brief Parse a currency expression like "100 USD to TRY"
   * @param expression The expression to parse
   * @param result Output: the converted amount
   * @return true if parsing and conversion succeeded
   */
  bool parseAndConvert(const QString &expression, double &result,
                       QString &fromCurrency, QString &toCurrency);

  /**
   * @brief Refresh exchange rates from API
   */
  void refreshRates();

  /**
   * @brief Check if rates are available
   */
  bool hasRates() const;

  /**
   * @brief Get supported currencies
   */
  QStringList supportedCurrencies() const;

signals:
  void ratesUpdated();
  void error(const QString &message);

private:
  explicit CurrencyConverter(QObject *parent = nullptr);
  void loadCachedRates();
  void saveCachedRates();
  QString cachePath() const;
  QString buildApiUrl() const;
  void parseApiResponse(const QByteArray &data);
  void refreshCryptoRates(); // FreeCryptoAPI + CoinGecko fallback
  void fetchCoinGeckoRate(const QString &symbol);
  void fetchAllCoinGeckoRates();

  QNetworkAccessManager *m_networkManager;
  QString m_apiKey;
  QString m_provider;
  QString m_baseCurrency;
  QMap<QString, double> m_rates; // Rates relative to base currency
  bool m_ratesLoaded;
};

#endif // LINNOTE_CURRENCYCONVERTER_H
