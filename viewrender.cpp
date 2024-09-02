#include "includes.h"

void Hooks::OnRenderStart( ) {
	// call og.
	g_hooks.m_view_render.GetOldMethod< OnRenderStart_t >( CViewRender::ONRENDERSTART )( this );

	if(g_menu.main.visuals.fov.get()) {
		float goal_fov = g_menu.main.visuals.fov_amt.get();

		if (g_cl.m_processing && g_cl.m_local->m_bIsScoped() && g_cl.m_weapon) {
			goal_fov *= (1.f - ((g_menu.main.visuals.scope_zoom.get() * g_cl.m_weapon->m_zoomLevel()) * 0.01));
		}

		g_visuals.m_fov_interpolated = g_menu.main.visuals.thirdperson_interpolate.get() ? math::Lerp(VIS_INTERP, g_visuals.m_fov_interpolated, goal_fov) : goal_fov;
		g_csgo.m_view_render->m_view.m_fov = g_visuals.m_fov_interpolated;
	}

	if (g_menu.main.visuals.viewmodel_fov.get()) {
		g_csgo.m_view_render->m_view.m_viewmodel_fov = g_menu.main.visuals.viewmodel_fov_amt.get();
	}
}

void Hooks::RenderView( const CViewSetup &view, const CViewSetup &hud_view, int clear_flags, int what_to_draw ) {
	// ...

	g_hooks.m_view_render.GetOldMethod< RenderView_t >( CViewRender::RENDERVIEW )( this, view, hud_view, clear_flags, what_to_draw );
}

void Hooks::Render2DEffectsPostHUD( const CViewSetup &setup ) {
	if( !g_menu.main.visuals.vis_removals.get(3) )
		g_hooks.m_view_render.GetOldMethod< Render2DEffectsPostHUD_t >( CViewRender::RENDER2DEFFECTSPOSTHUD )( this, setup );
}

void Hooks::RenderSmokeOverlay( bool unk ) {
	// do not render smoke overlay.
	if( !g_menu.main.visuals.vis_removals.get(1) )
		g_hooks.m_view_render.GetOldMethod< RenderSmokeOverlay_t >( CViewRender::RENDERSMOKEOVERLAY )( this, unk );
}
