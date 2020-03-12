#ifndef VIEW_ACTION_INTERFACE_HPP
#define VIEW_ACTION_INTERFACE_HPP

#include "wayfire/action/action_interface.hpp"
#include "wayfire/view.hpp"
#include <string>
#include <tuple>
#include <vector>

namespace wf
{

class view_action_interface_t : public action_interface_t
{
public:
    virtual ~view_action_interface_t() override;

    virtual bool execute(const std::string &name, const std::vector<variant_t> &args) override;

    void set_view(wayfire_view view);
private:
    void _maximize();
    void _unmaximize();

    std::tuple<bool, float> _validate_alpha(const std::vector<variant_t> &args);
    void _set_alpha(float alpha);

    wayfire_view _view;
};

} // End namespace wf.

#endif // VIEW_ACTION_INTERFACE_HPP
