#pragma once

#include "FilterTypes.h"
#include "OllamaClient.h"
#include <QThread>
#include <QString>

/**
 * @file OllamaWorker.h
 * @brief Wątek roboczy do asynchronicznej komunikacji z Ollama.
 */

/**
 * @class OllamaWorker
 * @brief Wykonuje zapytania do Ollama w osobnym wątku (QThread).
 *
 * Dzięki uruchomieniu w tle zapytania do modelu językowego nie blokują
 * interfejsu graficznego. Po zakończeniu emituje sygnał finished().
 */
class OllamaWorker : public QThread {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param params           Parametry filtru
     * @param result           Wyniki obliczeń
     * @param generatePlot     Generuj wykres
     * @param generateCode     Generuj kod
     * @param generateSchematic Generuj schemat
     * @param model            Nazwa modelu Ollama
     * @param parent           Rodzic Qt
     */
    explicit OllamaWorker(const FilterParams& params,
                          const FilterResult& result,
                          bool generatePlot,
                          bool generateCode,
                          bool generateSchematic,
                          const QString& model = "qwen3:8b",
                          QObject* parent = nullptr);

signals:
    /**
     * @brief Emitowany po zakończeniu zapytania.
     * @param response Odpowiedź modelu (lub komunikat błędu)
     * @param success  Czy zapytanie się powiodło
     */
    void finished(const QString& response, bool success);

    /**
     * @brief Emitowany podczas postępu (informacja tekstowa).
     * @param message Wiadomość statusu
     */
    void statusUpdate(const QString& message);

protected:
    /**
     * @brief Główna metoda wątku — wykonuje zapytanie do Ollama.
     */
    void run() override;

private:
    FilterParams  m_params;
    FilterResult  m_result;
    bool          m_generatePlot;
    bool          m_generateCode;
    bool          m_generateSchematic;
    QString       m_model;
};
