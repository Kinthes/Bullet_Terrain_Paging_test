#ifndef __BCHS_H_
#define __BCHS_H_

//#include <btBulletDynamicsCommon.h>
//#include <Bullet-C-Api.h>
//#include <btBulletCollisionCommon.h>
#include <BulletCollision\GImpact\btGImpactShape.h>
#include "OgreInit.h"
#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "MeshStrider.h"
#include "MyMotionState.h"

class BCHS //for Bullet
{
    public:
        BCHS(const Ogre::String &name,const Ogre::String &meshName, btDynamicsWorld *world, Ogre::SceneManager *smgr, const btVector3 &dim,const btVector3& position, float mass);
        virtual ~BCHS();

        btRigidBody* getRigidBody();
        Ogre::SceneNode* getSceneNode();
        Ogre::Entity* getEntity();

        MyMotionState* getMyMotionState();
    protected:
        btDynamicsWorld         *m_world;
        btCollisionShape        *m_shape;
      //  btGImpactMeshShape      *m_shape;
        btRigidBody             *m_body;

        Ogre::SceneManager      *m_smgr;
        Ogre::Entity            *m_ent;
        Ogre::SceneNode         *m_node;

        MyMotionState           *motionState;
};

#endif

