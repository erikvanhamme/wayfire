#include "wayfire/rule/rule_interface.hpp"
#include "wayfire/output.hpp"
#include "wayfire/view.hpp"
#include "view_action_interface.hpp"
#include "snap_signal.hpp"
#include "wayfire/view-transform.hpp"
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <string>
#include <vector>

namespace wf
{

view_action_interface_t::~view_action_interface_t()
{
}

bool view_action_interface_t::execute(const std::string &name, const std::vector<variant_t> &args)
{
    if (name == "set")
    {
        if ((args.size() < 2) || (wf::is_string(args.at(0)) == false))
        {
            std::cerr << "View action interface: Set execution requires at least 2 arguments, the first of which should be an identifier." << std::endl;
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
            std::cerr << "View action interface: Unsupported set operation to identifier " << id << std::endl;
            return true;
        }

        return false;
    }
    else if (name == "maximize")
    {
        _maximize();
        return false;
    }
    else if (name == "unmaximize")
    {
        _unmaximize();
        return false;
    }
    else if (name == "snap")
    {
        if ((args.size() < 1) || (wf::is_string(args.at(0)) == false))
        {
            std::cerr << "View action interface: Snap execution requires 1 string as argument." << std::endl;
            return true;
        }

        auto output = _view->get_output();
        if (output == nullptr)
        {
            std::cerr << "View action interface: Output associated with view was null." << std::endl;
            return true;
        }

        auto location = wf::get_string(args.at(0));

        snap_signal data;
        data.view = _view;

        if (location == "top")
        {
            data.slot = SLOT_TOP;
        }
        else if (location == "top_right")
        {
            data.slot = SLOT_TR;
        }
        else if (location == "right")
        {
            data.slot = SLOT_RIGHT;
        }
        else if (location == "bottom_right")
        {
            data.slot = SLOT_BR;
        }
        else if (location == "bottom")
        {
            data.slot = SLOT_BOTTOM;
        }
        else if (location == "bottom_left")
        {
            data.slot = SLOT_BL;
        }
        else if (location == "left")
        {
            data.slot = SLOT_LEFT;
        }
        else if (location == "top_left")
        {
            data.slot = SLOT_TL;
        }
        else if (location == "center")
        {
            data.slot = SLOT_CENTER;
        }
        else
        {
            std::cerr << "View action interface: Incorrect string literal for snap location: " << location << "." << std::endl;
            return true;
        }

        std::cout << "View action interface: Snap to " << location << "." << std::endl;

        output->emit_signal("view-snap", &data);

        return false;
    }

    std::cerr << "View action interface: Unsupported action execution requested. Name: " << name << "." << std::endl;
    return true;
}

void view_action_interface_t::set_view(wayfire_view view)
{
    _view = view;
}

void view_action_interface_t::_maximize()
{
    _view->tile_request(wf::TILED_EDGES_ALL);
}

void view_action_interface_t::_unmaximize()
{
    _view->tile_request(0);
}

std::tuple<bool, float> view_action_interface_t::_validate_alpha(const std::vector<variant_t> &args)
{
    auto alpha = 1.0f;

    if (args.size() != 2)
    {
        std::cerr << "View action interface: Invalid arguments. Expected 'set alpha [float|double]." << std::endl;
        return {false, alpha};
    }

    if (wf::is_float(args.at(1)))
    {
        alpha = wf::get_float(args.at(1));
    }
    else if (wf::is_double(args.at(1)))
    {
        alpha = static_cast<float>(wf::get_double(args.at(1)));
    }
    else
    {
        std::cerr << "View action interface: Invalid arguments. Expected 'set alpha [float|double]." << std::endl;
        return {false, alpha};
    }

    return {true, alpha};
}

void view_action_interface_t::_set_alpha(float alpha)
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

        std::cout << "View action interface: Alpha set to " << alpha << "." << std::endl;
    }
}

} // End namespace wf.
