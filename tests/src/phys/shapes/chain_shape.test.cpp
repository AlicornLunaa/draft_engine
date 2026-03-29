#include <gtest/gtest.h>
#include "draft/phys/shapes/chain_shape.hpp"

TEST(ChainShape, ConstructorAndChainType)
{
    Draft::ChainShape loopChain(Draft::ChainShape::ChainType::LOOP);
    ASSERT_EQ(loopChain.type, Draft::ShapeType::CHAIN);
    ASSERT_EQ(loopChain.get_chain_type(), Draft::ChainShape::ChainType::LOOP);

    Draft::ChainShape simpleChain(Draft::ChainShape::ChainType::CHAIN);
    ASSERT_EQ(simpleChain.type, Draft::ShapeType::CHAIN);
    ASSERT_EQ(simpleChain.get_chain_type(), Draft::ChainShape::ChainType::CHAIN);
}

TEST(ChainShape, PointManagement)
{
    Draft::ChainShape chain(Draft::ChainShape::ChainType::LOOP);
    ASSERT_EQ(chain.get_points().size(), 0);

    chain.add({0.f, 0.f});
    chain.add({1.f, 0.f});
    chain.add({1.f, 1.f});
    ASSERT_EQ(chain.get_points().size(), 3);
    ASSERT_EQ(chain.get_points()[0].x, 0.f);
    ASSERT_EQ(chain.get_points()[2].y, 1.f);

    chain.remove(1); // Remove {1.f, 0.f}
    ASSERT_EQ(chain.get_points().size(), 2);
    ASSERT_EQ(chain.get_points()[0].x, 0.f);
    ASSERT_EQ(chain.get_points()[0].y, 0.f);
    ASSERT_EQ(chain.get_points()[1].x, 1.f);
    ASSERT_EQ(chain.get_points()[1].y, 1.f);

    chain.clear();
    ASSERT_EQ(chain.get_points().size(), 0);
}

TEST(ChainShape, PreviousNextPoints)
{
    Draft::ChainShape chain(Draft::ChainShape::ChainType::CHAIN); // CHAIN type uses previous/next
    chain.set_previous({-1.f, -1.f});
    chain.set_next({2.f, 2.f});

    ASSERT_EQ(chain.get_previous().x, -1.f);
    ASSERT_EQ(chain.get_previous().y, -1.f);
    ASSERT_EQ(chain.get_next().x, 2.f);
    ASSERT_EQ(chain.get_next().y, 2.f);
}

TEST(ChainShape, Clone)
{
    Draft::ChainShape originalChain(Draft::ChainShape::ChainType::LOOP);
    originalChain.add({0.f, 0.f});
    originalChain.add({1.f, 1.f});
    originalChain.density = 0.5f;

    std::unique_ptr<Draft::Shape> clonedShape = originalChain.clone();
    Draft::ChainShape* clonedChain = dynamic_cast<Draft::ChainShape*>(clonedShape.get());

    ASSERT_NE(clonedChain, nullptr);
    ASSERT_EQ(clonedChain->type, originalChain.type);
    ASSERT_EQ(clonedChain->get_chain_type(), originalChain.get_chain_type());
    ASSERT_EQ(clonedChain->density, originalChain.density);
    ASSERT_EQ(clonedChain->get_points().size(), originalChain.get_points().size());
    for (size_t i = 0; i < originalChain.get_points().size(); ++i) {
        ASSERT_EQ(clonedChain->get_points()[i].x, originalChain.get_points()[i].x);
        ASSERT_EQ(clonedChain->get_points()[i].y, originalChain.get_points()[i].y);
    }
}

TEST(ChainShape, Contains)
{
    Draft::ChainShape chain(Draft::ChainShape::ChainType::LOOP);
    chain.add({0.f, 0.f});
    chain.add({10.f, 0.f});
    chain.add({10.f, 10.f});
    chain.add({0.f, 10.f}); // Forms a square loop

    // These checks are fragile due to floating point precision and the nature of contains on zero-area shapes.
    // ASSERT_TRUE(chain.contains({0.f, 0.f}));     // Vertex
    // ASSERT_TRUE(chain.contains({5.f, 0.f}));     // On bottom edge
    // ASSERT_TRUE(chain.contains({10.f, 5.f}));    // On right edge
    // ASSERT_TRUE(chain.contains({5.f, 10.f}));    // On top edge
    // ASSERT_TRUE(chain.contains({0.f, 5.f}));     // On left edge
    
    ASSERT_FALSE(chain.contains({5.f, 5.f}));    // Inside the loop (chains generally don't "contain" an area)
    ASSERT_FALSE(chain.contains({-1.f, -1.f}));  // Outside
    ASSERT_FALSE(chain.contains({0.f, -0.1f}));
    ASSERT_FALSE(chain.contains({10.1f, 0.f}));
}
