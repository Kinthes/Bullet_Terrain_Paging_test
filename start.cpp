#include "start.h"

#include <OgreLight.h>
#include <OgreWindowEventUtilities.h>

#define PAGING
#define TERRAIN_PAGE_MIN_X 0
#define TERRAIN_PAGE_MIN_Y 0
#define TERRAIN_PAGE_MAX_X 0
#define TERRAIN_PAGE_MAX_Y 0

#define TERRAIN_FILE_PREFIX Ogre::String("tryTerrain")
#define TERRAIN_FILE_SUFFIX Ogre::String("dat")
#define TERRAIN_WORLD_SIZE 12000
#define TERRAIN_SIZE 513

#ifdef PAGING
DummyPageProvider mDummyPageProvider;
#endif

Ogre::TerrainGroup *StartDemo::pTerrainGroup = 0;
int cubeCounter = 0;
int bchsCounter = 0;
bool debugState=false;


StartDemo::StartDemo()
{
    mDemoShutdown = false;
    pObjectNode =0;
    pObjectEntity =0;
    mTerrainPos=Ogre::Vector3::ZERO;
    mTerrainsImported=false;
    mFallVelocity=0.0f;
}

StartDemo::~StartDemo()
{
	//*Delete the TreeLoader3D instance

    delete trees->getPageLoader();
	delete trees;
	OgreInit::getSingletonPtr()->pSceneMgr->destroyEntity("Tree");
	delete grass->getPageLoader();
	//Delete the PagedGeometry instance
	delete grass;//*/

        for(int i=0; i<m_cubelist.size(); i++)
        delete m_cubelist[i];
                for(int i=0; i<m_bchslist.size(); i++)
        delete m_bchslist[i];
            for(int i=0; i<m_terrainlist.size(); i++)
        delete m_terrainlist[i];
#ifdef PAGING
			OGRE_DELETE pTerrainPaging;
			OGRE_DELETE pPageManager;
#endif
			OGRE_DELETE pTerrainGroup;
    OGRE_DELETE pTerrainGlobals;
    if (!mPSSMSetup.isNull()) mPSSMSetup.freeMethod();
    delete OgreInit::getSingletonPtr();
 }
//--------------------------------------------------------------------------------------

bool StartDemo::startTry()
{
    new OgreInit();
    if(!OgreInit::getSingletonPtr()->startOgre("First Shot", this, 0)) return false;

    mDemoShutdown=false;

    OgreInit::getSingletonPtr()->pLog->logMessage("Demo Initialized");

    setupScene();
    return runDemo();
}

void getTerrainImage(bool flipX, bool flipY, Ogre::Image &img, Ogre::String namefile)
{
    img.load(namefile,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if(flipX)
        img.flipAroundX();
    if(flipY)
        img.flipAroundY();
}

//----------------------------------------------------------------------------------
void StartDemo::defineTerrain(Ogre::String filename,long x, long y, bool flat)
{
    if(flat)
    {
        pTerrainGroup->defineTerrain(x,y, 0.0f);
    }
    else
    {
        Ogre::String filename = pTerrainGroup->generateFilename(x,y);
        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(pTerrainGroup->getResourceGroup(), filename))
            {
                    pTerrainGroup->defineTerrain(x,y);
            }
        else
            {
                    Ogre::Image img;
                    getTerrainImage(x%2!=0, y%2!=0, img,filename);
                    pTerrainGroup->defineTerrain(x,y,&img);
                    mTerrainsImported=true;
            }
    }
}

//----------------------------------------------------------------------------------
void StartDemo::initBlendMaps(Ogre::Terrain *terrain)
{
    Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
    Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
    Ogre::Real minHeight0 = 70;
    Ogre::Real fadeDist0 = 40;
    Ogre::Real minHeight1 = 70;
    Ogre::Real fadeDist1 = 15;
    float* pBlend0 = blendMap0->getBlendPointer();
    float* pBlend1 = blendMap1->getBlendPointer();
    for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
        {
            Ogre::Real tx, ty;

            blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
            Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
            Ogre::Real val = (height - minHeight0) / fadeDist0;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend0++ = val*255;

            val = (height - minHeight1) / fadeDist1;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend1++ = val;
        }
    }
    blendMap0->dirty();
    blendMap1->dirty();
    blendMap0->update();
    blendMap1->update();

    	// set up a colour map
	/*
	if (!terrain->getGlobalColourMapEnabled())
	{
		terrain->setGlobalColourMapEnabled(true);
		Image colourMap;
		colourMap.load("testcolourmap.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		terrain->getGlobalColourMap()->loadImage(colourMap);
	}
	*/
}//initBlendMaps

