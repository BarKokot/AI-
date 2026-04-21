#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

/**
 * @file CodeExecutor.h
 * @brief Klasa do ekstrakcji i wykonywania kodu Python wygenerowanego przez LLM.
 */

/**
 * @class CodeExecutor
 * @brief Wyodrębnia kod Python z odpowiedzi LLM i wykonuje go.
 *
 * Kod jest zapisywany do pliku tymczasowego i uruchamiany przez interpreter Python.
 * Wykres jest zapisywany do PNG i wczytywany do zakładki Wykres.
 */
class CodeExecutor : public QObject {
    Q_OBJECT

public:
    explicit CodeExecutor(QObject* parent = nullptr);

    /**
     * @brief Wyodrębnia kod Python z odpowiedzi LLM (między tagami <KOD>...</KOD>
     *        lub blokami ```python).
     */
    static QString extractCode(const QString& llmResponse);

    /**
     * @brief Wyodrębnia schemat ASCII (między tagami <SCHEMAT>...</SCHEMAT>).
     */
    static QString extractSchematic(const QString& llmResponse);

    /**
     * @brief Wyodrębnia wyjaśnienie (między tagami <WYJAŚNIENIE>...</WYJAŚNIENIE>).
     */
    static QString extractExplanation(const QString& llmResponse);

    /**
     * @brief Modyfikuje kod Python: zastępuje plt.show() zapisem do PNG.
     *
     * Dodaje na końcu kodu linię zapisującą wykres do pliku tymczasowego
     * i drukuje ścieżkę na stdout. ResultWidget odczyta tę ścieżkę i wczyta PNG.
     *
     * @param code Oryginalny kod Python
     * @return Zmodyfikowany kod z plt.savefig() zamiast plt.show()
     */
    static QString injectPlotSave(const QString& code);

    /**
     * @brief Sprawdza czy Python jest dostępny w systemie.
     */
    static bool isPythonAvailable();

public slots:
    /**
     * @brief Wykonuje kod Python asynchronicznie.
     * @param pythonCode Kod do wykonania
     */
    void executeCode(const QString& pythonCode);

signals:
    /**
     * @brief Emitowany gdy kod zakończył działanie.
     * @param output   stdout procesu (może zawierać ścieżkę PNG)
     * @param success  Czy zakończono bez błędu
     */
    void executionFinished(const QString& output, bool success);

    /**
     * @brief Emitowany przy błędzie uruchomienia.
     */
    void executionError(const QString& errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess* m_process    = nullptr;
    QString   m_tempFilePath;

    static QString extractBetweenTags(const QString& text,
                                      const QString& openTag,
                                      const QString& closeTag);
};
