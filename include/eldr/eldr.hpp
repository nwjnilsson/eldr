/*
 * Eldr: A Physically Based Renderer
 * Copyright (c) 2024 Johannes Nilsson <nwj.nilsson@outlook.com>
 *
 * Use of this source code is governed by a MIT license that can be found in the
 * LICENSE file.
 */

#pragma once

#define EL_STRINGIFY(x) #x
#define EL_TOSTRING(x) EL_STRINGIFY(x)

#define NAMESPACE_BEGIN(Name) namespace Name {
#define NAMESPACE_END(Name) }

#include <cstddef>
#include <cstdint>
#include <eldr/buildinfo.hpp>
// #include <eldr/gitinfo.hpp>
#include <eldr/core/platform.hpp>
