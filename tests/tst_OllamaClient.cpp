#include <QtTest/QtTest>
#include "OllamaClient.h"
#include "FilterTypes.h"

/**
 * @file tst_OllamaClient.cpp
 * @brief Testy jednostkowe statycznych metod OllamaClient.
 *
 * Testuje budowanie promptów bez konieczności połączenia z serwerem.
 */

class TestOllamaClient : public QObject {
    Q_OBJECT

private slots:

    // ── Testy buildSystemPrompt ───────────────────────────────────

    void test_systemPrompt_notEmpty() {
        for (auto type : {FilterType::LowPass, FilterType::HighPass,
                          FilterType::BandPass, FilterType::BandStop,
                          FilterType::Other}) {
            QString prompt = OllamaClient::buildSystemPrompt(type);
            QVERIFY2(!prompt.isEmpty(),
                     qPrintable(QString("System prompt pusty dla typu %1")
                                .arg(filterTypeName(type))));
        }
    }

    void test_systemPrompt_containsFilterName_LP() {
        QString prompt = OllamaClient::buildSystemPrompt(FilterType::LowPass);
        QVERIFY2(prompt.contains("dolnoprzepustow", Qt::CaseInsensitive) ||
                 prompt.contains("Dolnoprzepustowy", Qt::CaseInsensitive),
                 "System prompt LP powinien zawierać nazwę filtru");
    }

    void test_systemPrompt_containsFilterName_HP() {
        QString prompt = OllamaClient::buildSystemPrompt(FilterType::HighPass);
        QVERIFY2(prompt.contains("górnoprzepustow", Qt::CaseInsensitive) ||
                 prompt.contains("Górnoprzepustowy", Qt::CaseInsensitive),
                 "System prompt HP powinien zawierać nazwę filtru");
    }

    void test_systemPrompt_containsTags() {
        QString prompt = OllamaClient::buildSystemPrompt(FilterType::LowPass);
        QVERIFY2(prompt.contains("<KOD>"),
                 "System prompt powinien informować o tagu <KOD>");
        QVERIFY2(prompt.contains("<SCHEMAT>"),
                 "System prompt powinien informować o tagu <SCHEMAT>");
        QVERIFY2(prompt.contains("<WYJAŚNIENIE>"),
                 "System prompt powinien informować o tagu <WYJAŚNIENIE>");
    }

    void test_systemPrompt_containsPolishInstruction() {
        QString prompt = OllamaClient::buildSystemPrompt(FilterType::LowPass);
        QVERIFY2(prompt.contains("polsku", Qt::CaseInsensitive),
                 "System prompt powinien nakazywać odpowiedź po polsku");
    }

    // ── Testy buildUserPrompt ─────────────────────────────────────

    void test_userPrompt_LP_KnownRC_notEmpty() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1e-7;

        FilterResult r;
        r.valid = true;
        r.frequency = 1591.5;
        r.R_calc = 1000.0;
        r.C_calc = 1e-7;

        QString prompt = OllamaClient::buildUserPrompt(p, r, true, true, true);
        QVERIFY2(!prompt.isEmpty(), "User prompt nie może być pusty");
    }

    void test_userPrompt_containsFilterType() {
        FilterParams p;
        p.type = FilterType::HighPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 4700.0;
        p.C = 1e-8;

        FilterResult r;
        r.valid = true;
        r.frequency = 3386.0;
        r.R_calc = 4700.0;
        r.C_calc = 1e-8;

        QString prompt = OllamaClient::buildUserPrompt(p, r, false, false, false);
        QVERIFY2(prompt.contains("Górnoprzepustowy", Qt::CaseInsensitive),
                 "User prompt powinien zawierać typ filtru");
    }

    void test_userPrompt_generatePlot_mentionsBode() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1e-7;

        FilterResult r;
        r.valid = true;
        r.frequency = 1591.5;

        QString prompt = OllamaClient::buildUserPrompt(p, r, true, false, false);
        QVERIFY2(prompt.contains("Bodego", Qt::CaseInsensitive) ||
                 prompt.contains("charakterystyk", Qt::CaseInsensitive),
                 "Prompt z generatePlot=true powinien wspominać o charakterystyce");
    }

    void test_userPrompt_noOptions_stillValid() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1e-7;

        FilterResult r;
        r.valid = true;
        r.frequency = 1591.5;

        // Wszystkie opcje wyłączone — prompt i tak musi być niepusty
        QString prompt = OllamaClient::buildUserPrompt(p, r, false, false, false);
        QVERIFY(!prompt.isEmpty());
    }

    void test_userPrompt_BP_KnownF_containsF1F2() {
        FilterParams p;
        p.type = FilterType::BandPass;
        p.mode = ParameterMode::KnownF;
        p.f1 = 500.0;
        p.f2 = 2000.0;
        p.impedance = 50.0;

        FilterResult r;
        r.valid = true;
        r.frequency = 1000.0;
        r.bandwidth = 1500.0;
        r.Q = 0.67;
        r.R_calc = 50.0;
        r.C_calc = 3.18e-7;
        r.R2_calc = 50.0;
        r.C2_calc = 1.59e-8;

        QString prompt = OllamaClient::buildUserPrompt(p, r, true, true, true);
        QVERIFY2(prompt.contains("f1") || prompt.contains("500"),
                 "BP prompt powinien zawierać f1");
        QVERIFY2(prompt.contains("f2") || prompt.contains("2000"),
                 "BP prompt powinien zawierać f2");
    }

    void test_userPrompt_customDescription_included() {
        FilterParams p;
        p.type = FilterType::Other;
        p.customDescription = "Filtr antyaliasingowy 44100 Hz";

        FilterResult r;
        r.valid = true;

        QString prompt = OllamaClient::buildUserPrompt(p, r, false, false, false);
        QVERIFY2(prompt.contains("44100"),
                 "Custom description powinna być zawarta w prompcie");
    }

    void test_userPrompt_allOptions_containsAllKeywords() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1e-7;

        FilterResult r;
        r.valid = true;
        r.frequency = 1591.5;

        QString prompt = OllamaClient::buildUserPrompt(p, r, true, true, true);

        // Przy wszystkich opcjach ON prompt musi zawierać żądania kodu, schematu
        bool hasPlotReq    = prompt.contains("wykres") || prompt.contains("Bode") ||
                             prompt.contains("matplotlib");
        bool hasCodeReq    = prompt.contains("kod") || prompt.contains("Python");
        bool hasSchemaReq  = prompt.contains("schemat") || prompt.contains("ASCII");

        QVERIFY2(hasPlotReq,   "Prompt powinien zawierać żądanie wykresu");
        QVERIFY2(hasCodeReq,   "Prompt powinien zawierać żądanie kodu");
        QVERIFY2(hasSchemaReq, "Prompt powinien zawierać żądanie schematu");
    }

    // ── Test filterTypeName ───────────────────────────────────────

    void test_filterTypeName_allTypes() {
        QCOMPARE(filterTypeName(FilterType::LowPass),  QString("Dolnoprzepustowy"));
        QCOMPARE(filterTypeName(FilterType::HighPass), QString("Górnoprzepustowy"));
        QCOMPARE(filterTypeName(FilterType::BandPass), QString("Pasmo-przepustowy"));
        QCOMPARE(filterTypeName(FilterType::BandStop), QString("Pasmo-zaporowy"));
        QCOMPARE(filterTypeName(FilterType::Other),    QString("Inny"));
    }
};

int runTestOllamaClient(int argc, char *argv[]) {
    TestOllamaClient tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "tst_OllamaClient.moc"