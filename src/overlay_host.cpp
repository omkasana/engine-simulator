#include "../include/overlay_host.h"

#include "../include/engine_sim_application.h"
#include "../include/ui_utilities.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
// The controls dialog deliberately centralizes all spacing here. Adjust these
// values to retune the rhythm of every section without touching rendering code.
struct ControlsLayout {
    float panelInset = 30.0f;
    float titleHeight = 32.0f;
    float sectionTitleHeight = 20.0f;
    float rowHeight = 28.0f;
    float titleGap = 28.0f;
    float sectionHeaderGap = 16.0f;
    float rowGap = 16.0f;
    float sectionGap = 30.0f;
    float keyHorizontalGap = 7.0f;
    float keyLabelGap = 18.0f;
    float projectColumnStart = 0.58f;
    float projectColumnEnd = 0.98f;
    float projectTopOffset = 155.0f;
    float projectTextGap = 12.0f;
    float footerButtonHeight = 34.0f;
    float footerButtonGap = 24.0f;
    float closeButtonWidth = 0.22f;
    float closeButtonHeight = 54.0f;
};

constexpr ControlsLayout kControlsLayout{};

struct PickerLayout {
    float panelInset = 36.0f;
    float titleHeight = 44.0f;
    float footerHeight = 64.0f;
    float headerHeight = 34.0f;
    float rowHeight = 58.0f;
    float itemGap = 10.0f;
    float itemFontSize = 19.0f;
    float groupFontSize = 18.0f;
};

constexpr PickerLayout kModernPickerLayout{};

struct SettingsLayout {
    float panelInset = 42.0f;
    float titleHeight = 54.0f;
    float titleGap = 34.0f;
    float sectionHeight = 30.0f;
    float rowHeight = 58.0f;
    float rowGap = 18.0f;
    float controlGap = 12.0f;
    float labelWidth = 0.34f;
    float closeWidth = 190.0f;
    float resetWidth = 220.0f;
};

constexpr SettingsLayout kSettingsLayout{};

struct ControlAction {
    std::array<const char *, 4> keys;
    int keyCount;
    const char *label;
    const char *hint;
};

constexpr ControlAction kStartActions[] = {
    {{ "A", nullptr, nullptr, nullptr }, 1, "ENABLE IGNITION", "OR TAP IGNITION"},
    {{ "S", nullptr, nullptr, nullptr }, 1, "HOLD STARTER", "OR HOLD STARTER"},
};
constexpr ControlAction kDriveActions[] = {
    {{ "Q", "W", "E", "R" }, 4, "ADJUST THROTTLE", "OR DRAG THROTTLE"},
    {{ "UP", "DOWN", nullptr, nullptr }, 2, "CHANGE GEAR", "OR TAP ARROWS"},
    {{ "SPACE", nullptr, nullptr, nullptr }, 1, "HOLD CLUTCH", ""},
};

Bounds controlsContent(const Bounds &panel) { return panel.inset(kControlsLayout.panelInset); }

Bounds projectColumn(const Bounds &content) {
    return content.horizontalSplit(kControlsLayout.projectColumnStart, kControlsLayout.projectColumnEnd);
}

class VerticalLayoutCursor {
public:
    VerticalLayoutCursor(const Bounds &column, float y) : m_column(column), m_y(y) { }

    Bounds take(float height) {
        const Bounds result(m_column.width(), height, { m_column.left(), m_y }, Bounds::tl);
        m_y -= height;
        return result;
    }

    void gap(float amount) { m_y -= amount; }

private:
    Bounds m_column;
    float m_y;
};
}

