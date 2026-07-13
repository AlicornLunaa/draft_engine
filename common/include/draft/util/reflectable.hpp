#pragma once

#include <concepts>
#include <string_view>
#include <tuple>
#include <utility>

namespace Draft {
    /**
     * @brief Describes one reflected member of @p Class: its declared name, and a
     * pointer-to-member for reading/writing it on a specific instance.
     */
    template<typename Class, typename MemberType>
    struct Field {
        using ValueType = MemberType;

        std::string_view name;
        MemberType Class::* pointer;

        constexpr MemberType& get(Class& instance) const { return instance.*pointer; }
        constexpr const MemberType& get(const Class& instance) const { return instance.*pointer; }
        constexpr void set(Class& instance, MemberType value) const { instance.*pointer = std::move(value); }
    };

    /**
     * @brief Builds a Field, deducing @p Class and @p MemberType from @p pointer. Used by
     * DRAFT_REFLECTABLE(...), not normally called directly.
     */
    template<typename Class, typename MemberType>
    constexpr Field<Class, MemberType> make_field(std::string_view name, MemberType Class::* pointer){
        return Field<Class, MemberType>{name, pointer};
    }

    /**
     * @brief Satisfied by any type with a DRAFT_REFLECTABLE(...) declaration.
     */
    template<typename T>
    concept Reflectable = requires {
        T::reflect();
        { T::reflect_name() } -> std::convertible_to<std::string_view>;
    };

    /**
     * @brief Calls `visitor(name, valueRef)` for every reflected field of @p instance, in
     * declaration order.
     */
    template<Reflectable T, typename Visitor>
    constexpr void for_each_field(T& instance, Visitor&& visitor){
        std::apply([&](auto&&... field){
            (visitor(field.name, field.get(instance)), ...);
        }, T::reflect());
    }

    /**
     * @brief Const-instance overload of for_each_field().
     */
    template<Reflectable T, typename Visitor>
    constexpr void for_each_field(const T& instance, Visitor&& visitor){
        std::apply([&](auto&&... field){
            (visitor(field.name, field.get(instance)), ...);
        }, T::reflect());
    }

    /**
     * @brief Calls `visitor(valueRef)` for the reflected field of @p instance named @p name.
     * @return True if a field named @p name was found (and visited), false otherwise.
     */
    template<Reflectable T, typename Visitor>
    constexpr bool visit_field(T& instance, std::string_view name, Visitor&& visitor){
        bool found = false;

        std::apply([&](auto&&... field){
            auto try_visit = [&](const auto& f){
                if(f.name == name){
                    visitor(f.get(instance));
                    found = true;
                }
            };

            (try_visit(field), ...);
        }, T::reflect());

        return found;
    }

    /**
     * @brief Const-instance overload of visit_field().
     */
    template<Reflectable T, typename Visitor>
    constexpr bool visit_field(const T& instance, std::string_view name, Visitor&& visitor){
        bool found = false;

        std::apply([&](auto&&... field){
            auto try_visit = [&](const auto& f){
                if(f.name == name){
                    visitor(f.get(instance));
                    found = true;
                }
            };

            (try_visit(field), ...);
        }, T::reflect());

        return found;
    }
}

/// Macros
// A DRAFT_DETAIL_FOR_EACH(macro, data, a, b, c, ...) that expands to
// `macro(data, a) macro(data, b) macro(data, c) ...`, implemented via the recursive,
// __VA_OPT__-based expansion idiom (no external dependency, no fixed arity limit besides the
// recursion depth baked into DRAFT_DETAIL_EXPAND below - 4 levels of quadruple expansion
// supports up to 256 fields, far more than any real class needs).

#define DRAFT_DETAIL_PARENS ()

#define DRAFT_DETAIL_EXPAND(...) DRAFT_DETAIL_EXPAND4(DRAFT_DETAIL_EXPAND4(DRAFT_DETAIL_EXPAND4(DRAFT_DETAIL_EXPAND4(__VA_ARGS__))))
#define DRAFT_DETAIL_EXPAND4(...) DRAFT_DETAIL_EXPAND3(DRAFT_DETAIL_EXPAND3(DRAFT_DETAIL_EXPAND3(DRAFT_DETAIL_EXPAND3(__VA_ARGS__))))
#define DRAFT_DETAIL_EXPAND3(...) DRAFT_DETAIL_EXPAND2(DRAFT_DETAIL_EXPAND2(DRAFT_DETAIL_EXPAND2(DRAFT_DETAIL_EXPAND2(__VA_ARGS__))))
#define DRAFT_DETAIL_EXPAND2(...) DRAFT_DETAIL_EXPAND1(DRAFT_DETAIL_EXPAND1(DRAFT_DETAIL_EXPAND1(DRAFT_DETAIL_EXPAND1(__VA_ARGS__))))
#define DRAFT_DETAIL_EXPAND1(...) __VA_ARGS__

#define DRAFT_DETAIL_FOR_EACH(macro, data, ...) \
    __VA_OPT__(DRAFT_DETAIL_EXPAND(DRAFT_DETAIL_FOR_EACH_HELPER(macro, data, __VA_ARGS__)))

#define DRAFT_DETAIL_FOR_EACH_HELPER(macro, data, a1, ...) \
    macro(data, a1) \
    __VA_OPT__(, DRAFT_DETAIL_FOR_EACH_AGAIN DRAFT_DETAIL_PARENS (macro, data, __VA_ARGS__))

#define DRAFT_DETAIL_FOR_EACH_AGAIN() DRAFT_DETAIL_FOR_EACH_HELPER

#define DRAFT_DETAIL_REFLECT_FIELD(ClassName, field) ::Draft::make_field(#field, &ClassName::field)

/**
 * @def DRAFT_REFLECTABLE(ClassName, ...)
 * @brief Declares `static constexpr auto reflect()`, returning a `std::tuple` of a Field per
 * listed member. Each pairing that member's stringized name with `&ClassName::member`. Also
 * declares `static constexpr std::string_view reflect_name()`, returning @p ClassName stringized type name.
 *
 * Field names still have to be listed here, but DRAFT_REFLECTED() at the member declaration
 * itself keeps the two spots visually consistent and marks intent inline.
 *
 * @code
 * struct Point {
 *     DRAFT_REFLECTED(int, x) = 0;
 *     DRAFT_REFLECTED(int, y) = 0;
 *
 *     DRAFT_REFLECTABLE(Point, x, y)
 * };
 *
 * Point p{1, 2};
 * for_each_field(p, [](std::string_view name, auto& value){
 *     std::cout << name << " = " << value << "\n";
 * });
 * @endcode
 */
#define DRAFT_REFLECTABLE(ClassName, ...) \
    static constexpr std::string_view reflect_name(){ return #ClassName; } \
    static constexpr auto reflect(){ \
        return std::make_tuple(DRAFT_DETAIL_FOR_EACH(DRAFT_DETAIL_REFLECT_FIELD, ClassName, __VA_ARGS__)); \
    }

/**
 * @def DRAFT_REFLECTED(Type, Name)
 * @brief Declares a member `Type Name`, marking it (by convention, alongside a matching entry
 * in this class's DRAFT_REFLECTABLE(...) list) as part of its reflected state.
 *
 * This expands to a plain member declaration, the reflection metadata itself comes entirely
 * from DRAFT_REFLECTABLE(). Using this macro at the declaration site is optional but keeps
 * reflected members visually distinct from ordinary ones.
 */
#define DRAFT_REFLECTED(Type, Name) Type Name
