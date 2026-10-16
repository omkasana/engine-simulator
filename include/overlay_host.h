#ifndef ATG_ENGINE_SIM_OVERLAY_HOST_H
#define ATG_ENGINE_SIM_OVERLAY_HOST_H

#include "engine_catalog.h"
#include "ui_button.h"
#include "trainer_panel.h"

#include <vector>

class OverlayHost final : public UiElement {
public:
    enum class Kind {
        None,
        Controls,
        EnginePicker,
        Settings,
        Trainer
    };

    void initialize(EngineSimApplication *app) override;
    void update(float dt) override;
    void render() override;
    void signal(UiElement *element, Event event) override;
    void onMouseClick(const Point &mouseLocal) override;
    void onMouseScroll(int mouseScroll) override;

    void present(Kind kind);
    void dismiss();
    Kind kind() const { return m_kind; }

private:
    TrainerPanel *m_trainer = nullptr;
    UiButton *m_closeButton = nullptr;
    UiButton *m_githubButton = nullptr;
    UiButton *m_issuesButton = nullptr;
    UiButton *m_pickerScrollUpButton = nullptr;
    UiButton *m_pickerScrollDownButton = nullptr;
    UiButton *m_legacyUiButton = nullptr;
    UiButton *m_modernUiButton = nullptr;
    UiButton *m_imperialButton = nullptr;
    UiButton *m_metricButton = nullptr;
    UiButton *m_currencyPreviousButton = nullptr;
    UiButton *m_currencyButton = nullptr;
    UiButton *m_currencyNextButton = nullptr;
    UiButton *m_priceMinusLargeButton = nullptr;
    UiButton *m_priceMinusButton = nullptr;
    UiButton *m_pricePlusButton = nullptr;
    UiButton *m_pricePlusLargeButton = nullptr;
    UiButton *m_resetSettingsButton = nullptr;
    std::vector<UiButton *> m_engineButtons;
    Kind m_kind = Kind::None;
    float m_pickerScrollOffset = 0.0f;
    float m_pickerMaxScrollOffset = 0.0f;

    Bounds viewportBounds() const;
    Bounds dialogBounds() const;
    void layoutControls(const Bounds &panel);
    void layoutEnginePicker(const Bounds &panel);
    void layoutSettings(const Bounds &panel);
    void syncSettingsLabels();
    void setChildrenVisible();
};

#endif