void OverlayHost::initialize(EngineSimApplication *app) {
    UiElement::initialize(app);
    m_engineButtons.clear();
    m_closeButton = addElement<UiButton>(this);
    m_closeButton->m_text = "CLOSE";
    m_closeButton->m_fontSize = 16.0f;
    m_closeButton->m_inverted = true;
    m_closeButton->m_drawFrame = false;

    m_githubButton = addElement<UiButton>(this);
    m_githubButton->m_text = "GITHUB";
    m_githubButton->m_fontSize = 14.0f;
    m_githubButton->m_inverted = true;
    m_githubButton->m_drawFrame = false;

    m_issuesButton = addElement<UiButton>(this);
    m_issuesButton->m_text = "ISSUES";
    m_issuesButton->m_fontSize = 14.0f;
    m_issuesButton->m_inverted = true;
    m_issuesButton->m_drawFrame = false;

    m_pickerScrollUpButton = addElement<UiButton>(this);
    m_pickerScrollUpButton->m_text = "UP";
    m_pickerScrollUpButton->m_fontSize = 14.0f;
    m_pickerScrollUpButton->m_inverted = true;

    m_pickerScrollDownButton = addElement<UiButton>(this);
    m_pickerScrollDownButton->m_text = "DOWN";
    m_pickerScrollDownButton->m_fontSize = 14.0f;
    m_pickerScrollDownButton->m_inverted = true;

    m_legacyUiButton = addElement<UiButton>(this);
    m_legacyUiButton->m_text = "LEGACY";
    m_legacyUiButton->m_fontSize = 18.0f;
    m_legacyUiButton->m_inverted = true;
    m_modernUiButton = addElement<UiButton>(this);
    m_modernUiButton->m_text = "MODERN";
    m_modernUiButton->m_fontSize = 18.0f;
    m_modernUiButton->m_inverted = true;

    m_imperialButton = addElement<UiButton>(this);
    m_imperialButton->m_text = "IMPERIAL";
    m_imperialButton->m_fontSize = 18.0f;
    m_imperialButton->m_inverted = true;
    m_metricButton = addElement<UiButton>(this);
    m_metricButton->m_text = "METRIC";
    m_metricButton->m_fontSize = 18.0f;
    m_metricButton->m_inverted = true;

    m_currencyPreviousButton = addElement<UiButton>(this);
    m_currencyPreviousButton->m_text = "<";
    m_currencyPreviousButton->m_fontSize = 22.0f;
    m_currencyPreviousButton->m_inverted = true;
    m_currencyButton = addElement<UiButton>(this);
    m_currencyButton->m_fontSize = 20.0f;
    m_currencyButton->m_inverted = true;
    m_currencyNextButton = addElement<UiButton>(this);
    m_currencyNextButton->m_text = ">";
    m_currencyNextButton->m_fontSize = 22.0f;
    m_currencyNextButton->m_inverted = true;

    const auto addPriceButton = [this](const char *label) {
        UiButton *button = addElement<UiButton>(this);
        button->m_text = label;
        button->m_fontSize = 18.0f;
        button->m_inverted = true;
        return button;
    };
    m_priceMinusLargeButton = addPriceButton("- 10");
    m_priceMinusButton = addPriceButton("- 1");
    m_pricePlusButton = addPriceButton("+ 1");
    m_pricePlusLargeButton = addPriceButton("+ 10");
    m_resetSettingsButton = addPriceButton("RESET DEFAULTS");

    for (const EngineCatalogEntry &entry : engineCatalog()) {
        UiButton *button = addElement<UiButton>(this);
        button->m_text = entry.name;
        button->m_fontSize = 15.0f;
        button->m_inverted = true;
        m_engineButtons.push_back(button);
    }
    m_trainer = addElement<TrainerPanel>(this);
    dismiss();
}

Bounds OverlayHost::viewportBounds() const {
    return Bounds(
        static_cast<float>(m_app->getScreenWidth()),
        static_cast<float>(m_app->getScreenHeight()),
        { 0.0f, static_cast<float>(m_app->getScreenHeight()) });
}

Bounds OverlayHost::dialogBounds() const {
    const Bounds viewport = viewportBounds();
    const float inset = std::max(20.0f, std::min(viewport.width(), viewport.height()) * 0.07f);
    return viewport.inset(inset);
}

void OverlayHost::present(Kind kind) {
    if (m_kind == kind) {
        dismiss();
        return;
    }
    if (m_trainer) m_trainer->cancelEditing();
    m_kind = kind;
    m_pickerScrollOffset = 0.0f;
    setVisible(true);
    setChildrenVisible();
}

void OverlayHost::dismiss() {
    if (m_trainer) m_trainer->cancelEditing();
    m_kind = Kind::None;
    setVisible(false);
    setChildrenVisible();
}

void OverlayHost::setChildrenVisible() {
    if (m_trainer) m_trainer->setVisible(m_kind == Kind::Trainer);
    const bool controls = m_kind == Kind::Controls;
    const bool picker = m_kind == Kind::EnginePicker;
    const bool settings = m_kind == Kind::Settings;
    const bool modernSettings = settings
        && m_app->getUserPreferences().uiStyle == UiStyle::Modern;
    if (m_closeButton != nullptr) m_closeButton->setVisible(m_kind != Kind::None);
    if (m_githubButton != nullptr) m_githubButton->setVisible(controls);
    if (m_issuesButton != nullptr) m_issuesButton->setVisible(controls);
    if (m_pickerScrollUpButton != nullptr) m_pickerScrollUpButton->setVisible(picker);
    if (m_pickerScrollDownButton != nullptr) m_pickerScrollDownButton->setVisible(picker);
    for (UiButton *button : m_engineButtons) button->setVisible(picker);
    for (UiButton *button : { m_legacyUiButton, m_modernUiButton, m_resetSettingsButton }) {
        if (button != nullptr) button->setVisible(settings);
    }
    for (UiButton *button : {
        m_imperialButton, m_metricButton, m_currencyPreviousButton,
        m_currencyButton, m_currencyNextButton, m_priceMinusLargeButton,
        m_priceMinusButton, m_pricePlusButton, m_pricePlusLargeButton })
    {
        if (button != nullptr) button->setVisible(modernSettings);
    }
}

