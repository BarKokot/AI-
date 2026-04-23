#include <QtTest/QtTest>
#include "CodeExecutor.h"

// Makro pomocnicze (Qt nie ma QVERIFY_EXCEPTION_THROWN_NO_THROW poza Qt6.3+)
#ifndef QVERIFY_EXCEPTION_THROWN_NO_THROW
#define QVERIFY_EXCEPTION_THROWN_NO_THROW(expr) \
    try { expr; QVERIFY(true); } \
    catch (...) { QFAIL("Nieoczekiwany wyjątek"); }
#endif

/**
 * @file tst_CodeExecutor.cpp
 * @brief Testy jednostkowe klasy CodeExecutor.
 *
 * Testuje ekstrakcję kodu, schematów i wyjaśnień z odpowiedzi LLM.
 */

class TestCodeExecutor : public QObject {
    Q_OBJECT

private slots:

    // ── Testy extractCode ─────────────────────────────────────────

    void test_extractCode_withKodTags() {
        QString response = "Oto kod:\n<KOD>\nimport numpy as np\nprint('hello')\n</KOD>\nKoniec.";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(code.contains("import numpy"), "extractCode: powinien znaleźć kod między <KOD>");
        QVERIFY2(code.contains("print"), "extractCode: powinien zawierać print()");
    }

    void test_extractCode_withPythonFence() {
        QString response = "```python\nimport matplotlib\nx = 1\n```";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(code.contains("import matplotlib"),
                 "extractCode: powinien znaleźć kod w bloku ```python");
    }

    void test_extractCode_withGenericFence() {
        QString response = "```\nx = 42\nprint(x)\n```";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(code.contains("x = 42"),
                 "extractCode: powinien znaleźć kod w bloku ```");
    }

    void test_extractCode_noTags_returnsEmpty() {
        QString response = "Nie ma tutaj żadnego kodu, tylko tekst.";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(code.isEmpty(),
                 "extractCode: bez tagów powinien zwrócić pusty string");
    }

    void test_extractCode_emptyInput() {
        QVERIFY(CodeExecutor::extractCode("").isEmpty());
    }

    void test_extractCode_preferKodTagsOverFence() {
        // Jeśli są tagi <KOD>, powinny mieć priorytet nad ```
        QString response = "<KOD>\nkod_z_tagów = True\n</KOD>\n```python\nkod_z_fence = True\n```";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(code.contains("kod_z_tagów"),
                 "extractCode: tagi <KOD> powinny mieć priorytet");
        QVERIFY2(!code.contains("kod_z_fence"),
                 "extractCode: nie powinien zwracać kodu z fence gdy są tagi");
    }

    void test_extractCode_multilineCode() {
        QString response = "<KOD>\n"
                           "import numpy as np\n"
                           "import matplotlib.pyplot as plt\n"
                           "f = np.logspace(1, 6, 1000)\n"
                           "plt.semilogx(f, 20*np.log10(1/(1+f/1000)))\n"
                           "plt.show()\n"
                           "</KOD>";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(code.contains("logspace"), "extractCode: powinien zachować wieloliniowy kod");
        QVERIFY2(code.contains("plt.show"),  "extractCode: powinien zachować plt.show()");
    }

    void test_extractCode_trimmed() {
        QString response = "<KOD>\n\n   x = 1   \n\n</KOD>";
        QString code = CodeExecutor::extractCode(response);
        QVERIFY2(!code.startsWith('\n'), "extractCode: wynik nie powinien zaczynać się od \\n");
        QVERIFY2(!code.endsWith('\n'),   "extractCode: wynik nie powinien kończyć się \\n");
    }

    // ── Testy extractSchematic ────────────────────────────────────

