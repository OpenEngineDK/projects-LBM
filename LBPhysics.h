#ifndef _LB_PHYSICS_
#define _LB_PHYSICS_

#include <Core/EngineEvents.h>
#include <Core/IListener.h>
#include <Resources/Texture3D.h>
#include <boost/serialization/weak_ptr.hpp>

using namespace OpenEngine;

/**
 *  smart pointer.
 */
class LBPhysics;
typedef boost::shared_ptr<LBPhysics> LBPhysicsPtr;

class LBPhysics 
: public Core::IListener<Core::InitializeEventArg>,
    public Core::IListener<Core::ProcessEventArg>, 
    public virtual Resources::Texture3D<unsigned char> {

 private:
    boost::weak_ptr<LBPhysics> weak_this;
    LBPhysics();

	void StreamStep();
	void CollideStep();
	void Swap();

    unsigned int current;
    unsigned int other;

 public:
    static LBPhysicsPtr Create();
    ~LBPhysics() {}
    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
};

#endif // _LB_PHYSICS_
