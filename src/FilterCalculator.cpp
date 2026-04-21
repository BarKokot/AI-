#include "FilterCalculator.h"
#include <QString>
#include <cmath>

/**
 * @file FilterCalculator.cpp
 * @brief Implementacja klasy FilterCalculator.
 */

static constexpr double PI = 3.14159265358979323846;

double FilterCalculator::calcCutoffFrequency(double R, double C) {
    if (R <= 0.0 || C <= 0.0) return 0.0;
    return 1.0 / (2.0 * PI * R * C);
}

double FilterCalculator::calcCapacitance(double f, double R) {
    if (f <= 0.0 || R <= 0.0) return 0.0;
    return 1.0 / (2.0 * PI * f * R);
}

double FilterCalculator::calcResistance(double f, double C) {
    if (f <= 0.0 || C <= 0.0) return 0.0;
    return 1.0 / (2.0 * PI * f * C);
}

double FilterCalculator::calcQFactor(double f0, double bandwidth) {
    if (bandwidth <= 0.0) return 0.0;
    return f0 / bandwidth;
}

QString FilterCalculator::formatEngineering(double value, const QString& unit) {
    if (value == 0.0) return QString("0 %1").arg(unit);

    struct Prefix { double factor; const char* symbol; };
    static const Prefix prefixes[] = {
        {1e-12, "p"}, {1e-9, "n"}, {1e-6, "μ"}, {1e-3, "m"},
        {1.0, ""}, {1e3, "k"}, {1e6, "M"}, {1e9, "G"}
    };

    double abs_val = std::abs(value);
    const Prefix* chosen = &prefixes[4];
    for (int i = 7; i >= 0; --i) {
        if (abs_val >= prefixes[i].factor * 0.999) {
            chosen = &prefixes[i];
            break;
        }
    }

    double scaled = value / chosen->factor;
    return QString("%1 %2%3").arg(scaled, 0, 'g', 4).arg(chosen->symbol).arg(unit);
}

std::pair<bool, QString> FilterCalculator::validate(const FilterParams& params) {
    if (params.mode == ParameterMode::KnownRC) {
        if (params.R <= 0.0)
            return {false, "Rezystancja R musi być większa od zera."};
        if (params.C <= 0.0)
            return {false, "Pojemność C musi być większa od zera."};
        if ((params.type == FilterType::BandPass || params.type == FilterType::BandStop)) {
            if (params.R2 <= 0.0)
                return {false, "Rezystancja R2 musi być większa od zera dla filtrów BP/BS."};
            if (params.C2 <= 0.0)
                return {false, "Pojemność C2 musi być większa od zera dla filtrów BP/BS."};
        }
    } else {
        if (params.type == FilterType::BandPass || params.type == FilterType::BandStop) {
            if (params.f1 <= 0.0 || params.f2 <= 0.0)
                return {false, "Częstotliwości f1 i f2 muszą być większe od zera."};
            if (params.f1 >= params.f2)
                return {false, "Częstotliwość f1 musi być mniejsza od f2."};
        } else {
            if (params.f0 <= 0.0)
                return {false, "Częstotliwość graniczna musi być większa od zera."};
        }
        if (params.impedance <= 0.0)
            return {false, "Impedancja charakterystyczna musi być większa od zera."};
    }
    return {true, ""};
}

FilterResult FilterCalculator::calculate(const FilterParams& params) {
    auto [valid, err] = validate(params);
    if (!valid) {
        FilterResult r;
        r.valid = false;
        r.errorMessage = err;
        return r;
    }

    switch (params.type) {
        case FilterType::LowPass:  return calculateLowPass(params);
        case FilterType::HighPass: return calculateHighPass(params);
        case FilterType::BandPass: return calculateBandPass(params);
        case FilterType::BandStop: return calculateBandStop(params);
        case FilterType::Other:    return calculateOther(params);
    }
    FilterResult r;
    r.valid = false;
    r.errorMessage = "Nieznany typ filtru.";
    return r;
}

// ──────────────────── LOW PASS ────────────────────

FilterResult FilterCalculator::calculateLowPass(const FilterParams& p) {
    return (p.mode == ParameterMode::KnownRC) ? calcFromRC_LP(p) : calcFromF_LP(p);
}

FilterResult FilterCalculator::calcFromRC_LP(const FilterParams& p) {
    FilterResult r;
    r.R_calc = p.R;
    r.C_calc = p.C;
    r.frequency = calcCutoffFrequency(p.R, p.C);
    r.valid = true;
    r.description = QString(
        "Filtr dolnoprzepustowy RC 1. rzędu\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "R  = %1\n"
        "C  = %2\n"
        "f₀ = %3\n\n"
        "Charakterystyka:\n"
        "• Tłumienie: -20 dB/dek powyżej f₀\n"
        "• Przesunięcie fazy: -45° przy f₀\n"
        "• Wzmocnienie przy f₀: -3 dB (≈ 0.707)"
    ).arg(formatEngineering(p.R, "Ω"))
     .arg(formatEngineering(p.C, "F"))
     .arg(formatEngineering(r.frequency, "Hz"));
    return r;
}

