#include "BCHS.h"




BCHS::BCHS(const Ogre::String& name, const Ogre::String& meshName,btDynamicsWorld* world,
             Ogre::SceneManager* smgr, const btVector3& halfdim, const btVector3& position, float mass) :
                                                                m_world(world), m_smgr(smgr)
{

    OgreInit::getSingletonPtr()->pLog->logMessage(name+"_ent");
    OgreInit::getSingletonPtr()->pLog->logMessage(meshName+".mesh");
    OgreInit::getSingletonPtr()->pLog->logMessage(name+"_node");
        m_ent = m_smgr->createEntity(name+"_ent", meshName+".mesh");
        m_node = m_smgr->getRootSceneNode()->createChildSceneNode(name+"_node");
        m_node->attachObject(m_ent);
        m_node->setPosition((Ogre::Vector3)position);
        m_node->scale(Ogre::Vector3(halfdim.x(), halfdim.y(), halfdim.z()) * (1.f/50));

//Entity *ent = SceneManager->createEntity(MeshName);
Ogre::MeshPtr MeshPtr = Ogre::MeshManager::getSingletonPtr()->load(meshName+".mesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
MeshStrider* Strider = new MeshStrider(MeshPtr.get());
//m_shape = new btGImpactMeshShape(Strider);
//m_shape->setMargin(0.05f);
//m_shape->updateBound();
 m_shape=new btConvexTriangleMeshShape(Strider);


 /*       size_t vertex_count=0,index_count=0;
        Ogre::Vector3* vertices;
        btScalar *points;
        unsigned * indices;
//*/
     //   BCHS::getMeshInformation(m_ent->getMesh().getPointer(),vertex_count,vertices,index_count,indices,Ogre::Vector3::ZERO,Ogre::Quaternion::IDENTITY,Ogre::Vector3::UNIT_SCALE);
//        OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(vertex_count));

/*       points= new btScalar[vertex_count*3];
        for(int i=0; i<vertex_count; i++)
            {
                points[i*3]= vertices[i][0];
                points[i*3+1]= vertices[i][1];
                points[i*3+2]= vertices[i][2];
            }
//*/
//OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString((btScalar)vertices[0][0]));
//        m_shape = new btConvexHullShape(points,vertex_count);
     /*   for (int i=0; i<vertex_count; i++)
            {
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(vertices[i]));

            }
*/
        m_shape->setLocalScaling(btVector3(halfdim.x(), halfdim.y(), halfdim.z()) * (1.f/50));
        OgreInit::getSingletonPtr()->pLog->logMessage("shape is done.");

//m_shape=new btBoxShape(halfdim);
        btVector3 inertia;
        m_shape->calculateLocalInertia(mass, inertia);

        btTransform posMat;
        posMat.setOrigin(position);
        posMat.setRotation(btQuaternion(0,0,0,1));


        motionState = new MyMotionState(posMat, m_node);
        btRigidBody::btRigidBodyConstructionInfo BodyCI(mass, motionState, m_shape, inertia);
        m_body = new btRigidBody(BodyCI);

        m_world->addRigidBody(m_body);

    /*    delete vertices;
        delete indices;
        delete points;*/
}

BCHS::~BCHS()
{
        delete m_body->getMotionState();
        m_world->removeRigidBody(m_body);
        delete m_body;
        delete m_shape;
        m_smgr->destroySceneNode(m_node);
        m_smgr->destroyEntity(m_ent);
}


btRigidBody* BCHS::getRigidBody()
{
        return m_body;
}

Ogre::Entity* BCHS::getEntity()
{
        return m_ent;
}

Ogre::SceneNode* BCHS::getSceneNode()
{
        return m_node;
}

MyMotionState* BCHS::getMyMotionState()
{
        return motionState;
}

