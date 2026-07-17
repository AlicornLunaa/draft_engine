#include <gtest/gtest.h>
#include "draft/util/localization.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include <stdexcept>
#include <string>

using namespace Draft;

TEST(Localization, LoadsLanguageAndGetsContent)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"greeting":"Hello"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));

    ASSERT_EQ(loc.get_content("en-us", "greeting"), "Hello");

    provider.remove("en-us.lang");
}

TEST(Localization, GetContentFormatsArgs)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"welcome":"Hello, {}!"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));

    ASSERT_EQ(loc.get_content("en-us", "welcome", "world"), "Hello, world!");

    provider.remove("en-us.lang");
}

TEST(Localization, LanguageNameComesFromFileStem)
{
    DiskFileProvider provider;
    provider.write_string("fr-fr.lang", R"({"greeting":"Bonjour"})");

    Localization loc;
    loc.load_language(provider.open("fr-fr.lang"));

    ASSERT_EQ(loc.get_content("fr-fr", "greeting"), "Bonjour");

    provider.remove("fr-fr.lang");
}

TEST(Localization, MultipleLanguagesAreIndependent)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"greeting":"Hello"})");
    provider.write_string("fr-fr.lang", R"({"greeting":"Bonjour"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));
    loc.load_language(provider.open("fr-fr.lang"));

    ASSERT_EQ(loc.get_content("en-us", "greeting"), "Hello");
    ASSERT_EQ(loc.get_content("fr-fr", "greeting"), "Bonjour");

    provider.remove("en-us.lang");
    provider.remove("fr-fr.lang");
}

TEST(Localization, ReloadingLanguageMergesKeys)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"a":"1"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));

    provider.write_string("en-us.lang", R"({"b":"2"})");
    loc.load_language(provider.open("en-us.lang"));

    // both keys survive since load_language merges into the existing map
    ASSERT_EQ(loc.get_content("en-us", "a"), "1");
    ASSERT_EQ(loc.get_content("en-us", "b"), "2");

    provider.remove("en-us.lang");
}

TEST(Localization, GetContentFallsBackToIdentifierWhenFallbackLanguageMissing)
{
    Localization loc;
    ASSERT_EQ(loc.get_content("en-us", "greeting"), "greeting");
}

TEST(Localization, GetContentFallsBackToIdentifierWhenIdentifierMissing)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"greeting":"Hello"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));

    ASSERT_EQ(loc.get_content("en-us", "missing"), "missing");

    provider.remove("en-us.lang");
}

TEST(Localization, GetContentFallsBackToFallbackLanguageWhenLanguageMissing)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"greeting":"Hello"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));

    ASSERT_EQ(loc.get_content("fr-fr", "greeting"), "Hello");

    provider.remove("en-us.lang");
}

TEST(Localization, HasLanguageReflectsLoadedLanguages)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"greeting":"Hello"})");

    Localization loc;
    ASSERT_FALSE(loc.has_language("en-us"));

    loc.load_language(provider.open("en-us.lang"));
    ASSERT_TRUE(loc.has_language("en-us"));
    ASSERT_FALSE(loc.has_language("fr-fr"));

    provider.remove("en-us.lang");
}

TEST(Localization, UnloadLanguageRemovesIt)
{
    DiskFileProvider provider;
    provider.write_string("en-us.lang", R"({"greeting":"Hello"})");

    Localization loc;
    loc.load_language(provider.open("en-us.lang"));
    loc.unload_language("en-us");

    ASSERT_FALSE(loc.has_language("en-us"));
    ASSERT_EQ(loc.get_content("en-us", "greeting"), "greeting");

    provider.remove("en-us.lang");
}

TEST(Localization, UnloadLanguageThrowsWhenNotLoaded)
{
    Localization loc;
    ASSERT_THROW(loc.unload_language("en-us"), std::logic_error);
}
