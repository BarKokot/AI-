#include <QtTest/QtTest>
#include "FilterCalculator.h"
#include "FilterTypes.h"
#include <cmath>

/**
 * @file tst_FilterCalculator.cpp
 * @brief Testy jednostkowe klasy FilterCalculator.
 *
 * Weryfikuje poprawność obliczeń matematycznych dla wszystkich
 * typów filtrów RC i obu trybów parametrów.
 */

static constexpr double PI = 3.14159265358979323846;
static constexpr double EPS = 1e-6; ///< Tolerancja porównań zmiennoprzecinkowych

class TestFilterCalculator : public QObject {
    Q_OBJECT

private slots:

    // ── Testy statycznych metod pomocniczych ──────────────────────

    /**
     * @brief Test obliczania częstotliwości granicznej z R i C.
     * Wzór: f = 1 / (2π·R·C)
     */
    void test_calcCutoffFrequency_basic() {
        double R = 1000.0;   // 1 kΩ
        double C = 1.0e-7;   // 100 nF
        double expected = 1.0 / (2.0 * PI * R * C); // ≈ 1591.5 Hz
        double actual = FilterCalculator::calcCutoffFrequency(R, C);
        QVERIFY2(std::abs(actual - expected) < EPS * expected,
                 "calcCutoffFrequency: niepoprawny wynik dla R=1kΩ, C=100nF");
    }

    /**
     * @brief Test granicznego przypadku — zerowe wejście.
     */
    void test_calcCutoffFrequency_zeroR() {
        QCOMPARE(FilterCalculator::calcCutoffFrequency(0.0, 1e-6), 0.0);
    }

    void test_calcCutoffFrequency_zeroC() {
        QCOMPARE(FilterCalculator::calcCutoffFrequency(1000.0, 0.0), 0.0);
    }

    /**
     * @brief Test obliczania pojemności z f i R.
     */
    void test_calcCapacitance_basic() {
        double f = 1000.0;  // 1 kHz
        double R = 1000.0;  // 1 kΩ
        double expected = 1.0 / (2.0 * PI * f * R); // ≈ 159.15 nF
        double actual = FilterCalculator::calcCapacitance(f, R);
        QVERIFY2(std::abs(actual - expected) < EPS * expected,
                 "calcCapacitance: niepoprawny wynik");
    }

    /**
     * @brief Test obliczania rezystancji z f i C.
     */
    void test_calcResistance_basic() {
        double f = 1000.0;
        double C = 1.0e-7;
        double expected = 1.0 / (2.0 * PI * f * C);
        double actual = FilterCalculator::calcResistance(f, C);
        QVERIFY2(std::abs(actual - expected) < EPS * expected,
                 "calcResistance: niepoprawny wynik");
    }

    /**
     * @brief Weryfikacja spójności: calcCapacitance i calcCutoffFrequency
     * są operacjami odwrotnymi.
     */
    void test_RC_roundtrip() {
        double f = 2500.0;
        double R = 4700.0;
        double C = FilterCalculator::calcCapacitance(f, R);
        double f_back = FilterCalculator::calcCutoffFrequency(R, C);
        QVERIFY2(std::abs(f_back - f) < 1e-6 * f,
                 "Roundtrip RC: calcCapacitance ∘ calcCutoffFrequency ≠ id");
    }

    /**
     * @brief Test obliczania dobroci Q.
     */
    void test_calcQFactor() {
        QCOMPARE(FilterCalculator::calcQFactor(1000.0, 100.0), 10.0);
        QCOMPARE(FilterCalculator::calcQFactor(500.0, 0.0), 0.0); // bandwidth=0
    }

    // ── Testy formatowania ────────────────────────────────────────

    void test_formatEngineering_kHz() {
        QString s = FilterCalculator::formatEngineering(1500.0, "Hz");
        QVERIFY2(s.contains("kHz") || s.contains("k"),
                 "formatEngineering: brak prefiksu 'k' dla 1500 Hz");
    }