void OverlayHost::update(float dt) {
    if (m_app->getPlatform()->wasKeyPressed(DesktopKey::F1)) present(Kind::Controls);
    if (m_app->getPlatform()->wasKeyPressed(DesktopKey::F2)) present(Kind::EnginePicker);
    if (!m_app->trainerSession().empty() && m_app->getPlatform()->wasKeyPressed(DesktopKey::F3)) present(Kind::Trainer);
    if (m_app->getPlatform()->wasKeyPressed(DesktopKey::F4)) present(Kind::Settings);
    if (m_kind == Kind::None) return;
    setChildrenVisible();

    const Bounds viewport = viewportBounds();
    const Bounds panel = dialogBounds();
    m_mouseBounds = viewport;
    m_checkMouse = true;
    if (m_kind == Kind::Trainer) {
        const Bounds content = panel.inset(20);
        m_closeButton->m_fontSize = 18 * m_app->getScreenWidth() / 1440.0f;
        m_closeButton->m_bounds = content.verticalSplit(0.92f, 1.0f).horizontalSplit(0.8f, 1.0f);
        m_trainer->m_bounds = content.verticalSplit(0.0f, 0.90f);
    }
    else if (m_kind == Kind::Controls) layoutControls(panel);
    else if (m_kind == Kind::EnginePicker) layoutEnginePicker(panel);
    else layoutSettings(panel);
    UiElement::update(dt);
}

void OverlayHost::layoutControls(const Bounds &panel) {
    const Bounds content = controlsContent(panel);
    const float closeWidth = content.width() * kControlsLayout.closeButtonWidth;
    m_closeButton->m_bounds = Bounds(
        closeWidth,
        kControlsLayout.closeButtonHeight,
        { content.right() - closeWidth, content.top() - 8.0f }, Bounds::tl);

    const Bounds project = projectColumn(content);
    const float buttonWidth = (project.width() - kControlsLayout.footerButtonGap) / 2.0f;
    m_githubButton->m_bounds = Bounds(
        buttonWidth, kControlsLayout.footerButtonHeight,
        { project.left(), content.bottom() }, Bounds::bl);
    m_issuesButton->m_bounds = Bounds(
        buttonWidth, kControlsLayout.footerButtonHeight,
        { project.right(), content.bottom() }, Bounds::br);
}

