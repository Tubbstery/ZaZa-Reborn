#pragma once

struct matrices_t {
	int                         ent_index;
	ModelRenderInfo_t           info;
	DrawModelState_t            state;
	matrix3x4_t                 pBoneToWorld[128] = { };
	float                       time;
	matrix3x4_t                 model_to_world;
};

class Chams {
public:
	enum model_type_t : uint32_t {
		invalid = 0,
		player,
		weapon,
		arms,
		view_weapon
	};

public:
	void SetupMaterial(IMaterial* mat, float alpha, Color col, bool z_flag, bool wireframe);

	void init();

	bool GetHistoryMatrix(Player* player, matrix3x4_t* out, float& alphamod);
	int  FailsSanity(Player* player, const ModelRenderInfo_t& info);
	bool HandleChams(Player* entity, const ModelRenderInfo_t& info, std::function<void()> oFn);
	bool HandleChamsOther(const ModelRenderInfo_t& info, std::function<void()> oFn);
	void OnDME(void* ECX, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone);
	void AddHitMatrix(Player* player, matrix3x4_t* bones);
	void AddSkelMatrix(Player* player, matrix3x4_t* bones);
	void OnPSE();

public:
	bool init_materials;
	std::vector< IMaterial* > m_materials;

	bool m_in_pse;

	matrix3x4_t* m_override_matrix;

	std::vector< matrices_t > m_hit_matrices;
};

extern Chams g_chams;