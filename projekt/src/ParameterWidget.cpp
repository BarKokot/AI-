#include "ParameterWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDoubleValidator>

/**
 * @file ParameterWidget.cpp
 * @brief Implementacja widgetu parametrów filtru (bez opcji generowania — zawsze generuj wszystko).
 */

ParameterWidget::ParameterWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void ParameterWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // ── Tryb parametrów ──
    auto* modeGroup  = new QGroupBox("Tryb wprowadzania parametrów", this);
    auto* modeLayout = new QVBoxLayout(modeGroup);
    m_radioKnownRC   = new QRadioButton("Znam R i C → wyznaczam f");
    m_radioKnownF    = new QRadioButton("Znam f → dobieram R i C");
    m_radioKnownRC->setChecked(true);
    modeLayout->addWidget(m_radioKnownRC);
    modeLayout->addWidget(m_radioKnownF);
    mainLayout->addWidget(modeGroup);

    // ── Stos widgetów parametrów ──
    m_stack = new QStackedWidget(this);
    setupRCMode();
    setupFMode();
    m_stack->addWidget(m_rcWidget);
    m_stack->addWidget(m_fWidget);
    mainLayout->addWidget(m_stack);

    // ── Opis dla "Inne" ──
    m_customWidget    = new QWidget(this);
    auto* customLayout = new QFormLayout(m_customWidget);
    m_edtCustomDesc   = new QLineEdit(m_customWidget);
    m_edtCustomDesc->setPlaceholderText("Opisz wymagania filtru...");
    customLayout->addRow("Opis filtru:", m_edtCustomDesc);
    m_customWidget->setVisible(false);
    mainLayout->addWidget(m_customWidget);

    // ── Przycisk ──
    m_btnAnalyze = new QPushButton("🔍  Analizuj i generuj", this);
    m_btnAnalyze->setMinimumHeight(42);
    m_btnAnalyze->setObjectName("btnAnalyze");
    mainLayout->addWidget(m_btnAnalyze);

    mainLayout->addStretch();

    connect(m_radioKnownRC, &QRadioButton::toggled, this, &ParameterWidget::onModeChanged);
    connect(m_btnAnalyze,   &QPushButton::clicked,  this, &ParameterWidget::analyzeRequested);
}

void ParameterWidget::setupRCMode() {
    m_rcWidget  = new QWidget(this);
    auto* layout = new QFormLayout(m_rcWidget);
    layout->setSpacing(8);

    auto mkEdit = [&](const QString& placeholder) {
        auto* e = new QLineEdit(m_rcWidget);
        e->setPlaceholderText(placeholder);
        auto* v = new QDoubleValidator(0.0, 1e12, 12, e);
        v->setNotation(QDoubleValidator::ScientificNotation);
        e->setValidator(v);
        return e;
    };

    m_edtR = mkEdit("np. 1000");
    m_edtC = mkEdit("np. 1e-7");
    layout->addRow("Rezystancja R [Ω]:", m_edtR);
    layout->addRow("Pojemność C [F]:",   m_edtC);

    m_lblR2 = new QLabel("Rezystancja R2 [Ω]:");
    m_lblC2 = new QLabel("Pojemność C2 [F]:");
    m_edtR2 = mkEdit("np. 2200");
    m_edtC2 = mkEdit("np. 4.7e-8");
    layout->addRow(m_lblR2, m_edtR2);
    layout->addRow(m_lblC2, m_edtC2);

    m_lblR2->setVisible(false);
    m_edtR2->setVisible(false);
    m_lblC2->setVisible(false);
    m_edtC2->setVisible(false);
}

void ParameterWidget::setupFMode() {
    m_fWidget    = new QWidget(this);
    auto* layout  = new QFormLayout(m_fWidget);
    layout->setSpacing(8);

    auto mkEdit = [&](const QString& placeholder) {
        auto* e = new QLineEdit(m_fWidget);
        e->setPlaceholderText(placeholder);
        auto* v = new QDoubleValidator(0.0, 1e12, 12, e);
        v->setNotation(QDoubleValidator::ScientificNotation);
        e->setValidator(v);
        return e;
    };

    m_edtF0        = mkEdit("np. 1000");
    m_edtF1        = mkEdit("np. 500");
    m_edtF2        = mkEdit("np. 2000");
    m_edtImpedance = mkEdit("np. 50");

    m_lblF0   = new QLabel("Częstotliwość f₀ [Hz]:");
    m_lblF1F2 = new QLabel("Częstotliwość f1 [Hz]:");

    layout->addRow(m_lblF0,   m_edtF0);
    layout->addRow(m_lblF1F2, m_edtF1);
    layout->addRow("Częstotliwość f2 [Hz]:", m_edtF2);
    layout->addRow("Impedancja Z [Ω]:",      m_edtImpedance);
    m_edtImpedance->setText("50");

    m_edtF1->setVisible(false);
    m_edtF2->setVisible(false);
    m_lblF1F2->setVisible(false);
    for (auto* w : m_fWidget->findChildren<QLabel*>())
        if (w->text() == "Częstotliwość f2 [Hz]:") w->setVisible(false);
    for (auto* w : m_fWidget->findChildren<QLineEdit*>())
        if (w == m_edtF2) w->setVisible(false);
}

