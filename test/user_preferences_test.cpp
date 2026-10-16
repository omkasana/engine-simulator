#include "user_preferences.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>

TEST(UserPreferences, CurrencySelectionUsesEditableStartingValues) {
    UserPreferences preferences;
    preferences.cycleCurrency(3);
    EXPECT_EQ(preferences.currencyCode, "INR");
    EXPECT_DOUBLE_EQ(preferences.fuelPricePerLiter, 105.0);

    preferences.adjustFuelPrice(2);
    EXPECT_DOUBLE_EQ(preferences.fuelPricePerLiter, 107.0);
    preferences.adjustFuelPrice(-200);
    EXPECT_DOUBLE_EQ(preferences.fuelPricePerLiter, 0.0);
}

TEST(UserPreferences, PersistsUnitsCurrencyAndFuelPrice) {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path path = std::filesystem::temp_directory_path()
        / ("engine-sim-preferences-" + std::to_string(unique) + ".ini");

    UserPreferences saved;
    saved.unitSystem = UnitSystem::Metric;
    saved.uiStyle = UiStyle::Modern;
    saved.currencyCode = "INR";
    saved.fuelPricePerLiter = 102.75;
    ASSERT_TRUE(saved.save(path));

    UserPreferences loaded;
    ASSERT_TRUE(loaded.load(path));
    EXPECT_EQ(loaded.unitSystem, UnitSystem::Metric);
    EXPECT_EQ(loaded.uiStyle, UiStyle::Modern);
    EXPECT_EQ(loaded.currencyCode, "INR");
    EXPECT_DOUBLE_EQ(loaded.fuelPricePerLiter, 102.75);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

TEST(UserPreferences, IgnoresUnsupportedAndInvalidValues) {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path path = std::filesystem::temp_directory_path()
        / ("engine-sim-invalid-preferences-" + std::to_string(unique) + ".ini");
    {
        std::ofstream output(path);
        output << "units=metric\n"
               << "ui=unsupported\n"
               << "currency=BTC\n"
               << "fuel_price_per_liter=-1\n";
    }

    UserPreferences loaded;
    ASSERT_TRUE(loaded.load(path));
    EXPECT_EQ(loaded.unitSystem, UnitSystem::Metric);
    EXPECT_EQ(loaded.uiStyle, UiStyle::Legacy);
    EXPECT_EQ(loaded.currencyCode, "USD");
    EXPECT_DOUBLE_EQ(loaded.fuelPricePerLiter, 1.25);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}
