================================================================================
  FilterDesigner v1.0
  Projektant filtrów RC z analizą przez lokalny model językowy (Ollama)
  Projekt zaliczeniowy — Języki Programowania Obiektowego 2025/2026
================================================================================

OPIS PROJEKTU
─────────────
FilterDesigner to aplikacja desktopowa w C++/Qt do projektowania filtrów
elektronicznych RC. Program:

  • Oblicza parametry filtrów (LP, HP, BP, BS) ze wzorów matematycznych
  • Wysyła zapytanie do lokalnego modelu językowego przez REST API (Ollama)
  • Generuje kod Python rysujący charakterystykę Bodego (wyświetla wykres)
  • Generuje schemat ASCII obwodu elektrycznego
  • Wykonuje wygenerowany kod Python automatycznie (bez udziału użytkownika)
  • Posiada wielowątkowe zapytania do LLM (nieblokujący UI)
  • Obsługuje wyjątki: brak sieci, timeout, brak Pythona, błędne parametry


WYMAGANIA SYSTEMOWE
───────────────────
  • System operacyjny: Windows 10/11 lub Linux (Ubuntu 20.04+)
  • Qt 6.x lub Qt 5.15 (z modułami: Core, Gui, Widgets, Network)
  • Qt Creator 9.x lub nowszy
  • Kompilator: MSVC 2019+, MinGW 8+, lub GCC 10+
  • Python 3.8+ (w PATH) z bibliotekami: numpy, matplotlib
  • Ollama (https://ollama.com/) z modelem qwen3:8b


INSTALACJA I URUCHOMIENIE
─────────────────────────

1. KLONOWANIE REPOZYTORIUM
   ~~~~~~~~~~~~~~~~~~~~~~~~
   git clone https://github.com/<twoje-repo>/FilterDesigner.git
   cd FilterDesigner

2. INSTALACJA OLLAMY I MODELU
   ~~~~~~~~~~~~~~~~~~~~~~~~~~
   a) Pobierz Ollamę:
      Windows: https://ollama.com/download
      Linux:   curl -fsSL https://ollama.com/install.sh | sh

   b) Uruchom serwer Ollama:
      ollama serve

   c) Pobierz model (w osobnym terminalu):
      ollama pull qwen3:8b

   d) Sprawdź działanie:
      curl http://localhost:11434/api/tags

3. INSTALACJA ZALEŻNOŚCI PYTHON
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   pip install numpy matplotlib
   # lub:
   pip3 install numpy matplotlib

4. KOMPILACJA W QT CREATOR
   ~~~~~~~~~~~~~~~~~~~~~~~~
   a) Otwórz Qt Creator
   b) Plik → Otwórz projekt → wskaż FilterDesigner/FilterDesigner.pro
   c) Wybierz kit (np. Desktop Qt 6.x MinGW 64-bit)
   d) Kliknij "Buduj" (Ctrl+B)
   e) Uruchom (Ctrl+R)

5. KOMPILACJA Z WIERSZA POLECEŃ (Linux)
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cd FilterDesigner
   qmake FilterDesigner.pro
   make -j4
   ./FilterDesigner

   Windows (MinGW):
   qmake FilterDesigner.pro
   mingw32-make
   FilterDesigner.exe


URUCHAMIANIE TESTÓW JEDNOSTKOWYCH
──────────────────────────────────

W Qt Creator:
  a) Plik → Otwórz projekt → wskaż tests/FilterDesignerTests.pro
  b) Buduj → Uruchom testy

Z wiersza poleceń (Linux):
  cd tests
  qmake FilterDesignerTests.pro
  make -j4
  ./FilterDesignerTests -v2

Z wiersza poleceń (Windows MinGW):
  cd tests
  qmake FilterDesignerTests.pro
  mingw32-make
  FilterDesignerTests.exe -v2

Flagi testów Qt:
  -v1       minimalne wyjście
  -v2       szczegółowe wyniki każdego testu
  -xml      wyjście XML (CI/CD)
  -functions lista wszystkich testów


GENEROWANIE DOKUMENTACJI (Doxygen)
───────────────────────────────────
  # Zainstaluj Doxygen: https://doxygen.nl/download.html
  cd FilterDesigner
  doxygen Doxyfile
  # Dokumentacja zostanie zapisana w docs/doxygen/html/index.html


STRUKTURA PROJEKTU
──────────────────
FilterDesigner/
├── FilterDesigner.pro      ← plik projektu Qt (aplikacja)
├── Doxyfile                ← konfiguracja dokumentacji
├── resources.qrc           ← zasoby Qt
├── README.txt              ← ten plik
├── requirements.txt        ← zależności Python
│
├── include/                ← pliki nagłówkowe (.h)
│   ├── FilterTypes.h       ← typy danych, enumy, struktury
│   ├── FilterCalculator.h  ← obliczenia matematyczne
│   ├── OllamaClient.h      ← klient REST API Ollamy
│   ├── OllamaWorker.h      ← wątek roboczy (QThread)
│   ├── CodeExecutor.h      ← ekstrakcja i wykonywanie kodu Python
│   ├── ParameterWidget.h   ← widget parametrów (formularz)
│   ├── ResultWidget.h      ← widget wyników (zakładki)
│   └── MainWindow.h        ← główne okno aplikacji
│
├── src/                    ← implementacje (.cpp)
│   ├── main.cpp
│   ├── FilterCalculator.cpp
│   ├── OllamaClient.cpp
│   ├── OllamaWorker.cpp
│   ├── CodeExecutor.cpp
│   ├── ParameterWidget.cpp
│   ├── ResultWidget.cpp
│   └── MainWindow.cpp
│
├── tests/                  ← testy jednostkowe (Qt Test)
│   ├── FilterDesignerTests.pro
│   ├── tst_FilterCalculator.cpp  ← 25+ testów obliczeń
│   ├── tst_OllamaClient.cpp      ← 15+ testów promptów
│   └── tst_CodeExecutor.cpp      ← 15+ testów ekstrakcji kodu
│
└── docs/                   ← dokumentacja (generowana)
    └── doxygen/


OBSŁUGA BŁĘDÓW
──────────────
  • Brak Ollamy       → komunikat w statusie, aplikacja działa dalej
  • Timeout (>60s)    → informacja w zakładce wyników
  • Brak Pythona      → ostrzeżenie przy próbie uruchomienia kodu
  • Błędne parametry  → walidacja z opisem błędu przed wysłaniem
  • Puste opcje gen.  → ostrzeżenie z prośbą o zaznaczenie opcji


ARCHITEKTURA WIELOWĄTKOWOŚCI
────────────────────────────
  Wątek GUI (Qt main thread):
    MainWindow → ParameterWidget → ResultWidget

  Wątek roboczy (QThread):
    OllamaWorker → OllamaClient → REST API → Ollama
      ↓ sygnał finished()
    MainWindow::onLLMFinished() → ResultWidget::showLLMResponse()

  Wykonanie kodu (QProcess):
    CodeExecutor → python3 <plik_tymczasowy.py>

