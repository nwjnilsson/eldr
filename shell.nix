{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  packages = with pkgs; [
    # ==========================================================================
    # Pre-requisites, not handled by Meson
    # ==========================================================================
    meson
    glslang
    vulkan-headers
    vulkan-loader
    vulkan-tools
    vulkan-validation-layers
    spirv-tools
    ninja
    cmake   # required by fastgltf

    # ---- Linux specific ----
    pkg-config # meson finds dependencies this way

    # GLFW dependencies for Linux, either X or Wayland
    # if X11 (required)
    xorg.libX11
    xorg.libXi
    # if Wayland (required)
    kdePackages.wayland
    kdePackages.wayland-protocols
    wayland-scanner
    # ------------------------

    # ==========================================================================
    # Optional
    # ==========================================================================
    xorg.xkbcomp # optional

    # ==========================================================================
    # Downloaded by Meson unless pre-installed
    # ==========================================================================
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    libxkbcommon
    bison             # required by xkbcommon wrap (if used)
    zlib              # required by libpng
  ];
}
