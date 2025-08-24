{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell
{
        nativeBuildInputs = with pkgs; [
          gcc
          clang-tools
        ];
        shellHook = ''
                              export PATH="${pkgs.clang-tools}/bin:$PATH"
          					'';
}
