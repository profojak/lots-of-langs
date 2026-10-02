{
  description = "raygui library";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = import nixpkgs {inherit system;};

        version = pkgs.raygui.version;
        majorVersion = pkgs.lib.versions.major version;
        darwinVersion =
          if builtins.match "[0-9]+\\.[0-9]+\\.[0-9]+" version != null
          then version else "${version}.0";

        src = pkgs.raygui.src;
        isDarwin = pkgs.stdenv.hostPlatform.isDarwin;
        libExtension = if isDarwin then "dylib" else "so";

        buildCommand =
          if isDarwin then ''
            $CC -fPIC -I"$src/src" raygui_impl.c -lraylib -dynamiclib \
              -install_name "$out/lib/libraygui.dylib" \
              -compatibility_version ${darwinVersion} -current_version ${darwinVersion} \
              -o libraygui.dylib
          ''
          else ''
            $CC -fPIC -I"$src/src" raygui_impl.c -lraylib -shared \
              -Wl,-soname,libraygui.so.${majorVersion} -o libraygui.so.${version}
            ln -s libraygui.so.${version} libraygui.so.${majorVersion}
            ln -s libraygui.so.${majorVersion} libraygui.so
          '';

        installCommand =
          if isDarwin then ''
            install -m755 libraygui.dylib $out/lib/
          ''
          else ''
            install -m755 libraygui.so.${version} $out/lib/
            ln -s libraygui.so.${version} $out/lib/libraygui.so.${majorVersion}
            ln -s libraygui.so.${majorVersion} $out/lib/libraygui.so
          '';

        checkFile = if isDarwin then "libraygui.dylib" else "libraygui.so.${version}";

        libraygui = pkgs.stdenv.mkDerivation {
          pname = "libraygui";
          inherit version src;

          dontUnpack = true;
          dontConfigure = true;
          strictDeps = true;

          buildInputs = [ pkgs.raylib ];
          propagatedBuildInputs = [ pkgs.raylib ];
          nativeBuildInputs = [ pkgs.pkg-config ];

          buildPhase = ''
            runHook preBuild
            cat > raygui_impl.c <<'EOF'
            #define RAYGUI_IMPLEMENTATION
            #include <raygui.h>
            EOF

            $CC -std=c11 -O2 -fPIC -I"$src/src" -c raygui_impl.c -o raygui_impl.o
            $AR rcs libraygui.a raygui_impl.o
            ${buildCommand}
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out/lib $out/include $out/lib/pkgconfig
            install -m644 libraygui.a $out/lib/
            ${installCommand}
            install -m644 "$src/src/raygui.h" $out/include/raygui.h

            cat > $out/lib/pkgconfig/raygui.pc <<EOF
            prefix=$out
            libdir=$out/lib
            includedir=$out/include

            Name: raygui
            Description: raygui, immediate-mode GUI library for raylib
            URL: https://github.com/raysan5/raygui
            Version: ${version}
            Requires: raylib
            Libs: -L$out/lib -lraygui
            Cflags: -I$out/include
            EOF
            runHook postInstall
          '';

          doCheck = true;
          checkPhase = ''
            runHook preCheck
            $NM libraygui.a | grep -q GuiButton
            $NM ${checkFile} | grep -q GuiButton
            runHook postCheck
          '';

          doInstallCheck = true;
          installCheckPhase = ''
            runHook preInstallCheck
            test -f $out/include/raygui.h
            test -f $out/lib/libraygui.a
            test -f $out/lib/libraygui.${libExtension}
            PKG_CONFIG_PATH=$out/lib/pkgconfig pkg-config --validate raygui
            PKG_CONFIG_PATH=$out/lib/pkgconfig pkg-config --modversion raygui | grep -qx "${version}"
            cat > smoke.c <<'EOF'
            #include <raylib.h>
            #include <raygui.h>
            int main(void) {
              Rectangle b = { 0, 0, 100, 30 };
              (void)GuiButton(b, "Test");
              return 0;
            }
            EOF
            $CC -std=c11 -I$out/include $(pkg-config --cflags raylib) -c smoke.c -o smoke.o
            $CC smoke.o -L$out/lib -lraygui $(pkg-config --libs raylib) -o smoke
            runHook postInstallCheck
          '';

          meta = {
            description = "raygui, immediate-mode GUI library for raylib";
            homepage = "https://github.com/raysan5/raygui";
            license = pkgs.lib.licenses.zlib;
            platforms = pkgs.raylib.meta.platforms;
            pkgConfigModules = ["raygui"];
          };
        };
      in {
        packages = {
          default = libraygui;
          inherit libraygui;
        };

        checks = {
          inherit libraygui;
        };

        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.clang-tools
            pkgs.pkg-config
            pkgs.raylib
            libraygui
          ];
        };
      }
    )
    // {
      overlays.default = final: _prev: {
        inherit (self.packages.${final.stdenv.hostPlatform.system}) libraygui;
      };
    };
}
