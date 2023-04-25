{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };

        shellWithPkgs = packages: pkgs.mkShell {
          inherit packages;
        };
      in
      rec {
        devShells.default = shellWithPkgs [ pkgs.hello ];

        defaultPackage = packages.vanillaEDK2;

        packages = {
          vanillaEDK2 = pkgs.callPackage ./default.nix { buildBitcode = false; };
          bitcodeEDK2 = pkgs.callPackage ./default.nix { };
        };
      });
}


