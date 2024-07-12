<!----------------------------------------------------------------------------->

Written in C++23<sup>1</sup> and compiled with the development version of
`clang 19.0.0`.<sup>2</sup> Run `./build.sh` to compile `raytracer` executable
and `./build.sh clean` to clean everything up. The script also builds
`libmath.a` static library which the `raytracer` links to statically.

<sup>1</sup> With headers, `module std` was not yet supported at the time.<br>
<sup>2</sup> Built from
<a href="https://github.com/llvm/llvm-project.git">source</a>, commit hash
`6c2f5d6cdeee1a458700e33797fd8df263b748b9`.

<!-- Acknowledgments ---------------------------------------------------------->

<h3>Acknowledgments</h3>
All credit goes to the authors of
<a href="https://github.com/RayTracing/raytracing.github.io">Ray Tracing in One Weekend</a>.
The license of the original work is included. The entire code base of this
repository, including the derived version of this program, is licensed under the
MIT license.

<!----------------------------------------------------------------------------->
