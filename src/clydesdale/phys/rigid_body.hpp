#pragma once
#include <box2d/box2d.h>

namespace Clydesdale::Phys {
    class RigidBody : private b2Body {
    public:
        using b2Body::b2Body;

        /// Creates a fixture and attach it to this body. Use this function if you need
        /// to set some fixture parameters, like friction. Otherwise you can create the
        /// fixture directly from a shape.
        /// If the density is non-zero, this function automatically updates the mass of the body.
        /// Contacts are not created until the next time step.
        /// @param def the fixture definition.
        /// @warning This function is locked during callbacks.
        inline b2Fixture* createFixture(const b2FixtureDef* def){ return CreateFixture(def); }

        /// Creates a fixture from a shape and attach it to this body.
        /// This is a convenience function. Use b2FixtureDef if you need to set parameters
        /// like friction, restitution, user data, or filtering.
        /// If the density is non-zero, this function automatically updates the mass of the body.
        /// @param shape the shape to be cloned.
        /// @param density the shape density (set to zero for static bodies).
        /// @warning This function is locked during callbacks.
        inline b2Fixture* createFixture(const b2Shape* shape, float density){ return CreateFixture(shape, density); }

        /// Destroy a fixture. This removes the fixture from the broad-phase and
        /// destroys all contacts associated with this fixture. This will
        /// automatically adjust the mass of the body if the body is dynamic and the
        /// fixture has positive density.
        /// All fixtures attached to a body are implicitly destroyed when the body is destroyed.
        /// @param fixture the fixture to be removed.
        /// @warning This function is locked during callbacks.
        inline void destroyFixture(b2Fixture* fixture){ DestroyFixture(fixture); }

        /// Set the position of the body's origin and rotation.
        /// Manipulating a body's transform may cause non-physical behavior.
        /// Note: contacts are updated on the next call to b2World::Step.
        /// @param position the world position of the body's local origin.
        /// @param angle the world rotation in radians.
        inline void setTransform(const b2Vec2& position, float angle) { SetTransform(position, angle); }

        /// Get the body transform for the body's origin.
        /// @return the world transform of the body's origin.
        inline const b2Transform& getTransform() const { return GetTransform(); }

        /// Get the world body origin position.
        /// @return the world position of the body's origin.
        inline const b2Vec2& getPosition() const { return GetPosition(); }

        /// Get the angle in radians.
        /// @return the current world rotation angle in radians.
        inline float getAngle() const { return GetAngle(); }

        /// Get the world position of the center of mass.
        inline const b2Vec2& getWorldCenter() const { return GetWorldCenter(); }

        /// Get the local position of the center of mass.
        inline const b2Vec2& getLocalCenter() const { return GetLocalCenter(); }

        /// Set the linear velocity of the center of mass.
        /// @param v the new linear velocity of the center of mass.
        inline void setLinearVelocity(const b2Vec2& v) { SetLinearVelocity(v); }

        /// Get the linear velocity of the center of mass.
        /// @return the linear velocity of the center of mass.
        inline const b2Vec2& getLinearVelocity() const { return GetLinearVelocity(); }

        /// Set the angular velocity.
        /// @param omega the new angular velocity in radians/second.
        inline void setAngularVelocity(float omega) { SetAngularVelocity(omega); }

        /// Get the angular velocity.
        /// @return the angular velocity in radians/second.
        inline float getAngularVelocity() const { return GetAngularVelocity(); }

        /// Apply a force at a world point. If the force is not
        /// applied at the center of mass, it will generate a torque and
        /// affect the angular velocity. This wakes up the body.
        /// @param force the world force vector, usually in Newtons (N).
        /// @param point the world position of the point of application.
        /// @param wake also wake up the body
        inline void applyForce(const b2Vec2& force, const b2Vec2& point, bool wake) { ApplyForce(force, point, wake); }

        /// Apply a force to the center of mass. This wakes up the body.
        /// @param force the world force vector, usually in Newtons (N).
        /// @param wake also wake up the body
        inline void applyForceToCenter(const b2Vec2& force, bool wake) { ApplyForceToCenter(force, wake); }

