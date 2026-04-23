#include "SchematicRenderer.h"
#include "FilterCalculator.h"

/**
 * @file SchematicRenderer.cpp
 * @brief Implementacja renderera schematów ASCII dla filtrów RC.
 */

QString SchematicRenderer::formatVal(double v, const QString& unit) {
    return FilterCalculator::formatEngineering(v, unit);
}

QString SchematicRenderer::render(const FilterParams& params, const FilterResult& result) {
    switch (params.type) {
        case FilterType::LowPass:  return renderLowPass(params, result);
        case FilterType::HighPass: return renderHighPass(params, result);
        case FilterType::BandPass: return renderBandPass(params, result);
        case FilterType::BandStop: return renderBandStop(params, result);
        case FilterType::Other:    return renderOther(params);
        default: return "Nieznany typ filtru.";
    }
}

// ─────────────────────────────────────────────────────────────────
//  LOW PASS  (dolnoprzepustowy)
//
//           R
//  Vin o--[===]--+--o Vout
//                |
//               [C]
//                |
//               GND
// ─────────────────────────────────────────────────────────────────
QString SchematicRenderer::renderLowPass(const FilterParams& p, const FilterResult& r) {
    double R = (p.mode == ParameterMode::KnownRC) ? p.R : r.R_calc;
    double C = (p.mode == ParameterMode::KnownRC) ? p.C : r.C_calc;
    double f = r.frequency;

    QString rVal = formatVal(R, "Ω");
    QString cVal = formatVal(C, "F");
    QString fVal = formatVal(f, "Hz");

    // Wyśrodkuj etykietę R nad elementem
    QString line0 = QString("              %1").arg(rVal);
    QString line1 = "  Vin o--[===]--+--o Vout";
    QString line2 = "                |";
    QString line3 = "              [===]  " + cVal;
    QString line4 = "                |";
    QString line5 = "               GND";

    return QString(
        "Filtr dolnoprzepustowy RC (1. rzad)\n"
        "===================================\n\n"
        "         R = %1\n"
        "         ___\n"
        "        |   |\n"
        "Vin o---+   +---+---o Vout\n"
        "                    |\n"
        "                   ___\n"
        "                  |   |  C = %2\n"
        "                  |___|\n"
        "                    |\n"
        "                   GND\n\n"
        "f0 = %3\n"
        "Przepuszcza: f < f0\n"
        "Tlumi:       f > f0  (spadek -20 dB/dek)"
    ).arg(rVal).arg(cVal).arg(fVal);
}

// ─────────────────────────────────────────────────────────────────
//  HIGH PASS  (górnoprzepustowy)
//
//           C
//  Vin o--[===]--+--o Vout
//                |
//               [R]
//                |
//               GND
// ─────────────────────────────────────────────────────────────────
QString SchematicRenderer::renderHighPass(const FilterParams& p, const FilterResult& r) {
    double R = (p.mode == ParameterMode::KnownRC) ? p.R : r.R_calc;
    double C = (p.mode == ParameterMode::KnownRC) ? p.C : r.C_calc;
    double f = r.frequency;

    QString rVal = formatVal(R, "Ω");
    QString cVal = formatVal(C, "F");
    QString fVal = formatVal(f, "Hz");

    return QString(
        "Filtr gornoprzepustowy RC (1. rzad)\n"
        "====================================\n\n"
        "         C = %1\n"
        "        _|_\n"
        "       |   |\n"
        "Vin o--+   +--+---o Vout\n"
        "                  |\n"
        "                 ___\n"
        "                |   |  R = %2\n"
        "                |___|\n"
        "                  |\n"
        "                 GND\n\n"
        "f0 = %3\n"
        "Przepuszcza: f > f0\n"
        "Tlumi:       f < f0  (spadek -20 dB/dek)"
    ).arg(cVal).arg(rVal).arg(fVal);
}

