#pragma once

/// Include this header anywhere you want to use
/// the GLM libraries. It's mainly for convenience
/// so you don't have to include several different
/// GLM headers. It also ensures your GLM functions
/// take in radians across the board, which will also
/// suppress some compiler warnings.

#define GLM_FORCE_RADIANS
// Primary GLM library
#    include <glm/glm.hpp>
// For glm::translate, glm::rotate, and glm::scale.
#    include <glm/gtc/matrix_transform.hpp>
// For glm::to_string.
#    include <glm/gtx/string_cast.hpp>
// For glm::value_ptr.
#    include <glm/gtc/type_ptr.hpp>