        /// Apply a torque. This affects the angular velocity
        /// without affecting the linear velocity of the center of mass.
        /// @param torque about the z-axis (out of the screen), usually in N-m.
        /// @param wake also wake up the body
        inline void applyTorque(float torque, bool wake) { ApplyTorque(torque, wake); }

        /// Apply an impulse at a point. This immediately modifies the velocity.
        /// It also modifies the angular velocity if the point of application
        /// is not at the center of mass. This wakes up the body.
        /// @param impulse the world impulse vector, usually in N-seconds or kg-m/s.
        /// @param point the world position of the point of application.
        /// @param wake also wake up the body
        inline void applyLinearImpulse(const b2Vec2& impulse, const b2Vec2& point, bool wake) { ApplyLinearImpulse(impulse, point, wake); }

        /// Apply an impulse to the center of mass. This immediately modifies the velocity.
        /// @param impulse the world impulse vector, usually in N-seconds or kg-m/s.
        /// @param wake also wake up the body
        inline void applyLinearImpulseToCenter(const b2Vec2& impulse, bool wake) { ApplyLinearImpulseToCenter(impulse, wake); }

        /// Apply an angular impulse.
        /// @param impulse the angular impulse in units of kg*m*m/s
        /// @param wake also wake up the body
        inline void applyAngularImpulse(float impulse, bool wake) { ApplyAngularImpulse(impulse, wake); }

        /// Get the total mass of the body.
        /// @return the mass, usually in kilograms (kg).
        inline float getMass() const { return GetMass(); }

        /// Get the rotational inertia of the body about the local origin.
        /// @return the rotational inertia, usually in kg-m^2.
        inline float getInertia() const { return GetInertia(); }

        /// Get the mass data of the body.
        /// @return a struct containing the mass, inertia and center of the body.
        inline b2MassData getMassData() const { return GetMassData(); }

        /// Set the mass properties to override the mass properties of the fixtures.
        /// Note that this changes the center of mass position.
        /// Note that creating or destroying fixtures can also alter the mass.
        /// This function has no effect if the body isn't dynamic.
        /// @param data the mass properties.
        inline void setMassData(const b2MassData* data) { SetMassData(data); }

        /// This resets the mass properties to the sum of the mass properties of the fixtures.
        /// This normally does not need to be called unless you called SetMassData to override
        /// the mass and you later want to reset the mass.
        inline void resetMassData() { ResetMassData(); }

        /// Get the world coordinates of a point given the local coordinates.
        /// @param localPoint a point on the body measured relative the the body's origin.
        /// @return the same point expressed in world coordinates.
        inline b2Vec2 getWorldPoint(const b2Vec2& localPoint) const { return GetWorldPoint(localPoint); }

        /// Get the world coordinates of a vector given the local coordinates.
        /// @param localVector a vector fixed in the body.
        /// @return the same vector expressed in world coordinates.
        inline b2Vec2 getWorldVector(const b2Vec2& localVector) const { return GetWorldVector(localVector); }

        /// Gets a local point relative to the body's origin given a world point.
        /// @param worldPoint a point in world coordinates.
        /// @return the corresponding local point relative to the body's origin.
        inline b2Vec2 getLocalPoint(const b2Vec2& worldPoint) const { return GetLocalPoint(worldPoint); }

        /// Gets a local vector given a world vector.
        /// @param worldVector a vector in world coordinates.
        /// @return the corresponding local vector.
        inline b2Vec2 getLocalVector(const b2Vec2& worldVector) const { return GetLocalVector(worldVector); }

        /// Get the world linear velocity of a world point attached to this body.
        /// @param worldPoint a point in world coordinates.
        /// @return the world velocity of a point.
        inline b2Vec2 getLinearVelocityFromWorldPoint(const b2Vec2& worldPoint) const { return GetLinearVelocityFromWorldPoint(worldPoint); }

        /// Get the world velocity of a local point.
        /// @param localPoint a point in local coordinates.
        /// @return the world velocity of a point.
        inline b2Vec2 getLinearVelocityFromLocalPoint(const b2Vec2& localPoint) const { return GetLinearVelocityFromLocalPoint(localPoint); }