// ─────────────────────────────────────────────────────────────────
//  BAND PASS  (pasmo-przepustowy)
//  HP (C1+R1) szeregowo z LP (R2+C2)
// ─────────────────────────────────────────────────────────────────
QString SchematicRenderer::renderBandPass(const FilterParams& p, const FilterResult& r) {
    double R1 = (p.mode == ParameterMode::KnownRC) ? p.R  : r.R_calc;
    double C1 = (p.mode == ParameterMode::KnownRC) ? p.C  : r.C_calc;
    double R2 = (p.mode == ParameterMode::KnownRC) ? p.R2 : r.R2_calc;
    double C2 = (p.mode == ParameterMode::KnownRC) ? p.C2 : r.C2_calc;

    QString r1Val = formatVal(R1, "Ω");
    QString c1Val = formatVal(C1, "F");
    QString r2Val = formatVal(R2, "Ω");
    QString c2Val = formatVal(C2, "F");
    QString f0Val = formatVal(r.frequency, "Hz");
    QString bwVal = formatVal(r.bandwidth, "Hz");

    return QString(
        "Filtr pasmo-przepustowy RC\n"
        "==========================\n"
        "Kaskada: stopien HP + stopien LP\n\n"
        "      C1=%1        R2=%2\n"
        "      _|_           ___\n"
        "     |   |         |   |\n"
        "Vin -+   +-+-------+   +-+--- Vout\n"
        "               |              |\n"
        "              ___            ___\n"
        "             |   | R1=%3   |   | C2=%4\n"
        "             |___|         |___|\n"
        "               |              |\n"
        "              GND            GND\n\n"
        "f0 = %5  |  Pasmo = %6  |  Q = %7"
    ).arg(c1Val).arg(r2Val)
     .arg(r1Val).arg(c2Val)
     .arg(f0Val).arg(bwVal)
     .arg(r.Q, 0, 'f', 2);
}

// ─────────────────────────────────────────────────────────────────
//  BAND STOP / NOTCH  (pasmo-zaporowy)
//  LP || HP  (rownolegle, suma sygnalu)
// ─────────────────────────────────────────────────────────────────
QString SchematicRenderer::renderBandStop(const FilterParams& p, const FilterResult& r) {
    double R1 = (p.mode == ParameterMode::KnownRC) ? p.R  : r.R_calc;
    double C1 = (p.mode == ParameterMode::KnownRC) ? p.C  : r.C_calc;
    double R2 = (p.mode == ParameterMode::KnownRC) ? p.R2 : r.R2_calc;
    double C2 = (p.mode == ParameterMode::KnownRC) ? p.C2 : r.C2_calc;

    QString r1Val = formatVal(R1, "Ω");
    QString c1Val = formatVal(C1, "F");
    QString r2Val = formatVal(R2, "Ω");
    QString c2Val = formatVal(C2, "F");
    QString f0Val = formatVal(r.frequency, "Hz");
    QString bwVal = formatVal(r.bandwidth, "Hz");

    return QString(
        "Filtr pasmo-zaporowy RC (notch)\n"
        "================================\n"
        "Galaz dolna (LP) || galaz gorna (HP)\n\n"
        "              +---[R1=%1]---+---o Vout\n"
        "              |    ___      |\n"
        "              |   |   |    |\n"
        "Vin o---------+   |C1=%2|  |\n"
        "              |   |___|    |\n"
        "              |     |      |\n"
        "              |    GND     |\n"
        "              |            |\n"
        "              |    _|_     |\n"
        "              +---+   +----+\n"
        "                   C2=%3\n"
        "                    |\n"
        "                   ___\n"
        "                  |   |  R2=%4\n"
        "                  |___|\n"
        "                    |\n"
        "                   GND\n\n"
        "f0 = %5  |  Pasmo zaporowe = %6  |  Q = %7"
    ).arg(r1Val).arg(c1Val)
     .arg(c2Val).arg(r2Val)
     .arg(f0Val).arg(bwVal)
     .arg(r.Q, 0, 'f', 2);
}

// ─────────────────────────────────────────────────────────────────
//  OTHER
// ─────────────────────────────────────────────────────────────────
QString SchematicRenderer::renderOther(const FilterParams& p) {
    return QString(
        "Filtr niestandardowy\n"
        "====================\n\n"
        "Opis: %1\n\n"
        "Vin o---[ ? ]---o Vout\n\n"
        "Schemat zostanie wygenerowany przez model jezykowy\n"
        "na podstawie podanego opisu."
    ).arg(p.customDescription.isEmpty() ? "(brak opisu)" : p.customDescription);
}
