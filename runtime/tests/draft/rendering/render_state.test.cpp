#include <gtest/gtest.h>
#include "draft/rendering/render_state.hpp"

#include "glad/gl.h"

using namespace Draft;

TEST(RenderState, DefaultClearMaskCombinesColorAndDepthBits)
{
    RenderState state;
    EXPECT_EQ(state.clearMask, static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

TEST(RenderState, ViewportAndScissorDefaultToUnset)
{
    RenderState state;
    EXPECT_FALSE(state.viewport.has_value());
    EXPECT_FALSE(state.scissor.has_value());
}