void ParameterWidget::onModeChanged() {
    m_stack->setCurrentIndex(m_radioKnownRC->isChecked() ? 0 : 1);
}

void ParameterWidget::setFilterType(FilterType type) {
    m_filterType = type;
    updateVisibility();
}

bool ParameterWidget::isBandType() const {
    return (m_filterType == FilterType::BandPass ||
            m_filterType == FilterType::BandStop);
}

void ParameterWidget::updateVisibility() {
    bool band  = isBandType();
    bool other = (m_filterType == FilterType::Other);

    m_lblR2->setVisible(band);
    m_edtR2->setVisible(band);
    m_lblC2->setVisible(band);
    m_edtC2->setVisible(band);

    m_lblF0->setVisible(!band);
    m_edtF0->setVisible(!band);
    m_lblF1F2->setVisible(band);
    m_edtF1->setVisible(band);

    auto* fl = qobject_cast<QFormLayout*>(m_fWidget->layout());
    if (fl) {
        for (int i = 0; i < fl->rowCount(); ++i) {
            auto* lbl = fl->itemAt(i, QFormLayout::LabelRole)
                        ? qobject_cast<QLabel*>(fl->itemAt(i, QFormLayout::LabelRole)->widget())
                        : nullptr;
            auto* fld = fl->itemAt(i, QFormLayout::FieldRole)
                        ? fl->itemAt(i, QFormLayout::FieldRole)->widget()
                        : nullptr;
            if (lbl && fld && lbl->text().contains("f2")) {
                lbl->setVisible(band);
                fld->setVisible(band);
            }
        }
    }

    m_customWidget->setVisible(other);
}

void ParameterWidget::clearFields() {
    for (auto* e : {m_edtR, m_edtC, m_edtR2, m_edtC2, m_edtF0, m_edtF1, m_edtF2})
        e->clear();
    m_edtImpedance->setText("50");
    m_edtCustomDesc->clear();
}

FilterParams ParameterWidget::collectParams(bool& ok) {
    ok = false;
    FilterParams p;
    p.type = m_filterType;
    p.mode = m_radioKnownRC->isChecked() ? ParameterMode::KnownRC : ParameterMode::KnownF;
    // Zawsze generuj wszystko
    p.generatePlot      = true;
    p.generateCode      = true;
    p.generateSchematic = true;
    p.customDescription = m_edtCustomDesc->text().trimmed();

    auto readDouble = [](QLineEdit* e, double& out) -> bool {
        bool conv;
        out = e->text().replace(',', '.').toDouble(&conv);
        return conv && out > 0.0;
    };

    if (p.mode == ParameterMode::KnownRC) {
        if (!readDouble(m_edtR, p.R)) {
            QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawną wartość R > 0.");
            return p;
        }
        if (!readDouble(m_edtC, p.C)) {
            QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawną wartość C > 0.");
            return p;
        }
        if (isBandType()) {
            if (!readDouble(m_edtR2, p.R2)) {
                QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawną wartość R2 > 0.");
                return p;
            }
            if (!readDouble(m_edtC2, p.C2)) {
                QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawną wartość C2 > 0.");
                return p;
            }
        }
    } else {
        if (!readDouble(m_edtImpedance, p.impedance)) {
            QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawną impedancję > 0.");
            return p;
        }
        if (isBandType()) {
            if (!readDouble(m_edtF1, p.f1) || !readDouble(m_edtF2, p.f2)) {
                QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawne f1 i f2 > 0.");
                return p;
            }
            if (p.f1 >= p.f2) {
                QMessageBox::warning(nullptr, "Błąd walidacji", "f1 musi być mniejsze od f2.");
                return p;
            }
        } else {
            if (!readDouble(m_edtF0, p.f0)) {
                QMessageBox::warning(nullptr, "Błąd walidacji", "Podaj poprawną częstotliwość f₀ > 0.");
                return p;
            }
        }
    }

    ok = true;
    return p;
}
