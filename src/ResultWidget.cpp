#include "ResultWidget.h"
#include "CodeExecutor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPixmap>

/**
 * @file ResultWidget.cpp
 * @brief Implementacja widgetu wyników z zakładką wykresu PNG.
 */

ResultWidget::ResultWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void ResultWidget::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);

    // ── Status ──
    m_statusLabel = new QLabel("Gotowy.", this);
    m_statusLabel->setObjectName("statusLabel");
    layout->addWidget(m_statusLabel);

    // ── Zakładki ──
    m_tabs = new QTabWidget(this);

    // 1. Obliczenia
    m_calcResultEdit = new QTextEdit(this);
    m_calcResultEdit->setReadOnly(true);
    m_calcResultEdit->setFont(QFont("Consolas", 11));
    m_tabs->addTab(m_calcResultEdit, "📊 Obliczenia");

    // 2. Kod Python
    auto* codeContainer = new QWidget(this);
    auto* codeLayout    = new QVBoxLayout(codeContainer);
    codeLayout->setContentsMargins(4, 4, 4, 4);
    m_codeEdit = new QPlainTextEdit(codeContainer);
    m_codeEdit->setReadOnly(true);
    m_codeEdit->setFont(QFont("Consolas", 10));
    m_btnRunCode = new QPushButton("▶  Uruchom kod Python (generuj wykres)", codeContainer);
    m_btnRunCode->setObjectName("btnRunCode");
    m_btnRunCode->setEnabled(false);
    codeLayout->addWidget(m_codeEdit);
    codeLayout->addWidget(m_btnRunCode);
    m_tabs->addTab(codeContainer, "🐍 Kod Python");

    // 3. Schemat
    m_schematicEdit = new QPlainTextEdit(this);
    m_schematicEdit->setReadOnly(true);
    m_schematicEdit->setFont(QFont("Consolas", 11));
    m_tabs->addTab(m_schematicEdit, "🔌 Schemat");

    // 4. Wyjaśnienie
    m_explanationEdit = new QTextEdit(this);
    m_explanationEdit->setReadOnly(true);
    m_tabs->addTab(m_explanationEdit, "ℹ️ Wyjaśnienie");

    // 5. Wykres PNG — osadzony w zakładce
    m_plotScroll = new QScrollArea(this);
    m_plotScroll->setWidgetResizable(true);
    m_plotScroll->setAlignment(Qt::AlignCenter);
    m_plotScroll->setStyleSheet("background-color: #12141a; border: none;");

    m_plotLabel = new QLabel(this);
    m_plotLabel->setAlignment(Qt::AlignCenter);
    m_plotLabel->setText(
        "<span style='color:#4a5a6a; font-size:14px;'>"
        "Wykres pojawi się tutaj po uruchomieniu kodu Python"
        "</span>"
    );
    m_plotLabel->setMinimumSize(400, 300);
    m_plotScroll->setWidget(m_plotLabel);
    m_tabs->addTab(m_plotScroll, "📈 Wykres");

    // 6. Wyjście Python
    m_execOutputEdit = new QTextEdit(this);
    m_execOutputEdit->setReadOnly(true);
    m_execOutputEdit->setFont(QFont("Consolas", 10));
    m_tabs->addTab(m_execOutputEdit, "💻 Wyjście");

    layout->addWidget(m_tabs);

    connect(m_btnRunCode, &QPushButton::clicked, this, [this]() {
        if (!m_currentCode.isEmpty())
            emit executeCodeRequested(m_currentCode);
    });
}

void ResultWidget::setStatus(const QString& message, bool busy) {
    m_statusLabel->setText(busy ? QString("⏳ %1").arg(message) : message);
}

void ResultWidget::clearAll() {
    m_calcResultEdit->clear();
    m_codeEdit->clear();
    m_schematicEdit->clear();
    m_explanationEdit->clear();
    m_execOutputEdit->clear();
    m_plotLabel->setText(
        "<span style='color:#4a5a6a; font-size:14px;'>"
        "Wykres pojawi się tutaj po uruchomieniu kodu Python"
        "</span>"
    );
    m_btnRunCode->setEnabled(false);
    m_currentCode.clear();
    setStatus("Gotowy.");
}

void ResultWidget::showCalculationResult(const FilterResult& result) {
    if (!result.valid) {
        m_calcResultEdit->setHtml(
            QString("<span style='color:#e74c3c;font-weight:bold;'>Błąd:</span> %1")
            .arg(result.errorMessage)
        );
        return;
    }
    m_calcResultEdit->setPlainText(result.description);
    m_tabs->setCurrentIndex(0);
}

void ResultWidget::showLLMResponse(const QString& response, bool success) {
    if (!success) {
        m_calcResultEdit->append(
            QString("\n\n⚠️ Błąd modelu językowego:\n%1").arg(response)
        );
        setStatus("Błąd odpowiedzi modelu.");
        return;
    }

    QString code    = CodeExecutor::extractCode(response);
    QString schema  = CodeExecutor::extractSchematic(response);
    QString explain = CodeExecutor::extractExplanation(response);

    if (code.isEmpty() && schema.isEmpty() && explain.isEmpty())
        explain = response;

    if (!code.isEmpty()) {
        // Wstrzyknij zapis do pliku PNG zamiast plt.show()
        QString modifiedCode = CodeExecutor::injectPlotSave(code);
        m_codeEdit->setPlainText(modifiedCode);
        m_currentCode = modifiedCode;
        m_btnRunCode->setEnabled(true);
        m_tabs->setCurrentIndex(1); // pokaż zakładkę Kod
    }
    if (!schema.isEmpty())
        m_schematicEdit->setPlainText(schema);
    if (!explain.isEmpty())
        m_explanationEdit->setPlainText(explain);

    setStatus("✅ Odpowiedź modelu otrzymana. Kliknij '▶ Uruchom kod Python' aby zobaczyć wykres.");
}

void ResultWidget::showPlot(const QString& imagePath) {
    QPixmap pix(imagePath);
    if (pix.isNull()) {
        m_plotLabel->setText(
            "<span style='color:#e74c3c;'>Nie można wczytać wykresu.<br>"
            "Sprawdź czy matplotlib jest zainstalowany.</span>"
        );
        return;
    }
    // Skaluj do szerokości zakładki zachowując proporcje
    int maxW = m_plotScroll->width()  - 20;
    int maxH = m_plotScroll->height() - 20;
    if (maxW < 400) maxW = 800;
    if (maxH < 300) maxH = 600;
    m_plotLabel->setPixmap(
        pix.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    );
    m_tabs->setCurrentIndex(4); // przejdź do zakładki Wykres
}

void ResultWidget::showExecutionResult(const QString& output, bool success) {
    m_execOutputEdit->setPlainText(output);
    setStatus(success ? "✅ Kod wykonany pomyślnie." : "⚠️ Błąd wykonania kodu.");
}
