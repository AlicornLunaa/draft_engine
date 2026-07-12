#pragma once

#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

#include <cfloat>

namespace Draft {
    class Joint;

    /**
     * @brief Common base for every joint's configurable data
     */
    struct GenericJointData {
        virtual ~GenericJointData() = default;

        DRAFT_REFLECTED(bool, collideConnected) = false;

        DRAFT_REFLECTABLE(GenericJointData, collideConnected)
    };

    struct DistanceJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, anchorA) = {};
        DRAFT_REFLECTED(Vector2f, anchorB) = {};
        DRAFT_REFLECTED(float, length) = 1.f;
        DRAFT_REFLECTED(float, minLength) = 0.f;
        DRAFT_REFLECTED(float, maxLength) = FLT_MAX;
        DRAFT_REFLECTED(float, stiffness) = 0.f;
        DRAFT_REFLECTED(float, damping) = 0.f;

        DRAFT_REFLECTABLE(DistanceJointData, anchorA, anchorB, length, minLength, maxLength, stiffness, damping)
    };

    struct FrictionJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, anchorA) = {};
        DRAFT_REFLECTED(Vector2f, anchorB) = {};
        DRAFT_REFLECTED(float, maxForce) = 0.f;
        DRAFT_REFLECTED(float, maxTorque) = 0.f;

        DRAFT_REFLECTABLE(FrictionJointData, anchorA, anchorB, maxForce, maxTorque)
    };

    /**
     * @brief `joint1`/`joint2` are raw pointers to the two joints this gear joint couples, so
     * this compiles without a full `Joint` definition same reasoning as `FixtureDef::shape`.
     * Not reflected (a pointer isn't a plain value), only `ratio` is.
     */
    struct GearJointData : virtual public GenericJointData {
        Joint* joint1 = nullptr;
        Joint* joint2 = nullptr;
        DRAFT_REFLECTED(float, ratio) = 1.f;

        DRAFT_REFLECTABLE(GearJointData, ratio)
    };

    struct MotorJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, linearOffset) = {};
        DRAFT_REFLECTED(float, angularOffset) = 0.f;
        DRAFT_REFLECTED(float, maxForce) = 1.f;
        DRAFT_REFLECTED(float, maxTorque) = 1.f;
        DRAFT_REFLECTED(float, correctionFactor) = 0.3f;

        DRAFT_REFLECTABLE(MotorJointData, linearOffset, angularOffset, maxForce, maxTorque, correctionFactor)
    };

    struct MouseJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, target) = {};
        DRAFT_REFLECTED(float, maxForce) = 0.f;
        DRAFT_REFLECTED(float, stiffness) = 0.f;
        DRAFT_REFLECTED(float, damping) = 0.f;

        DRAFT_REFLECTABLE(MouseJointData, target, maxForce, stiffness, damping)
    };

    struct PrismaticJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, anchorA) = {};
        DRAFT_REFLECTED(Vector2f, anchorB) = {};
        DRAFT_REFLECTED(Vector2f, localAxisA) = {1, 0};
        DRAFT_REFLECTED(float, referenceAngle) = 0.f;
        DRAFT_REFLECTED(float, lowerTranslation) = 0.f;
        DRAFT_REFLECTED(float, upperTranslation) = 0.f;
        DRAFT_REFLECTED(float, maxMotorForce) = 0.f;
        DRAFT_REFLECTED(float, motorSpeed) = 0.f;
        DRAFT_REFLECTED(bool, enableLimit) = false;
        DRAFT_REFLECTED(bool, enableMotor) = false;

        DRAFT_REFLECTABLE(PrismaticJointData, anchorA, anchorB, localAxisA, referenceAngle,
            lowerTranslation, upperTranslation, maxMotorForce, motorSpeed, enableLimit, enableMotor)
    };

    struct PulleyJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, groundAnchorA) = {};
        DRAFT_REFLECTED(Vector2f, groundAnchorB) = {};
        DRAFT_REFLECTED(Vector2f, localAnchorA) = {};
        DRAFT_REFLECTED(Vector2f, localAnchorB) = {};
        DRAFT_REFLECTED(float, lengthA) = 0.f;
        DRAFT_REFLECTED(float, lengthB) = 0.f;
        DRAFT_REFLECTED(float, ratio) = 1.f;

        DRAFT_REFLECTABLE(PulleyJointData, groundAnchorA, groundAnchorB, localAnchorA, localAnchorB, lengthA, lengthB, ratio)
    };

    struct RevoluteJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, localAnchorA) = {};
        DRAFT_REFLECTED(Vector2f, localAnchorB) = {};
        DRAFT_REFLECTED(float, referenceAngle) = 0.f;
        DRAFT_REFLECTED(float, lowerAngle) = 0.f;
        DRAFT_REFLECTED(float, upperAngle) = 0.f;
        DRAFT_REFLECTED(float, maxMotorTorque) = 0.f;
        DRAFT_REFLECTED(float, motorSpeed) = 0.f;
        DRAFT_REFLECTED(bool, enableLimit) = false;
        DRAFT_REFLECTED(bool, enableMotor) = false;

        DRAFT_REFLECTABLE(RevoluteJointData, localAnchorA, localAnchorB, referenceAngle,
            lowerAngle, upperAngle, maxMotorTorque, motorSpeed, enableLimit, enableMotor)
    };

    struct WeldJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, anchorA) = {};
        DRAFT_REFLECTED(Vector2f, anchorB) = {};
        DRAFT_REFLECTED(float, referenceAngle) = 0.f;
        DRAFT_REFLECTED(float, stiffness) = 0.f;
        DRAFT_REFLECTED(float, damping) = 0.f;

        DRAFT_REFLECTABLE(WeldJointData, anchorA, anchorB, referenceAngle, stiffness, damping)
    };

    struct WheelJointData : virtual public GenericJointData {
        DRAFT_REFLECTED(Vector2f, anchorA) = {};
        DRAFT_REFLECTED(Vector2f, anchorB) = {};
        DRAFT_REFLECTED(Vector2f, localAxis) = {1, 0};
        DRAFT_REFLECTED(float, lowerTranslation) = 0.f;
        DRAFT_REFLECTED(float, upperTranslation) = 0.f;
        DRAFT_REFLECTED(float, maxMotorTorque) = 0.f;
        DRAFT_REFLECTED(float, motorSpeed) = 0.f;
        DRAFT_REFLECTED(float, stiffness) = 0.f;
        DRAFT_REFLECTED(float, damping) = 0.f;
        DRAFT_REFLECTED(bool, enableLimit) = false;
        DRAFT_REFLECTED(bool, enableMotor) = false;

        DRAFT_REFLECTABLE(WheelJointData, anchorA, anchorB, localAxis, lowerTranslation,
            upperTranslation, maxMotorTorque, motorSpeed, stiffness, damping, enableLimit, enableMotor)
    };
}
