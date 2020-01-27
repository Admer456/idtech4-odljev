#pragma once

#include <ostream>

//include Maya 2011 headers
#define _BOOL
#include "maya/MLibrary.h"
#include "maya/MFnDagNode.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatPointArray.h"
#include "maya/MMatrix.h"
#include "maya/MFileIO.h"
#include "maya/MTime.h"
#include "maya/MFnSkinCluster.h"
#include "maya/MGlobal.h"
#include "maya/MPlug.h"
#include "maya/MFnAttribute.h"
#include "maya/MFnMatrixData.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MFnMesh.h"
#include "maya/MFnCamera.h"
#include "maya/MFnEnumAttribute.h"
#include "maya/MFnSet.h"
#include "maya/MItDag.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MItGeometry.h"
#include "maya/MAnimControl.h"
#include "maya/MItDependencyGraph.h"
#include "maya/MItDependencyNodes.h"
#undef _BOOL
