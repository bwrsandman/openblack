#include "Registry.h"

#include <3D/Camera.h>
#include <3D/L3DMesh.h>
#include <3D/MeshPack.h>
#include <Entities/Components/AnimatedStatic.h>
#include <Entities/Components/Model.h>
#include <Entities/Components/Transform.h>
#include <Entities/Components/Villager.h>
#include <Game.h>
#include <Graphics/ShaderManager.h>

namespace openblack::Entities
{
void Registry::DebugCreateEntities(float x, float y, float z)
{
}

void Registry::DrawModels(uint8_t viewId, graphics::ShaderManager &shaderManager)
{
	graphics::ShaderProgram* objectShader = shaderManager.GetShader("Object");

	uint64_t state = 0u
		| BGFX_STATE_WRITE_MASK
		| BGFX_STATE_DEPTH_TEST_LESS
			// | BGFX_STATE_CULL_CCW  // TODO(bwrsandman): Some meshes wind one way and some others (i.e. rocks, gate)
		| BGFX_STATE_MSAA
	;

	_registry.view<Model, Transform>().each([viewId, state, objectShader](Model& model, Transform& transform) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix           = glm::translate(modelMatrix, transform.position);
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix           = glm::scale(modelMatrix, transform.scale);

//		objectShader->SetUniformValue("u_model", modelMatrix);
		bgfx::setTransform(&modelMatrix);

		const L3DMesh& mesh = Game::instance()->GetMeshPack().GetMesh(static_cast<uint32_t>(model.meshId));

		for (auto& submeshId : model.submeshIds)
			mesh.Draw(viewId, *objectShader, submeshId, state);
	});

	_registry.view<AnimatedStatic, Transform>().each([viewId, state, objectShader](AnimatedStatic& animated, Transform& transform) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix           = glm::translate(modelMatrix, transform.position);
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix           = glm::scale(modelMatrix, transform.scale);

//		objectShader->SetUniformValue("u_model", modelMatrix);
		bgfx::setTransform(&modelMatrix);

		// temporary-ish:
		MeshId meshID = MeshId::Dummy;
		if (animated.type == "Norse Gate")
		{
			const L3DMesh& mesh = Game::instance()->GetMeshPack().GetMesh((int)MeshId::BuildingNorseGate);
			mesh.Draw(viewId, *objectShader, 0, state);
		}
		else if (animated.type == "Gate Stone Plinth")
		{
			const L3DMesh& mesh = Game::instance()->GetMeshPack().GetMesh((int)MeshId::ObjectGateTotemPlinthe);
			mesh.Draw(viewId, *objectShader, 0, state);
		}
		else if (animated.type == "Piper Cave Entrance")
		{
			const L3DMesh& mesh = Game::instance()->GetMeshPack().GetMesh((int)MeshId::BuildingMineEntrance);
			mesh.Draw(viewId, *objectShader, 0, state);
		}
		else
		{
			const L3DMesh& mesh = Game::instance()->GetMeshPack().GetMesh(0);
			mesh.Draw(viewId, *objectShader, 0, state);
		}
	});

	_registry.view<MobileStatic, Transform>().each([viewId, state, objectShader](MobileStatic& mobile, Transform& transform) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix           = glm::translate(modelMatrix, transform.position);
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix           = glm::rotate(modelMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix           = glm::scale(modelMatrix, transform.scale);

//		objectShader->SetUniformValue("u_model", modelMatrix);
		bgfx::setTransform(&modelMatrix);

		// temporary-ish until we read info.dat:
		MeshId meshID = MeshId::Dummy;
		if (mobile.type == 0)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::BuildingCelticFenceShort).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 1)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::BuildingCelticFenceTall).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 2)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Z_SpellRock01).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 3)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Dummy).Draw(viewId, *objectShader, 0, state);
		else if (mobile.type == 4)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::SpellSolidShield).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 5)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::SingingStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 6)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::SingingStoneCenter).Draw(viewId, *objectShader, 0, state);
		else if (mobile.type == 7)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::SingingStoneCenter).Draw(viewId, *objectShader, 0, state);
		else if (mobile.type == 8)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::BuildingCampfire).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 9)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Dummy).Draw(viewId, *objectShader, 0, state);
		else if (mobile.type == 10)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::FeatureIdol).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 11)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::FeatureIdol).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 12)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Dummy).Draw(viewId, *objectShader, 0, state);
		else if (mobile.type == 13)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Dummy).Draw(viewId, *objectShader, 0, state);
		else if (mobile.type == 14)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::BuildingAztecOlmecHead).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 15)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::BuildingAztecOlmecHead).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 16)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder1Chalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 17)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder1Lime).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 18)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder1Sand).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 19)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder1Volcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 20)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder2Chalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 21)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder2Lime).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 22)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder2Sand).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 23)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder2Volcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 24)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder3Chalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 25)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder3Lime).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 26)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder3Sand).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 27)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::Boulder3Volcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 28)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectFlatRockChalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 29)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectFlatRockLimeStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 30)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectFlatRockSandStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 31)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectFlatRockVolcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 32)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectLongRockChalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 33)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectLongRockLimeStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 34)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectLongRockSandStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 35)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectLongRockVolcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 36)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectRockChalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 37)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectRockLimeStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 38)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectRockSandStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 39)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectRockVolcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 40)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSharpRockChalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 41)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSharpRockLimeStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 42)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSharpRockSandStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 43)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSharpRockVolcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 44)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSquareRockChalk).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 45)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSquareRockLimeStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 46)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSquareRockSandStone).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 47)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectSquareRockVolcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 48)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectFlatRockVolcanic).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 49)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectGateTotemApe).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 50)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectGateTotemBlank).Draw(viewId, *objectShader, 0, state); // 1 is the phys mesh.... ???
		else if (mobile.type == 51)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectGateTotemCow).Draw(viewId, *objectShader, 1, state);
		else if (mobile.type == 52)
			Game::instance()->GetMeshPack().GetMesh((int) MeshId::ObjectGateTotemTiger).Draw(viewId, *objectShader, 1, state);
		else
			Game::instance()->GetMeshPack().GetMesh(0).Draw(viewId, *objectShader, 0, state);
	});

	//
}

void Registry::Update()
{
}
} // namespace openblack::Entities
