#pragma once

class activity_modifiers_wrapper {
private:
	void add_activity_modifier(const char* name) {
		static auto add_modifier = pattern::find(g_csgo.m_server_dll, "55 8B EC 8B 55 08 83 EC 24 56 8B F1 85 D2 0F 84 ? ? ? ? 8D 45 DC").as<void(__thiscall*)(void*, const char*)>();
		add_modifier(this, name);
	}

	PAD(0x148);
	CUtlVector<uint16_t> modifiers{};

public:
	activity_modifiers_wrapper() = default;

	explicit activity_modifiers_wrapper(CUtlVector<uint16_t> current_modifiers)
	{
		modifiers.RemoveAll();
		modifiers.GrowVector(current_modifiers.Count());

		for (auto i = 0; i < current_modifiers.Count(); i++)
			modifiers[i] = current_modifiers[i];
	}

	void add_modifier(const char* name)
	{
		add_activity_modifier(name);
	}

	CUtlVector<uint16_t> get() const
	{
		return modifiers;
	}
};

class local_animations {
public:
	// local_animations.cpp stuff.
	void GhettoUpdateClientSideAnimationsEx();
	void UpdateInformation();
	void HandleAnimationEvents(Player* pLocal, CCSGOPlayerAnimState& pred_state, C_AnimationLayer* layers, CUserCmd* cmd);

public:
	CUtlVector<uint16_t> build_activity_modifiers(Player* player);
	std::pair<CCSGOPlayerAnimState, C_AnimationLayer*> predict_animation_state(Player* player);
	void try_initiate_animation(Player* player, size_t layer, int32_t activity, CUtlVector<uint16_t> modifiers);

public:
	void IncrementLayerCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, bool bAllowLoop, const float delta);
	void IncrementLayerWeight(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer);
	void IncrementLayerCycleWeightRateGeneric(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, const float delta);
	float GetLayerIdealWeightFromSeqCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer);
	//void SetLayerSequence(Player* pEntity, C_AnimationLayer* pLayer, int32_t activity, CUtlVector<uint16_t> modifiers, int nOverrideSequence = -1);

public:
	int vm_thing;

};

extern local_animations g_localanimations;