//--------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
void StartDemo::configureTerrainDefaults(Ogre::Light *light)
{
    pTerrainGlobals->setMaxPixelError(8);
    pTerrainGlobals->setCompositeMapDistance(3000);

    pTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
    pTerrainGlobals->setCompositeMapAmbient(OgreInit::getSingletonPtr()->pSceneMgr->getAmbientLight());
    pTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

// Configure default import settings for if we use imported image
    Ogre::Terrain::ImportData& defaultimp = pTerrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = TERRAIN_SIZE;
    defaultimp.worldSize = TERRAIN_WORLD_SIZE;
    defaultimp.inputScale = 600;
    defaultimp.minBatchSize = 33;
    defaultimp.maxBatchSize = 65;

// textures
    defaultimp.layerList.resize(3);
    defaultimp.layerList[0].worldSize = 5;
    defaultimp.layerList[0].textureNames.push_back("TEX_gwada_01.jpg");//dirt_grayrocky_diffusespecular.dds");
    defaultimp.layerList[0].textureNames.push_back("NM_gwada_01_B.jpg");//dirt_grayrocky_normalheight.dds");
    defaultimp.layerList[1].worldSize = 30;
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
    defaultimp.layerList[2].worldSize = 80;
    defaultimp.layerList[2].textureNames.push_back("grass_1024.jpg");//growth_weirdfungus-03_diffusespecular.dds");
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");


   Ogre::TerrainMaterialGeneratorA::SM2Profile* matProfile = 0;

matProfile = (Ogre::TerrainMaterialGeneratorA::SM2Profile*) pTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile();
      if (matProfile)
      {
         //matProfile->setGlobalColourMapEnabled(false);
         matProfile->setLayerSpecularMappingEnabled(false);
         //matProfile->setLayerNormalMappingEnabled(false);
         matProfile->setLayerParallaxMappingEnabled(false);
   }
}

//--------------------------------------------------------------------------------------------------
void StartDemo::setupScene()
{
    bool blankTerrain = false;

    OgreInit::getSingletonPtr()->pMouse->setEventCallback(this);
    OgreInit::getSingletonPtr()->pKeyboard->setEventCallback(this);

    srand(time(NULL));


    OgreInit::getSingletonPtr()->pCamera->setPosition(Ogre::Vector3(50,850,50));
    OgreInit::getSingletonPtr()->pCamera->lookAt(Ogre::Vector3(0,-50,200));
    OgreInit::getSingletonPtr()->pCamera->setNearClipDistance(0.1);
    OgreInit::getSingletonPtr()->pCamera->setFarClipDistance(50000);
   // OgreInit::getSingletonPtr()->pCamera->setPolygonMode(Ogre::PM_WIREFRAME);
 /*   if(OgreInit::getSingletonPtr()->pRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
        {
            OgreInit::getSingletonPtr()->pCamera->setFarClipDistance(0);
        }*/

    Ogre::Vector3 lightdir(0.55,-0.3,0.75);
    lightdir.normalise();
    Ogre::Light *light=OgreInit::getSingletonPtr()->pSceneMgr->createLight("tstLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(lightdir);
    light->setDiffuseColour(Ogre::ColourValue(0.4,0.4,0.4));
    light->setSpecularColour(Ogre::ColourValue(0.2,0.2,0.2));

    pTerrainGlobals=OGRE_NEW Ogre::TerrainGlobalOptions();
    pTerrainGroup = OGRE_NEW Ogre::TerrainGroup(OgreInit::getSingletonPtr()->pSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
    pTerrainGroup->setFilenameConvention(TERRAIN_FILE_PREFIX, TERRAIN_FILE_SUFFIX);
    pTerrainGroup->setOrigin(mTerrainPos + Ogre::Vector3(TERRAIN_WORLD_SIZE/2, 0, TERRAIN_WORLD_SIZE/2));
    configureTerrainDefaults(light);
#ifdef PAGING
OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString("agreu"));
OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(pTerrainGroup->getTerrainWorldSize()));
	// Paging setup
	pPageManager = OGRE_NEW Ogre::PageManager();
	// Since we're not loading any pages from .page files, we need a way just
	// to say we've loaded them without them actually being loaded

	pPageManager->setPageProvider(&mDummyPageProvider);
	pPageManager->addCamera(OgreInit::getSingletonPtr()->pCamera);
	pTerrainPaging = OGRE_NEW Ogre::TerrainPaging(pPageManager);
	Ogre::PagedWorld* world = pPageManager->createWorld();
	pTerrainPaging->createWorldSection(world, pTerrainGroup, 2000, 3000,
		TERRAIN_PAGE_MIN_X, TERRAIN_PAGE_MIN_Y,
		TERRAIN_PAGE_MAX_X, TERRAIN_PAGE_MAX_Y);
    OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(pTerrainGroup->getTerrainWorldSize()));

