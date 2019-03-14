#include "SSLevel.h"
#include <Core/GlobalSystems.h>
#include <Core/components/TransformComponent.h>
#include <Core/components/ModelComponent.h>
#include <Core/components/RigidBodyComponent.h>
#include <par_shapes.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb-master/stb_perlin.h>
#include <algorithm>
#include <dualmc/dualmc.h>

#include <Imgui/imgui.h>
using namespace smug;

SSLevel::SSLevel(){

}

SSLevel::~SSLevel(){
	int i = 0;
}

void SSLevel::Startup() {
	//Gen level
	m_ProcVars.scale = glm::vec3(0.1f);
	m_Voxels = nullptr;
	GenerateWorld(m_ProcVars);
	if (m_Indices.size() > 0) {
		m_ModelHandle = g_AssetLoader.LoadGeneratedModel(m_WorldModel, "Voxels");
		m_VoxelModelHandle = g_AssetLoader.LoadGeneratedModel(m_VoxelModel, "VoxelCubes");

		Entity& e = globals::g_EntityManager->CreateEntity();

		TransformComponent tc;
		tc.Position = glm::vec3(0, 1, 0);
		tc.Scale = glm::vec3(1.0f, 1.0f, 1.0f);
		globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);

		ModelComponent mc;
		mc.ModelHandle = m_ModelHandle;
		mc.Static = false;
		mc.Visible = true;
		mc.Tint = glm::vec4(0.4f, 0.5f, 0.4f, 1.0f);
		globals::g_Components->CreateComponent(&mc, e, ModelComponent::Flag);

		RigidBodyComponent rc;
		PhysicsMesh worldPhysicsMesh;
		for (uint32_t i = 0; i < m_WorldModel.MeshCount; ++i) {
			MeshInfo& m = m_WorldModel.Meshes[i];
			for (uint32_t k = 0; k < m.VertexCount; ++k) {
				worldPhysicsMesh.Vertices.push_back(m.Vertices[k].Position);
			}
			for (uint32_t k = 0; k < m.IndexCount; ++k) {
				worldPhysicsMesh.Indices.push_back(m.Indices[k]);
			}
		}
		rc.Body = globals::g_Physics->CreateStaticActorFromTriMesh(tc.Position, tc.Orientation, tc.Scale, worldPhysicsMesh);
		globals::g_Components->CreateComponent(&rc, e, RigidBodyComponent::Flag);


		m_WorldEntityUID = e.UID;
		//voxel world entity
		Entity& voxelEntity = globals::g_EntityManager->CreateEntity();
		globals::g_Components->CreateComponent(&tc, voxelEntity, TransformComponent::Flag);

		mc.ModelHandle = m_VoxelModelHandle;
		mc.Tint = glm::vec4(0.3, 0.2, 0.2, 1.0f);
		globals::g_Components->CreateComponent(&mc, voxelEntity, ModelComponent::Flag);

		/*PhysicsMesh voxelPhysicsMesh;
		for (uint32_t i = 0; i < m_VoxelModel.MeshCount; ++i) {
			MeshInfo& m = m_VoxelModel.Meshes[i];
			for (uint32_t k = 0; k < m.VertexCount; ++k) {
				voxelPhysicsMesh.Vertices.push_back(m.Vertices[k].Position);
			}
			for (uint32_t k = 0; k < m.IndexCount; ++k) {
				voxelPhysicsMesh.Indices.push_back(m.Indices[k]);
			}
		}*/
		//rc.Body = globals::g_Physics->CreateStaticActorFromTriMesh(tc.Position, tc.Orientation, tc.Scale, voxelPhysicsMesh);
		//globals::g_Components->CreateComponent(&rc, voxelEntity, RigidBodyComponent::Flag);
		m_VoxelEntityUID = voxelEntity.UID;
	}
}

