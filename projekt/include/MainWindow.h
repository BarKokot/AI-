#pragma once

#include "FilterTypes.h"
#include "FilterCalculator.h"
#include "OllamaWorker.h"
#include "CodeExecutor.h"
#include "ParameterWidget.h"
#include "ResultWidget.h"

#include <QMainWindow>
#include <QComboBox>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>

/**
 * @file MainWindow.h
 * @brief Główne okno aplikacji FilterDesigner.
 */

/**
 * @class MainWindow
 * @brief Główne okno — orkiestruje wszystkie komponenty aplikacji.
 *
 * Zarządza cyklem życia zapytania:
 *   1. Użytkownik wybiera typ filtru i podaje parametry
 *   2. FilterCalculator oblicza wyniki matematyczne
 *   3. OllamaWorker asynchronicznie pobiera odpowiedź LLM
 *   4. CodeExecutor uruchamia wygenerowany kod Python
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Rodzic Qt
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destruktor — sprząta wątek roboczy.
     */
    ~MainWindow() override;

private slots:
    void onFilterTypeChanged(int index);
    void onAnalyzeRequested();
    void onLLMFinished(const QString& response, bool success);
    void onLLMStatus(const QString& message);
    void onExecuteCode(const QString& code);
    void onExecutionFinished(const QString& output, bool success);
    void onExecutionError(const QString& error);

private:
    void setupUi();
    void setupMenuBar();
    void setUiBusy(bool busy);
    void applyStyleSheet();

    // Komponenty UI
    QComboBox*       m_filterTypeCombo;
    QTabWidget*      m_mainTabs;
    ParameterWidget* m_paramWidget;
    ResultWidget*    m_resultWidget;
    QLineEdit*       m_modelNameEdit;
    QLabel*          m_ollamaStatusLabel;

    // Logika
    FilterCalculator m_calculator;
    OllamaWorker*    m_worker   = nullptr;
    CodeExecutor*    m_executor = nullptr;
    FilterParams     m_lastParams;
    FilterResult     m_lastResult;
};
