#pragma once

#include "FilterTypes.h"
#include <QString>

/**
 * @file SchematicRenderer.h
 * @brief Generuje schematy ASCII filtrów RC bezpośrednio w C++.
 *
 * Schematy są zawsze poziome, czytelne i niezależne od modelu językowego.
 */
class SchematicRenderer {
public:
    /**
     * @brief Generuje schemat ASCII dla podanych parametrów filtru.
     * @param params Parametry filtru
     * @param result Wyniki obliczeń
     * @return Gotowy schemat jako QString (wieloliniowy)
     */
    static QString render(const FilterParams& params, const FilterResult& result);

private:
    static QString renderLowPass(const FilterParams& p, const FilterResult& r);
    static QString renderHighPass(const FilterParams& p, const FilterResult& r);
    static QString renderBandPass(const FilterParams& p, const FilterResult& r);
    static QString renderBandStop(const FilterParams& p, const FilterResult& r);
    static QString renderOther(const FilterParams& p);
    static QString formatVal(double v, const QString& unit);
};