void SSLevel::Update(const double deltaTime) {
	ImGui::Begin("ProcGen");
	ImGui::InputInt("Width", &m_ProcVars.width);
	ImGui::InputInt("Height", &m_ProcVars.height);
	ImGui::InputInt("Depth", &m_ProcVars.depth);
	ImGui::DragFloat("Lacunarity", &m_ProcVars.lacunarity, 0.001f, 0.0f, 100.0f);
	ImGui::DragFloat("Gain", &m_ProcVars.gain, 0.001f, 0.0f, 100.0f);
	ImGui::DragFloat3("Offset", &m_ProcVars.offset[0], 0.001f, 0.0f, 100.0f);
	ImGui::DragFloat3("Scale", &m_ProcVars.scale[0], 0.001f, 0.0f, 100.0f);
	ImGui::InputInt("Octaves", &m_ProcVars.octaves);
	int threshold = m_ProcVars.threshold;
	ImGui::InputInt("Threshold", &threshold);
	m_ProcVars.threshold = threshold;

	if (ImGui::Button("Rebuild voxel map")) {
		GenerateWorld(m_ProcVars);
		if (m_Indices.size() > 0) {
			g_AssetLoader.UpdateModel(m_ModelHandle, m_WorldModel);
			Entity& worldEntity = globals::g_EntityManager->GetEntity(m_WorldEntityUID);
			RigidBodyComponent* rc = (RigidBodyComponent*)globals::g_Components->GetComponent(worldEntity, RigidBodyComponent::Flag);
			globals::g_Physics->DeleteActor(rc->Body->Actor);
			PhysicsMesh worldPhysicsMesh;
			for (uint32_t i = 0; i < m_WorldModel.MeshCount; ++i) {
				MeshInfo& m = m_WorldModel.Meshes[i];
				for (uint32_t k = 0; k < m.VertexCount; ++k) {
					worldPhysicsMesh.Vertices.push_back(m.Vertices[k].Position);
				}
				for (uint32_t k = 0; k < m.IndexCount; ++k) {
					worldPhysicsMesh.Indices.push_back(m.Indices[k]);
				}
			}
			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(worldEntity, TransformComponent::Flag);
			rc->Body = globals::g_Physics->CreateStaticActorFromTriMesh(tc->Position, tc->Orientation, tc->Scale, worldPhysicsMesh);

			g_AssetLoader.UpdateModel(m_VoxelModelHandle, m_VoxelModel);
		}
	}
	if (ImGui::Button("Toggle Voxel render")) {
		Entity& m_VoxelEnity = globals::g_EntityManager->GetEntity(m_VoxelEntityUID);
		ModelComponent* mc = (ModelComponent*)globals::g_Components->GetComponent(m_VoxelEnity, ModelComponent::Flag);
		mc->Visible = !mc->Visible;

	}

	Entity& worldEntity = globals::g_EntityManager->GetEntity(m_WorldEntityUID);
	TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(worldEntity, TransformComponent::Flag);
	ImGui::DragFloat("WorldScale", &tc->Scale[0], 0.1f);
	tc->Scale[1] = tc->Scale[2] = tc->Scale[0];
	ImGui::DragFloat3("WorldOffset", &tc->Position[0], 0.1f);
	Entity& m_VoxelEnity = globals::g_EntityManager->GetEntity(m_VoxelEntityUID);
	TransformComponent* tc2 = (TransformComponent*)globals::g_Components->GetComponent(m_VoxelEnity, TransformComponent::Flag);
	tc2->Position = tc->Position;
	tc2->Scale = tc->Scale;
	ImGui::End();
}

void SSLevel::Shutdown() {
}