FilterResult FilterCalculator::calcFromF_LP(const FilterParams& p) {
    FilterResult r;
    r.frequency = p.f0;
    r.R_calc = p.impedance;
    r.C_calc = calcCapacitance(p.f0, p.impedance);
    r.valid = true;
    r.description = QString(
        "Filtr dolnoprzepustowy RC 1. rzędu\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "Wymagana f₀ = %1\n"
        "Impedancja  = %2\n\n"
        "Dobrane elementy:\n"
        "R = %3\n"
        "C = %4\n\n"
        "• Tłumienie: -20 dB/dek powyżej f₀"
    ).arg(formatEngineering(p.f0, "Hz"))
     .arg(formatEngineering(p.impedance, "Ω"))
     .arg(formatEngineering(r.R_calc, "Ω"))
     .arg(formatEngineering(r.C_calc, "F"));
    return r;
}

// ──────────────────── HIGH PASS ────────────────────

FilterResult FilterCalculator::calculateHighPass(const FilterParams& p) {
    return (p.mode == ParameterMode::KnownRC) ? calcFromRC_HP(p) : calcFromF_HP(p);
}

FilterResult FilterCalculator::calcFromRC_HP(const FilterParams& p) {
    FilterResult r;
    r.R_calc = p.R;
    r.C_calc = p.C;
    r.frequency = calcCutoffFrequency(p.R, p.C);
    r.valid = true;
    r.description = QString(
        "Filtr górnoprzepustowy RC 1. rzędu\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "R  = %1\n"
        "C  = %2\n"
        "f₀ = %3\n\n"
        "Charakterystyka:\n"
        "• Tłumienie: -20 dB/dek poniżej f₀\n"
        "• Przesunięcie fazy: +45° przy f₀\n"
        "• Wzmocnienie przy f₀: -3 dB"
    ).arg(formatEngineering(p.R, "Ω"))
     .arg(formatEngineering(p.C, "F"))
     .arg(formatEngineering(r.frequency, "Hz"));
    return r;
}

FilterResult FilterCalculator::calcFromF_HP(const FilterParams& p) {
    FilterResult r;
    r.frequency = p.f0;
    r.R_calc = p.impedance;
    r.C_calc = calcCapacitance(p.f0, p.impedance);
    r.valid = true;
    r.description = QString(
        "Filtr górnoprzepustowy RC 1. rzędu\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "Wymagana f₀ = %1\n"
        "Impedancja  = %2\n\n"
        "Dobrane elementy:\n"
        "R = %3\n"
        "C = %4"
    ).arg(formatEngineering(p.f0, "Hz"))
     .arg(formatEngineering(p.impedance, "Ω"))
     .arg(formatEngineering(r.R_calc, "Ω"))
     .arg(formatEngineering(r.C_calc, "F"));
    return r;
}

// ──────────────────── BAND PASS ────────────────────

FilterResult FilterCalculator::calculateBandPass(const FilterParams& p) {
    return (p.mode == ParameterMode::KnownRC) ? calcFromRC_BP(p) : calcFromF_BP(p);
}

FilterResult FilterCalculator::calcFromRC_BP(const FilterParams& p) {
    FilterResult r;
    r.R_calc  = p.R;
    r.C_calc  = p.C;
    r.R2_calc = p.R2;
    r.C2_calc = p.C2;
    double f1 = calcCutoffFrequency(p.R, p.C);
    double f2 = calcCutoffFrequency(p.R2, p.C2);
    if (f1 > f2) std::swap(f1, f2);
    r.frequency = std::sqrt(f1 * f2);
    r.bandwidth = f2 - f1;
    r.Q = calcQFactor(r.frequency, r.bandwidth);
    r.valid = true;
    r.description = QString(
        "Filtr pasmo-przepustowy RC\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "Stopień HP: R1=%1, C1=%2  →  f1=%3\n"
        "Stopień LP: R2=%4, C2=%5  →  f2=%6\n\n"
        "f_środkowe = %7\n"
        "Pasmo      = %8\n"
        "Q          = %9"
    ).arg(formatEngineering(p.R,  "Ω")).arg(formatEngineering(p.C,  "F")).arg(formatEngineering(f1, "Hz"))
     .arg(formatEngineering(p.R2, "Ω")).arg(formatEngineering(p.C2, "F")).arg(formatEngineering(f2, "Hz"))
     .arg(formatEngineering(r.frequency, "Hz"))
     .arg(formatEngineering(r.bandwidth, "Hz"))
     .arg(QString::number(r.Q, 'f', 2));
    return r;
}