void OverlayHost::layoutEnginePicker(const Bounds &panel) {
    const bool modern = m_app->getUserPreferences().uiStyle == UiStyle::Modern;
    if (!modern) {
        const Bounds content = panel.inset(28.0f);
        const Bounds listBounds = content.verticalSplit(0.12f, 0.84f);
        const int columns = listBounds.width() >= 700.0f ? 3 : 2;
        constexpr float headerHeight = 28.0f;
        constexpr float rowHeight = 44.0f;
        float contentHeight = 0.0f;
        std::string previousGroup;
        int indexInGroup = 0;
        for (const EngineCatalogEntry &entry : engineCatalog()) {
            if (entry.group != previousGroup) {
                contentHeight += headerHeight;
                previousGroup = entry.group;
                indexInGroup = 0;
            }
            if (indexInGroup % columns == 0) contentHeight += rowHeight;
            ++indexInGroup;
        }
        m_pickerMaxScrollOffset = std::max(0.0f, contentHeight - listBounds.height());
        m_pickerScrollOffset = std::min(m_pickerScrollOffset, m_pickerMaxScrollOffset);
        m_closeButton->m_bounds = content.verticalSplit(0.88f, 0.98f).horizontalSplit(0.78f, 1.0f);
        m_pickerScrollUpButton->m_bounds = content.verticalSplit(0.88f, 0.98f).horizontalSplit(0.50f, 0.63f);
        m_pickerScrollDownButton->m_bounds = content.verticalSplit(0.88f, 0.98f).horizontalSplit(0.64f, 0.77f);

        float y = listBounds.top() - m_pickerScrollOffset;
        previousGroup.clear();
        indexInGroup = 0;
        for (size_t i = 0; i < engineCatalog().size(); ++i) {
            const EngineCatalogEntry &entry = engineCatalog()[i];
            if (entry.group != previousGroup) {
                previousGroup = entry.group;
                indexInGroup = 0;
                y -= headerHeight;
            }
            const int column = indexInGroup % columns;
            if (column == 0) y -= rowHeight;
            const float buttonWidth = listBounds.width() / columns;
            m_engineButtons[i]->m_fontSize = 15.0f;
            m_engineButtons[i]->m_bounds = Bounds(
                buttonWidth - 8.0f, rowHeight - 8.0f,
                { listBounds.left() + column * buttonWidth + 4.0f, y + rowHeight - 4.0f }, Bounds::tl);
            const Bounds &bounds = m_engineButtons[i]->m_bounds;
            m_engineButtons[i]->setVisible(bounds.bottom() >= listBounds.bottom() && bounds.top() <= listBounds.top());
            ++indexInGroup;
        }
        return;
    }

    const PickerLayout &layout = kModernPickerLayout;
    const Bounds content = panel.inset(layout.panelInset);
    const Bounds listBounds(
        content.width(),
        content.height() - layout.titleHeight - layout.footerHeight,
        { content.left(), content.top() - layout.titleHeight }, Bounds::tl);
    const int columns = listBounds.width() >= 760.0f ? 2 : 1;
    float contentHeight = 0.0f;
    std::string previousGroup;
    int indexInGroup = 0;
    for (const EngineCatalogEntry &entry : engineCatalog()) {
        if (entry.group != previousGroup) {
            contentHeight += layout.headerHeight;
            previousGroup = entry.group;
            indexInGroup = 0;
        }
        if (indexInGroup % columns == 0) contentHeight += layout.rowHeight;
        ++indexInGroup;
    }
    m_pickerMaxScrollOffset = std::max(0.0f, contentHeight - listBounds.height());
    m_pickerScrollOffset = std::min(m_pickerScrollOffset, m_pickerMaxScrollOffset);
    const Bounds footer(content.width(), layout.footerHeight,
        { content.left(), content.bottom() }, Bounds::bl);
    m_closeButton->m_bounds = footer.horizontalSplit(0.78f, 1.0f).inset(5.0f);
    m_pickerScrollUpButton->m_bounds = footer.horizontalSplit(0.50f, 0.63f).inset(5.0f);
    m_pickerScrollDownButton->m_bounds = footer.horizontalSplit(0.64f, 0.77f).inset(5.0f);

    float y = listBounds.top() - m_pickerScrollOffset;
    previousGroup.clear();
    indexInGroup = 0;
    for (size_t i = 0; i < engineCatalog().size(); ++i) {
        const EngineCatalogEntry &entry = engineCatalog()[i];
        if (entry.group != previousGroup) {
            previousGroup = entry.group;
            indexInGroup = 0;
            y -= layout.headerHeight;
        }
        const int column = indexInGroup % columns;
        if (column == 0) y -= layout.rowHeight;
        const float buttonWidth = listBounds.width() / columns;
        m_engineButtons[i]->m_fontSize = layout.itemFontSize;
        m_engineButtons[i]->m_bounds = Bounds(
            buttonWidth - layout.itemGap, layout.rowHeight - layout.itemGap,
            { listBounds.left() + column * buttonWidth + layout.itemGap / 2.0f,
              y + layout.rowHeight - layout.itemGap / 2.0f }, Bounds::tl);
        const Bounds &bounds = m_engineButtons[i]->m_bounds;
        m_engineButtons[i]->setVisible(bounds.bottom() >= listBounds.bottom() && bounds.top() <= listBounds.top());
        ++indexInGroup;
    }
}

void OverlayHost::layoutSettings(const Bounds &panel) {
    const Bounds content = panel.inset(kSettingsLayout.panelInset);
    m_closeButton->m_bounds = Bounds(
        kSettingsLayout.closeWidth, kSettingsLayout.rowHeight,
        { content.right(), content.top() }, Bounds::tr);

    const float rowsTop = content.top() - kSettingsLayout.titleHeight - kSettingsLayout.titleGap;
    const float labelWidth = content.width() * kSettingsLayout.labelWidth;
    const Bounds controls(content.width() - labelWidth, kSettingsLayout.rowHeight,
        { content.left() + labelWidth, rowsTop }, Bounds::tl);
    const float half = (controls.width() - kSettingsLayout.controlGap) / 2.0f;
    m_legacyUiButton->m_bounds = Bounds(half, controls.height(), controls.getPosition(Bounds::tl), Bounds::tl);
    m_modernUiButton->m_bounds = Bounds(half, controls.height(), controls.getPosition(Bounds::tr), Bounds::tr);

    const float unitsY = rowsTop - kSettingsLayout.rowHeight - kSettingsLayout.rowGap;
    const Bounds unitControls(controls.width(), kSettingsLayout.rowHeight,
        { controls.left(), unitsY }, Bounds::tl);
    m_imperialButton->m_bounds = Bounds(half, unitControls.height(), unitControls.getPosition(Bounds::tl), Bounds::tl);
    m_metricButton->m_bounds = Bounds(half, unitControls.height(), unitControls.getPosition(Bounds::tr), Bounds::tr);

    const float currencyY = unitsY - kSettingsLayout.rowHeight - kSettingsLayout.rowGap
        - kSettingsLayout.sectionHeight;
    const Bounds currencyControls(controls.width(), kSettingsLayout.rowHeight,
        { controls.left(), currencyY }, Bounds::tl);
    const float arrowWidth = std::min(90.0f, currencyControls.width() * 0.18f);
    m_currencyPreviousButton->m_bounds = Bounds(arrowWidth, currencyControls.height(),
        currencyControls.getPosition(Bounds::tl), Bounds::tl);
    m_currencyNextButton->m_bounds = Bounds(arrowWidth, currencyControls.height(),
        currencyControls.getPosition(Bounds::tr), Bounds::tr);
    m_currencyButton->m_bounds = Bounds(
        currencyControls.width() - arrowWidth * 2.0f - kSettingsLayout.controlGap * 2.0f,
        currencyControls.height(),
        { currencyControls.left() + arrowWidth + kSettingsLayout.controlGap, currencyControls.top() }, Bounds::tl);

    const float priceY = currencyY - kSettingsLayout.rowHeight - kSettingsLayout.rowGap;
    const Bounds priceControls(controls.width(), kSettingsLayout.rowHeight,
        { controls.left(), priceY }, Bounds::tl);
    const float priceButtonWidth = (priceControls.width() - kSettingsLayout.controlGap * 3.0f) / 4.0f;
    UiButton *priceButtons[] = {
        m_priceMinusLargeButton, m_priceMinusButton, m_pricePlusButton, m_pricePlusLargeButton
    };
    for (int i = 0; i < 4; ++i) {
        priceButtons[i]->m_bounds = Bounds(priceButtonWidth, priceControls.height(),
            { priceControls.left() + i * (priceButtonWidth + kSettingsLayout.controlGap), priceControls.top() }, Bounds::tl);
    }

    m_resetSettingsButton->m_bounds = Bounds(
        kSettingsLayout.resetWidth, kSettingsLayout.rowHeight,
        { content.left(), content.bottom() }, Bounds::bl);
    syncSettingsLabels();
}

