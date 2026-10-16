#include "../include/user_preferences.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {
std::string trim(const std::string &value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}
}

const CurrencyOption &UserPreferences::currency() const {
    for (const CurrencyOption &option : currencies) {
        if (currencyCode == option.code) return option;
    }
    return currencies.front();
}

void UserPreferences::reset() {
    uiStyle = UiStyle::Legacy;
    unitSystem = UnitSystem::Imperial;
    currencyCode = currencies.front().code;
    fuelPricePerLiter = currencies.front().startingPricePerLiter;
}

void UserPreferences::cycleCurrency(int direction) {
    std::size_t index = 0;
    for (std::size_t i = 0; i < currencies.size(); ++i) {
        if (currencyCode == currencies[i].code) { index = i; break; }
    }
    const int count = static_cast<int>(currencies.size());
    const int next = (static_cast<int>(index) + direction % count + count) % count;
    currencyCode = currencies[static_cast<std::size_t>(next)].code;
    fuelPricePerLiter = currencies[static_cast<std::size_t>(next)].startingPricePerLiter;
}

void UserPreferences::adjustFuelPrice(int steps) {
    const double step = currency().adjustmentStep;
    fuelPricePerLiter = std::max(0.0, fuelPricePerLiter + step * steps);
    fuelPricePerLiter = std::round(fuelPricePerLiter / step) * step;
}

bool UserPreferences::load(const std::filesystem::path &path) {
    std::ifstream input(path);
    if (!input) return false;

    UserPreferences loaded = *this;
    std::string line;
    while (std::getline(input, line)) {
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos) continue;
        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));
        if (key == "units") {
            if (value == "metric") loaded.unitSystem = UnitSystem::Metric;
            else if (value == "imperial") loaded.unitSystem = UnitSystem::Imperial;
        }
        else if (key == "ui") {
            if (value == "modern") loaded.uiStyle = UiStyle::Modern;
            else if (value == "legacy") loaded.uiStyle = UiStyle::Legacy;
        }
        else if (key == "currency") {
            for (const CurrencyOption &option : currencies) {
                if (value == option.code) { loaded.currencyCode = value; break; }
            }
        }
        else if (key == "fuel_price_per_liter") {
            try {
                const double price = std::stod(value);
                if (std::isfinite(price) && price >= 0.0) loaded.fuelPricePerLiter = price;
            }
            catch (...) { }
        }
    }
    *this = loaded;
    return true;
}

bool UserPreferences::save(const std::filesystem::path &path) const {
    std::error_code error;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), error);
    if (error) return false;
    std::ofstream output(path, std::ios::trunc);
    if (!output) return false;
    output << "ui=" << (uiStyle == UiStyle::Modern ? "modern" : "legacy") << '\n';
    output << "units=" << (unitSystem == UnitSystem::Metric ? "metric" : "imperial") << '\n';
    output << "currency=" << currency().code << '\n';
    output << "fuel_price_per_liter=" << std::fixed << std::setprecision(4)
           << fuelPricePerLiter << '\n';
    return output.good();
}
