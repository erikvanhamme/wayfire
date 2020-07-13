#include <algorithm>
#include <cfloat>
#include <memory>
#include <vector>

#include <iostream>

#include <wayfire/plugin.hpp>
#include <wayfire/view.hpp>
#include <wayfire/view-access-interface.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/view-transform.hpp>
#include <wayfire/parser/rule_parser.hpp>
#include <wayfire/lexer/lexer.hpp>
#include <wayfire/variant.hpp>
#include <wayfire/rule/lambda_rule.hpp>
#include <wayfire/rule/rule_interface.hpp>
#include <wayfire/rule/rule.hpp>

#include "../common/lambda-rules-registration.hpp"
#include "../common/view-action-interface.hpp"

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

    nonstd::observer_ptr<wf::lambda_rules_registrations_t> _lambda_registrations;
};

void wayfire_window_rules_t::init()
{
    // Get the lambda rules registrations.
    _lambda_registrations = wf::lambda_rules_registrations_t::getInstance();

    // TODO: Remove debug/test code.
    // Test rule
    auto reg = std::make_shared<wf::lambda_rule_registration_t>();
    reg->rule = "on created if title contains \"Alacritty\"";
    reg->if_lambda = [] () {
        std::cout << "test rule IF_LAMBDA!" << std::endl;
        return false;
    };
    reg->else_lambda = [] () {
        std::cout << "test rule ELSE_LAMBDA!" << std::endl;
        return false;
    };
    auto error = _lambda_registrations->registerLambdaRule("test_rule", reg);
    if (error) {
        std::cerr << "Window-rules: Error while registering test rule." << std::endl;
    }

    // Build rule list.
    auto section = wf::get_core().config.get_section("window-rules");
    for (auto opt : section->get_registered_options())
    {
        _lexer.reset(opt->get_value_str());
        auto rule = wf::rule_parser_t().parse(_lexer);
        if (rule != nullptr) {
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
    if (data == nullptr) {
        return;
    }

    for (const auto &rule : _rules)
    {
        auto view = get_signaled_view(data);
        _access_interface.set_view(view);
        _action_interface.set_view(view);
        auto error = rule->apply(signal, _access_interface, _action_interface);
        if (error) {
            std::cerr << "Window-rules: Error while executing rule on " << signal << " signal." << std::endl;
        }
    }

    auto bounds = _lambda_registrations->rules();
    auto begin = std::get<0>(bounds);
    auto end = std::get<1>(bounds);
    while (begin != end)
    {
        auto registration = std::get<1>(*begin);
        bool error = false;

        if (registration->access_interface == nullptr)
        {
            auto view = get_signaled_view(data);
            _access_interface.set_view(view);

            // TODO: Remove debug/test code.
            std::cout << "Apply rule to view: " << view->get_app_id() << ": ";

            error = registration->rule_instance->apply(signal, _access_interface);
        }
        else {
            error = registration->rule_instance->apply(signal, *registration->access_interface);
        }

        if (error) {
            std::cerr << "Window-rules: Error while executing rule on " << signal << " signal." << std::endl;
        }

        ++begin;
    }
}

DECLARE_WAYFIRE_PLUGIN(wayfire_window_rules_t);
