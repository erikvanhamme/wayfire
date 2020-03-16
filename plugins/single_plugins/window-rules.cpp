#include <algorithm>
#include <cfloat>
#include <memory>
#include <vector>

#include <iostream>

#include <wayfire/plugin.hpp>
#include <wayfire/view.hpp>
#include <wayfire/view_access_interface.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/view-transform.hpp>
#include <wayfire/parser/rule_parser.hpp>
#include <wayfire/lexer/lexer.hpp>
#include <wayfire/variant.hpp>
#include <wayfire/rule/rule_interface.hpp>
#include <wayfire/rule/rule.hpp>

#include "../common/view_action_interface.hpp"

class wayfire_window_rules_t : public wf::plugin_interface_t
{
public:
    void init() override;
    void fini() override;
    void apply(const std::string &signal, wf::signal_data_t *data);
private:
    wf::lexer_t _lexer;

    wf::signal_callback_t _created;
    wf::signal_callback_t _maximized;
    wf::signal_callback_t _minimized;
    wf::signal_callback_t _fullscreened;

    std::vector<std::shared_ptr<wf::rule_t>> _rules;

    wf::view_access_interface_t _access_interface;
    wf::view_action_interface_t _action_interface;
};

void wayfire_window_rules_t::init()
{
    // Build rule list.
    auto section = wf::get_core().config.get_section("window-rules");
    for (auto opt : section->get_registered_options())
    {
        _lexer.reset(opt->get_value_str());
        auto rule = wf::rule_parser_t().parse(_lexer);
        if (rule != nullptr)
        {
            _rules.push_back(rule);
        }
    }

    // Created rule handler.
    _created = [=] (wf::signal_data_t *data)
    {
        apply("created", data);
    };
    output->connect_signal("map-view", &_created);

    // Maximized rule handler.
    _maximized = [=] (wf::signal_data_t *data)
    {
        apply("maximized", data);
    };
    output->connect_signal("view-maximized", &_maximized);

    // Minimized rule handler.
    _minimized = [=] (wf::signal_data_t *data)
    {
        apply("minimized", data);
    };
    output->connect_signal("view-minimized", &_minimized);

    // Fullscreened rule handler.
    _fullscreened = [=] (wf::signal_data_t *data)
    {
        apply("fullscreened", data);
    };
    output->connect_signal("view-fullscreen", &_fullscreened);
}

void wayfire_window_rules_t::fini()
{
    output->disconnect_signal("map-view", &_created);
    output->disconnect_signal("view-maximized", &_maximized);
    output->disconnect_signal("view-minimized", &_minimized);
    output->disconnect_signal("view-fullscreen", &_fullscreened);
}

void wayfire_window_rules_t::apply(const std::string &signal, wf::signal_data_t *data)
{
    if (data == nullptr)
    {
        return;
    }

    for (const auto &rule : _rules)
    {
        auto view = get_signaled_view(data);
        _access_interface.set_view(view);
        _action_interface.set_view(view);
        auto error = rule->apply(signal, _access_interface, _action_interface);
        if (error)
        {
            std::cerr << "Window-rules: Error while executing rule on " << signal << " signal." << std::endl;
        }
    }
}

DECLARE_WAYFIRE_PLUGIN(wayfire_window_rules_t);
