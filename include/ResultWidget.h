#pragma once

#include "FilterTypes.h"
#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QScrollArea>

/**
 * @file ResultWidget.h
 * @brief Widget do wyświetlania wyników analizy filtru.
 */

/**
 * @class ResultWidget
 * @brief Wielozakładkowy panel wyników: obliczenia, kod, schemat, wyjaśnienie, wykres, wyjście.
 */
class ResultWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResultWidget(QWidget* parent = nullptr);

    /** @brief Wyświetla wyniki obliczeń matematycznych. */
    void showCalculationResult(const FilterResult& result);

    /** @brief Wyświetla odpowiedź LLM (kod, schemat, wyjaśnienie). */
    void showLLMResponse(const QString& response, bool success);

    /** @brief Wyświetla schemat ASCII wygenerowany w C++. */
    void showSchematic(const QString& schematic);

    /** @brief Wyświetla wygenerowany wykres PNG w zakładce Wykres. */
    void showPlot(const QString& imagePath);

    /** @brief Wyświetla wynik wykonania kodu Python. */
    void showExecutionResult(const QString& output, bool success);

    /** @brief Ustawia komunikat statusu. */
    void setStatus(const QString& message, bool busy = false);

    /** @brief Czyści wszystkie pola wynikowe. */
    void clearAll();

signals:
    /** @brief Emitowany gdy użytkownik chce wykonać wygenerowany kod. */
    void executeCodeRequested(const QString& code);

private:
    void setupUi();

    QTabWidget*     m_tabs;
    QTextEdit*      m_calcResultEdit;   ///< Zakładka: Obliczenia
    QPlainTextEdit* m_codeEdit;         ///< Zakładka: Kod Python
    QPlainTextEdit* m_schematicEdit;    ///< Zakładka: Schemat ASCII
    QTextEdit*      m_explanationEdit;  ///< Zakładka: Wyjaśnienie
    QLabel*         m_plotLabel;        ///< Zakładka: Wykres (obraz PNG)
    QScrollArea*    m_plotScroll;       ///< Scroll dla wykresu
    QTextEdit*      m_execOutputEdit;   ///< Zakładka: Wyjście Python
    QLabel*         m_statusLabel;
    QPushButton*    m_btnRunCode;
    QString         m_currentCode;
};