void OverlayHost::syncSettingsLabels() {
    const UserPreferences &preferences = m_app->getUserPreferences();
    m_currencyButton->m_text = preferences.currencyCode;
    const auto priceDelta = [&](double amount) {
        std::ostringstream label;
        label << (amount < 0.0 ? "- " : "+ ") << std::fixed
              << std::setprecision(preferences.currency().adjustmentStep >= 1.0 ? 0 : 2)
              << std::abs(amount);
        return label.str();
    };
    const double step = preferences.currency().adjustmentStep;
    m_priceMinusLargeButton->m_text = priceDelta(-step * 10.0);
    m_priceMinusButton->m_text = priceDelta(-step);
    m_pricePlusButton->m_text = priceDelta(step);
    m_pricePlusLargeButton->m_text = priceDelta(step * 10.0);
    const bool metric = preferences.unitSystem == UnitSystem::Metric;
    const bool modern = preferences.uiStyle == UiStyle::Modern;
    m_legacyUiButton->m_drawFrame = true;
    m_modernUiButton->m_drawFrame = true;
    m_legacyUiButton->m_inverted = !modern;
    m_modernUiButton->m_inverted = modern;
    m_metricButton->m_drawFrame = true;
    m_imperialButton->m_drawFrame = true;
    m_metricButton->m_inverted = metric;
    m_imperialButton->m_inverted = !metric;
}

void OverlayHost::signal(UiElement *element, Event event) {
    if (event != Event::Clicked) return;
    if (element == m_closeButton) dismiss();
    else if (element == m_githubButton) m_app->getPlatform()->openUrl("https://github.com/carlesonielfa/open-engine-sim");
    else if (element == m_issuesButton) m_app->getPlatform()->openUrl("https://github.com/carlesonielfa/open-engine-sim/issues");
    else if (element == m_pickerScrollUpButton) m_pickerScrollOffset = std::max(0.0f, m_pickerScrollOffset - 180.0f);
    else if (element == m_pickerScrollDownButton) m_pickerScrollOffset = std::min(m_pickerMaxScrollOffset, m_pickerScrollOffset + 180.0f);
    else if (element == m_legacyUiButton) m_app->setUiStyle(UiStyle::Legacy);
    else if (element == m_modernUiButton) m_app->setUiStyle(UiStyle::Modern);
    else if (element == m_imperialButton) m_app->setUnitSystem(UnitSystem::Imperial);
    else if (element == m_metricButton) m_app->setUnitSystem(UnitSystem::Metric);
    else if (element == m_currencyPreviousButton) m_app->cycleCurrency(-1);
    else if (element == m_currencyButton || element == m_currencyNextButton) m_app->cycleCurrency(1);
    else if (element == m_priceMinusLargeButton) m_app->adjustFuelPrice(-10);
    else if (element == m_priceMinusButton) m_app->adjustFuelPrice(-1);
    else if (element == m_pricePlusButton) m_app->adjustFuelPrice(1);
    else if (element == m_pricePlusLargeButton) m_app->adjustFuelPrice(10);
    else if (element == m_resetSettingsButton) m_app->resetUserPreferences();
    else {
        for (size_t i = 0; i < m_engineButtons.size(); ++i) {
            if (element == m_engineButtons[i]) {
                m_app->requestEngineScript(engineCatalog()[i].relativeScriptPath);
                dismiss();
                break;
            }
        }
    }
}