    void test_extractSchematic_withTags() {
        QString response = "Schemat:\n<SCHEMAT>\n"
                           "Vin --[R]--+-- Vout\n"
                           "          |\n"
                           "         [C]\n"
                           "          |\n"
                           "         GND\n"
                           "</SCHEMAT>\nKoniec.";
        QString schema = CodeExecutor::extractSchematic(response);
        QVERIFY2(schema.contains("Vin"), "extractSchematic: powinien znaleźć schemat");
        QVERIFY2(schema.contains("GND"), "extractSchematic: powinien zawierać GND");
    }

    void test_extractSchematic_noTags_returnsEmpty() {
        QString response = "Brak schematu w odpowiedzi.";
        QVERIFY(CodeExecutor::extractSchematic(response).isEmpty());
    }

    void test_extractSchematic_emptyInput() {
        QVERIFY(CodeExecutor::extractSchematic("").isEmpty());
    }

    void test_extractSchematic_preservesAsciiArt() {
        QString ascii =
            " o--[R1]--o--[R2]--o\n"
            "          |        |\n"
            "         [C1]     [C2]\n"
            "          |        |\n"
            "         GND      GND\n";
        QString response = "<SCHEMAT>" + ascii + "</SCHEMAT>";
        QString schema = CodeExecutor::extractSchematic(response);
        QVERIFY2(schema.contains("[R1]"), "Schemat powinien zachować ASCII art");
        QVERIFY2(schema.contains("[C2]"), "Schemat powinien zachować [C2]");
    }

    // ── Testy extractExplanation ──────────────────────────────────

    void test_extractExplanation_withTags() {
        QString response = "<WYJAŚNIENIE>Filtr dolnoprzepustowy przepuszcza "
                           "sygnały poniżej częstotliwości granicznej.</WYJAŚNIENIE>";
        QString expl = CodeExecutor::extractExplanation(response);
        QVERIFY2(expl.contains("dolnoprzepustowy"),
                 "extractExplanation: powinien znaleźć wyjaśnienie");
    }

    void test_extractExplanation_noTags_returnsEmpty() {
        QVERIFY(CodeExecutor::extractExplanation("Brak wyjaśnienia.").isEmpty());
    }

    void test_extractExplanation_emptyInput() {
        QVERIFY(CodeExecutor::extractExplanation("").isEmpty());
    }

    // ── Testy z pełną odpowiedzią LLM ────────────────────────────

    void test_fullResponse_allTagsPresent() {
        QString response =
            "Oto analiza filtru.\n"
            "<KOD>\n"
            "import numpy as np\n"
            "print('bode')\n"
            "</KOD>\n"
            "<SCHEMAT>\n"
            "Vin--[R]--+--Vout\n"
            "          |\n"
            "         [C]\n"
            "          |\n"
            "         GND\n"
            "</SCHEMAT>\n"
            "<WYJAŚNIENIE>\n"
            "Filtr RC pierwszego rzędu.\n"
            "</WYJAŚNIENIE>\n";

        QVERIFY(!CodeExecutor::extractCode(response).isEmpty());
        QVERIFY(!CodeExecutor::extractSchematic(response).isEmpty());
        QVERIFY(!CodeExecutor::extractExplanation(response).isEmpty());
    }

    void test_fullResponse_onlyCode() {
        QString response = "Odpowiedź:\n<KOD>\nx=1\n</KOD>";
        QVERIFY(!CodeExecutor::extractCode(response).isEmpty());
        QVERIFY(CodeExecutor::extractSchematic(response).isEmpty());
        QVERIFY(CodeExecutor::extractExplanation(response).isEmpty());
    }

    // ── Test isPythonAvailable (nie blokujący) ────────────────────

    void test_isPythonAvailable_doesNotCrash() {
        // Nie sprawdzamy wartości — tylko że metoda nie rzuca wyjątku
        bool result = false;
        QVERIFY_EXCEPTION_THROWN_NO_THROW(result = CodeExecutor::isPythonAvailable());
        Q_UNUSED(result);
    }
};

int runTestCodeExecutor(int argc, char *argv[]) {
    TestCodeExecutor tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "tst_CodeExecutor.moc"