    void test_formatEngineering_nF() {
        QString s = FilterCalculator::formatEngineering(1.5e-9, "F");
        QVERIFY2(s.contains("n"),
                 "formatEngineering: brak prefiksu 'n' dla 1.5 nF");
    }

    void test_formatEngineering_zero() {
        QString s = FilterCalculator::formatEngineering(0.0, "Ω");
        QVERIFY(!s.isEmpty());
    }

    // ── Testy walidacji ───────────────────────────────────────────

    void test_validate_validRC_LP() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1e-7;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(ok, qPrintable(QString("Walidacja LP RC powinna przejść: ") + msg));
    }

    void test_validate_invalidRC_zeroR() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 0.0;
        p.C = 1e-7;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(!ok, "Walidacja z R=0 powinna zwrócić błąd");
    }

    void test_validate_invalidRC_zeroC() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 0.0;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(!ok, "Walidacja z C=0 powinna zwrócić błąd");
    }

    void test_validate_validF_LP() {
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownF;
        p.f0 = 1000.0;
        p.impedance = 50.0;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(ok, qPrintable(msg));
    }

    void test_validate_invalidF_zeroF0() {
        FilterParams p;
        p.type = FilterType::HighPass;
        p.mode = ParameterMode::KnownF;
        p.f0 = 0.0;
        p.impedance = 50.0;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(!ok, "f0=0 powinna zwrócić błąd walidacji");
    }

    void test_validate_BP_f1_ge_f2() {
        FilterParams p;
        p.type = FilterType::BandPass;
        p.mode = ParameterMode::KnownF;
        p.f1 = 2000.0;
        p.f2 = 500.0;  // f1 > f2 — błąd
        p.impedance = 50.0;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(!ok, "f1 > f2 dla BP powinna zwrócić błąd");
    }

    void test_validate_BP_RC_missingR2() {
        FilterParams p;
        p.type = FilterType::BandPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1e-7;
        p.R2 = 0.0;  // brak R2
        p.C2 = 1e-8;
        auto [ok, msg] = FilterCalculator::validate(p);
        QVERIFY2(!ok, "Brak R2 dla BP RC powinien zwrócić błąd");
    }

    // ── Testy obliczeniowe LP ─────────────────────────────────────

    void test_calculate_LP_KnownRC() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 1000.0;
        p.C = 1.0e-7;

        FilterResult r = calc.calculate(p);

        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QVERIFY2(r.R_calc == 1000.0, "R_calc musi być równe R");
        QVERIFY2(r.C_calc == 1.0e-7, "C_calc musi być równe C");

        double expected_f = 1.0 / (2.0 * PI * 1000.0 * 1e-7);
        QVERIFY2(std::abs(r.frequency - expected_f) < EPS * expected_f,
                 "LP KnownRC: niepoprawna częstotliwość");
        QVERIFY(!r.description.isEmpty());
    }

    void test_calculate_LP_KnownF() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownF;
        p.f0 = 1000.0;
        p.impedance = 50.0;

        FilterResult r = calc.calculate(p);

        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QCOMPARE(r.frequency, 1000.0);
        QCOMPARE(r.R_calc, 50.0);

        double expected_C = 1.0 / (2.0 * PI * 1000.0 * 50.0);
        QVERIFY2(std::abs(r.C_calc - expected_C) < EPS * expected_C,
                 "LP KnownF: niepoprawna pojemność");
    }

    // ── Testy obliczeniowe HP ─────────────────────────────────────

    void test_calculate_HP_KnownRC() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::HighPass;
        p.mode = ParameterMode::KnownRC;
        p.R = 4700.0;
        p.C = 1.0e-8;

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, qPrintable(r.errorMessage));

        double expected_f = 1.0 / (2.0 * PI * 4700.0 * 1e-8);
        QVERIFY2(std::abs(r.frequency - expected_f) < EPS * expected_f,
                 "HP KnownRC: niepoprawna częstotliwość");
    }

    void test_calculate_HP_KnownF() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::HighPass;
        p.mode = ParameterMode::KnownF;
        p.f0 = 5000.0;
        p.impedance = 75.0;

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QCOMPARE(r.frequency, 5000.0);
        QCOMPARE(r.R_calc, 75.0);
    }

    // ── Testy obliczeniowe BP ─────────────────────────────────────

    void test_calculate_BP_KnownRC() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::BandPass;
        p.mode = ParameterMode::KnownRC;
        p.R  = 1000.0; p.C  = 1e-7;  // f1 ≈ 1592 Hz
        p.R2 = 2200.0; p.C2 = 1e-8;  // f2 ≈ 7234 Hz

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QVERIFY2(r.bandwidth > 0, "Pasmo musi być > 0");
        QVERIFY2(r.Q > 0, "Q musi być > 0");

        double f1 = FilterCalculator::calcCutoffFrequency(p.R,  p.C);
        double f2 = FilterCalculator::calcCutoffFrequency(p.R2, p.C2);
        double expected_f0 = std::sqrt(f1 * f2);
        QVERIFY2(std::abs(r.frequency - expected_f0) < EPS * expected_f0,
                 "BP: niepoprawna f środkowa");
    }

    void test_calculate_BP_KnownF() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::BandPass;
        p.mode = ParameterMode::KnownF;
        p.f1 = 500.0;
        p.f2 = 2000.0;
        p.impedance = 50.0;

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QVERIFY2(std::abs(r.bandwidth - 1500.0) < 0.1, "BP: pasmo musi być 1500 Hz");
        QVERIFY2(r.frequency > 0, "BP: f środkowa musi być > 0");
    }

    // ── Testy obliczeniowe BS ─────────────────────────────────────

    void test_calculate_BS_KnownRC() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::BandStop;
        p.mode = ParameterMode::KnownRC;
        p.R  = 1000.0; p.C  = 1e-7;
        p.R2 = 2200.0; p.C2 = 1e-8;

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QVERIFY2(r.bandwidth > 0, "BS: pasmo musi być > 0");
    }

    void test_calculate_BS_KnownF() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::BandStop;
        p.mode = ParameterMode::KnownF;
        p.f1 = 1000.0;
        p.f2 = 3000.0;
        p.impedance = 50.0;

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, qPrintable(r.errorMessage));
        QVERIFY2(r.frequency > 0, "BS: f środkowa musi być > 0");
    }

    // ── Testy filtra "Inne" ───────────────────────────────────────

    void test_calculate_Other() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::Other;
        p.customDescription = "Filtr testowy";

        FilterResult r = calc.calculate(p);
        QVERIFY2(r.valid, "Filtr 'Inne' powinien zwrócić valid=true");
        QVERIFY(!r.description.isEmpty());
    }

    // ── Testy błędnych parametrów ─────────────────────────────────

    void test_calculate_invalid_returnsError() {
        FilterCalculator calc;
        FilterParams p;
        p.type = FilterType::LowPass;
        p.mode = ParameterMode::KnownRC;
        p.R = -100.0;  // niepoprawne
        p.C = 1e-7;

        FilterResult r = calc.calculate(p);
        QVERIFY2(!r.valid, "Ujemne R powinno zwrócić valid=false");
        QVERIFY(!r.errorMessage.isEmpty());
    }

    // ── Test Q dla różnych konfiguracji BP ───────────────────────

    void test_Q_increases_with_narrower_band() {
        FilterCalculator calc;

        // Szerokie pasmo
        FilterParams p1;
        p1.type = FilterType::BandPass;
        p1.mode = ParameterMode::KnownF;
        p1.f1 = 100.0; p1.f2 = 5000.0;
        p1.impedance = 50.0;
        FilterResult r1 = calc.calculate(p1);

        // Wąskie pasmo
        FilterParams p2;
        p2.type = FilterType::BandPass;
        p2.mode = ParameterMode::KnownF;
        p2.f1 = 900.0; p2.f2 = 1100.0;
        p2.impedance = 50.0;
        FilterResult r2 = calc.calculate(p2);

        QVERIFY2(r2.Q > r1.Q,
                 "Wąskie pasmo powinno dać wyższe Q niż szerokie");
    }
};

QTEST_APPLESS_MAIN(TestFilterCalculator)
#include "tst_FilterCalculator.moc"
