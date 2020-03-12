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

#include "view_action_interface.hpp"

class wayfire_window_rules : public wf::plugin_interface_t
{
public:
    void init()
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
            for (const auto &rule : _rules)
            {
                auto view = get_signaled_view(data);
                _access_interface.set_view(view);
                _action_interface.set_view(view);
                auto error = rule->apply("created", _access_interface, _action_interface);
                if (error)
                {
                    std::cerr << "Window-rules: Error while executing rule on created signal." << std::endl;
                }
            }
        };
        output->connect_signal("map-view", &_created);

//        maximized = [=] (wf::signal_data_t *data)
//        {
//            auto conv = static_cast<view_tiled_signal*> (data);
//            assert(conv);

//            if (conv->edges != wf::TILED_EDGES_ALL)
//                return;

//            for (const auto& rule : rules_list["maximized"])
//                rule(conv->view);
//        };
//        output->connect_signal("view-maximized", &maximized);

//        fullscreened = [=] (wf::signal_data_t *data)
//        {
//            auto conv = static_cast<view_fullscreen_signal*> (data);
//            assert(conv);

//            if (!conv->state || conv->carried_out)
//                return;

//            for (const auto& rule : rules_list["fullscreened"])
//                rule(conv->view);
//            conv->carried_out = true;
//        };
//        output->connect_signal("view-fullscreen", &fullscreened);
    }

    void fini()
    {
        output->disconnect_signal("map-view", &_created);
//        output->disconnect_signal("view-maximized", &maximized);
//        output->disconnect_signal("view-fullscreen", &fullscreened);
    }
private:
    wf::lexer_t _lexer;

    wf::signal_callback_t _created; //, maximized, fullscreened;

    std::vector<std::shared_ptr<wf::rule_t>> _rules;
    wf::view_access_interface_t _access_interface;
    wf::view_action_interface_t _action_interface;
};

DECLARE_WAYFIRE_PLUGIN(wayfire_window_rules);
