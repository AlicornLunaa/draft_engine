#include <gtest/gtest.h>
#include "draft/util/serialization/context.hpp"
#include <stdexcept>

using namespace Draft::Serializer;

namespace {
    struct DemoContext {
        int value = 0;
    };
}

TEST(SerializerContext, ThrowsWhenNoneActive)
{
    ASSERT_THROW(context<DemoContext>(), std::logic_error);
}

TEST(SerializerContext, ScopedContextMakesItReachable)
{
    DemoContext ctx{42};
    ScopedContext<DemoContext> scope(ctx);

    ASSERT_EQ(context<DemoContext>().value, 42);
}

TEST(SerializerContext, RestoresPreviousOnDestruction)
{
    DemoContext outer{1};
    ScopedContext<DemoContext> outerScope(outer);

    {
        DemoContext inner{2};
        ScopedContext<DemoContext> innerScope(inner);
        ASSERT_EQ(context<DemoContext>().value, 2);
    }

    ASSERT_EQ(context<DemoContext>().value, 1);
}

TEST(SerializerContext, NoLongerReachableAfterScopeEnds)
{
    {
        DemoContext ctx{5};
        ScopedContext<DemoContext> scope(ctx);
    }

    ASSERT_THROW(context<DemoContext>(), std::logic_error);
}
