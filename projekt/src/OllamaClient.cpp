#include "OllamaClient.h"
#include "FilterCalculator.h"
#include <QNetworkRequest>
#include <QEventLoop>
#include <QTimer>
#include <QJsonArray>
#include <QJsonParseError>

/**
 * @file OllamaClient.cpp
 * @brief Implementacja klienta REST dla serwera Ollama.
 */

OllamaClient::OllamaClient(QObject* parent, const QString& host, int port)
    : QObject(parent), m_host(host), m_port(port)
{}

bool OllamaClient::isServerAvailable() {
    QNetworkRequest req{QUrl(m_tagsUrl())};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    bool available = false;

    QNetworkReply* reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, &loop, [&]() {
        available = (reply->error() == QNetworkReply::NoError);
        loop.quit();
    });
    connect(&timer, &QTimer::timeout, &loop, [&]() {
        reply->abort();
        loop.quit();
    });

    timer.start(5000);
    loop.exec();
    reply->deleteLater();
    return available;
}

QString OllamaClient::sendRequest(const QString& systemPrompt,
                                  const QString& userPrompt,
                                  const QString& model) {
    QJsonObject body;
    body["model"]  = model;
    body["system"] = systemPrompt;
    body["prompt"] = userPrompt;
    body["stream"] = false;

    QNetworkRequest req{QUrl(m_apiUrl())};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QString result;
    bool timedOut = false;

    QNetworkReply* reply = m_nam.post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, &loop, [&]() {
        if (reply->error() != QNetworkReply::NoError && !timedOut) {
            result = QString("BŁĄD_SIECI: %1").arg(reply->errorString());
        } else if (!timedOut) {
            QJsonParseError parseErr;
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseErr);
            if (parseErr.error != QJsonParseError::NoError) {
                result = QString("BŁĄD_JSON: %1").arg(parseErr.errorString());
            } else {
                result = doc.object().value("response").toString();
            }
        }
        loop.quit();
    });

    connect(&timer, &QTimer::timeout, &loop, [&]() {
        timedOut = true;
        reply->abort();
        result = "BŁĄD_TIMEOUT: Serwer Ollama nie odpowiedział w wyznaczonym czasie.";
        loop.quit();
    });

    timer.start(m_timeout);
    loop.exec();
    reply->deleteLater();
    return result;
}

QString OllamaClient::buildSystemPrompt(FilterType filterType) {
    QString filterName = filterTypeName(filterType);
    return QString(
        "Jesteś ekspertem od elektroniki i projektowania filtrów analogowych. "
        "Specjalizujesz się w filtrach RC (%1). "
        "Twoim zadaniem jest generowanie gotowego kodu Python (matplotlib/numpy) "
        "do rysowania charakterystyk Bodego filtrów oraz schematów ASCII obwodów. "
        "Zawsze generujesz kompletny, działający kod bez komentarzy w sekcjach kodu. "
        "Schematy ASCII otaczaj tagami <SCHEMAT> i </SCHEMAT>. "
        "Kod Python otaczaj tagami <KOD> i </KOD>. "
        "Wyjaśnienia otaczaj tagami <WYJAŚNIENIE> i </WYJAŚNIENIE>. "
        "Odpowiadaj wyłącznie po polsku."
    ).arg(filterName);
}

QString OllamaClient::buildUserPrompt(const FilterParams& params,
                                       const FilterResult& result,
                                       bool generatePlot,
                                       bool generateCode,
                                       bool generateSchematic) {
    QString prompt;
    prompt += QString("Typ filtru: %1\n").arg(filterTypeName(params.type));

    if (result.valid) {
        if (params.mode == ParameterMode::KnownRC) {
            prompt += QString("Rezystancja R = %1 Ω\n")
                .arg(FilterCalculator::formatEngineering(params.R, "Ω"));
            prompt += QString("Pojemność C = %1 F\n")
                .arg(FilterCalculator::formatEngineering(params.C, "F"));
            if (params.type == FilterType::BandPass || params.type == FilterType::BandStop) {
                prompt += QString("R2 = %1, C2 = %2\n")
                    .arg(FilterCalculator::formatEngineering(params.R2, "Ω"))
                    .arg(FilterCalculator::formatEngineering(params.C2, "F"));
            }
            prompt += QString("Wyznaczona częstotliwość graniczna: %1\n")
                .arg(FilterCalculator::formatEngineering(result.frequency, "Hz"));
        } else {
            if (params.type == FilterType::BandPass || params.type == FilterType::BandStop) {
                prompt += QString("f1 = %1, f2 = %2\n")
                    .arg(FilterCalculator::formatEngineering(params.f1, "Hz"))
                    .arg(FilterCalculator::formatEngineering(params.f2, "Hz"));
            } else {
                prompt += QString("Częstotliwość graniczna: %1\n")
                    .arg(FilterCalculator::formatEngineering(params.f0, "Hz"));
            }
            prompt += QString("Dobrana rezystancja R = %1\n")
                .arg(FilterCalculator::formatEngineering(result.R_calc, "Ω"));
            prompt += QString("Dobrana pojemność C = %1\n")
                .arg(FilterCalculator::formatEngineering(result.C_calc, "F"));
        }
        if (result.Q > 0)
            prompt += QString("Dobroć Q = %1\n").arg(result.Q, 0, 'f', 2);
        if (result.bandwidth > 0)
            prompt += QString("Pasmo = %1\n")
                .arg(FilterCalculator::formatEngineering(result.bandwidth, "Hz"));
    }

    if (!params.customDescription.isEmpty())
        prompt += QString("Dodatkowe wymagania: %1\n").arg(params.customDescription);

    prompt += "\nWygeneruj:\n";
    if (generatePlot)
        prompt += "1. Kod Python (matplotlib + numpy) rysujący charakterystykę Bodego "
                  "(amplitudową i fazową) dla tego filtru z zaznaczoną częstotliwością graniczną. "
                  "Kod musi być kompletny i gotowy do uruchomienia.\n";
    if (generateCode)
        prompt += "2. Kod Python obliczający i wyświetlający pełną charakterystykę filtru.\n";
    if (generateSchematic)
        prompt += "3. Schemat ASCII obwodu elektrycznego filtru.\n";
    prompt += "4. Krótkie wyjaśnienie zasady działania filtru.\n";

    return prompt;
}
