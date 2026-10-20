// Plugins Policy by Piradex — milestone M1 desktop shell.
// Layout follows the approved "interface 2": dark navy/charcoal, electric blue, teal.
// Analysis is not implemented yet: the score area always shows "Not tested".
#include <juce_gui_basics/juce_gui_basics.h>
#include "pp/Scanner.h"

namespace
{
const juce::Colour kBackground { 0xff091523 };
const juce::Colour kPanel      { 0xff10243a };
const juce::Colour kSidebar    { 0xff0e1a2d };
const juce::Colour kText       { 0xffeaf3ff };
const juce::Colour kMuted      { 0xff7f93ae };
const juce::Colour kBlue       { 0xff3b82f6 };
const juce::Colour kTeal       { 0xff5eead4 };

constexpr int kSidebarWidth = 280;
constexpr const char* kNavItems[] = { "Analyze", "Plugin Scanner", "Comparison", "Library", "Reports", "Settings" };
}

class PluginListModel : public juce::ListBoxModel
{
public:
    explicit PluginListModel(std::function<void(int)> onSelect) : onRowSelected(std::move(onSelect)) {}

    void setPlugins(std::vector<pp::PluginInfo> newPlugins)
    {
        plugins = std::move(newPlugins);
        if (box != nullptr) box->updateContent();
    }

    const pp::PluginInfo* at(int row) const
    {
        return (row >= 0 && row < static_cast<int>(plugins.size())) ? &plugins[static_cast<size_t>(row)] : nullptr;
    }

    int getNumRows() override { return static_cast<int>(plugins.size()); }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (const auto* p = at(row))
        {
            g.fillAll(selected ? kBlue.withAlpha(0.35f) : juce::Colours::transparentBlack);
            // Two lines per row: name on top, format and state below. Full width for the name.
            g.setColour(kText);
            g.setFont(juce::FontOptions(14.0f));
            g.drawText(juce::String(p->name), 12, 6, width - 24, 22, juce::Justification::centredLeft);

            g.setColour(kTeal);
            g.setFont(juce::FontOptions(12.0f));
            g.drawText(juce::String(pp::toString(p->format)), 12, 28, 60, 18, juce::Justification::centredLeft);

            g.setColour(p->state == pp::ScanState::Failed ? juce::Colour(0xfff87171) : kMuted);
            g.drawText(juce::String(pp::toString(p->state)), width - 112, 28, 100, 18, juce::Justification::centredRight);
        }
    }

    void selectedRowsChanged(int lastRowSelected) override { onRowSelected(lastRowSelected); }

    void attach(juce::ListBox& listBox) { box = &listBox; }

private:
    std::vector<pp::PluginInfo> plugins;
    std::function<void(int)> onRowSelected;
    juce::ListBox* box = nullptr;
};

class MainComponent : public juce::Component
{
public:
    MainComponent()
        : pluginModel([this](int row) { onPluginSelected(row); })
    {
        setSize(1200, 800);

        for (const auto* name : kNavItems)
        {
            auto* button = navButtons.add(new juce::TextButton(name));
            button->setClickingTogglesState(false);
            addAndMakeVisible(button);
        }
        navButtons[0]->setToggleState(true, juce::dontSendNotification);

        titleLabel.setText("PLUGINS POLICY", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(26.0f).withStyle("Bold"));
        titleLabel.setColour(juce::Label::textColourId, kText);
        addAndMakeVisible(titleLabel);

        subtitleLabel.setText("by PIRADEX  -  AUDIO PLUGIN ANALYSIS", juce::dontSendNotification);
        subtitleLabel.setFont(juce::FontOptions(12.0f));
        subtitleLabel.setColour(juce::Label::textColourId, kBlue);
        addAndMakeVisible(subtitleLabel);

        pluginList.setModel(&pluginModel);
        pluginModel.attach(pluginList);
        pluginList.setRowHeight(52);
        pluginList.setColour(juce::ListBox::backgroundColourId, kSidebar);
        addAndMakeVisible(pluginList);

        scanButton.onClick = [this] { rescan(); };

        // Analysis is not implemented yet (M2–M4). Say so instead of doing nothing.
        analyzeButton.onClick = [] {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::InfoIcon,
                "Análise ainda não disponível",
                "A análise real ainda não está implementada. O host de plugins (M2) e as medições (M3) "
                "vêm a seguir. Não mostramos resultados simulados.");
        };
        addAndMakeVisible(scanButton);