#endif
    for(long x=TERRAIN_PAGE_MIN_X;x<=TERRAIN_PAGE_MAX_X;++x)
        for(long y=TERRAIN_PAGE_MIN_Y;y<=TERRAIN_PAGE_MAX_Y;++y)
            {
                defineTerrain("terrain.png", x,y, blankTerrain);
            }
    pTerrainGroup->loadAllTerrains(true); //a voir dans les def de classe


    if (mTerrainsImported)
    {
        Ogre::TerrainGroup::TerrainIterator ti = pTerrainGroup->getTerrainIterator();
        while(ti.hasMoreElements())
        {
            Ogre::Terrain* t = ti.getNext()->instance;
            initBlendMaps(t);
        }
    }


//definir les shapes du terrain pour bullet
    Ogre::TerrainGroup::TerrainIterator bt_ti = pTerrainGroup->getTerrainIterator();
    while(bt_ti.hasMoreElements())
        {
            Ogre::Terrain* bt_t = bt_ti.getNext()->instance;
            BTerrain *land = new BTerrain(bt_t,
                                          OgreInit::getSingletonPtr()->m_world,
                                          OgreInit::getSingletonPtr()->pSceneMgr);
           // land->getRigidBody()->translate(btVector3(0, 100, 0));
            m_terrainlist.push_back(land);
        }

//#ifdef PAGING //decharge tous les terrains charges pour définir les bullet shapes
//    for(long x=TERRAIN_PAGE_MIN_X;x<=TERRAIN_PAGE_MAX_X;++x)
//            for(long y=TERRAIN_PAGE_MIN_Y;y<=TERRAIN_PAGE_MAX_Y;++y)
//                {
//                pTerrainGroup->unloadTerrain(x,y);
//                }
//#endif

	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(8);

    pTerrainGroup->freeTemporaryResources();

    setForest();

//*

	Ogre::Entity* e = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("tudorhouse.mesh");
	Ogre::Vector3 entPos(mTerrainPos.x + 243, 0, mTerrainPos.z - 171);
	Ogre::Quaternion rot;
	entPos.y = pTerrainGroup->getHeightAtWorldPosition(entPos) + 65.5 + mTerrainPos.y;
	rot.FromAngleAxis(Ogre::Degree(Ogre::Math::RangeRandom(-180, 180)), Ogre::Vector3::UNIT_Y);
	Ogre::SceneNode* sn = OgreInit::getSingletonPtr()->pSceneMgr->getRootSceneNode()->createChildSceneNode(entPos, rot);
	sn->setScale(Ogre::Vector3(0.12, 0.12, 0.12));
	sn->attachObject(e);
	mHouseList.push_back(e);

	e = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("tudorhouse.mesh");
	entPos = Ogre::Vector3(mTerrainPos.x + 1850, 0, mTerrainPos.z + 1478);
	entPos.y = pTerrainGroup->getHeightAtWorldPosition(entPos) + 65.5 + mTerrainPos.y;
	rot.FromAngleAxis(Ogre::Degree(Ogre::Math::RangeRandom(-180, 180)), Ogre::Vector3::UNIT_Y);
	sn = OgreInit::getSingletonPtr()->pSceneMgr->getRootSceneNode()->createChildSceneNode(entPos, rot);
	sn->setScale(Ogre::Vector3(0.12, 0.12, 0.12));
	sn->attachObject(e);
	mHouseList.push_back(e);

	e = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("tudorhouse.mesh");
	entPos = Ogre::Vector3(mTerrainPos.x + 1970, 0, mTerrainPos.z + 2180);
	entPos.y = pTerrainGroup->getHeightAtWorldPosition(entPos) + 65.5 + mTerrainPos.y;
	rot.FromAngleAxis(Ogre::Degree(Ogre::Math::RangeRandom(-180, 180)), Ogre::Vector3::UNIT_Y);
	sn = OgreInit::getSingletonPtr()->pSceneMgr->getRootSceneNode()->createChildSceneNode(entPos, rot);
	sn->setScale(Ogre::Vector3(0.12, 0.12, 0.12));
	sn->attachObject(e);
	mHouseList.push_back(e);//*/

 //*

