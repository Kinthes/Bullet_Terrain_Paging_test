#include "BTerrain.h"



BTerrain::BTerrain(Ogre::Terrain *ogTerrain, btDynamicsWorld *world,Ogre::SceneManager *smgr) :  m_world(world), m_smgr(smgr)
{

//*
    float *terrainHeightData = ogTerrain->getHeightData();
    Ogre::Vector3 terrainPosition = ogTerrain->getPosition();

    float *pDataConvert = new float[ogTerrain->getSize() * ogTerrain->getSize()];
    for(int i=0; i<ogTerrain->getSize();i++)
        memcpy(pDataConvert+ogTerrain->getSize()*i,   //source
               terrainHeightData+ogTerrain->getSize()*(ogTerrain->getSize()-1-i), //target
               sizeof(float)*(ogTerrain->getSize()) //size
               );
    float metersBetweenVertices = ogTerrain->getWorldSize()/(ogTerrain->getSize()-1);

    btVector3 localScaling(metersBetweenVertices, 1, metersBetweenVertices);

    groundShape = new btHeightfieldTerrainShape(ogTerrain->getSize(),
                                                                           ogTerrain->getSize(),
                                                                           pDataConvert,
                                                                           1,
                                                                           ogTerrain->getMinHeight(),
                                                                           ogTerrain->getMaxHeight(),
                                                                           1,
                                                                           PHY_FLOAT,
                                                                           true);
    groundShape->setUseDiamondSubdivision(true);
    groundShape->setLocalScaling(localScaling);


    m_body = new btRigidBody(0,new btDefaultMotionState(), groundShape);

    m_body->getWorldTransform().setOrigin( btVector3(terrainPosition.x, terrainPosition.y + (ogTerrain->getMaxHeight()- ogTerrain->getMinHeight())/2, terrainPosition.z ));
    m_body->getWorldTransform().setRotation(btQuaternion(Ogre::Quaternion::IDENTITY.x,Ogre::Quaternion::IDENTITY.y, Ogre::Quaternion::IDENTITY.z, Ogre::Quaternion::IDENTITY.w));
    m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    m_world->addRigidBody(m_body);

}

BTerrain::~BTerrain()
{
        delete m_body->getMotionState();
        m_world->removeRigidBody(m_body);
        delete m_body;
        delete groundShape;
       // m_smgr->destroySceneNode(m_node);
        //m_smgr->destroyEntity(m_ent);
}


btRigidBody* BTerrain::getRigidBody()
{
        return m_body;
}


Ogre::SceneNode* BTerrain::getSceneNode()
{
        return m_node;
}

MyMotionState* BTerrain::getMyMotionState()
{
        return motionState;
}

