#ifndef COMPOUND_TEMPLATES_HPP
#define COMPOUND_TEMPLATES_HPP

#include "compound.hpp"
#include <string>
#include <unordered_map>

extern std::unordered_map<std::string, Compound> compounds_templates;

std::function<CompoundInteractionType(
Compound*, int,
Compound*, int)> getInteraction(CompoundInteractionType id);

#endif // !COMPOUND_TEMPLATES_HPP