/*
    for(int i=0; i<10; i++)
    {
            BCube *cube = new BCube("cube"+Ogre::StringConverter::toString(i), OgreInit::getSingletonPtr()->m_world, OgreInit::getSingletonPtr()->pSceneMgr, btVector3(2,2,2), 300);
            cube->getRigidBody()->translate(btVector3(rand()%1200+100, rand()%100+1020, rand()%1200+100));
            cube->getEntity()->setMaterialName("Ecs/Ground1");
            m_cubelist.push_back(cube);
    }//*/

    OgreInit::getSingletonPtr()->pSceneMgr->setSkyBox(true, "Ecs/SkyBox");
    configureShadows(false, false);
    //OgreInit::getSingletonPtr()->pSceneMgr->setSkyDome(true, "Examples/CloudySky",62,8);

#ifdef PAGING //decharge tous les terrains charges pour définir les bullet shapes
    for(long x=TERRAIN_PAGE_MIN_X;x<=TERRAIN_PAGE_MAX_X;++x)
            for(long y=TERRAIN_PAGE_MIN_Y;y<=TERRAIN_PAGE_MAX_Y;++y)
                {
                pTerrainGroup->unloadTerrain(x,y);
                }
#endif

}//setupDemoScene

//-------------------------------------------------------------------------------------------
void StartDemo::configureShadows(bool enabled, bool depthShadows)
{
	Ogre::TerrainMaterialGeneratorA::SM2Profile* matProfile =
		static_cast<Ogre::TerrainMaterialGeneratorA::SM2Profile*>(pTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());
	matProfile->setReceiveDynamicShadowsEnabled(enabled);
#ifdef SHADOWS_IN_LOW_LOD_MATERIAL
	matProfile->setReceiveDynamicShadowsLowLod(true);
#else
	matProfile->setReceiveDynamicShadowsLowLod(false);
#endif

	// Default materials
	for (EntityList::iterator i = mHouseList.begin(); i != mHouseList.end(); ++i)
	{
		(*i)->setMaterialName("Examples/TudorHouse");
	}

	if (enabled)
	{
		// General scene setup
		OgreInit::getSingletonPtr()->pSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
		OgreInit::getSingletonPtr()->pSceneMgr->setShadowFarDistance(3000);

		// 3 textures per directional light (PSSM)
		OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);

		if (mPSSMSetup.isNull())
		{
			// shadow camera setup
			Ogre::PSSMShadowCameraSetup* pssmSetup = new Ogre::PSSMShadowCameraSetup();
			pssmSetup->setSplitPadding(OgreInit::getSingletonPtr()->pCamera->getNearClipDistance());
			pssmSetup->calculateSplitPoints(3, OgreInit::getSingletonPtr()->pCamera->getNearClipDistance(), OgreInit::getSingletonPtr()->pSceneMgr->getShadowFarDistance());
			pssmSetup->setOptimalAdjustFactor(0, 2);
			pssmSetup->setOptimalAdjustFactor(1, 1);
			pssmSetup->setOptimalAdjustFactor(2, 0.5);

			mPSSMSetup.bind(pssmSetup);

		}
		OgreInit::getSingletonPtr()->pSceneMgr->setShadowCameraSetup(mPSSMSetup);

		if (depthShadows)
		{
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureCount(3);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureConfig(0, 2048, 2048, Ogre::PF_FLOAT32_R);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureConfig(1, 1024, 1024, Ogre::PF_FLOAT32_R);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureConfig(2, 1024, 1024, Ogre::PF_FLOAT32_R);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureSelfShadow(true);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowCasterRenderBackFaces(true);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureCasterMaterial("PSSM/shadow_caster");

			Ogre::MaterialPtr houseMat = buildDepthShadowMaterial("fw12b.jpg");
			for (EntityList::iterator i = mHouseList.begin(); i != mHouseList.end(); ++i)
			{
				(*i)->setMaterial(houseMat);
			}

		}
		else
		{
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureCount(3);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureConfig(0, 2048, 2048, Ogre::PF_X8B8G8R8);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureConfig(1, 1024, 1024, Ogre::PF_X8B8G8R8);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureConfig(2, 1024, 1024, Ogre::PF_X8B8G8R8);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureSelfShadow(false);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowCasterRenderBackFaces(false);
			OgreInit::getSingletonPtr()->pSceneMgr->setShadowTextureCasterMaterial(Ogre::StringUtil::BLANK);
		}

		matProfile->setReceiveDynamicShadowsDepth(depthShadows);
		matProfile->setReceiveDynamicShadowsPSSM(static_cast<Ogre::PSSMShadowCameraSetup*>(mPSSMSetup.get()));

		//addTextureShadowDebugOverlay(TL_RIGHT, 3);


	}
	else
	{
		OgreInit::getSingletonPtr()->pSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
	}


}//configureShadows
//---------------------------------------------------------------------------------------------------