void SSLevel::GenerateWorld(ProcVars vars) {
	m_Vertices.clear();
	m_Indices.clear();
	m_VoxelVertices.clear();
	m_VoxelIndices.clear();

	const int size2 = vars.width * vars.height;
	m_Voxels = (uint8_t*)malloc(vars.width * vars.height * vars.depth * sizeof(uint8_t));
	memset(m_Voxels, 0x0, vars.width * vars.height * vars.depth * sizeof(uint8_t));
	for (int z = 0; z < vars.depth; ++z) {
		for (int y = 0; y < vars.height; ++y) {
			for (int x = 0; x < vars.width; ++x) {
				glm::vec3 pos = (glm::vec3(x, y, z) + vars.offset) * vars.scale;
				float p = stb_perlin_turbulence_noise3(pos.x, pos.y, pos.z, vars.lacunarity, vars.gain, vars.octaves, vars.width, vars.height, vars.depth);
				//p = p + stb_perlin_fbm_noise3(x / (float)width, 1.0f, z / (float)depth, 2.0f, 0.5f, 6, width, height, depth);
				m_Voxels[z * size2 + y * vars.height + x] = (p * 255U);
			}
		}
	}
	dualmc::DualMC<uint8_t> dmc;
	std::vector<dualmc::Vertex> mcVertices;
	std::vector<dualmc::Quad> mcQuads;
	dmc.build(m_Voxels, vars.width, vars.height, vars.depth, vars.threshold, false, false, mcVertices, mcQuads);

	for (auto& mv : mcVertices) {
		Vertex v;
		v.Position = glm::vec3(mv.x, mv.y, mv.z);
		v.Normal = glm::vec3(0, 1, 0);
		v.Tangent = glm::vec3(1, 0, 0);
		v.TexCoord = glm::vec2(0, 0);
		m_Vertices.push_back(v);
	}

	for (auto& q : mcQuads) {
		Vertex v0 = m_Vertices[q.i0];
		Vertex v1 = m_Vertices[q.i2];
		Vertex v2 = m_Vertices[q.i1];
		glm::vec3 normal = glm::normalize(glm::cross(v1.Position - v0.Position, v2.Position - v0.Position));
		if (!glm::any(glm::isinf(normal)) && !glm::any(glm::isnan(normal))) {
			m_Vertices[q.i0].Normal += normal;
			m_Vertices[q.i2].Normal += normal;
			m_Vertices[q.i1].Normal += normal;
		}
		v0 = m_Vertices[q.i0];
		v1 = m_Vertices[q.i3];
		v2 = m_Vertices[q.i2];
		normal = glm::normalize(glm::cross(v1.Position - v0.Position, v2.Position - v0.Position));
		if (!glm::any(glm::isinf(normal)) && !glm::any(glm::isnan(normal))) {
			m_Vertices[q.i0].Normal += normal;
			m_Vertices[q.i3].Normal += normal;
			m_Vertices[q.i2].Normal += normal;
		}
		m_Indices.push_back(q.i0);
		m_Indices.push_back(q.i1);
		m_Indices.push_back(q.i2);

		m_Indices.push_back(q.i0);
		m_Indices.push_back(q.i2);
		m_Indices.push_back(q.i3);
	}

	for (auto& v : m_Vertices) {
		glm::vec3& normal = v.Normal = -glm::normalize(v.Normal);
		glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
		glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
		glm::vec3 tangent;
		if (glm::length(c1) > glm::length(c2)) {
			tangent = c1;
		}
		else {
			tangent = c2;
		}
		v.Tangent = glm::normalize(tangent);
	}

	m_Mesh.IndexCount = (uint32_t)m_Indices.size();
	m_Mesh.Indices = (uint32_t*)m_Indices.data();
	m_Mesh.Material = 0;
	m_Mesh.VertexCount = (uint32_t)m_Vertices.size();
	m_Mesh.Vertices = (Vertex*)m_Vertices.data();

	m_WorldModel.MaterialCount = 0;
	m_WorldModel.MeshCount = 1;
	m_WorldModel.Materials = nullptr;
	m_WorldModel.Meshes = &m_Mesh;


	par_shapes_mesh* cubeMesh;
	cubeMesh = par_shapes_create_cube();
	//par_shapes_unweld(cubeMesh, true);
	par_shapes_translate(cubeMesh, -0.5f, -0.5f, -0.5f);
	par_shapes_unweld(cubeMesh, true);
	par_shapes_compute_normals(cubeMesh);
	std::vector<Vertex> verts;
	for (int i = 0; i < cubeMesh->npoints; i++) {
		Vertex v;
		v.Position = glm::vec3(cubeMesh->points[i * 3 + 0], cubeMesh->points[i * 3 + 1], cubeMesh->points[i * 3 + 2]);
		glm::vec3 normal = glm::vec3(cubeMesh->normals[i * 3], cubeMesh->normals[i * 3 + 1], cubeMesh->normals[i * 3 + 2]);
		glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
		glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
		glm::vec3 tangent;
		if (glm::length(c1) > glm::length(c2)) {
			tangent = c1;
		}
		else {
			tangent = c2;
		}
		v.Normal = glm::vec3(glm::normalize(normal));
		v.Tangent = glm::vec3(glm::normalize(tangent));
		glm::vec2 uv;
		uv.x = glm::dot(glm::vec3(v.Normal), glm::vec3(1, 0, 0)) * 0.5f + 0.5f;
		uv.y = glm::dot(glm::vec3(v.Normal), glm::vec3(0, 1, 0)) * 0.5f + 0.5f;
		v.TexCoord = glm::vec2(uv);
		verts.push_back(v);
	}
	std::vector<uint32_t> indices;
	for (int i = 0; i < cubeMesh->ntriangles; i++) {
		indices.push_back(cubeMesh->triangles[i * 3 + 0]);
		indices.push_back(cubeMesh->triangles[i * 3 + 1]);
		indices.push_back(cubeMesh->triangles[i * 3 + 2]);
	}
	par_shapes_free_mesh(cubeMesh);
	for (int z = 0; z < vars.depth; ++z) {
		for (int y = 0; y < vars.height; ++y) {
			for (int x = 0; x < vars.width; ++x) {
				if (m_Voxels[z * size2 + y * vars.height + x] > vars.threshold) {
					uint32_t indexOffset = (uint32_t)m_VoxelIndices.size();
					for (auto& v : verts) {
						Vertex vert = v;
						vert.Position = v.Position + glm::vec3(x, y, z);
						m_VoxelVertices.push_back(vert);
					}
					for (auto& i : indices) {
						m_VoxelIndices.push_back(i + indexOffset);
					}
				}
			}
		}
	}

	m_VoxelMesh.IndexCount = (uint32_t)m_VoxelIndices.size();
	m_VoxelMesh.Indices = (uint32_t*)m_VoxelIndices.data();
	m_VoxelMesh.Material = 0;
	m_VoxelMesh.VertexCount = (uint32_t)m_VoxelVertices.size();
	m_VoxelMesh.Vertices = (Vertex*)m_VoxelVertices.data();

	m_VoxelModel.MaterialCount = 0;
	m_VoxelModel.MeshCount = 1;
	m_VoxelModel.Materials = nullptr;
	m_VoxelModel.Meshes = &m_VoxelMesh;
	free(m_Voxels);
	m_Voxels = nullptr;
}
