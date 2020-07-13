#include "lambda-rules-registration.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "wayfire/core.hpp"
#include "wayfire/object.hpp"
#include "wayfire/nonstd/observer_ptr.h"
#include "wayfire/parser/lambda_rule_parser.hpp"
#include "wayfire/rule/lambda_rule.hpp"

namespace wf
{

nonstd::observer_ptr<lambda_rules_registrations_t> lambda_rules_registrations_t::getInstance()
{
    auto instance = get_core().get_data<lambda_rules_registrations_t>();
    if (instance == nullptr)
    {
        get_core().store_data(std::unique_ptr<lambda_rules_registrations_t>(new lambda_rules_registrations_t()));

        instance = get_core().get_data<lambda_rules_registrations_t>();

        if (instance == nullptr) {
            std::cerr << "Window lambda rules: Lazy-init of lambda registrations failed." << std::endl;
        } else {
            std::cout << "Window lambda rules: Lazy-init of lambda registrations succeeded." << std::endl;
        }
    }

    return instance;
}

bool lambda_rules_registrations_t::registerLambdaRule(std::string key, std::shared_ptr<lambda_rule_registration_t> registration)
{
    if (_registrations.find(key) != _registrations.end()) {
        return true; // Error, key already exists.
    }

    if (registration->if_lambda == nullptr) {
        return true; // Error, no if lambda specified.
    }

    registration->rule_instance = lambda_rule_parser_t().parse(registration->rule, registration->if_lambda, registration->else_lambda);
    if (registration->rule_instance == nullptr) {
        return true; // Error, failed to parse rule.
    }

    _registrations.emplace(key, registration);

    return false;
}

void lambda_rules_registrations_t::unregisterLambdaRule(std::string key)
{
    _registrations.erase(key);
}

std::tuple<map_type::const_iterator, map_type::const_iterator> lambda_rules_registrations_t::rules()
{
    return std::tuple(_registrations.cbegin(), _registrations.cend());
}

lambda_rules_registrations_t::lambda_rules_registrations_t()
{
}

} // End namespace wf.
