
#include "DebugDrawer.h"


/*
* =============================================================================================
* BtOgre::DynamicRenderable
* =============================================================================================
*/

DynamicRenderable::DynamicRenderable()
{
}
//------------------------------------------------------------------------------------------------
DynamicRenderable::~DynamicRenderable()
{
delete mRenderOp.vertexData;
delete mRenderOp.indexData;
}
//------------------------------------------------------------------------------------------------
void DynamicRenderable::initialize(Ogre::RenderOperation::OperationType operationType,
bool useIndices)
{
// Initialize render operation
mRenderOp.operationType = operationType;
mRenderOp.useIndexes = useIndices;
mRenderOp.vertexData = new Ogre::VertexData;
if (mRenderOp.useIndexes)
mRenderOp.indexData = new Ogre::IndexData;

// Reset buffer capacities
mVertexBufferCapacity = 0;
mIndexBufferCapacity = 0;

// Create vertex declaration
createVertexDeclaration();
}
//------------------------------------------------------------------------------------------------
void DynamicRenderable::prepareHardwareBuffers(size_t vertexCount,
size_t indexCount)
{
// Prepare vertex buffer
size_t newVertCapacity = mVertexBufferCapacity;
if ((vertexCount > mVertexBufferCapacity) ||
(!mVertexBufferCapacity))
{
// vertexCount exceeds current capacity!
// It is necessary to reallocate the buffer.

// Check if this is the first call
if (!newVertCapacity)
newVertCapacity = 1;

// Make capacity the next power of two
while (newVertCapacity < vertexCount)
newVertCapacity <<= 1;
}
else if (vertexCount < mVertexBufferCapacity>>1) {
// Make capacity the previous power of two
while (vertexCount < newVertCapacity>>1)
newVertCapacity >>= 1;
}
if (newVertCapacity != mVertexBufferCapacity)
{
mVertexBufferCapacity = newVertCapacity;
// Create new vertex buffer
Ogre::HardwareVertexBufferSharedPtr vbuf =
Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
mRenderOp.vertexData->vertexDeclaration->getVertexSize(0),
mVertexBufferCapacity,
Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY); // TODO: Custom HBU_?

// Bind buffer
mRenderOp.vertexData->vertexBufferBinding->setBinding(0, vbuf);
}
// Update vertex count in the render operation
mRenderOp.vertexData->vertexCount = vertexCount;

if (mRenderOp.useIndexes)
{
OgreAssert(indexCount <= std::numeric_limits<unsigned short>::max(), "indexCount exceeds 16 bit");

size_t newIndexCapacity = mIndexBufferCapacity;
// Prepare index buffer
if ((indexCount > newIndexCapacity) ||
(!newIndexCapacity))
{
// indexCount exceeds current capacity!
// It is necessary to reallocate the buffer.

// Check if this is the first call
if (!newIndexCapacity)
newIndexCapacity = 1;

// Make capacity the next power of two
while (newIndexCapacity < indexCount)
newIndexCapacity <<= 1;

}
else if (indexCount < newIndexCapacity>>1)
{
// Make capacity the previous power of two
while (indexCount < newIndexCapacity>>1)
newIndexCapacity >>= 1;
}

if (newIndexCapacity != mIndexBufferCapacity)
{
mIndexBufferCapacity = newIndexCapacity;
// Create new index buffer
mRenderOp.indexData->indexBuffer =
Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
Ogre::HardwareIndexBuffer::IT_16BIT,
mIndexBufferCapacity,
Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY); // TODO: Custom HBU_?
}

// Update index count in the render operation
mRenderOp.indexData->indexCount = indexCount;
}
}
//------------------------------------------------------------------------------------------------
Ogre::Real DynamicRenderable::getBoundingRadius(void) const
{
return Ogre::Math::Sqrt(std::max(mBox.getMaximum().squaredLength(), mBox.getMinimum().squaredLength()));
}
//------------------------------------------------------------------------------------------------
Ogre::Real DynamicRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
{
Ogre::Vector3 vMin, vMax, vMid, vDist;
vMin = mBox.getMinimum();
vMax = mBox.getMaximum();
vMid = ((vMax - vMin) * 0.5) + vMin;
vDist = cam->getDerivedPosition() - vMid;

return vDist.squaredLength();
}

