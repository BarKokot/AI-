#pragma once

#include "FilterTypes.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QUrl>

/**
 * @file OllamaClient.h
 * @brief Klient HTTP do komunikacji z lokalnym serwerem Ollama.
 */

/**
 * @class OllamaClient
 * @brief Obsługuje komunikację z API serwera Ollama przez REST.
 *
 * Klasa buduje zapytania JSON i wysyła je synchronicznie do endpointu
 * /api/generate działającego serwera Ollama. Przeznaczona do użycia
 * wewnątrz wątku roboczego (OllamaWorker).
 */
class OllamaClient : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Rodzic Qt
     * @param host   Adres hosta (domyślnie "localhost")
     * @param port   Port serwera (domyślnie 11434)
     */
    explicit OllamaClient(QObject* parent = nullptr,
                          const QString& host = "localhost",
                          int port = 11434);

    /**
     * @brief Wysyła zapytanie do modelu i zwraca odpowiedź (blokujące).
     * @param systemPrompt Prompt systemowy (rola modelu)
     * @param userPrompt   Prompt użytkownika (zapytanie)
     * @param model        Nazwa modelu Ollama
     * @return Tekst odpowiedzi lub komunikat błędu
     */
    QString sendRequest(const QString& systemPrompt,
                        const QString& userPrompt,
                        const QString& model = "qwen3:8b");

    /**
     * @brief Sprawdza czy serwer Ollama jest dostępny.
     * @return true jeśli serwer odpowiada na /api/tags
     */
    bool isServerAvailable();

    /**
     * @brief Buduje prompt systemowy dla projektanta filtrów.
     * @param filterType Typ filtru
     * @return Gotowy system prompt
     */
    static QString buildSystemPrompt(FilterType filterType);

    /**
     * @brief Buduje prompt użytkownika z parametrami filtru.
     * @param params Parametry filtru
     * @param result Wyniki obliczeń
     * @param generatePlot     Czy generować kod wykresu
     * @param generateCode     Czy generować pełny kod
     * @param generateSchematic Czy generować schemat ASCII
     * @return Gotowy user prompt
     */
    static QString buildUserPrompt(const FilterParams& params,
                                   const FilterResult& result,
                                   bool generatePlot,
                                   bool generateCode,
                                   bool generateSchematic);

    /**
     * @brief Ustawia timeout zapytania.
     * @param ms Timeout w milisekundach
     */
    void setTimeout(int ms) { m_timeout = ms; }

private:
    QString m_host;
    int     m_port;
    int     m_timeout = 6000000; ///< Domyślny timeout: 60 s
    QNetworkAccessManager m_nam;

    QString m_apiUrl() const {
        return QString("http://%1:%2/api/generate").arg(m_host).arg(m_port);
    }
    QString m_tagsUrl() const {
        return QString("http://%1:%2/api/tags").arg(m_host).arg(m_port);
    }
};