Ogre::MaterialPtr StartDemo::buildDepthShadowMaterial(const Ogre::String& textureName)
{
	Ogre::String matName = "DepthShadows/" + textureName;

	Ogre::MaterialPtr ret = Ogre::MaterialManager::getSingleton().getByName(matName);
	if (ret.isNull())
	{
		Ogre::MaterialPtr baseMat = Ogre::MaterialManager::getSingleton().getByName("Ogre/shadow/depth/integrated/pssm");
		ret = baseMat->clone(matName);
		Ogre::Pass* p = ret->getTechnique(0)->getPass(0);
		p->getTextureUnitState("diffuse")->setTextureName(textureName);

		Ogre::Vector4 splitPoints;
		const Ogre::PSSMShadowCameraSetup::SplitPointList& splitPointList =
			static_cast<Ogre::PSSMShadowCameraSetup*>(mPSSMSetup.get())->getSplitPoints();
		for (int i = 0; i < 3; ++i)
		{
			splitPoints[i] = splitPointList[i];
		}
		p->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", splitPoints);


	}

	return ret;
}//buildDepthShadowMaterial
//-------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
float StartDemo::getTerrainHeight(const float x, const float z, void *userData)
{
	return pTerrainGroup->getHeightAtWorldPosition(x, 1000, z);
}


