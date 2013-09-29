#ifndef __start_h
#define __start_h

#include "OgreInit.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include <Terrain/OgreTerrainQuadTreeNode.h>
#include <Terrain/OgreTerrainMaterialGeneratorA.h>
#include <Terrain/OgreTerrainPaging.h>

#include "PagedGeometry/PagedGeometry.h"
#include "PagedGeometry/BatchPage.h"
#include "PagedGeometry/WindBatchPage.h"
#include "PagedGeometry/TreeLoader3D.h"
#include "PagedGeometry/TreeLoader2D.h"
#include "PagedGeometry/ImpostorPage.h"
#include "PagedGeometry/GrassLoader.h"


#include "BCube.h"
#include "BTerrain.h"
#include "BCHS.h"
//#include "BSphere.h"
#include "DebugDrawer.h"

class StartDemo : public OIS::KeyListener, public OIS::MouseListener
{
  public:
        StartDemo();
        ~StartDemo();

         bool startTry(void);
         bool runDemo(void);
         void setupScene(void);

        bool keyPressed(const OIS::KeyEvent &keyEventRef);
        bool keyReleased(const OIS::KeyEvent &keyEventRef);
         bool mouseMoved(const OIS::MouseEvent &evt);
         bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
         bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);


  private:

        void defineTerrain(Ogre::String, long x, long y, bool flat=false);
        void initBlendMaps(Ogre::Terrain *terrain);
        void configureTerrainDefaults(Ogre::Light* light);
        void configureShadows(bool enabled, bool depthShadows);
        //float getTerrainHeight(const float x, const float z, void *userData);
        static float getTerrainHeight(const float x, const float z, void *userData = NULL);
        void setForest(void);
        Ogre::MaterialPtr buildDepthShadowMaterial(const Ogre::String &textureName);
        Ogre::ShadowCameraSetupPtr  mPSSMSetup;

        typedef std::list<Ogre::Entity*> EntityList;
        EntityList mHouseList;

		bool                        mTerrainsImported;
		Ogre::TerrainGlobalOptions 	*pTerrainGlobals;
        static Ogre::TerrainGroup 			*pTerrainGroup;
        Ogre::Vector3               mTerrainPos;
        bool                        mPaging;
        Ogre::TerrainPaging         *pTerrainPaging;

        Ogre::PageManager           *pPageManager;

        bool                        mDemoShutdown;
        Ogre::SceneNode             *pObjectNode;
        Ogre::Entity                *pObjectEntity;

        OgreBites::Label            *pInfoLabel;
        Ogre::Real                  mFallVelocity;
        Forests::PagedGeometry      *trees, *bushes,*grass;

        std::vector<BTerrain*> m_terrainlist;
        std::vector<BCube*> m_cubelist;
        std::vector<BCHS*> m_bchslist;
        BTDebug                     *pBtDebug;
};

	/// This class just pretends to provide prcedural page content to avoid page loading
class DummyPageProvider : public Ogre::PageProvider
	{
	public:
		bool prepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
		bool loadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
		bool unloadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
		bool unprepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
	};


#endif
