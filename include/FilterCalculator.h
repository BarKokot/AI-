#pragma once

#include "FilterTypes.h"
#include <cmath>

/**
 * @file FilterCalculator.h
 * @brief Klasa do obliczeń parametrów filtrów RC.
 */

/**
 * @class FilterCalculator
 * @brief Realizuje obliczenia matematyczne dla filtrów elektronicznych.
 *
 * Klasa oblicza parametry filtrów RC (dolnoprzepustowych, górnoprzepustowych,
 * pasmo-przepustowych i pasmo-zaporowych) na podstawie podanych przez użytkownika
 * wartości elementów lub wymaganych częstotliwości.
 */
class FilterCalculator {
public:
    /**
     * @brief Konstruktor domyślny.
     */
    FilterCalculator() = default;

    /**
     * @brief Przeprowadza obliczenia na podstawie podanych parametrów.
     * @param params Parametry wejściowe filtru
     * @return Wyniki obliczeń
     */
    FilterResult calculate(const FilterParams& params);

    /**
     * @brief Oblicza częstotliwość graniczną filtru RC.
     * @param R Rezystancja [Ohm]
     * @param C Pojemność [F]
     * @return Częstotliwość graniczna [Hz]
     */
    static double calcCutoffFrequency(double R, double C);

    /**
     * @brief Oblicza pojemność na podstawie f i R.
     * @param f Częstotliwość [Hz]
     * @param R Rezystancja [Ohm]
     * @return Pojemność [F]
     */
    static double calcCapacitance(double f, double R);

    /**
     * @brief Oblicza rezystancję na podstawie f i C.
     * @param f Częstotliwość [Hz]
     * @param C Pojemność [F]
     * @return Rezystancja [Ohm]
     */
    static double calcResistance(double f, double C);

    /**
     * @brief Oblicza dobroć filtru BP/BS.
     * @param f0 Częstotliwość środkowa [Hz]
     * @param bandwidth Pasmo [Hz]
     * @return Dobroć Q
     */
    static double calcQFactor(double f0, double bandwidth);

    /**
     * @brief Formatuje wartość w jednostkach inżynierskich.
     * @param value Wartość
     * @param unit Jednostka (np. "F", "Ω", "Hz")
     * @return Sformatowany string
     */
    static QString formatEngineering(double value, const QString& unit);

    /**
     * @brief Waliduje parametry wejściowe.
     * @param params Parametry do walidacji
     * @return Para: (czy poprawne, komunikat błędu)
     */
    static std::pair<bool, QString> validate(const FilterParams& params);

private:
    FilterResult calculateLowPass(const FilterParams& p);
    FilterResult calculateHighPass(const FilterParams& p);
    FilterResult calculateBandPass(const FilterParams& p);
    FilterResult calculateBandStop(const FilterParams& p);
    FilterResult calculateOther(const FilterParams& p);

    FilterResult calcFromRC_LP(const FilterParams& p);
    FilterResult calcFromF_LP(const FilterParams& p);
    FilterResult calcFromRC_HP(const FilterParams& p);
    FilterResult calcFromF_HP(const FilterParams& p);
    FilterResult calcFromRC_BP(const FilterParams& p);
    FilterResult calcFromF_BP(const FilterParams& p);
    FilterResult calcFromRC_BS(const FilterParams& p);
    FilterResult calcFromF_BS(const FilterParams& p);
};
