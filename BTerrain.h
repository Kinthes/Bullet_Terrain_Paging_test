#ifndef __BTERRAIN_H_
#define __BTERRAIN_H_

#include <BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h>
#include <BulletCollision\CollisionShapes\btTriangleCallback.h>
//#include <btBulletDynamicsCommon.h>
//#include <Bullet-C-Api.h>
//#include <btBulletCollisionCommon.h>

#include "OgreInit.h"
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <Terrain/OgreTerrain.h>

#include "MyMotionState.h"

class BTerrain //for Bullet
{
    public:
        BTerrain(Ogre::Terrain *ogTerrain, btDynamicsWorld *world, Ogre::SceneManager *smgr);
       // BTerrain(const Ogre::String &terrainFileName,int terrainSize, int terrainWorldSize,int terrainHeight,
       //             btDynamicsWorld *world, Ogre::SceneManager *smgr, bool Flip);
        virtual ~BTerrain();

    btRigidBody* getRigidBody();
    Ogre::SceneNode* getSceneNode();


        MyMotionState* getMyMotionState();
    protected:
        btDynamicsWorld         *m_world;
        //btCollisionShape        *m_shape;
        btHeightfieldTerrainShape *groundShape;
        btRigidBody             *m_body;

       // btHeightfieldTerrainShape* pTerrainShape;

        Ogre::SceneManager      *m_smgr;
        Ogre::Entity            *m_ent;
        Ogre::SceneNode         *m_node;

        MyMotionState           *motionState;
};

#endif

