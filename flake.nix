{
  description = "Basic flake with package imports";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { nixpkgs, ... }: let
    system = "x86_64-linux";
  in {
    devShells."${system}".default = let
      pkgs = import nixpkgs {
        inherit system;
      };
    in pkgs.mkShell{
       name = "cpp-raylib";

      packages = with pkgs; [
      llvmPackages_21.clang
      llvmPackages_21.clang-tools
      llvmPackages_21.libcxx
      cmake
      gcc

         raylib
         raygui

         glfw
         xorg.libX11
         xorg.libXrandr
         xorg.libXinerama
         xorg.libXcursor
         xorg.libXi
         xorg.xeyes
         mesa
         libglvnd
         pkg-config
      ];
      # buildInputs = with pkgs; [
      # ];

		LD_LIBRARY_PATH="${pkgs.xorg.libX11}/lib:${pkgs.xorg.libXrandr}/lib:${pkgs.xorg.libXinerama}/lib:${pkgs.xorg.libXcursor}/lib:${pkgs.xorg.libXi}/lib:${pkgs.raylib}/lib:${pkgs.mesa}/lib:${pkgs.libglvnd}/lib:$LD_LIBRARY_PATH";
		
      # needed for clangd lsp to work correctly
      shellHook = ''
         echo "C Raylib environment running..."
      '';
    };
};
  # outputs = { self, nixpkgs }: {

  #   #packages.x86_64-linux.hello = nixpkgs.legacyPackages.x86_64-linux.hello;

  #   #packages.x86_64-linux.default = self.packages.x86_64-linux.hello;
  #   packages = with pkg

  # };
}
