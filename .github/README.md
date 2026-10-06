<!-- Lots of Langs ------------------------------------------ Jakub Profota --->

<div align="center">
  <h1>
    <sub>🌊</sub>🌊<sub>🌊</sub>
    <br>
    Lots of Langs
  </h1>

  A&nbsp;single fluid simulation program written in&nbsp;many languages!

  <img src="https://raw.githubusercontent.com/profojak/profojak/main/media/lots-of-langs/header.gif"
    alt="Blue fluid particles falling and splashing." width="640"/>
</div>

  A&nbsp;real-time 3D Position Based Fluids<sup>&dagger;</sup> simulation,
  written from scratch to&nbsp;explore and compare these programming languages:

<div align="center"><i>
  C++
  <br><br></i>
</div>

  Every implementation mirrors exactly the&nbsp;same file structure and function
  call order, so any two ports can be read side by side, line by line.  Each
  port attempts to&nbsp;exhaustively showcase its language features
  in&nbsp;a&nbsp;codebase that is small enough to&nbsp;read in&nbsp;one sitting.
  Ports are benchmarked with an&nbsp;identical headless run, recording build and
  run time on&nbsp;these CPUs:

<div align="center"><i>
  Apple M3 Max
  <br><br></i>
  <sup>&dagger;</sup>Macklin & Müller, Position Based Fluids, ACM Transactions on Graphics 32(4), 2013.<br>
  <a href="https://doi.org/10.1145/2492045.2492069">doi:10.1145/2492045.2492069</a>
</div>

## Build & Run

`lots-of-langs` uses Nix flakes for ease of&nbsp;use. One command to&nbsp;build,
one to&nbsp;run:

```console
cd langs/<lang>
nix build
nix run
```

Each flake prints its build time and runs tests as&nbsp;part
of&nbsp;the&nbsp;build.  The&nbsp;logs are hidden by&nbsp;default, show them:

```console
nix build -L
```

Nix caches successful builds, so a&nbsp;repeated `nix build` finishes instantly
and shows nothing.  To&nbsp;see the&nbsp;build time and tests again, force
a&nbsp;fresh build:

```console
nix build -L --rebuild
```

Arguments after `--` are passed to&nbsp;the&nbsp;`pbf` executable:

```console
nix run . -- --help
```

A&nbsp;development shell with the&nbsp;full language toolchain is also
available:

```console
nix develop
```

---

<div align="center">
  <sup>Written with ❤️ by&nbsp;hand to&nbsp;learn!</sup>
</div>

<!----------------------------------------------------------------------------->
