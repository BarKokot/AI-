#include "CodeExecutor.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUuid>

/**
 * @file CodeExecutor.cpp
 * @brief Implementacja klasy CodeExecutor z obsługą zapisu wykresu do PNG.
 */

CodeExecutor::CodeExecutor(QObject* parent) : QObject(parent) {}

// ── Ekstrakcja między tagami ──────────────────────────────────────────────────

QString CodeExecutor::extractBetweenTags(const QString& text,
                                          const QString& openTag,
                                          const QString& closeTag) {
    int start = text.indexOf(openTag);
    if (start == -1) return {};
    start += openTag.length();
    int end = text.indexOf(closeTag, start);
    if (end == -1) return {};
    return text.mid(start, end - start).trimmed();
}

QString CodeExecutor::extractCode(const QString& llmResponse) {
    QString code = extractBetweenTags(llmResponse, "<KOD>", "</KOD>");
    if (!code.isEmpty()) return code;
    code = extractBetweenTags(llmResponse, "```python", "```");
    if (!code.isEmpty()) return code;
    return extractBetweenTags(llmResponse, "```", "```");
}

QString CodeExecutor::extractSchematic(const QString& llmResponse) {
    return extractBetweenTags(llmResponse, "<SCHEMAT>", "</SCHEMAT>");
}

QString CodeExecutor::extractExplanation(const QString& llmResponse) {
    return extractBetweenTags(llmResponse, "<WYJAŚNIENIE>", "</WYJAŚNIENIE>");
}

// ── Wstrzykiwanie zapisu PNG ──────────────────────────────────────────────────

QString CodeExecutor::injectPlotSave(const QString& code) {
    // Ścieżka do pliku PNG w katalogu tymczasowym
    QString pngPath = QDir(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
    ).filePath(QString("filter_plot_%1.png")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8)));

    // Normalizuj separatory dla Windows
    pngPath.replace("\\", "/");

    QString modified = code;

    // Upewnij się że matplotlib używa backendu Agg (bez GUI) przed importem pyplot
    if (!modified.contains("matplotlib.use") && !modified.contains("import matplotlib\n")) {
        modified.prepend("import matplotlib\nmatplotlib.use('Agg')\n");
    } else if (modified.contains("import matplotlib\n") &&
               !modified.contains("matplotlib.use")) {
        modified.replace("import matplotlib\n",
                         "import matplotlib\nmatplotlib.use('Agg')\n");
    }

    // Zamień plt.show() na plt.savefig() + drukowanie ścieżki
    QString saveBlock = QString(
        "plt.tight_layout()\n"
        "plt.savefig('%1', dpi=120, bbox_inches='tight')\n"
        "print('PLOT_PATH:%1')\n"
    ).arg(pngPath);

    if (modified.contains("plt.show()")) {
        modified.replace("plt.show()", saveBlock);
    } else {
        // Dodaj na końcu jeśli nie ma plt.show()
        modified += "\n" + saveBlock;
    }

    return modified;
}

// ── Sprawdzanie Pythona ───────────────────────────────────────────────────────

bool CodeExecutor::isPythonAvailable() {
    QProcess p;
#ifdef Q_OS_WIN
    p.start("python", {"--version"});
#else
    p.start("python3", {"--version"});
#endif
    p.waitForFinished(3000);
    return (p.exitCode() == 0);
}

// ── Wykonanie kodu ────────────────────────────────────────────────────────────

void CodeExecutor::executeCode(const QString& pythonCode) {
    if (pythonCode.isEmpty()) {
        emit executionError("Brak kodu do wykonania.");
        return;
    }

    QString tempDir   = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_tempFilePath    = QDir(tempDir).filePath(
        QString("filter_%1.py")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8))
    );

    QFile file(m_tempFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit executionError(QString("Nie można utworzyć pliku tymczasowego: %1")
                            .arg(m_tempFilePath));
        return;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << pythonCode;
    file.close();

    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CodeExecutor::onProcessFinished);

#ifdef Q_OS_WIN
    m_process->start("python", {m_tempFilePath});
#else
    m_process->start("python3", {m_tempFilePath});
#endif

    if (!m_process->waitForStarted(5000)) {
        emit executionError(
            "Nie można uruchomić interpretera Python.\n"
            "Upewnij się, że Python 3 jest zainstalowany i dostępny w PATH."
        );
    }
}

void CodeExecutor::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    QString output = QString::fromUtf8(m_process->readAll());
    QFile::remove(m_tempFilePath);

    if (status == QProcess::CrashExit) {
        emit executionFinished("Proces Python zakończył się niespodziewanie.\n" + output, false);
        return;
    }
    if (exitCode != 0) {
        emit executionFinished(
            QString("Błąd wykonania (kod %1):\n%2").arg(exitCode).arg(output), false
        );
        return;
    }
    emit executionFinished(output, true);
}
