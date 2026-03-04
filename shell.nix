{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell
{
  nativeBuildInputs = with pkgs; [
    nixpkgs-fmt
    gcc
    clang-tools
    emscripten
    perf
  ];
  shellHook = ''
    					export PATH="${pkgs.clang-tools}/bin:$PATH"
    					'';
}
