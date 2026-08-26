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
      in {
        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.clang
            pkgs.clang-tools
            pkgs.raylib
          ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
            pkgs.apple-sdk
          ];

          shellHook = ''
            cat > .clangd <<EOF
            CompileFlags:
              Add: [-std=c++23, -I${pkgs.raylib}/include]
            EOF
            clang++ --version | head -n1
          '';
        };
      }
    );
}
