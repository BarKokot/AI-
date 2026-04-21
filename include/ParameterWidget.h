#pragma once

#include "FilterTypes.h"
#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>

/**
 * @file ParameterWidget.h
 * @brief Widget do wprowadzania parametrów filtru przez użytkownika.
 */

/**
 * @class ParameterWidget
 * @brief Dynamiczny panel parametrów dostosowany do wybranego trybu.
 *
 * Wyświetla odpowiednie pola formularza w zależności od trybu
 * (znam RC / znam f) i typu filtru. Zawsze generuje wykres, kod i schemat.
 */
class ParameterWidget : public QWidget {
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget* parent = nullptr);

    /**
     * @brief Ustawia typ filtru i aktualizuje widoczne pola.
     * @param type Typ filtru
     */
    void setFilterType(FilterType type);

    /**
     * @brief Zbiera dane z formularza i zwraca strukturę FilterParams.
     * @param ok Ustawione na false jeśli wystąpił błąd walidacji
     * @return Parametry filtru
     */
    FilterParams collectParams(bool& ok);

    /**
     * @brief Czyści wszystkie pola formularza.
     */
    void clearFields();

signals:
    /**
     * @brief Emitowany gdy użytkownik chce uruchomić analizę.
     */
    void analyzeRequested();

private slots:
    void onModeChanged();

private:
    void setupUi();
    void setupRCMode();
    void setupFMode();
    void updateVisibility();
    bool isBandType() const;

    FilterType m_filterType = FilterType::LowPass;

    // Tryb
    QRadioButton* m_radioKnownRC;
    QRadioButton* m_radioKnownF;

    // Stacki dla trybów
    QStackedWidget* m_stack;

    // ── Pola trybu RC ──
    QWidget*   m_rcWidget;
    QLineEdit* m_edtR;
    QLineEdit* m_edtC;
    QLineEdit* m_edtR2;
    QLineEdit* m_edtC2;
    QLabel*    m_lblR2;
    QLabel*    m_lblC2;

    // ── Pola trybu F ──
    QWidget*   m_fWidget;
    QLineEdit* m_edtF0;
    QLineEdit* m_edtF1;
    QLineEdit* m_edtF2;
    QLineEdit* m_edtImpedance;
    QLabel*    m_lblF0;
    QLabel*    m_lblF1F2;

    // Opis dla "Inne"
    QWidget*   m_customWidget;
    QLineEdit* m_edtCustomDesc;

    QPushButton* m_btnAnalyze;
};
