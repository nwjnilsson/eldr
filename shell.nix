# Build prerequisites. Remaining deps are fetched by meson
{pkgs ? import <nixpkgs> {}}:
pkgs.stdenv.mkDerivation {
  name = "eldr-dev";
  nativeBuildInputs = with pkgs; [
    meson
    cmake
    ninja
    pkg-config
    glslang
    spirv-tools
  ];
  buildInputs = with pkgs; [
    vulkan-headers
    vulkan-loader
    vulkan-tools
    vulkan-validation-layers
    vulkan-utility-libraries
    libGL
    glfw
  ];
}
