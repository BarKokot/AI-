#include "MainWindow.h"
#include "SchematicRenderer.h"
#include "OllamaClient.h"

#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QFormLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QStatusBar>
#include <QLabel>
#include <QFrame>

/**
 * @file MainWindow.cpp
 * @brief Implementacja głównego okna aplikacji.
 */

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setupMenuBar();
    applyStyleSheet();
    setWindowTitle("FilterDesigner — Projektant Filtrów RC");
    resize(1100, 720);
    statusBar()->showMessage("Gotowy. Wybierz typ filtru i podaj parametry.");
}

MainWindow::~MainWindow() {
    if (m_worker && m_worker->isRunning()) {
        m_worker->quit();
        m_worker->wait(3000);
    }
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setSpacing(0);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    // ── Nagłówek ──
    auto* headerWidget = new QWidget(central);
    headerWidget->setObjectName("header");
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 10, 16, 10);

    auto* titleLabel = new QLabel("<b>FilterDesigner</b> — Projektant Filtrów RC", headerWidget);
    titleLabel->setObjectName("titleLabel");
    headerLayout->addWidget(titleLabel, 1);

    auto* modelLabel = new QLabel("Model Ollama:", headerWidget);
    m_modelNameEdit = new QLineEdit("qwen3:8b", headerWidget);
    m_modelNameEdit->setMaximumWidth(160);
    m_modelNameEdit->setObjectName("modelEdit");
    m_ollamaStatusLabel = new QLabel("🔴 sprawdzanie...", headerWidget);
    m_ollamaStatusLabel->setObjectName("ollamaStatus");

    headerLayout->addWidget(modelLabel);
    headerLayout->addWidget(m_modelNameEdit);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(m_ollamaStatusLabel);
    rootLayout->addWidget(headerWidget);

    // ── Separator ──
    auto* line = new QFrame(central);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("separator");
    rootLayout->addWidget(line);

    // ── Wybór typu filtru ──
    auto* typeBar = new QWidget(central);
    typeBar->setObjectName("typeBar");
    auto* typeLayout = new QHBoxLayout(typeBar);
    typeLayout->setContentsMargins(16, 8, 16, 8);

    auto* typeLabel = new QLabel("Typ filtru:", typeBar);
    typeLabel->setObjectName("typeLabel");
    m_filterTypeCombo = new QComboBox(typeBar);
    m_filterTypeCombo->setObjectName("filterCombo");
    m_filterTypeCombo->addItem("🔽  Dolnoprzepustowy (LP)",  static_cast<int>(FilterType::LowPass));
    m_filterTypeCombo->addItem("🔼  Górnoprzepustowy (HP)",  static_cast<int>(FilterType::HighPass));
    m_filterTypeCombo->addItem("🎵  Pasmo-przepustowy (BP)", static_cast<int>(FilterType::BandPass));
    m_filterTypeCombo->addItem("🚫  Pasmo-zaporowy (BS/Notch)", static_cast<int>(FilterType::BandStop));
    m_filterTypeCombo->addItem("⚙️  Inny",                   static_cast<int>(FilterType::Other));
    m_filterTypeCombo->setMinimumWidth(260);

    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(m_filterTypeCombo);
    typeLayout->addStretch();
    rootLayout->addWidget(typeBar);

    // ── Splitter: Parametry | Wyniki ──
    auto* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setHandleWidth(6);

    // Panel parametrów
    auto* leftPanel = new QWidget(splitter);
    leftPanel->setObjectName("leftPanel");
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(12, 12, 6, 12);

    auto* paramsTitle = new QLabel("Parametry filtru", leftPanel);
    paramsTitle->setObjectName("panelTitle");
    leftLayout->addWidget(paramsTitle);

    m_paramWidget = new ParameterWidget(leftPanel);
    leftLayout->addWidget(m_paramWidget);

    // Panel wyników
    auto* rightPanel = new QWidget(splitter);
    rightPanel->setObjectName("rightPanel");
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(6, 12, 12, 12);

    auto* resultsTitle = new QLabel("Wyniki i generacja", rightPanel);
    resultsTitle->setObjectName("panelTitle");
    rightLayout->addWidget(resultsTitle);

    m_resultWidget = new ResultWidget(rightPanel);
    rightLayout->addWidget(m_resultWidget);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    rootLayout->addWidget(splitter, 1);

    // ── Połączenia sygnałów ──
    connect(m_filterTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterTypeChanged);
    connect(m_paramWidget, &ParameterWidget::analyzeRequested,
            this, &MainWindow::onAnalyzeRequested);
    connect(m_resultWidget, &ResultWidget::executeCodeRequested,
            this, &MainWindow::onExecuteCode);

    m_executor = new CodeExecutor(this);
    connect(m_executor, &CodeExecutor::executionFinished,
            this, &MainWindow::onExecutionFinished);
    connect(m_executor, &CodeExecutor::executionError,
            this, &MainWindow::onExecutionError);

    // Sprawdź dostępność Ollamy w tle
    QTimer::singleShot(500, this, [this]() {
        OllamaClient c;
        bool avail = c.isServerAvailable();
        m_ollamaStatusLabel->setText(avail ? "🟢 Ollama online" : "🔴 Ollama offline");
    });
}