void OverlayHost::onMouseClick(const Point &) { if (m_kind != Kind::Trainer) dismiss(); }

void OverlayHost::onMouseScroll(int mouseScroll) {
    if (m_kind == Kind::Trainer) { m_trainer->onMouseScroll(mouseScroll); return; }
    if (m_kind != Kind::EnginePicker) return;
    m_pickerScrollOffset = std::clamp(m_pickerScrollOffset - mouseScroll * 45.0f, 0.0f, m_pickerMaxScrollOffset);
}

void OverlayHost::render() {
    if (m_kind == Kind::None) return;
    const Bounds viewport = viewportBounds();
    const Bounds panel = dialogBounds();
    const Bounds content = panel.inset(30.0f);
    const ysVector foreground = m_app->getForegroundColor();
    const ysVector background = m_app->getBackgroundColor();
    const ysVector secondary = mix(foreground, background, 0.45f);
    const ysVector scrim(background.x, background.y, background.z, 0.78f);

    // Geometry with a lower layer draws behind text and child controls. Keep
    // this ordering inside the host instead of relying on a dashboard
    // cluster's render-layer side effects.
    drawBox(viewport, scrim, -0x20);
    drawFrame(panel, 2.0f, foreground, background, true, -0x10);

    if (m_kind == Kind::Trainer) {
        drawAlignedText("ENGINE SOUND LAB / ENGINE WORKSPACE", content.verticalSplit(0.92f, 1.0f), 28 * m_app->getScreenWidth() / 1440.0f, Bounds::lm, Bounds::lm);
    }
    else if (m_kind == Kind::EnginePicker) {
        const bool modern = m_app->getUserPreferences().uiStyle == UiStyle::Modern;
        if (!modern) {
            const Bounds listBounds = content.verticalSplit(0.12f, 0.84f);
            const int columns = listBounds.width() >= 700.0f ? 3 : 2;
            drawAlignedText("SELECT ENGINE", content.verticalSplit(0.90f, 0.98f), 28.0f, Bounds::lm, Bounds::lm);
            m_app->getTextRenderer()->SetColor(secondary);
            drawAlignedText("PACKAGED ENGINES", content.verticalSplit(0.02f, 0.09f), 16.0f, Bounds::lm, Bounds::lm);
            m_app->getTextRenderer()->SetColor(foreground);

            float y = listBounds.top() - m_pickerScrollOffset;
            std::string previousGroup;
            int indexInGroup = 0;
            for (const EngineCatalogEntry &entry : engineCatalog()) {
                if (entry.group != previousGroup) {
                    previousGroup = entry.group;
                    indexInGroup = 0;
                    y -= 28.0f;
                    const Bounds heading(listBounds.width(), 28.0f,
                        { listBounds.left(), y + 28.0f }, Bounds::tl);
                    if (heading.bottom() >= listBounds.bottom() && heading.top() <= listBounds.top()) {
                        m_app->getTextRenderer()->SetColor(secondary);
                        drawAlignedText(entry.group, heading, 16.0f, Bounds::lm, Bounds::lm);
                        m_app->getTextRenderer()->SetColor(foreground);
                    }
                }
                if (indexInGroup % columns == 0) y -= 44.0f;
                ++indexInGroup;
            }
        }
        else {
        const PickerLayout &layout = kModernPickerLayout;
        const Bounds pickerContent = panel.inset(layout.panelInset);
        const Bounds listBounds(
            pickerContent.width(),
            pickerContent.height() - layout.titleHeight - layout.footerHeight,
            { pickerContent.left(), pickerContent.top() - layout.titleHeight }, Bounds::tl);
        const int columns = listBounds.width() >= 760.0f ? 2 : 1;
        drawAlignedText("SELECT ENGINE", Bounds(
            pickerContent.width(), layout.titleHeight,
            pickerContent.getPosition(Bounds::tl), Bounds::tl),
            modern ? 32.0f : 28.0f, Bounds::lm, Bounds::lm);
        m_app->getTextRenderer()->SetColor(secondary);
        drawAlignedText("PACKAGED ENGINES", Bounds(
            pickerContent.width() * 0.46f, layout.footerHeight,
            pickerContent.getPosition(Bounds::bl), Bounds::bl),
            modern ? 16.0f : 14.0f, Bounds::lm, Bounds::lm);
        m_app->getTextRenderer()->SetColor(foreground);

        float y = listBounds.top() - m_pickerScrollOffset;
        std::string previousGroup;
        int indexInGroup = 0;
        for (const EngineCatalogEntry &entry : engineCatalog()) {
            if (entry.group != previousGroup) {
                previousGroup = entry.group;
                indexInGroup = 0;
                y -= layout.headerHeight;
                const Bounds heading(listBounds.width(), layout.headerHeight,
                    { listBounds.left(), y + layout.headerHeight }, Bounds::tl);
                if (heading.bottom() >= listBounds.bottom() && heading.top() <= listBounds.top()) {
                    m_app->getTextRenderer()->SetColor(secondary);
                    drawAlignedText(entry.group, heading, layout.groupFontSize, Bounds::lm, Bounds::lm);
                    m_app->getTextRenderer()->SetColor(foreground);
                }
            }
            if (indexInGroup % columns == 0) y -= layout.rowHeight;
            ++indexInGroup;
        }
        }
    }
    else if (m_kind == Kind::Settings) {
        const Bounds settingsContent = panel.inset(kSettingsLayout.panelInset);
        drawAlignedText("DISPLAY & FUEL", Bounds(
            settingsContent.width() - kSettingsLayout.closeWidth - 20.0f,
            kSettingsLayout.titleHeight,
            settingsContent.getPosition(Bounds::tl), Bounds::tl),
            32.0f, Bounds::lm, Bounds::lm);

        const float rowsTop = settingsContent.top() - kSettingsLayout.titleHeight - kSettingsLayout.titleGap;
        const float labelWidth = settingsContent.width() * kSettingsLayout.labelWidth;
        const auto labelBounds = [&](float top) {
            return Bounds(labelWidth - 20.0f, kSettingsLayout.rowHeight,
                { settingsContent.left(), top }, Bounds::tl);
        };
        drawAlignedText("UI VERSION", labelBounds(rowsTop), 20.0f, Bounds::lm, Bounds::lm);

        const UserPreferences &preferences = m_app->getUserPreferences();
        const bool modern = preferences.uiStyle == UiStyle::Modern;
        const float unitsY = rowsTop - kSettingsLayout.rowHeight - kSettingsLayout.rowGap;
        const float currencyY = unitsY - kSettingsLayout.rowHeight - kSettingsLayout.rowGap
            - kSettingsLayout.sectionHeight;
        const float priceY = currencyY - kSettingsLayout.rowHeight - kSettingsLayout.rowGap;
        if (modern) {
            drawAlignedText("UNIT SYSTEM", labelBounds(unitsY), 20.0f, Bounds::lm, Bounds::lm);
            drawAlignedText("CURRENCY", labelBounds(currencyY), 20.0f, Bounds::lm, Bounds::lm);
            drawAlignedText("PRICE / LITRE", labelBounds(priceY), 20.0f, Bounds::lm, Bounds::lm);

            std::ostringstream price;
            price << preferences.currency().symbol << std::fixed
                  << std::setprecision(preferences.currency().adjustmentStep >= 1.0 ? 0 : 2)
                  << preferences.fuelPricePerLiter
                  << ' ' << preferences.currencyCode << " / L";
            const Bounds valueBounds(
                settingsContent.width() - labelWidth,
                kSettingsLayout.sectionHeight,
                { settingsContent.left() + labelWidth,
                  currencyY - kSettingsLayout.rowHeight - 2.0f }, Bounds::tl);
            drawAlignedText(price.str(), valueBounds, 24.0f, Bounds::lm, Bounds::lm);
        }

        m_app->getTextRenderer()->SetColor(secondary);
        drawAlignedText(modern
                ? "SET THE LOCAL PUMP PRICE FOR YOUR AREA. SETTINGS ARE SAVED AUTOMATICALLY."
                : "LEGACY PRESERVES THE ORIGINAL DASHBOARD. SELECT MODERN FOR UNITS AND FUEL SETTINGS.",
            Bounds(settingsContent.width(), 30.0f,
                { settingsContent.left(), settingsContent.bottom() + kSettingsLayout.rowHeight + 20.0f }, Bounds::bl),
            15.0f, Bounds::lm, Bounds::lm);
        m_app->getTextRenderer()->SetColor(foreground);
    }
    else {
        const Bounds controls = controlsContent(panel);
        const Bounds leftColumn = controls.horizontalSplit(0.0f, kControlsLayout.projectColumnStart - 0.04f);
        const auto drawKey = [&](const char *key, Point &cursor, float baseline) {
            const float width = m_app->getTextRenderer()->CalculateWidth(key, 14.0f) + 18.0f;
            const Bounds keyBounds(width, kControlsLayout.rowHeight, { cursor.x, baseline }, Bounds::bl);
            drawFrame(keyBounds, 1.0f, foreground, foreground, true, -0x09);
            m_app->getTextRenderer()->SetColor(background);
            drawCenteredText(key, keyBounds, 14.0f, Bounds::center);
            m_app->getTextRenderer()->SetColor(foreground);
            cursor.x += width + kControlsLayout.keyHorizontalGap;
        };
        const auto drawAction = [&](const Bounds &row, const ControlAction &action) {
            Point cursor(row.left(), row.center_v() - kControlsLayout.rowHeight / 2.0f);
            for (int i = 0; i < action.keyCount; ++i) drawKey(action.keys[i], cursor, cursor.y);
            drawAlignedText(action.label, Bounds(row.right() - cursor.x, kControlsLayout.rowHeight, cursor, Bounds::bl),
                16.0f, Bounds::lm, Bounds::lm);
            cursor.x += m_app->getTextRenderer()->CalculateWidth(action.label, 16.0f) + kControlsLayout.keyLabelGap;
            if (action.hint[0] != '\0') {
                m_app->getTextRenderer()->SetColor(secondary);
                drawAlignedText(action.hint, Bounds(row.right() - cursor.x, kControlsLayout.rowHeight, cursor, Bounds::bl),
                    14.0f, Bounds::lm, Bounds::lm);
                m_app->getTextRenderer()->SetColor(foreground);
            }
        };
        const auto drawSection = [&](VerticalLayoutCursor &cursor, const char *title,
                                     const ControlAction *actions, int count) {
            drawAlignedText(title, cursor.take(kControlsLayout.sectionTitleHeight),
                kControlsLayout.sectionTitleHeight, Bounds::lm, Bounds::lm);
            cursor.gap(kControlsLayout.sectionHeaderGap);
            for (int i = 0; i < count; ++i) {
                drawAction(cursor.take(kControlsLayout.rowHeight), actions[i]);
                if (i + 1 < count) cursor.gap(kControlsLayout.rowGap);
            }
            cursor.gap(kControlsLayout.sectionGap);
        };

        drawAlignedText("CONTROLS", Bounds(leftColumn.width(), kControlsLayout.titleHeight,
            { leftColumn.left(), controls.top() }, Bounds::tl), kControlsLayout.titleHeight, Bounds::lm, Bounds::lm);
        VerticalLayoutCursor left(leftColumn, controls.top() - kControlsLayout.titleHeight - kControlsLayout.titleGap);
        drawSection(left, "START", kStartActions, 2);
        drawSection(left, "DRIVE", kDriveActions, 3);

        const ControlAction extras[] = {
            {{ "D", nullptr, nullptr, nullptr }, 1, "DYNO", ""},
            {{ "H", nullptr, nullptr, nullptr }, 1, "HOLD", ""},
            {{ "F", nullptr, nullptr, nullptr }, 1, "FULLSCREEN", ""},
            {{ "F1", nullptr, nullptr, nullptr }, 1, "GUIDE", ""},
            {{ "F2", nullptr, nullptr, nullptr }, 1, "ENGINES", ""},
            {{ "F4", nullptr, nullptr, nullptr }, 1, "SETTINGS", ""},
        };
        drawAlignedText("EXTRAS", left.take(kControlsLayout.sectionTitleHeight),
            kControlsLayout.sectionTitleHeight, Bounds::lm, Bounds::lm);
        left.gap(kControlsLayout.sectionHeaderGap);
        const Bounds extrasRow = left.take(kControlsLayout.rowHeight);
        Point extrasCursor(extrasRow.left(), extrasRow.center_v() - kControlsLayout.rowHeight / 2.0f);
        for (const ControlAction &extra : extras) {
            drawKey(extra.keys[0], extrasCursor, extrasCursor.y);
            drawAlignedText(extra.label, Bounds(extrasRow.right() - extrasCursor.x,
                kControlsLayout.rowHeight, extrasCursor, Bounds::bl), 14.0f, Bounds::lm, Bounds::lm);
            extrasCursor.x += m_app->getTextRenderer()->CalculateWidth(extra.label, 14.0f) + kControlsLayout.keyLabelGap;
        }

        const Bounds project = projectColumn(controls);
        VerticalLayoutCursor projectCursor(project, controls.bottom() + kControlsLayout.projectTopOffset);
        drawAlignedText("PROJECT", projectCursor.take(kControlsLayout.sectionTitleHeight),
            kControlsLayout.sectionTitleHeight, Bounds::lm, Bounds::lm);
        projectCursor.gap(kControlsLayout.projectTextGap);
        m_app->getTextRenderer()->SetColor(secondary);
        drawAlignedText("OPEN-SOURCE, CROSS-PLATFORM ENGINE SOUND SIMULATOR.",
            projectCursor.take(kControlsLayout.rowHeight), 14.0f, Bounds::lm, Bounds::lm);
        projectCursor.gap(kControlsLayout.projectTextGap);
        drawAlignedText("HAVING TROUBLE? REPORT IT ON GITHUB.",
            projectCursor.take(kControlsLayout.rowHeight), 14.0f, Bounds::lm, Bounds::lm);
        m_app->getTextRenderer()->SetColor(foreground);
    }
    UiElement::render();
}
