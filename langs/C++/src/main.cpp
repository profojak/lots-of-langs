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
  pbf::Particles particles(configuration.particles);
  pbf::PBFSolver solver{configuration};

  if (arguments.no_gui) {
    for (unsigned step = 0; step < configuration.parameters.simulation_steps; ++step)
      solver.Step(particles);
  } else {
    pbf::Renderer renderer{configuration};
    renderer.Run(particles, solver);
  }

  return 0;
}