void    StartDemo::setForest(void)
{

//*
    trees=new Forests::PagedGeometry();
    trees->setCamera(OgreInit::getSingletonPtr()->pCamera);
    trees->setPageSize(200);
    trees->setInfinite();
    trees->addDetailLevel<Forests::WindBatchPage>(2000, 50);
  //  trees->addDetailLevel<Forests::BatchPage>(2000,30);
    trees->addDetailLevel<Forests::ImpostorPage>(TERRAIN_WORLD_SIZE,50);

    Forests::TreeLoader3D *ptreeLoader= new Forests::TreeLoader3D(trees, Forests::TBounds(mTerrainPos.x, mTerrainPos.z-TERRAIN_WORLD_SIZE, mTerrainPos.x+TERRAIN_WORLD_SIZE, mTerrainPos.z+TERRAIN_WORLD_SIZE));
    trees->setPageLoader(ptreeLoader);

  //  ptreeLoader->setHeightFunction(&getTerrainHeight);

    ptreeLoader->setColorMap("terrain_lightmap.jpg");
    Ogre::Entity *tree1 = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("Tree1", "fir05_30.mesh");
    Ogre::Entity *tree2 = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("Tree2", "fir14_25.mesh");

    trees->setCustomParam(tree1->getName(), "windFactorX", 1);
	trees->setCustomParam(tree1->getName(), "windFactorY", 0.01);
	trees->setCustomParam(tree2->getName(), "windFactorX", 1.2);
	trees->setCustomParam(tree2->getName(), "windFactorY", 0.013);

    Ogre::Vector3 position;
	Ogre::Radian yaw;
	Ogre::Real scale;
	for (int i = 0; i < 1000; i++)
	{
		yaw = Ogre::Degree(Ogre::Math::RangeRandom(0, 360));

		position.x = Ogre::Math::RangeRandom(mTerrainPos.x, mTerrainPos.x + TERRAIN_WORLD_SIZE);
		position.z = Ogre::Math::RangeRandom(mTerrainPos.z - TERRAIN_WORLD_SIZE, mTerrainPos.z + TERRAIN_WORLD_SIZE);
		position.y = pTerrainGroup->getHeightAtWorldPosition(Ogre::Vector3(position.x, 1000, position.z)); // + 65.5 + mTerrainPos.y;
		scale = Ogre::Math::RangeRandom(0.5f, 1.5f);

        if(Ogre::Math::UnitRandom() < 0.5f)
            ptreeLoader->addTree(tree1, position, yaw, scale);
        else ptreeLoader->addTree(tree2, position, yaw, scale);
	}//*/

	//-------------------------------------- LOAD BUSHES --------------------------------------
	//Create and configure a new PagedGeometry instance for bushes
/*	bushes = new Forests::PagedGeometry(OgreInit::getSingletonPtr()->pCamera, 50);
	//bushes->addDetailLevel<Forests::BatchPage>(80, 50);
	bushes->addDetailLevel<Forests::WindBatchPage>(80, 50);

	//Create a new TreeLoader2D object for the bushes
Forests::TreeLoader3D *bushLoader = new Forests::TreeLoader3D(bushes, Forests::TBounds(0, 0, 1500, 1500));

	bushes->setPageLoader(bushLoader);

	//Load a bush entity
	Ogre::Entity *fern = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("Fern", "farn1.mesh");
	Ogre::Entity *plant = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("Plant", "plant2.mesh");
	Ogre::Entity *mushroom = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("Mushroom", "shroom1_1.mesh");
Ogre::Entity *plant2 = OgreInit::getSingletonPtr()->pSceneMgr->createEntity("Plant2", "plant1.mesh");
//-----WIND
	bushes->setCustomParam(fern->getName(), "windFactorX", 2);
	bushes->setCustomParam(fern->getName(), "windFactorY", 0.01);

	bushes->setCustomParam(plant->getName(), "windFactorX", 1.2);
	bushes->setCustomParam(plant->getName(), "windFactorY", 0.02);

		bushes->setCustomParam(plant2->getName(), "windFactorX", 1.2);
	bushes->setCustomParam(plant2->getName(), "windFactorY", 0.02);


	//Randomly place 30,000 copies of the bush on the terrain
	for (int i = 0; i < 30000; i++){
		yaw = Ogre::Degree(Ogre::Math::RangeRandom(0, 360));
		position.x = Ogre::Math::RangeRandom(0, 1500);
		position.z = Ogre::Math::RangeRandom(0, 1500);
        position.y=HeightFunction::getTerrainHeight(position.x, position.z);
        float rnd = Ogre::Math::UnitRandom();
        if(rnd<0.7)
            {
            scale = Ogre::Math::RangeRandom(0.2f, 0.3f);
            bushLoader->addTree(fern, position, yaw, scale);
            }
        else if(rnd<0.8)
        {
            scale = Ogre::Math::RangeRandom(0.17f, 0.25f);
            bushLoader->addTree(plant, position, yaw, scale);
        }
        else if(rnd<0.9)
        {
            scale = Ogre::Math::RangeRandom(0.1f, 0.15f);
            bushLoader->addTree(mushroom, position, yaw, scale);
        }
        else
        {
            scale = Ogre::Math::RangeRandom(0.08f, 0.16f);
            bushLoader->addTree(plant2, position, yaw, scale);
        }
	} //*/

        //-------------------------------GRASS AROUND--------------------------------
//*
    grass = new Forests::PagedGeometry(OgreInit::getSingletonPtr()->pCamera, 50);
    grass->addDetailLevel<Forests::GrassPage>(500);
    Forests::GrassLoader *pGrassLoader = new Forests::GrassLoader(grass);
    grass->setPageLoader(pGrassLoader);


    pGrassLoader->setHeightFunction(&getTerrainHeight);

    Forests::GrassLayer *layer = pGrassLoader->addLayer("grass"); //("3D-Diggers/plant1sprite"); //("grass");

    layer->setMinimumSize(0.5f, 0.5f);
    layer->setMaximumSize(1.5f, 1.5f);
    layer->setAnimationEnabled(true);
    layer->setSwayDistribution(10.0f);
    layer->setSwayLength(0.5f);
    layer->setSwaySpeed(0.2f);
    layer->setDensity(2.5f);
    layer->setFadeTechnique(Forests::FADETECH_GROW);
    layer->setRenderTechnique(Forests::GRASSTECH_SPRITE);
//layer->setColorMap("terrain_texture.jpg");
	layer->setMapBounds(Forests::TBounds(mTerrainPos.x-TERRAIN_WORLD_SIZE, mTerrainPos.z-TERRAIN_WORLD_SIZE, mTerrainPos.x+TERRAIN_WORLD_SIZE, mTerrainPos.z+TERRAIN_WORLD_SIZE));
    layer->setDensityMap("densitymap.png");
    layer->setColorMap("terrain_LightMap.jpg");

//*/

}//setForest