/*
* =============================================================================================
* BtOgre::DynamicLines
* =============================================================================================
*/

enum {
POSITION_BINDING,
TEXCOORD_BINDING
};
//------------------------------------------------------------------------------------------------
DynamicLines::DynamicLines(OperationType opType)
{
initialize(opType,false);
setMaterial("BaseWhiteNoLighting");
mDirty = true;
}
//------------------------------------------------------------------------------------------------
DynamicLines::~DynamicLines()
{
}
//------------------------------------------------------------------------------------------------
void DynamicLines::setOperationType(OperationType opType)
{
mRenderOp.operationType = opType;
}
//------------------------------------------------------------------------------------------------
Ogre::RenderOperation::OperationType DynamicLines::getOperationType() const
{
return mRenderOp.operationType;
}
//------------------------------------------------------------------------------------------------
void DynamicLines::addPoint(const Vector3 &p)
{
mPoints.push_back(p);
mDirty = true;
}
//------------------------------------------------------------------------------------------------
void DynamicLines::addPoint(Real x, Real y, Real z)
{
mPoints.push_back(Vector3(x,y,z));
mDirty = true;
}
//------------------------------------------------------------------------------------------------
const Ogre::Vector3& DynamicLines::getPoint(unsigned short index) const
{
assert(index < mPoints.size() && "Point index is out of bounds!!");
return mPoints[index];
}
//------------------------------------------------------------------------------------------------
unsigned short DynamicLines::getNumPoints(void) const
{
return (unsigned short)mPoints.size();
}
//------------------------------------------------------------------------------------------------
void DynamicLines::setPoint(unsigned short index, const Ogre::Vector3 &value)
{
assert(index < mPoints.size() && "Point index is out of bounds!!");

mPoints[index] = value;
mDirty = true;
}
//------------------------------------------------------------------------------------------------
void DynamicLines::clear()
{
mPoints.clear();
mDirty = true;
}
//------------------------------------------------------------------------------------------------
void DynamicLines::update()
{
if (mDirty) fillHardwareBuffers();
}
//------------------------------------------------------------------------------------------------
void DynamicLines::createVertexDeclaration()
{
Ogre::VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
decl->addElement(POSITION_BINDING, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
}
//------------------------------------------------------------------------------------------------
void DynamicLines::fillHardwareBuffers()
{
int size = mPoints.size();

prepareHardwareBuffers(size,0);

if (!size) {
mBox.setExtents(Vector3::ZERO,Vector3::ZERO);
mDirty=false;
return;
}

Vector3 vaabMin = mPoints[0];
Vector3 vaabMax = mPoints[0];

Ogre::HardwareVertexBufferSharedPtr vbuf =
mRenderOp.vertexData->vertexBufferBinding->getBuffer(0);

Ogre::Real *prPos = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
{
for(int i = 0; i < size; i++)
{
*prPos++ = mPoints[i].x;
*prPos++ = mPoints[i].y;
*prPos++ = mPoints[i].z;

if(mPoints[i].x < vaabMin.x)
vaabMin.x = mPoints[i].x;
if(mPoints[i].y < vaabMin.y)
vaabMin.y = mPoints[i].y;
if(mPoints[i].z < vaabMin.z)
vaabMin.z = mPoints[i].z;

if(mPoints[i].x > vaabMax.x)
vaabMax.x = mPoints[i].x;
if(mPoints[i].y > vaabMax.y)
vaabMax.y = mPoints[i].y;
if(mPoints[i].z > vaabMax.z)
vaabMax.z = mPoints[i].z;
}
}
vbuf->unlock();

mBox.setExtents(vaabMin, vaabMax);

mDirty = false;
}

