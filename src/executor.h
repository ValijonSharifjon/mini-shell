#pragma once
#include "parser.h"
#include <string>

void execute_pipeline(std::vector<Command>& commands);
void execute_command(ParsedLine& parsed, const std::string& line);