        /// Get the linear damping of the body.
        inline float getLinearDamping() const { return GetLinearDamping(); }

        /// Set the linear damping of the body.
        inline void setLinearDamping(float linearDamping) { SetLinearDamping(linearDamping); }

        /// Get the angular damping of the body.
        inline float getAngularDamping() const { return GetAngularDamping(); }

        /// Set the angular damping of the body.
        inline void setAngularDamping(float angularDamping) { SetAngularDamping(angularDamping); }

        /// Get the gravity scale of the body.
        inline float getGravityScale() const { return GetGravityScale(); }

        /// Set the gravity scale of the body.
        inline void setGravityScale(float scale) { SetGravityScale(scale); }

        /// Set the type of this body. This may alter the mass and velocity.
        inline void setType(b2BodyType type) { SetType(type); }

        /// Get the type of this body.
        inline b2BodyType getType() const { return GetType(); }

        /// Should this body be treated like a bullet for continuous collision detection?
        inline void setBullet(bool flag) { SetBullet(flag); }

        /// Is this body treated like a bullet for continuous collision detection?
        inline bool isBullet() const { return IsBullet(); }

        /// You can disable sleeping on this body. If you disable sleeping, the
        /// body will be woken.
        inline void setSleepingAllowed(bool flag) { SetSleepingAllowed(flag); }

        /// Is this body allowed to sleep
        inline bool isSleepingAllowed() const { return IsSleepingAllowed(); }

        /// Set the sleep state of the body. A sleeping body has very
        /// low CPU cost.
        /// @param flag set to true to wake the body, false to put it to sleep.
        inline void setAwake(bool flag) { SetAwake(flag); }

        /// Get the sleeping state of this body.
        /// @return true if the body is awake.
        inline bool isAwake() const { return IsAwake(); }

        /// Allow a body to be disabled. A disabled body is not simulated and cannot
        /// be collided with or woken up.
        /// If you pass a flag of true, all fixtures will be added to the broad-phase.
        /// If you pass a flag of false, all fixtures will be removed from the
        /// broad-phase and all contacts will be destroyed.
        /// Fixtures and joints are otherwise unaffected. You may continue
        /// to create/destroy fixtures and joints on disabled bodies.
        /// Fixtures on a disabled body are implicitly disabled and will
        /// not participate in collisions, ray-casts, or queries.
        /// Joints connected to a disabled body are implicitly disabled.
        /// An diabled body is still owned by a b2World object and remains
        /// in the body list.
        inline void setEnabled(bool flag) { SetEnabled(flag); }

        /// Get the active state of the body.
        inline bool isEnabled() const { return IsEnabled(); }

        /// Set this body to have fixed rotation. This causes the mass
        /// to be reset.
        inline void setFixedRotation(bool flag) { SetFixedRotation(flag); }

        /// Does this body have fixed rotation?
        inline bool isFixedRotation() const { return IsFixedRotation(); }

        /// Get the list of all fixtures attached to this body.
        inline b2Fixture* getFixtureList() { return GetFixtureList(); }
        inline const b2Fixture* getFixtureList() const { return GetFixtureList(); }

        /// Get the list of all joints attached to this body.
        inline b2JointEdge* getJointList() { return GetJointList(); }
        inline const b2JointEdge* getJointList() const { return GetJointList(); }

        /// Get the list of all contacts attached to this body.
        /// @warning this list changes during the time step and you may
        /// miss some collisions if you don't use b2ContactListener.
        inline b2ContactEdge* getContactList() { return GetContactList(); }
        inline const b2ContactEdge* getContactList() const { return GetContactList(); }

        /// Get the next body in the world's body list.
        inline b2Body* getNext() { return GetNext(); }
        inline const b2Body* getNext() const { return GetNext(); }

        /// Get the user data pointer that was provided in the body definition.
        inline b2BodyUserData& getUserData() { return GetUserData(); }
        inline const b2BodyUserData& getUserData() const { return GetUserData(); }

        /// Get the parent world of this body.
        inline b2World* getWorld() { return GetWorld(); }
        inline const b2World* getWorld() const { return GetWorld(); }

        /// Dump this body to a file
        inline void dump() { Dump(); }
    };
}