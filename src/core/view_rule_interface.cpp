#include "wayfire/rule/rule_interface.hpp"
#include "wayfire/view.hpp"
#include "wayfire/view_rule_interface.hpp"
#include "wayfire/view-transform.hpp"
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <string>
#include <vector>

namespace wf
{

view_rule_interface_t::~view_rule_interface_t()
{
}

variant_t view_rule_interface_t::get(const std::string &identifier, bool &error)
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

bool view_rule_interface_t::execute(const std::string &name, const std::vector<variant_t> &args)
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
            auto validated = _validate_alpha(args);
            if (std::get<bool>(validated))
            {
                _set_alpha(std::get<float>(validated));
            }
        }
        else
        {
            std::cerr << "View rule interface: Unsupported set operation to identifier " << id << std::endl;
            return true;
        }

        return false;
    }
    else if (name == "maximize")
    {
        _maximize();
    }

    std::cerr << "View rule interface: Unsupported action execution requested. Name: " << name << std::endl;
    return true;
}

void view_rule_interface_t::set_view(wayfire_view view)
{
    _view = view;
}

void view_rule_interface_t::_maximize()
{
    // TODO: Figure out how to maximize a view.

}

std::tuple<bool, float> view_rule_interface_t::_validate_alpha(const std::vector<variant_t> &args)
{
    auto alpha = 1.0f;

    if (args.size() != 2)
    {
        std::cerr << "View rule interface: invalid arguments. Expected 'set alpha [float|double]" << std::endl;
        return {false, alpha};
    }

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
        return {false, alpha};
    }

    return {true, alpha};
}

void view_rule_interface_t::_set_alpha(float alpha)
{
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

} // End namespace wf.
