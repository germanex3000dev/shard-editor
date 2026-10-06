{
  description = "shard - a very small text editor";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      # Flakes must be pure, so the systems come from nixpkgs rather than from
      # whatever happens to be running.
      eachSystem = nixpkgs.lib.genAttrs (
        nixpkgs.lib.systems.flakeExposed
      );
      pkgsFor = eachSystem (system:
        nixpkgs.legacyPackages.${system}
      );
    in {
      devShells = eachSystem (system: {
        default = (pkgsFor.${system}).mkShell {
          packages = with pkgsFor.${system}; [
            cmake
            gcc

            # cmake defaults to the Makefiles generator, so a build tool has to
            # be on PATH. gcc's stdenv happens to bring in gnumake, but relying
            # on that is a hidden dependency: name it instead.
            gnumake
          ];
        };
      });

      packages = eachSystem (system:
        let
          pkgs = pkgsFor.${system};
        in {
          shard = pkgs.stdenv.mkDerivation {
            pname = "shard";
            version = "0.1.0";

            src = ./.;

            nativeBuildInputs = with pkgs; [ cmake makeWrapper ];

            cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];

            # cmakeBuildHook configures and builds in ./build, then the install
            # comes from CMakeLists.txt's install() rule rather than a
            # hand-rolled copy of the binary.
            installPhase = ''
              runHook preInstall
              cmake --install . --prefix "$out"
              runHook postInstall
            '';

            doCheck = true;

            meta = with pkgs.lib; {
              description = "A very small text editor";
              mainProgram = "shard";
              platforms = platforms.unix;
            };
          };

          default = self.packages.${system}.shard;
        }
      );

      apps = eachSystem (system: {
        default = {
          type = "app";
          program = "${self.packages.${system}.shard}/bin/shard";
          meta.description = "A very small text editor";
        };
      });
    };
}