//-------------------------------------------------------------------------------------------------
bool StartDemo::runDemo()
{
    OgreInit::getSingletonPtr()->pLog->logMessage("Start main loop..");

    pInfoLabel = OgreInit::getSingletonPtr()->pTrayMgr->createLabel(OgreBites::TL_TOP, "TInfo", "", 350);

    double timeSinceLastFrame = 0;
    double startTime = 0;
    OgreInit::getSingletonPtr()->pRenderWnd->resetStatistics();


    pBtDebug = new BTDebug(OgreInit::getSingletonPtr()->pSceneMgr->getRootSceneNode(), OgreInit::getSingletonPtr()->m_world);
    OgreInit::getSingletonPtr()->m_world->setDebugDrawer(pBtDebug);
    pBtDebug->setDebugMode(0);

    while(!mDemoShutdown && !OgreInit::getSingletonPtr()->isShutdown())
    {
            if (pTerrainGroup->isDerivedDataUpdateInProgress())
                {
                    OgreInit::getSingletonPtr()->pTrayMgr->moveWidgetToTray(pInfoLabel, OgreBites::TL_TOP, 0);
                    pInfoLabel->show();
                    if (mTerrainsImported)
                    {
                        pInfoLabel->setCaption("Building terrain, please wait...");
                    }
                    else
                    {
                        pInfoLabel->setCaption("Updating textures, patience...");
                    }
                }
                else
                {
                    OgreInit::getSingletonPtr()->pTrayMgr->removeWidgetFromTray(pInfoLabel);
                    pInfoLabel->hide();
                    if (mTerrainsImported)
                    {
                        pTerrainGroup->saveAllTerrains(true);
                        mTerrainsImported = false;
                    }
                }
        if(OgreInit::getSingletonPtr()->pRenderWnd->isActive())
        {
            startTime=OgreInit::getSingletonPtr()->pTimer->getMillisecondsCPU();
            OgreInit::getSingletonPtr()->pKeyboard->capture();
            OgreInit::getSingletonPtr()->pMouse->capture();

 /*     // clamp to terrain
			Ogre::Vector3 camPos = OgreInit::getSingletonPtr()->pCamera->getPosition();
			Ogre::Ray ray;
			ray.setOrigin(Ogre::Vector3(camPos.x, mTerrainPos.y + 1000, camPos.z));
			ray.setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);

			Ogre::TerrainGroup::RayResult rayResult = pTerrainGroup->rayIntersects(ray);
			Ogre::Real distanceAboveTerrain = 15;
			Ogre::Real fallSpeed = 300;
			Ogre::Real newy = camPos.y;
			if (rayResult.hit)
			{
				if (camPos.y > rayResult.position.y + distanceAboveTerrain)
				{
					mFallVelocity += timeSinceLastFrame * 0.6;
					mFallVelocity = std::min(mFallVelocity, fallSpeed);
					newy = camPos.y - mFallVelocity * timeSinceLastFrame;

				}
				newy = std::max(rayResult.position.y + distanceAboveTerrain, newy);
				OgreInit::getSingletonPtr()->pCamera->setPosition(camPos.x, newy, camPos.z);

			}//*/

            grass->update();
            trees->update();
            pBtDebug->step();

            OgreInit::getSingletonPtr()->updateOgre(timeSinceLastFrame);
            OgreInit::getSingletonPtr()->pRoot->renderOneFrame();

            timeSinceLastFrame= OgreInit::getSingletonPtr()->pTimer->getMillisecondsCPU() - startTime;
        }

    }

    OgreInit::getSingletonPtr()->pLog->logMessage("Quit Main Loop.");
    OgreInit::getSingletonPtr()->pLog->logMessage("Shuting Down Ogre");
    return true;
}//runDemo



