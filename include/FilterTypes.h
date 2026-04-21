#pragma once

#include <QString>
#include <QStringList>

/**
 * @file FilterTypes.h
 * @brief Definicje typów i struktur danych używanych w projekcie FilterDesigner.
 * @author Student JPO 2025/2026
 */

/**
 * @brief Enum reprezentujący typ filtru.
 */
enum class FilterType {
    LowPass,    ///< Dolnoprzepustowy
    HighPass,   ///< Górnoprzepustowy
    BandPass,   ///< Pasmo-przepustowy
    BandStop,   ///< Pasmo-zaporowy
    Other       ///< Inny
};

/**
 * @brief Enum reprezentujący tryb podawania parametrów.
 */
enum class ParameterMode {
    KnownRC,    ///< Znam R i C, wyznaczam f
    KnownF      ///< Nie znam R i C, znam f
};

/**
 * @brief Struktura przechowująca parametry filtru podane przez użytkownika.
 */
struct FilterParams {
    FilterType type = FilterType::LowPass;  ///< Typ filtru
    ParameterMode mode = ParameterMode::KnownRC; ///< Tryb parametrów

    // Parametry w trybie KnownRC
    double R = 0.0;         ///< Rezystancja [Ohm]
    double C = 0.0;         ///< Pojemność [F]
    double R2 = 0.0;        ///< Rezystancja 2 (dla BP/BS) [Ohm]
    double C2 = 0.0;        ///< Pojemność 2 (dla BP/BS) [F]
    double L = 0.0;         ///< Indukcyjność (opcjonalnie) [H]

    // Parametry w trybie KnownF
    double f0 = 0.0;        ///< Częstotliwość graniczna/środkowa [Hz]
    double f1 = 0.0;        ///< Dolna częstotliwość (dla BP/BS) [Hz]
    double f2 = 0.0;        ///< Górna częstotliwość (dla BP/BS) [Hz]
    double impedance = 50.0; ///< Impedancja charakterystyczna [Ohm]

    // Opcje generowania
    bool generatePlot = true;    ///< Czy generować wykres
    bool generateCode = true;    ///< Czy generować kod Python
    bool generateSchematic = true; ///< Czy generować schemat

    QString customDescription; ///< Opis dodatkowy dla trybu "Inne"
};

/**
 * @brief Struktura przechowująca wyniki obliczeń filtru.
 */
struct FilterResult {
    double frequency = 0.0;     ///< Wyznaczona częstotliwość [Hz]
    double R_calc = 0.0;        ///< Wyznaczona rezystancja [Ohm]
    double C_calc = 0.0;        ///< Wyznaczona pojemność [F]
    double R2_calc = 0.0;       ///< Wyznaczona rezystancja 2 [Ohm]
    double C2_calc = 0.0;       ///< Wyznaczona pojemność 2 [F]
    double Q = 0.0;             ///< Dobroć filtru (dla BP/BS)
    double bandwidth = 0.0;     ///< Pasmo przenoszenia [Hz]
    QString description;        ///< Opis słowny wyników
    bool valid = false;         ///< Czy wyniki są poprawne
    QString errorMessage;       ///< Komunikat błędu (jeśli wystąpił)
};

/**
 * @brief Struktura przechowująca odpowiedź z modelu językowego.
 */
struct LLMResponse {
    QString generatedCode;      ///< Wygenerowany kod Python
    QString schematicDescription; ///< Opis schematu ASCII
    QString explanation;        ///< Wyjaśnienie działania filtru
    bool success = false;       ///< Czy zapytanie się powiodło
    QString errorMessage;       ///< Komunikat błędu
};

/**
 * @brief Zwraca nazwę filtru w języku polskim.
 * @param type Typ filtru
 * @return Nazwa filtru
 */
inline QString filterTypeName(FilterType type) {
    switch (type) {
        case FilterType::LowPass:  return "Dolnoprzepustowy";
        case FilterType::HighPass: return "Górnoprzepustowy";
        case FilterType::BandPass: return "Pasmo-przepustowy";
        case FilterType::BandStop: return "Pasmo-zaporowy";
        case FilterType::Other:    return "Inny";
        default: return "Nieznany";
    }
}
