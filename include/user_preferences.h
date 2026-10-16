#ifndef OPEN_ENGINE_SIM_USER_PREFERENCES_H
#define OPEN_ENGINE_SIM_USER_PREFERENCES_H

#include <array>
#include <filesystem>
#include <string>

enum class UnitSystem {
    Imperial,
    Metric
};

enum class UiStyle {
    Legacy,
    Modern
};

struct CurrencyOption {
    const char *code;
    const char *symbol;
    double startingPricePerLiter;
    double adjustmentStep;
};

class UserPreferences {
public:
    static constexpr std::array<CurrencyOption, 7> currencies{{
        { "USD", "$", 1.25, 0.05 },
        { "EUR", u8"€", 1.70, 0.05 },
        { "GBP", u8"£", 1.45, 0.05 },
        { "INR", u8"₹", 105.0, 1.0 },
        { "JPY", u8"¥", 180.0, 1.0 },
        { "AUD", "A$", 1.90, 0.05 },
        { "CAD", "C$", 1.70, 0.05 },
    }};

    UiStyle uiStyle = UiStyle::Legacy;
    UnitSystem unitSystem = UnitSystem::Imperial;
    std::string currencyCode = "USD";
    double fuelPricePerLiter = 1.25;

    bool load(const std::filesystem::path &path);
    bool save(const std::filesystem::path &path) const;
    void reset();
    void cycleCurrency(int direction);
    void adjustFuelPrice(int steps);
    const CurrencyOption &currency() const;
};

#endif
