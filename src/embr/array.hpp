#pragma once
/*
 * Main header that bundles all embr array components in the correct order
 */

// Forward declarations and traits
#include "fwd.hpp"

#include "traits.hpp"

#include "constants.hpp"

// Array base classes (in dependency order)
#include "arrayutils.hpp"

// Dispatching to array endpoints (must come before array headers)
#include "arrayrouter.hpp"

#include "arraybase.hpp"

#include "arraystatic.hpp"

#include "arraygeneric.hpp"

// Concrete array types
#include "arrayiface.hpp"

#include "packet.hpp"

#include "matrix.hpp"
