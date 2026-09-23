{
  description = "Position Based Fluids in C++23";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        clangStdenv = pkgs.overrideCC pkgs.stdenv pkgs.clang;
        mkShell = pkgs.mkShell.override { stdenv = clangStdenv; };
        cxxFlags = [
          "-std=c++23"
          "-O3"
          "-DPBF_LANGUAGE=\"C++23\""
        ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
          "-mmacosx-version-min=26.0"
        ];
        cxxModules = [
          "argument"
          "vector"
          "boundary"
          "configuration"
          "particle"
          "kernel"
          "thread"
          "grid"
          "solver"
          "renderer"
        ];

        configurePhase = ''
          runHook preConfigure
          mkdir -p build
          runHook postConfigure
        '';

        buildPhase = ''
          runHook preBuild
          for m in ${toString cxxModules}; do
            $CXX ${pkgs.lib.escapeShellArgs cxxFlags} -fprebuilt-module-path=build \
              -fmodule-output="build/$m.pcm" -c "src/$m.cppm" -o "build/$m.o"
          done
          $CXX ${pkgs.lib.escapeShellArgs cxxFlags} -fprebuilt-module-path=build -c src/main.cpp -o build/main.o
          $CXX ${pkgs.lib.escapeShellArgs cxxFlags} build/*.o -o pbf -lraylib
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/bin $out/build
          install -m755 pbf $out/bin/pbf
          cp build/*.pcm $out/build/
          runHook postInstall
        '';

      in {
        packages.default = clangStdenv.mkDerivation {
          pname = "pbf";
          version = "0.0.0";
          src = ./.;
          inherit configurePhase buildPhase installPhase;
          buildInputs = [ pkgs.raylib ];
          meta.mainProgram = "pbf";
        };

        defaultPackage = self.packages.${system}.default;
        packages.pbf = self.packages.${system}.default;

        formatter = pkgs.writeShellScriptBin "formatter" ''
          find src -type f \( -name '*.cpp' -o -name '*.cppm' -o -name '*.h' -o -name '*.hpp' \) \
            -exec ${pkgs.clang-tools}/bin/clang-format -i {} +
        '';

        devShells.default = mkShell {
          inherit configurePhase buildPhase;

          packages = [
            pkgs.clang
            pkgs.clang-tools
            pkgs.raylib
          ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
            pkgs.apple-sdk
          ];

          shellHook = ''
            {
              echo 'CompileFlags:'
              echo '  Compiler: ${pkgs.llvmPackages.clang-unwrapped}/bin/clang++'
              echo '  Add:'
              echo '    - -std=c++23'
              ${pkgs.lib.optionalString pkgs.stdenv.hostPlatform.isDarwin "echo '    - -mmacosx-version-min=26.0'"}
              echo '    - -DPBF_LANGUAGE="C++23"'
              echo '    - -I${pkgs.raylib}/include'
              echo '    - -isystem'
              echo '    - ${pkgs.libcxx.dev}/include/c++/v1'
              echo '    - -isystem'
              echo '    - ${pkgs.libcxx.dev}/include'
              for f in result/build/*.pcm; do
                [ -e "$f" ] || continue
                m=$(basename "$f" .pcm)
                echo "    - -fmodule-file=''${m}=$PWD/result/build/''${m}.pcm"
              done
            } > .clangd

            clang++ --version | head -n1
          '';
        };
      });
}