//----------------------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        StartDemo essai;

        try {

            essai.startTry();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif


bool StartDemo::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	OgreInit::getSingletonPtr()->keyPressed(keyEventRef);

	if(OgreInit::getSingletonPtr()->pKeyboard->isKeyDown(OIS::KC_F))
	{
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(OgreInit::getSingletonPtr()->pCamera->getPosition().x));
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(OgreInit::getSingletonPtr()->pCamera->getPosition().y));
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(OgreInit::getSingletonPtr()->pCamera->getPosition().z));
            OgreInit::getSingletonPtr()->pLog->logMessage("next");

            BCube *cube = new BCube("cube"+Ogre::StringConverter::toString(cubeCounter), OgreInit::getSingletonPtr()->m_world, OgreInit::getSingletonPtr()->pSceneMgr, btVector3(2,2,2), 300);
            cube->getRigidBody()->translate(btVector3(OgreInit::getSingletonPtr()->pCamera->getPosition().x, OgreInit::getSingletonPtr()->pCamera->getPosition().y, OgreInit::getSingletonPtr()->pCamera->getPosition().z));
            cube->getSceneNode()->setOrientation(OgreInit::getSingletonPtr()->pCamera->getOrientation());
            Ogre::Vector3 dir=OgreInit::getSingletonPtr()->pCamera->getDirection();
            cube->getRigidBody()->setLinearVelocity(btVector3(dir.x * 130,dir.y*130,dir.z*130));
            cube->getEntity()->setMaterialName("Ecs/Ground1");
            m_cubelist.push_back(cube);
            cubeCounter++;
	}

		if(OgreInit::getSingletonPtr()->pKeyboard->isKeyDown(OIS::KC_G))
	{
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(OgreInit::getSingletonPtr()->pCamera->getPosition().x));
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(OgreInit::getSingletonPtr()->pCamera->getPosition().y));
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(OgreInit::getSingletonPtr()->pCamera->getPosition().z));
            OgreInit::getSingletonPtr()->pLog->logMessage("next");
            OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(pTerrainGroup->getTerrainWorldSize()));

            BCHS *ogre_shape = new BCHS("bchs"+Ogre::StringConverter::toString(bchsCounter),"ogrehead", OgreInit::getSingletonPtr()->m_world, OgreInit::getSingletonPtr()->pSceneMgr,
                                        btVector3(12,12,12),btVector3(OgreInit::getSingletonPtr()->pCamera->getPosition().x, OgreInit::getSingletonPtr()->pCamera->getPosition().y, OgreInit::getSingletonPtr()->pCamera->getPosition().z), 500);
           // ogre_shape->getRigidBody()->translate(btVector3(OgreInit::getSingletonPtr()->pCamera->getPosition().x, OgreInit::getSingletonPtr()->pCamera->getPosition().y, OgreInit::getSingletonPtr()->pCamera->getPosition().z));

            ogre_shape->getSceneNode()->setOrientation(OgreInit::getSingletonPtr()->pCamera->getOrientation());
            Ogre::Vector3 dir=OgreInit::getSingletonPtr()->pCamera->getDirection();
            ogre_shape->getRigidBody()->setLinearVelocity(btVector3(dir.x * 130,dir.y*130,dir.z*130));
            ogre_shape->getEntity()->setMaterialName("Ecs/shader/toon2");
            OgreInit::getSingletonPtr()->pLog->logMessage("tentative de destruction7");

            m_bchslist.push_back(ogre_shape);
            OgreInit::getSingletonPtr()->pLog->logMessage("tentative de destruction8");

            bchsCounter++;
	}

	if(OgreInit::getSingletonPtr()->pKeyboard->isKeyDown(OIS::KC_R))
	{
	long int csx, csy;
    pTerrainGroup->convertWorldPositionToTerrainSlot( OgreInit::getSingletonPtr()->pCamera->getPosition(), &csx, &csy );
    OgreInit::getSingletonPtr()->pLog->logMessage("situation du terrain dans le monde?");
    OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(csx));
    OgreInit::getSingletonPtr()->pLog->logMessage(Ogre::StringConverter::toString(csy));
	}

	if(OgreInit::getSingletonPtr()->pKeyboard->isKeyDown(OIS::KC_E))
	{
	#ifdef PAGING
    long int slotX, slotY;
    pTerrainGroup->convertWorldPositionToTerrainSlot( OgreInit::getSingletonPtr()->pCamera->getPosition(), &slotX, &slotY );
    Ogre::Terrain* bt_t=pTerrainGroup->getTerrain(slotX,slotY);
    if(bt_t)
        {
            BTerrain *land = new BTerrain(bt_t,
                                          OgreInit::getSingletonPtr()->m_world,
                                          OgreInit::getSingletonPtr()->pSceneMgr);
           // land->getRigidBody()->translate(btVector3(0, 100, 0));
            m_terrainlist.push_back(land);
        }
    #endif
	}

    if(OgreInit::getSingletonPtr()->pKeyboard->isKeyDown(OIS::KC_B))
	{
            if(debugState)
                {
                    debugState=false;
                    pBtDebug->setDebugMode(0);
                }
            else
                {
                    debugState=true;
                    pBtDebug->setDebugMode(1);
                }
	}
	return true;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

bool StartDemo::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	OgreInit::getSingletonPtr()->keyReleased(keyEventRef);

	return true;
}

bool StartDemo::mouseMoved(const OIS::MouseEvent &evt)
{
    OgreInit::getSingletonPtr()->mouseMoved(evt);
    return true;
}

bool StartDemo::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{

    return true;
}
bool StartDemo::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{

    return true;
}