void MainWindow::setupMenuBar() {
    auto* menuBar = this->menuBar();

    auto* fileMenu = menuBar->addMenu("&Plik");
    auto* actClear = new QAction("Wyczyść wyniki", this);
    actClear->setShortcut(QKeySequence("Ctrl+L"));
    connect(actClear, &QAction::triggered, m_resultWidget, &ResultWidget::clearAll);
    fileMenu->addAction(actClear);
    fileMenu->addSeparator();
    auto* actQuit = new QAction("Zakończ", this);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actQuit, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(actQuit);

    auto* helpMenu = menuBar->addMenu("&Pomoc");
    auto* actAbout = new QAction("O programie", this);
    connect(actAbout, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "O programie",
                           "<b>FilterDesigner v1.0</b><br>"
                           "Projektant filtrów RC z generacją kodu Python<br>"
                           "i analizą przez lokalny model językowy (Ollama).<br><br>"
                           "Projekt JPO 2025/2026");
    });
    helpMenu->addAction(actAbout);
}

void MainWindow::onFilterTypeChanged(int index) {
    int val = m_filterTypeCombo->itemData(index).toInt();
    FilterType type = static_cast<FilterType>(val);
    m_paramWidget->setFilterType(type);
}

void MainWindow::onAnalyzeRequested() {
    if (m_worker && m_worker->isRunning()) {
        QMessageBox::information(this, "Trwa zapytanie",
                                 "Poczekaj na zakończenie poprzedniego zapytania.");
        return;
    }

    bool ok;
    FilterParams params = m_paramWidget->collectParams(ok);
    if (!ok) return;

    m_lastParams = params;
    m_resultWidget->clearAll();

    // Obliczenia lokalne
    m_lastResult = m_calculator.calculate(params);
    m_resultWidget->showCalculationResult(m_lastResult);

    if (!m_lastResult.valid) return;

    // Schemat renderowany lokalnie w C++
    m_resultWidget->showSchematic(SchematicRenderer::render(params, m_lastResult));

    // Zapytanie do LLM w osobnym wątku
    setUiBusy(true);
    m_resultWidget->setStatus("Wysyłanie zapytania do modelu...", true);

    m_worker = new OllamaWorker(
        params, m_lastResult,
        params.generatePlot, params.generateCode, params.generateSchematic,
        m_modelNameEdit->text().trimmed()
        );
    connect(m_worker, &OllamaWorker::finished,   this, &MainWindow::onLLMFinished);
    connect(m_worker, &OllamaWorker::statusUpdate, this, &MainWindow::onLLMStatus);
    connect(m_worker, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &QObject::destroyed, this, [this]() { m_worker = nullptr; });
    m_worker->start();
}

void MainWindow::onLLMStatus(const QString& message) {
    m_resultWidget->setStatus(message, true);
    statusBar()->showMessage(message);
}

void MainWindow::onLLMFinished(const QString& response, bool success) {
    setUiBusy(false);
    m_resultWidget->showLLMResponse(response, success);
    statusBar()->showMessage(success ? "Analiza zakończona." : "Błąd modelu językowego.");
}

void MainWindow::onExecuteCode(const QString& code) {
    if (!CodeExecutor::isPythonAvailable()) {
        QMessageBox::warning(this, "Brak Pythona",
                             "Python nie jest dostępny w PATH.\n"
                             "Zainstaluj Python 3 i upewnij się, że jest w PATH.");
        return;
    }
    m_resultWidget->setStatus("Wykonywanie kodu Python...", true);
    statusBar()->showMessage("Uruchamianie kodu Python...");
    m_executor->executeCode(code);
}

void MainWindow::onExecutionFinished(const QString& output, bool success) {
    m_resultWidget->showExecutionResult(output, success);
    if (success) {
        // Szukaj ścieżki wykresu w stdout (format: PLOT_PATH:/tmp/...)
        for (const QString& line : output.split("\n")) {
            if (line.startsWith("PLOT_PATH:")) {
                QString plotPath = line.mid(QString("PLOT_PATH:").length()).trimmed();
                m_resultWidget->showPlot(plotPath);
                break;
            }
        }
        statusBar()->showMessage("Kod wykonany — wykres wczytany.");
    } else {
        statusBar()->showMessage("Błąd wykonania kodu.");
    }
}

void MainWindow::onExecutionError(const QString& error) {
    m_resultWidget->showExecutionResult(error, false);
    statusBar()->showMessage("Błąd uruchomienia Pythona.");
}