FilterResult FilterCalculator::calcFromF_BP(const FilterParams& p) {
    FilterResult r;
    r.frequency = std::sqrt(p.f1 * p.f2);
    r.bandwidth = p.f2 - p.f1;
    r.Q = calcQFactor(r.frequency, r.bandwidth);
    r.R_calc  = p.impedance;
    r.C_calc  = calcCapacitance(p.f1, p.impedance);
    r.R2_calc = p.impedance;
    r.C2_calc = calcCapacitance(p.f2, p.impedance);
    r.valid = true;
    r.description = QString(
        "Filtr pasmo-przepustowy RC\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "f1 = %1,  f2 = %2\n"
        "f₀ = %3,  BW = %4,  Q = %5\n\n"
        "Dobrane elementy:\n"
        "HP: R1=%6, C1=%7\n"
        "LP: R2=%8, C2=%9"
    ).arg(formatEngineering(p.f1, "Hz")).arg(formatEngineering(p.f2, "Hz"))
     .arg(formatEngineering(r.frequency, "Hz"))
     .arg(formatEngineering(r.bandwidth, "Hz"))
     .arg(QString::number(r.Q, 'f', 2))
     .arg(formatEngineering(r.R_calc,  "Ω")).arg(formatEngineering(r.C_calc,  "F"))
     .arg(formatEngineering(r.R2_calc, "Ω")).arg(formatEngineering(r.C2_calc, "F"));
    return r;
}

// ──────────────────── BAND STOP ────────────────────

FilterResult FilterCalculator::calculateBandStop(const FilterParams& p) {
    return (p.mode == ParameterMode::KnownRC) ? calcFromRC_BS(p) : calcFromF_BS(p);
}

FilterResult FilterCalculator::calcFromRC_BS(const FilterParams& p) {
    FilterResult r;
    r.R_calc  = p.R;
    r.C_calc  = p.C;
    r.R2_calc = p.R2;
    r.C2_calc = p.C2;
    double f1 = calcCutoffFrequency(p.R, p.C);
    double f2 = calcCutoffFrequency(p.R2, p.C2);
    if (f1 > f2) std::swap(f1, f2);
    r.frequency = std::sqrt(f1 * f2);
    r.bandwidth = f2 - f1;
    r.Q = calcQFactor(r.frequency, r.bandwidth);
    r.valid = true;
    r.description = QString(
        "Filtr pasmo-zaporowy (notch) RC\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "R1=%1, C1=%2  →  f1=%3\n"
        "R2=%4, C2=%5  →  f2=%6\n\n"
        "Częstotliwość środkowa = %7\n"
        "Pasmo zaporowe        = %8\n"
        "Q                     = %9"
    ).arg(formatEngineering(p.R,  "Ω")).arg(formatEngineering(p.C,  "F")).arg(formatEngineering(f1, "Hz"))
     .arg(formatEngineering(p.R2, "Ω")).arg(formatEngineering(p.C2, "F")).arg(formatEngineering(f2, "Hz"))
     .arg(formatEngineering(r.frequency, "Hz"))
     .arg(formatEngineering(r.bandwidth, "Hz"))
     .arg(QString::number(r.Q, 'f', 2));
    return r;
}

FilterResult FilterCalculator::calcFromF_BS(const FilterParams& p) {
    FilterResult r;
    r.frequency = std::sqrt(p.f1 * p.f2);
    r.bandwidth = p.f2 - p.f1;
    r.Q = calcQFactor(r.frequency, r.bandwidth);
    r.R_calc  = p.impedance;
    r.C_calc  = calcCapacitance(p.f1, p.impedance);
    r.R2_calc = p.impedance;
    r.C2_calc = calcCapacitance(p.f2, p.impedance);
    r.valid = true;
    r.description = QString(
        "Filtr pasmo-zaporowy (notch) RC\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "f1 = %1,  f2 = %2\n"
        "f₀ = %3,  BW = %4,  Q = %5\n\n"
        "Dobrane elementy:\n"
        "R1=%6, C1=%7\n"
        "R2=%8, C2=%9"
    ).arg(formatEngineering(p.f1, "Hz")).arg(formatEngineering(p.f2, "Hz"))
     .arg(formatEngineering(r.frequency, "Hz"))
     .arg(formatEngineering(r.bandwidth, "Hz"))
     .arg(QString::number(r.Q, 'f', 2))
     .arg(formatEngineering(r.R_calc,  "Ω")).arg(formatEngineering(r.C_calc,  "F"))
     .arg(formatEngineering(r.R2_calc, "Ω")).arg(formatEngineering(r.C2_calc, "F"));
    return r;
}

// ──────────────────── OTHER ────────────────────

FilterResult FilterCalculator::calculateOther(const FilterParams& p) {
    FilterResult r;
    r.valid = true;
    r.description = QString(
        "Filtr niestandardowy\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Opis: %1\n\n"
        "Parametry zostaną przeanalizowane przez model językowy."
    ).arg(p.customDescription.isEmpty() ? "(brak opisu)" : p.customDescription);
    return r;
}
