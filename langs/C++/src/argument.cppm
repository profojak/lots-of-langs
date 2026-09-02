module;

#include <cstdlib>
#include <format>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module argument;

namespace pbf {

export struct Arguments {
  std::optional<std::string> configuration_file;
  bool no_gui{false};
  std::vector<std::pair<std::string, std::string>> parameters;
};

void PrintHelp() {
  std::println("Position Based Fluids in {}", PBF_LANGUAGE);
}

export Arguments ParseArguments(std::span<char *const> args) {
  Arguments arguments;
  for (int i = 1; i < args.size(); ++i) {
    std::string_view argument = args[i];
    if (argument == "-h" || argument == "--help") {
      PrintHelp();
      std::exit(0);
    } else if (argument == "-n" || argument == "--no-gui") {
      arguments.no_gui = true;
    } else if (argument == "-c" || argument == "--config") {
      if (i + 1 >= args.size())
        throw std::invalid_argument("Missing value for configuration file!");
      arguments.configuration_file = args[++i];
    } else if (const auto equals = argument.find("="); equals != std::string_view::npos) {
      const auto key = argument.substr(0, equals);
      const auto value = argument.substr(equals + 1);
      if (key.empty() || value.empty())
        throw std::invalid_argument(
            std::format("Key or value cannot be empty, got '{}={}'!", key, value));
      arguments.parameters.emplace_back(key, value);
    } else
      throw std::invalid_argument(std::format("Unknown argument '{}'!", argument));
  }
  return arguments;
}

} // namespace pbf
