/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Mesh.h"

#include <3D/L3DMesh.h>
#include <3D/L3DSubMesh.h>
#include <3D/MeshPack.h>
#include <Game.h>

using namespace openblack;
using namespace openblack::entities::components;

[[nodiscard]] const AxisAlignedBoundingBox& Mesh::GetBoundingBox() const
{
	const L3DMesh& l3dMesh = Game::instance()->GetMeshPack().GetMesh(id);
	const auto numSubMeshes = l3dMesh.GetNumSubMeshes();
	auto bbSubmeshIdPositive = (numSubMeshes + bbSubmeshId) % numSubMeshes;
	return l3dMesh.GetSubMeshes()[bbSubmeshIdPositive]->GetBoundingBox();
}
