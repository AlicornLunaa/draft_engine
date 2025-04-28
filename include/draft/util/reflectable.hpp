/// Simple macro-based reflection utility
#pragma once
#include <tuple>
#define DRAFT_ENABLE_REFLECTION(...) static constexpr auto get_members() { return std::make_tuple(__VA_ARGS__); }