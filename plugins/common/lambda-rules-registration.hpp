#ifndef LAMBDARULESREGISTRATION_HPP
#define LAMBDARULESREGISTRATION_HPP

#include <map>
#include <memory>
#include <string>
#include <tuple>

#include "wayfire/object.hpp"
#include "wayfire/nonstd/observer_ptr.h"
#include "wayfire/rule/lambda_rule.hpp"

namespace wf
{

struct lambda_rule_registration_t;

using map_type = std::map<std::string, std::shared_ptr<lambda_rule_registration_t>>;

/**
 * @brief The lambda_rule_registration_t struct represents registration information for a single lambda rule.
 *
 * To make a registration, create one of these structures in a shared_ptr, fill in the appropriate values and
 * register it on the lambda_rules_registrations_t singleton instance.
 *
 * At minimum, the rule string and if_lambda need to be set.
 */
struct lambda_rule_registration_t
{
    /**
     * @brief rule This is the rule text.
     *
     * @note The registering plugin is supposed to set this value before registering.
     */
    std::string rule;

    /**
     * @brief if_lambda This is the lambda method to be executed if the specified condition holds.
     *
     * @note The registering plugin is supposed to set this value before registering.
     */
    wf::lambda_t if_lambda;

    /**
     * @brief else_lambda This is the lambda method to be executed if the specified condition does not hold.
     *
     * @note The registering plugin is supposed to set this value before registering.
     * @note In most cases this should be left blank.
     *
     * @attention: Be very careful with this lambda because it will be executed on the signal for each view
     *             that did NOT match the condition.
     */
    wf::lambda_t else_lambda;

    /**
     * @brief access_interface Access interface to be used when evaluating the rule.
     *
     * @note If this is left blank (nullptr), the standard view_access_interface_t instance will be used.
     */
    std::shared_ptr<wf::access_interface_t> access_interface;

    /**
     * @brief rule_instance Pointer to the parsed rule object.
     *
     * @attention You should not set this. Leave it at nullptr, the registration process will fill in this
     *            variable. Window rules can then use this cached rule instance on each signal occurrence.
     */
    std::shared_ptr<wf::lambda_rule_t> rule_instance;
};

/**
 * @brief The lambda_rules_registrations_t class is a helper class for easy registration and unregistration of
 *        lambda rules for the window rules plugin.
 *
 * This class is a singleton and can only be used via the getInstance() method.
 *
 * The instance is stored in wf::core. The getInstance() method will fetch from wf:core, and lazy-init if the
 * instance is not yet present.
 */
class lambda_rules_registrations_t : public custom_data_t
{
  public:
    /**
     * @brief getInstance Static accessor for the singleton.
     *
     * @return Observer pointer to the singleton instance, fetched from wf::core.
     */
    static nonstd::observer_ptr<lambda_rules_registrations_t> getInstance();

    /**
     * @brief registerLambdaRule Registers a lambda rule with its associated key.
     *
     * This method will return error result if the key is not unique or the registration struct is incomplete.
     *
     * @param[in] key Unique key for the registration.
     * @param[in] registration The registration structure.
     *
     * @return <code>True</code> in case of error, <code>false</code> if ok.
     */
    bool registerLambdaRule(std::string key, std::shared_ptr<lambda_rule_registration_t> registration);

    /**
     * @brief unregisterLambdaRule Unregisters a lambda rule with its associated key.
     *
     * Has no effect if no rule is registered with this key.
     *
     * @param[in] key Unique key for the registration.
     */
    void unregisterLambdaRule(std::string key);

    /**
     * @brief rules Gets the boundaries of the rules map as a tuple of cbegin() and cend() const_iterators.
     *
     * @return Boundaries of the rules map.
     */
    std::tuple<map_type::const_iterator, map_type::const_iterator> rules();
  private:
    /**
     * @brief lambda_rules_registrations_t Constructor, private to enforce singleton design pattern.
     */
    lambda_rules_registrations_t();

    /**
     * @brief _registrations The map holding all the current registrations.
     */
    map_type _registrations;
};

} // End namespace wf.

#endif // LAMBDARULESREGISTRATION_HPP
