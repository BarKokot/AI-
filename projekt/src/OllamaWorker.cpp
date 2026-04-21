#include "OllamaWorker.h"

/**
 * @file OllamaWorker.cpp
 * @brief Implementacja wątku roboczego OllamaWorker.
 */

OllamaWorker::OllamaWorker(const FilterParams& params,
                             const FilterResult& result,
                             bool generatePlot,
                             bool generateCode,
                             bool generateSchematic,
                             const QString& model,
                             QObject* parent)
    : QThread(parent),
      m_params(params),
      m_result(result),
      m_generatePlot(generatePlot),
      m_generateCode(generateCode),
      m_generateSchematic(generateSchematic),
      m_model(model)
{}

void OllamaWorker::run() {
    emit statusUpdate("Sprawdzanie dostępności serwera Ollama...");

    // Tworzymy klienta wewnątrz wątku (QNetworkAccessManager nie jest thread-safe)
    OllamaClient client(nullptr);

    if (!client.isServerAvailable()) {
        emit finished(
            "BŁĄD: Serwer Ollama nie jest dostępny.\n\n"
            "Upewnij się, że:\n"
            "• Ollama jest uruchomiona (ollama serve)\n"
            "• Model jest pobrany (ollama pull qwen3:8b)\n"
            "• Port 11434 jest otwarty",
            false
        );
        return;
    }

    emit statusUpdate(QString("Wysyłanie zapytania do modelu %1...").arg(m_model));

    QString systemPrompt = OllamaClient::buildSystemPrompt(m_params.type);
    QString userPrompt   = OllamaClient::buildUserPrompt(
        m_params, m_result,
        m_generatePlot, m_generateCode, m_generateSchematic
    );

    QString response = client.sendRequest(systemPrompt, userPrompt, m_model);

    if (response.startsWith("BŁĄD")) {
        emit finished(response, false);
    } else {
        emit statusUpdate("Odpowiedź otrzymana.");
        emit finished(response, true);
    }
}