void MainWindow::setUiBusy(bool busy) {
    m_paramWidget->setEnabled(!busy);
    m_filterTypeCombo->setEnabled(!busy);
}

void MainWindow::applyStyleSheet() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1a1d23;
        }
        QWidget {
            background-color: #1a1d23;
            color: #e8eaf0;
            font-family: 'Segoe UI', sans-serif;
            font-size: 13px;
        }
        #header {
            background-color: #12141a;
            border-bottom: 1px solid #2e3340;
        }
        #titleLabel {
            font-size: 15px;
            color: #7eb8f7;
        }
        #modelEdit {
            background-color: #252932;
            border: 1px solid #3a4055;
            border-radius: 4px;
            padding: 4px 8px;
            color: #b8c8e8;
        }
        #ollamaStatus {
            font-size: 12px;
            color: #8899aa;
        }
        #separator {
            color: #2e3340;
            background-color: #2e3340;
            max-height: 1px;
        }
        #typeBar {
            background-color: #1e2230;
            border-bottom: 1px solid #2e3340;
        }
        #typeLabel {
            font-weight: bold;
            color: #7eb8f7;
        }
        QComboBox#filterCombo {
            background-color: #252932;
            border: 2px solid #3a5080;
            border-radius: 6px;
            padding: 6px 12px;
            color: #e8eaf0;
            font-size: 13px;
            min-height: 30px;
        }
        QComboBox#filterCombo:hover { border-color: #5a80c0; }
        QComboBox#filterCombo::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #252932;
            border: 1px solid #3a5080;
            selection-background-color: #3a5080;
        }
        #leftPanel, #rightPanel {
            background-color: #1e2230;
            border-radius: 8px;
        }
        #panelTitle {
            font-size: 14px;
            font-weight: bold;
            color: #7eb8f7;
            padding: 4px 0px 8px 0px;
        }
        QGroupBox {
            border: 1px solid #2e3a50;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 8px;
            color: #99aabb;
            font-size: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            color: #7eb8f7;
        }
        QLineEdit {
            background-color: #252932;
            border: 1px solid #3a4055;
            border-radius: 4px;
            padding: 5px 8px;
            color: #e8eaf0;
        }
        QLineEdit:focus { border-color: #5a80c0; }
        QLineEdit:disabled { background-color: #1a1d23; color: #4a5060; }
        QRadioButton, QCheckBox {
            color: #c8d8e8;
            spacing: 8px;
        }
        QRadioButton::indicator, QCheckBox::indicator {
            width: 16px; height: 16px;
            border: 2px solid #3a5080;
            border-radius: 3px;
            background-color: #252932;
        }
        QRadioButton::indicator { border-radius: 8px; }
        QRadioButton::indicator:checked, QCheckBox::indicator:checked {
            background-color: #3a7fd4;
            border-color: #5a9ff4;
        }
        QPushButton#btnAnalyze {
            background-color: #2a5fa8;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            color: #ffffff;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton#btnAnalyze:hover { background-color: #3a70c0; }
        QPushButton#btnAnalyze:pressed { background-color: #1a4a88; }
        QPushButton#btnAnalyze:disabled { background-color: #2a3040; color: #5a6070; }
        QPushButton#btnRunCode {
            background-color: #2a7a40;
            border: none;
            border-radius: 5px;
            padding: 6px 16px;
            color: #ffffff;
            font-weight: bold;
        }
        QPushButton#btnRunCode:hover { background-color: #3a9a55; }
        QPushButton#btnRunCode:disabled { background-color: #2a3040; color: #5a6070; }
        QTabWidget::pane {
            border: 1px solid #2e3a50;
            border-radius: 4px;
            background-color: #1a1d23;
        }
        QTabBar::tab {
            background-color: #252932;
            color: #8899aa;
            padding: 7px 14px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #2e3a50;
            color: #c8daf0;
            border-bottom: 2px solid #5a9ff4;
        }
        QTabBar::tab:hover { color: #c8daf0; }
        QTextEdit, QPlainTextEdit {
            background-color: #12141a;
            border: 1px solid #2e3340;
            border-radius: 4px;
            color: #d8e8f8;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 12px;
        }
        #statusLabel {
            color: #7aaa88;
            font-size: 12px;
            padding: 2px 0px;
        }
        QLabel { color: #c8d8e8; }
        QSplitter::handle { background-color: #2e3340; }
        QScrollBar:vertical {
            background: #1a1d23; width: 10px; border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #3a4a60; border-radius: 5px; min-height: 20px;
        }
        QMenuBar {
            background-color: #12141a;
            color: #c8d8e8;
            border-bottom: 1px solid #2e3340;
        }
        QMenuBar::item:selected { background-color: #2e3a50; }
        QMenu {
            background-color: #252932;
            border: 1px solid #3a4055;
        }
        QMenu::item:selected { background-color: #3a5080; }
        QStatusBar { background-color: #12141a; color: #7a8a9a; font-size: 12px; }
    )");
}