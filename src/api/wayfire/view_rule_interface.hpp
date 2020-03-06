#ifndef VIEW_RULE_INTERFACE_HPP
#define VIEW_RULE_INTERFACE_HPP

#include "wayfire/rule/rule_interface.hpp"
#include "wayfire/view.hpp"
#include <string>
#include <tuple>
#include <vector>

namespace wf
{

class view_rule_interface_t : public rule_interface_t
{
public:
    virtual ~view_rule_interface_t() override;

    virtual variant_t get(const std::string &identifier, bool &error) override;

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

#endif // VIEW_RULE_INTERFACE_HPP
