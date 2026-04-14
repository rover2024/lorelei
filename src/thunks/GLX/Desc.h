#pragma once

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#ifdef Success
#  undef Success
#endif

#include "GLX_private.h"

#include <lorelei/Tools/ThunkInterface/Proc.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

namespace lore::thunk {}
