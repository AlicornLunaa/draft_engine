#pragma once

#include "draft/util/serializer.hpp"
#include "draft/math/glm.hpp"

template<glm::length_t L, typename T, glm::qualifier Q>
struct Draft::Serializer::CustomSerializer<Draft::Math::vec<L, T, Q>> {
    static void serialize(const Draft::Math::vec<L, T, Q>& v, JSON& json){
        json = JSON::array();

        for(glm::length_t i = 0; i < L; i++)
            json.push_back(v[i]);
    }

    static void deserialize(Draft::Math::vec<L, T, Q>& v, JSON& json){
        for(glm::length_t i = 0; i < L; i++)
            v[i] = json[i].template get<T>();
    }
};