        analyzeButton.setEnabled(false); // enabled once a plugin is selected
        analyzeButton.setColour(juce::TextButton::buttonColourId, kBlue);
        addAndMakeVisible(analyzeButton);

        pluginNameLabel.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
        pluginNameLabel.setColour(juce::Label::textColourId, kText);
        pluginNameLabel.setText("No plugin selected", juce::dontSendNotification);
        addAndMakeVisible(pluginNameLabel);

        scoreLabel.setText("Not tested", juce::dontSendNotification);
        scoreLabel.setFont(juce::FontOptions(40.0f).withStyle("Bold"));
        scoreLabel.setColour(juce::Label::textColourId, kMuted);
        scoreLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(scoreLabel);

        scoreHint.setText("Run an analysis to see the Policy Score, grade and confidence.",
                          juce::dontSendNotification);
        scoreHint.setColour(juce::Label::textColourId, kMuted);
        scoreHint.setJustificationType(juce::Justification::centred);
        scoreHint.setFont(juce::FontOptions(13.0f));
        addAndMakeVisible(scoreHint);

        rescan();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(kBackground);

        auto content = getLocalBounds();
        auto sidebar = content.removeFromLeft(kSidebarWidth);
        g.setColour(kSidebar);
        g.fillRect(sidebar);

        // Central score panel (dashed outline reserved for the "Not tested" state).
        g.setColour(kPanel);
        g.fillRoundedRectangle(scorePanelBounds.toFloat(), 14.0f);
        g.setColour(juce::Colour(0xff1b2740));
        g.drawRoundedRectangle(scorePanelBounds.toFloat(), 14.0f, 1.0f);

        // Teal accent for the status card, blue for the primary action.
        g.setColour(kTeal);
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("Scan - Available", statusBounds, juce::Justification::centredLeft, false);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto sidebar = area.removeFromLeft(kSidebarWidth).reduced(18, 28);

        titleLabel.setBounds(sidebar.removeFromTop(34));
        subtitleLabel.setBounds(sidebar.removeFromTop(20));
        sidebar.removeFromTop(24);

        for (auto* button : navButtons)
        {
            button->setBounds(sidebar.removeFromTop(38).reduced(0, 3));
            sidebar.removeFromTop(2);
        }

        sidebar.removeFromTop(16);
        scanButton.setBounds(sidebar.removeFromBottom(40));
        sidebar.removeFromBottom(10);
        pluginList.setBounds(sidebar);

        area.reduce(32, 28);

        auto header = area.removeFromTop(64);
        analyzeButton.setBounds(header.removeFromRight(180).reduced(0, 8));
        pluginNameLabel.setBounds(header);

        area.removeFromTop(24);
        auto scoreArea = area.removeFromLeft(340);
        scorePanelBounds = scoreArea;

        scoreLabel.setBounds(scoreArea.withSizeKeepingCentre(260, 70).translated(0, -30));
        scoreHint.setBounds(scoreArea.withSizeKeepingCentre(280, 50).translated(0, 60));
        statusBounds = area.removeFromBottom(24);
    }

private:
    void rescan()
    {
        pluginModel.setPlugins(pp::scanDirectories(pp::defaultPluginDirectories()));
        pluginList.updateContent();
        pluginList.repaint();
    }

    void onPluginSelected(int row)
    {
        const auto* info = pluginModel.at(row);
        if (info == nullptr)
        {
            analyzeButton.setEnabled(false);
            pluginNameLabel.setText("No plugin selected", juce::dontSendNotification);
            return;
        }

        pluginNameLabel.setText(juce::String(info->name), juce::dontSendNotification);
        // Analyze is wired in M3/M4; keep the control visible but inert until then.
        analyzeButton.setEnabled(info->state == pp::ScanState::Available);
    }

    juce::OwnedArray<juce::TextButton> navButtons;
    juce::Label titleLabel, subtitleLabel, pluginNameLabel, scoreLabel, scoreHint;
    juce::ListBox pluginList;
    PluginListModel pluginModel;
    juce::TextButton scanButton { "Scan plugins" }, analyzeButton { "Analyze" };
    juce::Rectangle<int> scorePanelBounds, statusBounds;
};

class PluginsPolicyApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "Plugins Policy"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted(const juce::String&) override {}

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name)
            : DocumentWindow(name, kBackground, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(PluginsPolicyApplication)
