#include "includes.h"

void Hooks::DrawModelExecute( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {
	// fuck a nigga named shadow.
	if (strstr(info.m_model->m_name, XOR("shadow")) != nullptr) {
		return;
	}

	g_chams.OnDME(this, ctx, state, info, bone);
}