#pragma once
#include <string>

#include "parser.h"

void execute_pipeline(std::vector<Command>& commands);
void execute_command(ParsedLine& parsed, const std::string& line);
