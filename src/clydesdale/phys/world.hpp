#pragma once
#include <box2d/box2d.h>
#include "rigid_body.hpp"

namespace Clydesdale::Phys {
    class World : private b2World {
    public:
        using b2World::b2World;

        /// Register a destruction listener. The listener is owned by you and must
        /// remain in scope.
        void setDestructionListener(b2DestructionListener* listener){ SetDestructionListener(listener); }

        /// Register a contact filter to provide specific control over collision.
        /// Otherwise the default filter is used (b2_defaultFilter). The listener is
        /// owned by you and must remain in scope.
        void setContactFilter(b2ContactFilter* filter){ SetContactFilter(filter); }

        /// Register a contact event listener. The listener is owned by you and must
        /// remain in scope.
        void setContactListener(b2ContactListener* listener){ SetContactListener(listener); }

        /// Register a routine for debug drawing. The debug draw functions are called
        /// inside with b2World::DebugDraw method. The debug draw object is owned
        /// by you and must remain in scope.
        void setDebugDraw(b2Draw* debugDraw){ SetDebugDraw(debugDraw); }

        /// Create a rigid body given a definition. No reference to the definition
        /// is retained.
        /// @warning This function is locked during callbacks.
        RigidBody createBody(const b2BodyDef* def){ return RigidBody(CreateBody(def)); }

        /// Destroy a rigid body given a definition. No reference to the definition
        /// is retained. This function is locked during callbacks.
        /// @warning This automatically deletes all associated shapes and joints.
        /// @warning This function is locked during callbacks.
        void destroyBody(b2Body* body){ DestroyBody(body); }

        /// Create a joint to constrain bodies together. No reference to the definition
        /// is retained. This may cause the connected bodies to cease colliding.
        /// @warning This function is locked during callbacks.
        b2Joint* createJoint(const b2JointDef* def){ return CreateJoint(def); }

        /// Destroy a joint. This may cause the connected bodies to begin colliding.
        /// @warning This function is locked during callbacks.
        void destroyJoint(b2Joint* joint){ DestroyJoint(joint); }

        /// Take a time step. This performs collision detection, integration,
        /// and constraint solution.
        /// @param timeStep the amount of time to simulate, this should not vary.
        /// @param velocityIterations for the velocity constraint solver.
        /// @param positionIterations for the position constraint solver.
        void step(float timeStep, int32 velocityIterations, int32 positionIterations){ Step(timeStep, velocityIterations, positionIterations); };

        /// Manually clear the force buffer on all bodies. By default, forces are cleared automatically
        /// after each call to Step. The default behavior is modified by calling SetAutoClearForces.
        /// The purpose of this function is to support sub-stepping. Sub-stepping is often used to maintain
        /// a fixed sized time step under a variable frame-rate.
        /// When you perform sub-stepping you will disable auto clearing of forces and instead call
        /// ClearForces after all sub-steps are complete in one pass of your game loop.
        /// @see SetAutoClearForces
        void clearForces(){ ClearForces(); }

        /// Call this to draw shapes and other debug draw data. This is intentionally non-const.
        void debugDraw(){ DebugDraw(); }

        /// Query the world for all fixtures that potentially overlap the
        /// provided AABB.
        /// @param callback a user implemented callback class.
        /// @param aabb the query box.
        void queryAABB(b2QueryCallback* callback, const b2AABB& aabb) const { QueryAABB(callback, aabb); }

        /// Ray-cast the world for all fixtures in the path of the ray. Your callback
        /// controls whether you get the closest point, any point, or n-points.
        /// The ray-cast ignores shapes that contain the starting point.
        /// @param callback a user implemented callback class.
        /// @param point1 the ray starting point
        /// @param point2 the ray ending point
        void rayCast(b2RayCastCallback* callback, const b2Vec2& point1, const b2Vec2& point2) const { RayCast(callback, point1, point2); }

        /// Get the world body list. With the returned body, use b2Body::GetNext to get
        /// the next body in the world list. A nullptr body indicates the end of the list.
        /// @return the head of the world body list.
        b2Body* getBodyList(){ return GetBodyList(); }
        const b2Body* getBodyList() const { return GetBodyList(); };

        /// Get the world joint list. With the returned joint, use b2Joint::GetNext to get
        /// the next joint in the world list. A nullptr joint indicates the end of the list.
        /// @return the head of the world joint list.
        b2Joint* getJointList(){ return GetJointList(); }
        const b2Joint* getJointList() const { return GetJointList(); };

        /// Get the world contact list. With the returned contact, use b2Contact::GetNext to get
        /// the next contact in the world list. A nullptr contact indicates the end of the list.
        /// @return the head of the world contact list.
        /// @warning contacts are created and destroyed in the middle of a time step.
        /// Use b2ContactListener to avoid missing contacts.
        b2Contact* getContactList(){ return GetContactList(); }
        const b2Contact* getContactList() const { return GetContactList(); };

        /// Enable/disable sleep.
        void setAllowSleeping(bool flag){ SetAllowSleeping(flag); }
        bool getAllowSleeping() const { return GetAllowSleeping(); }

        /// Enable/disable warm starting. For testing.
        void setWarmStarting(bool flag) { SetWarmStarting(flag); }
        bool getWarmStarting() const { return GetWarmStarting(); }

        /// Enable/disable continuous physics. For testing.
        void setContinuousPhysics(bool flag) { SetContinuousPhysics(flag); }
        bool getContinuousPhysics() const { return GetContinuousPhysics(); }

        /// Enable/disable single stepped continuous physics. For testing.
        void setSubStepping(bool flag) { SetSubStepping(flag); }
        bool getSubStepping() const { return GetSubStepping(); }

        /// Get the number of broad-phase proxies.
        int32 getProxyCount() const { return GetProxyCount(); }

        /// Get the number of bodies.
        int32 getBodyCount() const { return GetBodyCount(); }

        /// Get the number of joints.
        int32 getJointCount() const { return GetJointCount(); }

        /// Get the number of contacts (each may have 0 or more contact points).
        int32 getContactCount() const { return GetContactCount(); }

        /// Get the height of the dynamic tree.
        int32 getTreeHeight() const { return GetTreeHeight(); }

        /// Get the balance of the dynamic tree.
        int32 getTreeBalance() const { return GetTreeBalance(); }

        /// Get the quality metric of the dynamic tree. The smaller the better.
        /// The minimum is 1.
        float getTreeQuality() const { return GetTreeQuality(); }

        /// Change the global gravity vector.
        void setGravity(const b2Vec2& gravity){ SetGravity(gravity); }

        /// Get the global gravity vector.
        b2Vec2 getGravity() const { return GetGravity(); }

        /// Is the world locked (in the middle of a time step).
        bool isLocked() const { return IsLocked(); }

        /// Set flag to control automatic clearing of forces after each time step.
        void setAutoClearForces(bool flag){ SetAutoClearForces(flag); }

        /// Get the flag that controls automatic clearing of forces after each time step.
        bool getAutoClearForces() const { return GetAutoClearForces(); }

        /// Shift the world origin. Useful for large worlds.
        /// The body shift formula is: position -= newOrigin
        /// @param newOrigin the new origin with respect to the old origin
        void shiftOrigin(const b2Vec2& newOrigin){ ShiftOrigin(newOrigin); }

        /// Get the contact manager for testing.
        const b2ContactManager& getContactManager() const { return GetContactManager(); }

        /// Get the current profile.
        const b2Profile& getProfile() const { return GetProfile(); };

        /// Dump the world into the log file.
        /// @warning this should be called outside of a time step.
        void dump(){ Dump(); };
    };
}