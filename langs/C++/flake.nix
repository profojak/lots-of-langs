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
        cxxFlags = [ "-std=c++23" "-O3" ];
        cxxModules = [
          "vector"
          "config"
          "particle"
          "kernel"
          "thread"
          "grid"
          "solver"
        ];

        configurePhase = ''
          runHook preConfigure
          mkdir -p build
          runHook postConfigure
        '';

        buildPhase = ''
          runHook preBuild
          for m in ${toString cxxModules}; do
            $CXX ${toString cxxFlags} -fprebuilt-module-path=build \
              -fmodule-output="build/$m.pcm" -c "src/$m.cppm" -o "build/$m.o"
          done
          $CXX ${toString cxxFlags} -fprebuilt-module-path=build -c src/main.cpp -o build/main.o
          $CXX build/*.o -o pbf
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
          NIX_LDFLAGS = "-lraylib";
          meta.mainProgram = "pbf";
        };

        defaultPackage = self.packages.${system}.default;
        packages.pbf = self.packages.${system}.default;

        formatter = pkgs.writeShellScriptBin "formatter" ''
          find src -type f \( -name '*.cpp' -o -name '*.cppm' -o -name '*.h' -o -name '*.hpp' \) \
            -exec ${pkgs.clang-tools}/bin/clang-format -i {} +
        '';

        devShells.default = pkgs.mkShell {
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
              echo '  Add:'
              echo '    - -std=c++23'
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
