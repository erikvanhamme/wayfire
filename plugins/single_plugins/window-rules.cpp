#include <algorithm>
#include <cfloat>
#include <memory>
#include <vector>

#include <iostream>

#include <wayfire/plugin.hpp>
#include <wayfire/view.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/view-transform.hpp>
#include <wayfire/parser/rule_parser.hpp>
#include <wayfire/lexer/lexer.hpp>
#include <wayfire/variant.hpp>
#include <wayfire/rule/rule_interface.hpp>
#include <wayfire/rule/rule.hpp>

class view_rule_interface_t : public wf::rule_interface_t
{
public:
    virtual ~view_rule_interface_t() override;
    virtual wf::variant_t get(const std::string &identifier, bool &error) override;
    virtual bool execute(const std::string &name, const std::vector<wf::variant_t> &args) override;
    void set_view(wayfire_view view);
private:
    wayfire_view _view;
};

view_rule_interface_t::~view_rule_interface_t()
{
}

wf::variant_t view_rule_interface_t::get(const std::string &identifier, bool &error)
{
    // Assume things will go well.
    error = false;
    if (identifier == "app_id")
    {
        return _view->get_app_id();
    }
    else if (identifier == "title")
    {
        return _view->get_title();
    }

    std::cerr << "View rule interface: Get operation triggered to unsupported view property " << identifier << std::endl;
    error = true;
    return "";
}

bool view_rule_interface_t::execute(const std::string &name, const std::vector<wf::variant_t> &args)
{
    if (name == "set")
    {
        if ((args.size() < 2) || (wf::is_string(args.at(0)) == false))
        {
            std::cerr << "View rule interface: Set execution requires at least 2 arguments, the first of which should be an identifier." << std::endl;
            return true;
        }

        auto id = wf::get_string(args.at(0));

        if (id == "alpha")
        {
            if (args.size() != 2)
            {
                std::cerr << "View rule interface: invalid arguments. Expected 'set alpha [float|double]" << std::endl;
                return true;
            }

            float alpha = 1.0f;
            if (wf::is_float(args.at(1)))
            {
                alpha = wf::get_float(args.at(1));
            }
            else if (wf::is_double(args.at(1)))
            {
                alpha = wf::get_double(args.at(1));
            }
            else
            {
                std::cerr << "View rule interface: invalid arguments. Expected 'set alpha [float|double]" << std::endl;
                return true;
            }
            alpha = std::clamp(alpha, 0.1f, 1.0f);

            // Apply view transformer if needed and set alpha.
            wf::view_2D *transformer;

            if (!_view->get_transformer("alpha"))
            {
                _view->add_transformer(std::make_unique<wf::view_2D> (_view), "alpha");
            }

            transformer = dynamic_cast<wf::view_2D *> (_view->get_transformer("alpha").get());
            if (fabs(transformer->alpha - alpha) > FLT_EPSILON)
            {
                transformer->alpha = alpha;
                _view->damage();

                std::cout << "View rule interface: alpha set to " << alpha << std::endl;
            }
        }
        else
        {
            std::cerr << "View rule interface: Unsupported set operation to identifier " << id << std::endl;
            return true;
        }

        return false;
    }

    std::cerr << "View rule interface: Unsupported action execution requested. Name: " << name << std::endl;
    return true;
}

void view_rule_interface_t::set_view(wayfire_view view)
{
    _view = view;
}

class wayfire_window_rules : public wf::plugin_interface_t
{
private:
    wf::lexer_t _lexer;

    wf::signal_callback_t _created; //, maximized, fullscreened;

    std::vector<std::shared_ptr<wf::rule_t>> _rules;
    view_rule_interface_t _rule_interface;
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
                _rule_interface.set_view(view);
                auto error = rule->apply("created", _rule_interface);
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
};

DECLARE_WAYFIRE_PLUGIN(wayfire_window_rules);
