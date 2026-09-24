#include <print>

import argument;
import configuration;
import particle;
import solver;
import vector;
import renderer;

int main(int argc, char **argv) {
  pbf::Arguments arguments;
  try {
    arguments = pbf::ParseArguments({argv + 1, argv + argc});
  } catch (const std::exception &e) {
    std::println("Argument error: {}", e.what());
    return 1;
  }

  pbf::Configuration configuration;
  try {
    if (arguments.configuration_file)
      configuration.LoadFromFile(*arguments.configuration_file);
    configuration.LoadFromArguments(arguments);
  } catch (const std::exception &e) {
    std::println("Configuration error: {}", e.what());
    return 2;
  }

  pbf::Particles particles(configuration.particles);
  if (arguments.load_file) {
    try {
      particles.Load(*arguments.load_file);
    } catch (const std::exception &e) {
      std::println("Particle load error: {}", e.what());
      return 3;
    }
  }

  pbf::PBFSolver solver{configuration};

  if (arguments.no_gui) {
    for (unsigned step = 0; step < configuration.parameters.simulation_steps; ++step)
      solver.Step(particles);
  } else {
    pbf::Renderer renderer{configuration};
    renderer.Run(particles, solver);
  }

  if (arguments.dump) {
    try {
      particles.Dump();
    } catch (const std::exception &e) {
      std::println("Particle dump error: {}", e.what());
      return 3;
    }
  }

  return 0;
}
