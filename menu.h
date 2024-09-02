#pragma once

class AimbotTab : public Tab {
public:
	// col1.
	Checkbox	  enable;
	Checkbox      enable_goofy_whitelist;
	Checkbox	  silent;
	Checkbox      autofire;
	Checkbox	  autowall;
	Dropdown	  target_sorting;
	MultiDropdown aimbot_optimizations;
	MultiDropdown aimbot_debug;
	MultiDropdown hitbox;
	Slider        hitbox_preference;
	MultiDropdown multipoint;
	Dropdown      multipoint_intensity;
	Slider		  body_pointscale;
	Slider		  head_pointscale;
	Slider        head_aimheight;
	Slider		  minimum_damage;
	Checkbox      mimimum_damage_half_pistol;

	// col2.
	Slider	      hitchance_amount_zeus;
	Slider	      hitchance_amount;
	MultiDropdown autostop_modes;
	MultiDropdown delay_shot_modes;
	//Dropdown      fake_lag_correction;
	Checkbox      force_baim_after_misses;
	Checkbox      force_baim_after_misses_disable_resolved;
	Slider        force_baim_miss_count;
	MultiDropdown safety_conditions;
	MultiDropdown damage_conditions;
	Keybind       baim_key;
	Keybind       override_resolver;
	Checkbox      override_resolver_moving_feetyaw;
	Keybind       override_damage;
	Dropdown      override_damage_mode;
	Slider        override_damage_amount;

public:
	void init() {
		// title.
		SetTitle(XOR("aimbot"));

		enable.setup(XOR("enable"), XOR("enable"));
		RegisterElement(&enable);

		enable_goofy_whitelist.setup(XOR("whitelist goofyhook users"), XOR("enable_goofy_whitelist"));
		enable_goofy_whitelist.SetCallback(callbacks::UpdateWhitelist);
		RegisterElement(&enable_goofy_whitelist);

		silent.setup(XOR("silent aimbot"), XOR("silent"));
		RegisterElement(&silent);

		autofire.setup(XOR("autofire"), XOR("autofire"));
		RegisterElement(&autofire);

		autowall.setup(XOR("autowall"), XOR("autowall"));
		RegisterElement(&autowall);

		target_sorting.setup(XOR("priority target"), XOR("target_sorting"), { XOR("distance"), XOR("fov"), XOR("lag"), XOR("health"), XOR("cycle") });
		RegisterElement(&target_sorting);

		aimbot_optimizations.setup(XOR("optimizations"), XOR("aimbot_optimizations"), { XOR("limit targets per tick"), XOR("smart multipoint") });
		RegisterElement(&aimbot_optimizations);

		aimbot_debug.setup(XOR("debug features"), XOR("aimbot_debug"), { XOR("draw shot matrices"), XOR("draw multipoint"), XOR("draw debug overlay") });
		RegisterElement(&aimbot_debug);

		hitbox.setup(XOR("hitbox"), XOR("hitbox"), { XOR("head"), XOR("chest"), XOR("body"), XOR("arms"), XOR("legs"), XOR("feet") });
		RegisterElement(&hitbox);

		hitbox_preference.setup(XOR("prefer safety over damage"), XOR("hitbox_preference"), -100.f, 100.f, true, 0, 0.f, 1.f, L"%");
		RegisterElement(&hitbox_preference);

		multipoint.setup(XOR("multi-point"), XOR("multipoint"), { XOR("head"), XOR("chest"), XOR("body"), XOR("legs") });
		RegisterElement(&multipoint);

		multipoint_intensity.setup(XOR("multipoint intensity"), XOR("multipoint_intensity"), { XOR("low"), XOR("medium"), XOR("high") }, false);
		RegisterElement(&multipoint_intensity);

		body_pointscale.setup(XOR("body pointscale"), XOR("body_hitbox_scale"), 1.f, 100.f, false, 0, 50.f, 1.f, XOR(L"%"));
		body_pointscale.AddShowCallback(callbacks::IsMultipointOn);
		RegisterElement(&body_pointscale);

		head_pointscale.setup("head pointscale", XOR("hitbox_scale"), 1.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		head_pointscale.AddShowCallback(callbacks::IsMultipointOn);
		RegisterElement(&head_pointscale);

		head_aimheight.setup("maximum aim-height", XOR("head_aimheight"), 1.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		head_aimheight.AddShowCallback(callbacks::IsMultipointOn);
		RegisterElement(&head_aimheight);

		minimum_damage.setup(XOR("minimum damage"), XOR("minimum_damage"), 1.f, 130.f, true, 0, 40.f, 1.f);
		RegisterElement(&minimum_damage);

		mimimum_damage_half_pistol.setup(XOR("half damage pistol"), XOR("minimum_damage_half_pistol"));
		RegisterElement(&mimimum_damage_half_pistol);

		// col2.
		hitchance_amount_zeus.setup(XOR("minimum hitchance taser"), XOR("hitchance_amount_zeus"), 1.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		hitchance_amount_zeus.AddShowCallback(callbacks::IsConfigMM);
		RegisterElement(&hitchance_amount_zeus, 1);

		hitchance_amount.setup(XOR("minimum hitchance"), XOR("hitchance_amount"), 1.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		hitchance_amount.AddShowCallback(callbacks::IsConfigMM);
		RegisterElement(&hitchance_amount, 1);

		autostop_modes.setup(XOR("autostop"), XOR("autostop_modes"), { XOR("slow"), XOR("early") });
		RegisterElement(&autostop_modes, 1);

		delay_shot_modes.setup(XOR("delay shot"), XOR("delay_shot_modes"), { XOR("damage"), XOR("resolver") });
		RegisterElement(&delay_shot_modes, 1);

		//fake_lag_correction.setup( XOR( "fake-lag correction" ), XOR( "fake_lag_correction" ), { XOR( "wait" ), XOR( "regular" ) } );
		//RegisterElement( &fake_lag_correction, 1 );

		force_baim_after_misses.setup(XOR("force baim after x misses"), XOR("force_baim_after_misses"));
		RegisterElement(&force_baim_after_misses, 1);

		force_baim_after_misses_disable_resolved.setup(XOR("disable on lby updates"), XOR("force_baim_after_misses_disable_resolved"));
		force_baim_after_misses_disable_resolved.AddShowCallback(callbacks::IsForceBaimAfterMisses);
		RegisterElement(&force_baim_after_misses_disable_resolved, 1);

		force_baim_miss_count.setup(XOR(""), XOR("force_baim_miss_count"), 0, 6, false, 0, 2, 1, XOR(L""));
		force_baim_miss_count.AddShowCallback(callbacks::IsForceBaimAfterMisses);
		RegisterElement(&force_baim_miss_count, 1);

		safety_conditions.setup(XOR("safety conditions"), XOR("safety_conditions"), { XOR("unlikely resolve"), XOR("in air"), XOR("no lag and moving (desync)") });
		RegisterElement(&safety_conditions, 1);

		damage_conditions.setup(XOR("damage conditions"), XOR("damage_conditions"), { XOR("standing lby update"), XOR("moving lby update"), XOR("standing and no lag") });
		RegisterElement(&damage_conditions, 1);

		baim_key.setup(XOR("force baim"), XOR("force_baim"));
		RegisterElement(&baim_key, 1);

		override_resolver.setup(XOR("override resolver"), XOR("override"));
		RegisterElement(&override_resolver, 1);

		override_resolver_moving_feetyaw.setup(XOR("override moving feet yaw"), XOR("override_resolver_moving_feetyaw"));
		RegisterElement(&override_resolver_moving_feetyaw, 1);

		override_damage.setup(XOR("override minimum damage"), XOR("override_damage"));
		override_damage.SetToggleCallback(callbacks::ToggleDamageOverride);
		RegisterElement(&override_damage, 1);

		override_damage_mode.setup(XOR("override minimum damage keybind mode"), XOR("override_damage_mode"), { XOR("toggle"), XOR("hold"), }, false);
		RegisterElement(&override_damage_mode, 1);

		override_damage_amount.setup(XOR("override minimum damage"), XOR("override_damage_amount"), 1.f, 130.f, true, 0, 40.f, 1.f);
		RegisterElement(&override_damage_amount, 1);
	}
};

class AntiAimTab : public Tab {
public:
	// col 1.
	Checkbox enable;
	Dropdown pitch;
	Dropdown base_yaw;
	Slider   base_yaw_offset;
	Dropdown base_yaw_modifiers;
	Slider   modifier_range;
	Slider   modifier_speed;
	Checkbox freestand_yaw;
	Checkbox moving_feetyaw_desync;
	Checkbox feetyaw_desync_extend;
	Checkbox feetyaw_desync_disable_on_peek;
	Dropdown lby_modifiers;
	Checkbox lby_breaker_lastmoving;
	Slider   lby_breaker_range;


	// col 2.
	Checkbox override_retardmode;
	Checkbox retardmode_hide_angles;
	Slider   retardmode_x1;
	Slider   retardmode_y1;
	Slider   retardmode_x2;
	Slider   retardmode_y2;

	Checkbox      lag_enable;
	MultiDropdown lag_active;
	Dropdown      lag_mode;
	Slider        lag_limit;
	Checkbox      lag_land;

	Dropdown fake_yaw_type;
	Dropdown fake_yaw_base;
	Checkbox fake_yaw_safety;

	Keybind  fakewalk;
	Checkbox fakewalk_disable_desync;
	Keybind  manual_forward;
	Keybind  manual_left;
	Keybind  manual_right;
	Keybind  manual_back;
	Checkbox manual_arrows;
	Colorpicker manual_arrows_color;

	Dropdown leg_movement;

public:
	void init() {
		SetTitle(XOR("anti-aim"));

		// col 1.
		enable.setup(XOR("enable"), XOR("enable"));
		RegisterElement(&enable);

		pitch.setup(XOR("pitch"), XOR("pitch"), { XOR("down"), XOR("up"), XOR("zero"), XOR("random") });
		RegisterElement(&pitch);

		base_yaw.setup(XOR("base yaw"), XOR("base_yaw"), { XOR("backwards"), XOR("world zero") });
		RegisterElement(&base_yaw);

		base_yaw_offset.setup("offset", XOR("base_yaw_offset"), -180.f, 180.f, true, 0, 0.f, 1.f, XOR(L"°"));
		RegisterElement(&base_yaw_offset);

		base_yaw_modifiers.setup(XOR("yaw modifiers"), XOR("base_yaw_modifiers"), { XOR("none"), XOR("jitter"), XOR("rotate") });
		RegisterElement(&base_yaw_modifiers);

		modifier_range.setup("range", XOR("modifier_range"), -180.f, 180.f, true, 0, 0.f, 1.f, XOR(L"°"));
		modifier_range.AddShowCallback(callbacks::IsYawModifierJitter);
		RegisterElement(&modifier_range);

		modifier_speed.setup("speed", XOR("modifier_speed"), 0.f, 100.f, true, 0, 0.f, 1.f, XOR(L"%"));
		modifier_speed.AddShowCallback(callbacks::IsYawModifierRotate);
		RegisterElement(&modifier_speed);

		freestand_yaw.setup(XOR("freestanding"), XOR("freestand_yaw"));
		RegisterElement(&freestand_yaw);

		moving_feetyaw_desync.setup(XOR("desync moving feet yaw"), XOR("moving_feetyaw_desync"));
		RegisterElement(&moving_feetyaw_desync);

		feetyaw_desync_extend.setup(XOR("extend desync time"), XOR("feetyaw_desync_extend"));
		feetyaw_desync_extend.AddShowCallback(callbacks::IsDesyncEnabled);
		RegisterElement(&feetyaw_desync_extend);

		feetyaw_desync_disable_on_peek.setup(XOR("disable desync on peek"), XOR("feetyaw_desync_disable_on_peek"));
		feetyaw_desync_disable_on_peek.AddShowCallback(callbacks::IsDesyncEnabled);
		RegisterElement(&feetyaw_desync_disable_on_peek);

		lby_modifiers.setup(XOR("lby modifiers"), XOR("lby_modifiers"), { XOR("none"), XOR("avoid lby updates"), XOR("break lby") });
		RegisterElement(&lby_modifiers);

		lby_breaker_lastmoving.setup(XOR("break lby to last moving yaw"), XOR("lby_breaker_lastmoving"));
		lby_breaker_lastmoving.AddShowCallback(callbacks::IsLbyBreakerOn);
		RegisterElement(&lby_breaker_lastmoving);

		lby_breaker_range.setup("lby breaker offset", XOR("lby_breaker_range"), -180.f, 180.f, true, 0, 0.f, 1.f, XOR(L"°"));
		lby_breaker_range.AddShowCallback(callbacks::IsLbyBreakAndNotToLastMoveLol);
		RegisterElement(&lby_breaker_range);

		// col 2.
		override_retardmode.setup(XOR("override aa for retard-mode"), XOR("override_retardmode"));
		RegisterElement(&override_retardmode, 1);

		retardmode_hide_angles.setup(XOR("hide second retard-mode angle"), XOR("retardmode_hide_angles"));
		retardmode_hide_angles.AddShowCallback(callbacks::IsRetardModeOn);
		RegisterElement(&retardmode_hide_angles, 1);

		retardmode_x1.setup("retard-mode pitch 1", XOR("retardmode_x1"), -90.f, 90.f, true, 0, 0.f, 1.f, XOR(L"°"));
		retardmode_x1.AddShowCallback(callbacks::IsRetardModeOn);
		RegisterElement(&retardmode_x1, 1);

		retardmode_y1.setup("retard-mode yaw 1", XOR("retardmode_y1"), -180.f, 180.f, true, 0, 0.f, 1.f, XOR(L"°"));
		retardmode_y1.AddShowCallback(callbacks::IsRetardModeOn);
		RegisterElement(&retardmode_y1, 1);

		retardmode_x2.setup("retard-mode pitch 2", XOR("retardmode_x2"), -90.f, 90.f, true, 0, 0.f, 1.f, XOR(L"°"));
		retardmode_x2.AddShowCallback(callbacks::IsRetardModeOn);
		RegisterElement(&retardmode_x2, 1);

		retardmode_y2.setup("retard-mode yaw 2", XOR("retardmode_y2"), -180.f, 180.f, true, 0, 0.f, 1.f, XOR(L"°"));
		retardmode_y2.AddShowCallback(callbacks::IsRetardModeOn);
		RegisterElement(&retardmode_y2, 1);

		lag_enable.setup(XOR("fake-lag"), XOR("lag_enable"));
		RegisterElement(&lag_enable, 1);

		lag_active.setup("", XOR("lag_active"), { XOR("move"), XOR("air"), XOR("crouch") }, false);
		RegisterElement(&lag_active, 1);

		lag_mode.setup("", XOR("lag_mode"), { XOR("max"), XOR("break"), XOR("random"), XOR("break step") }, false);
		RegisterElement(&lag_mode, 1);

		lag_limit.setup(XOR("limit"), XOR("lag_limit"), 2, 16, true, 0, 2, 1.f);
		RegisterElement(&lag_limit, 1);

		lag_land.setup(XOR("on land"), XOR("lag_land"));
		RegisterElement(&lag_land, 1);

		fake_yaw_type.setup(XOR("fake yaw"), XOR("fake_yaw_type"), { XOR("off"), XOR("custom"), XOR("random") });
		RegisterElement(&fake_yaw_type, 1);

		fake_yaw_base.setup(XOR("fake yaw base"), XOR("fake_yaw_base"), { XOR("local view"), XOR("lower body"), XOR("zero") });
		RegisterElement(&fake_yaw_base, 1);

		fake_yaw_safety.setup(XOR("avoid fake overlap"), XOR("fake_yaw_safety"));
		RegisterElement(&fake_yaw_safety, 1);

		fakewalk.setup(XOR("fake-walk"), XOR("fakewalk"));
		RegisterElement(&fakewalk, 1);

		fakewalk_disable_desync.setup(XOR("slow-walk while desyncing"), XOR("fakewalk_disable_desync"));
		RegisterElement(&fakewalk_disable_desync, 1);

		manual_forward.setup(XOR("manual aa forwards"), XOR("manual_forward"));
		manual_forward.SetToggleCallback(callbacks::ManualForward);
		RegisterElement(&manual_forward, 1);

		manual_left.setup(XOR("manual aa left"), XOR("manual_left"));
		manual_left.SetToggleCallback(callbacks::ManualLeft);
		RegisterElement(&manual_left, 1);

		manual_right.setup(XOR("manual aa right"), XOR("manual_right"));
		manual_right.SetToggleCallback(callbacks::ManualRight);
		RegisterElement(&manual_right, 1);

		manual_back.setup(XOR("manual aa back"), XOR("manual_back"));
		manual_back.SetToggleCallback(callbacks::ManualBack);
		RegisterElement(&manual_back, 1);

		manual_arrows.setup(XOR("draw manual aa arrows"), XOR("manual_arrows"));
		RegisterElement(&manual_arrows, 1);

		manual_arrows_color.setup(XOR("manual arrows color"), XOR("manual_arrows_color"), colors::white);
		RegisterElement(&manual_arrows_color, 1);

		leg_movement.setup(XOR("leg movement"), XOR("fix_leg_movement"), { "off", "always slide", "never slide" });
		RegisterElement(&leg_movement, 1);
	}
};

class PlayersTab : public Tab {
public:
	MultiDropdown box;
	Colorpicker   box_enemy;
	Colorpicker   box_friendly;
	MultiDropdown esp_extras;
	Checkbox      dormant;
	Checkbox      offscreen;
	Colorpicker   offscreen_color;
	MultiDropdown name;
	Colorpicker   name_color;
	MultiDropdown health;
	Colorpicker   health_starting_color;
	Colorpicker   health_ending_color;
	Checkbox      health_number_lethal_only;
	MultiDropdown flags_enemy;
	MultiDropdown flags_friendly;
	MultiDropdown weapon;
	MultiDropdown weapon_mode;
	Checkbox      ammo;
	Colorpicker   ammo_color;
	Checkbox      lby_update;
	Colorpicker   lby_update_color;

	// col2.
	MultiDropdown skeleton;
	Colorpicker   skeleton_enemy;
	Colorpicker   skeleton_friendly;

	MultiDropdown glow;
	Colorpicker   glow_enemy;
	Colorpicker   glow_friendly;
	Slider        glow_blend;
	Checkbox      glow_visualize_aimbot;
	Colorpicker   glow_visualize_aimbot_color;
	Slider        glow_visualize_aimbot_blend;

	Dropdown      chams_selection;

	Checkbox      chams_enemy_enable;
	MultiDropdown chams_enemy;
	Dropdown      chams_enemy_mat;
	Colorpicker   chams_enemy_vis;
	Colorpicker   chams_enemy_invis;
	Checkbox      chams_enemy_wireframe;
	Slider        chams_enemy_blend;
	Checkbox      chams_enemy_double_enable;
	Dropdown      chams_enemy_double_mat;
	Colorpicker   chams_enemy_double_col;
	//Checkbox      chams_enemy_double_wireframe;
	//Slider        chams_enemy_double_blend;

	Checkbox      chams_friendly_enable;
	MultiDropdown chams_friendly;
	Dropdown      chams_friendly_mat;
	Colorpicker   chams_friendly_vis;
	Colorpicker   chams_friendly_invis;
	Checkbox      chams_friendly_wireframe;
	Slider        chams_friendly_blend;
	Checkbox      chams_friendly_double_enable;
	Dropdown      chams_friendly_double_mat;
	Colorpicker   chams_friendly_double_col;

	Checkbox      chams_local_enable;
	Dropdown      chams_local_mat;
	Colorpicker   chams_local_col;
	Checkbox      chams_local_wireframe;
	Slider        chams_local_blend;
	Checkbox      chams_local_double_enable;
	Dropdown      chams_local_double_mat;
	Colorpicker   chams_local_double_col;

	Checkbox      chams_history_enable;
	Dropdown      chams_history_mat;
	Colorpicker   chams_history_col;
	Checkbox      chams_history_wireframe;
	Slider        chams_history_blend;
	Checkbox      chams_history_double_enable;
	Dropdown      chams_history_double_mat;
	Colorpicker   chams_history_double_col;

	Checkbox      chams_weapon_enable;
	Dropdown      chams_weapon_mat;
	Colorpicker   chams_weapon_vis;
	Checkbox      chams_weapon_wireframe;
	Slider        chams_weapon_blend;
	Checkbox      chams_weapon_double_enable;
	Dropdown      chams_weapon_double_mat;
	Colorpicker   chams_weapon_double_col;

	Checkbox      chams_shot_enable;
	Slider        chams_shot_time;
	Dropdown      chams_shot_mat;
	Colorpicker   chams_shot_col;
	Checkbox      chams_shot_wireframe;
	Slider        chams_shot_blend;
	Checkbox      chams_shot_double_enable;
	Dropdown      chams_shot_double_mat;
	Colorpicker   chams_shot_double_col;

	Checkbox      chams_autopeek_enable;
	Dropdown      chams_autopeek_mat;
	Colorpicker   chams_autopeek_col;
	Checkbox      chams_autopeek_wireframe;
	Slider        chams_autopeek_blend;
	Checkbox      chams_autopeek_pulsate_alpha;
	Checkbox      chams_autopeek_double_enable;
	Dropdown      chams_autopeek_double_mat;
	Colorpicker   chams_autopeek_double_col;

public:
	void init() {
		SetTitle(XOR("players"));

		box.setup(XOR("boxes"), XOR("box"), { XOR("enemy"), XOR("friendly") });
		RegisterElement(&box);

		box_enemy.setup(XOR("box enemy color"), XOR("box_enemy"), colors::white);
		RegisterElement(&box_enemy);

		box_friendly.setup(XOR("box friendly color"), XOR("box_friendly"), colors::white);
		RegisterElement(&box_friendly);

		esp_extras.setup(XOR("esp extras"), XOR("esp_extras"), { XOR("outline"), XOR("interpolate") });
		RegisterElement(&esp_extras);

		dormant.setup(XOR("dormant enemies"), XOR("dormant"));
		RegisterElement(&dormant);

		offscreen.setup(XOR("enemy offscreen esp"), XOR("offscreen"));
		RegisterElement(&offscreen);

		offscreen_color.setup(XOR("offscreen esp color"), XOR("offscreen_color"), colors::white);
		RegisterElement(&offscreen_color);

		name.setup(XOR("name"), XOR("name"), { XOR("enemy"), XOR("friendly") });
		RegisterElement(&name);

		name_color.setup(XOR("name color"), XOR("name_color"), colors::white);
		RegisterElement(&name_color);

		health.setup(XOR("health"), XOR("health"), { XOR("enemy"), XOR("friendly") });
		RegisterElement(&health);

		health_starting_color.setup(XOR("health start color"), XOR("health_starting_color"), Color(50, 155, 30));
		RegisterElement(&health_starting_color);

		health_ending_color.setup(XOR("health end color"), XOR("health_ending_color"), Color(210, 35, 35));
		RegisterElement(&health_ending_color);

		health_number_lethal_only.setup(XOR("only show number if lethal"), XOR("health_number_lethal_only"));
		RegisterElement(&health_number_lethal_only);

		flags_enemy.setup(XOR("flags enemy"), XOR("flags_enemy"), { XOR("money"), XOR("armor"), XOR("ping"), XOR("scoped"), XOR("flashed"), XOR("reloading"), XOR("bomb"), XOR("fake"), XOR("safety condition"), XOR("damage condition"), XOR("aimbot debug"), XOR("resolver debug")});
		RegisterElement(&flags_enemy);

		flags_friendly.setup(XOR("flags friendly"), XOR("flags_friendly"), { XOR("money"), XOR("armor"), XOR("ping"), XOR("scoped"), XOR("flashed"), XOR("reloading"), XOR("bomb")});
		RegisterElement(&flags_friendly);

		weapon.setup(XOR("weapon"), XOR("weapon"), { XOR("enemy"), XOR("friendly") });
		RegisterElement(&weapon);

		weapon_mode.setup("", XOR("weapon_mode"), { XOR("text"), XOR("icon") }, false);
		RegisterElement(&weapon_mode);

		ammo.setup(XOR("ammo"), XOR("ammo"));
		RegisterElement(&ammo);

		ammo_color.setup(XOR("color"), XOR("ammo_color"), { 60, 120, 180 });
		RegisterElement(&ammo_color);

		lby_update.setup(XOR("lby update"), XOR("lby_update"));
		RegisterElement(&lby_update);

		lby_update_color.setup(XOR("color"), XOR("lby_update_color"), colors::purple);
		RegisterElement(&lby_update_color);

		// col2.
		skeleton.setup(XOR("skeleton"), XOR("skeleton"), { XOR("enemy"), XOR("friendly") });
		RegisterElement(&skeleton, 1);

		skeleton_enemy.setup(XOR("enemy color"), XOR("skeleton_enemy"), { 255, 255, 255 });
		RegisterElement(&skeleton_enemy, 1);

		skeleton_friendly.setup(XOR("friendly color"), XOR("skeleton_friendly"), { 255, 255, 255 });
		RegisterElement(&skeleton_friendly, 1);

		glow.setup(XOR("glow"), XOR("glow"), { XOR("enemy"), XOR("friendly") });
		RegisterElement(&glow, 1);

		glow_enemy.setup(XOR("enemy color"), XOR("glow_enemy"), { 180, 60, 120 });
		RegisterElement(&glow_enemy, 1);

		glow_friendly.setup(XOR("friendly color"), XOR("glow_friendly"), { 150, 200, 60 });
		RegisterElement(&glow_friendly, 1);

		glow_blend.setup("", XOR("glow_blend"), 10.f, 100.f, false, 0, 60.f, 1.f, XOR(L"%"));
		RegisterElement(&glow_blend, 1);

		glow_visualize_aimbot.setup(XOR("visualize aimbot"), XOR("glow_visualize_aimbot"));
		RegisterElement(&glow_visualize_aimbot, 1);

		glow_visualize_aimbot_color.setup(XOR("color"), XOR("glow_visualize_aimbot_color"), { 150, 200, 60 });
		RegisterElement(&glow_visualize_aimbot_color, 1);

		glow_visualize_aimbot_blend.setup("", XOR("glow_visualize_aimbot_blend"), 10.f, 100.f, false, 0, 60.f, 1.f, XOR(L"%"));
		RegisterElement(&glow_visualize_aimbot_blend, 1);

		chams_selection.setup(XOR("chams options"), XOR("chams_selection"), { XOR("enemy"), XOR("friendly"), XOR("local"), XOR("history"), XOR("weapon"), XOR("shot"), XOR("autopeek")});
		RegisterElement(&chams_selection, 1);

		chams_enemy_enable.setup(XOR("enable"), XOR("chams_enemy_enable"));
		chams_enemy_enable.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_enable, 1);

		chams_enemy.setup(XOR("chams enemy"), XOR("chams_enemy"), { XOR("visible"), XOR("invisible") });
		chams_enemy.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy, 1);

		chams_enemy_mat.setup(XOR("chams enemy material"), XOR("chams_enemy_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass")});
		chams_enemy_mat.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_mat, 1);

		chams_enemy_vis.setup(XOR("color visible"), XOR("chams_enemy_vis"), { 150, 200, 60 });
		chams_enemy_vis.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_vis, 1);

		chams_enemy_invis.setup(XOR("color invisible"), XOR("chams_enemy_invis"), { 60, 120, 180 });
		chams_enemy_invis.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_invis, 1);

		chams_enemy_wireframe.setup(XOR("wireframe"), XOR("chams_enemy_wireframe"));
		chams_enemy_wireframe.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_wireframe, 1);

		chams_enemy_blend.setup("", XOR("chams_enemy_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_enemy_blend.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_blend, 1);

		chams_enemy_double_enable.setup(XOR("enable double material"), XOR("chams_enemy_double_enable"));
		chams_enemy_double_enable.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_double_enable, 1);

		chams_enemy_double_mat.setup(XOR("double material"), XOR("chams_enemy_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_enemy_double_mat.AddShowCallback(callbacks::IsEnemyChams);
		chams_enemy_double_mat.AddShowCallback(callbacks::IsEnemyDoubleChams);
		RegisterElement(&chams_enemy_double_mat, 1);

		chams_enemy_double_col.setup(XOR("color"), XOR("chams_enemy_double_col"), { 255, 255, 255 });
		chams_enemy_double_col.AddShowCallback(callbacks::IsEnemyChams);
		chams_enemy_double_col.AddShowCallback(callbacks::IsEnemyDoubleChams);
		RegisterElement(&chams_enemy_double_col, 1);

		// to-do: make menu bigger because these elements dont fit.
		/*chams_enemy_double_wireframe.setup(XOR("wireframe"), XOR("chams_enemy_double_wireframe"));
		chams_enemy_double_wireframe.AddShowCallback(callbacks::IsEnemyChams);
		chams_enemy_double_wireframe.AddShowCallback(callbacks::IsEnemyDoubleChams);
		RegisterElement(&chams_enemy_double_wireframe, 1);

		chams_enemy_double_blend.setup("", XOR("chams_enemy_double_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_enemy_double_blend.AddShowCallback(callbacks::IsEnemyChams);
		chams_enemy_double_blend.AddShowCallback(callbacks::IsEnemyDoubleChams);
		RegisterElement(&chams_enemy_double_blend, 1);*/

		chams_friendly_enable.setup(XOR("enable"), XOR("chams_friendly_enable"));
		chams_friendly_enable.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_enable, 1);

		chams_friendly.setup(XOR("chams friendly"), XOR("chams_friendly"), { XOR("visible"), XOR("invisible") });
		chams_friendly.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly, 1);

		chams_friendly_mat.setup(XOR("chams friendly material"), XOR("chams_friendly_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass")});
		chams_friendly_mat.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_mat, 1);

		chams_friendly_vis.setup(XOR("color visible"), XOR("chams_friendly_vis"), { 255, 200, 0 });
		chams_friendly_vis.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_vis, 1);

		chams_friendly_invis.setup(XOR("color invisible"), XOR("chams_friendly_invis"), { 255, 50, 0 });
		chams_friendly_invis.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_invis, 1);

		chams_friendly_wireframe.setup(XOR("wireframe"), XOR("chams_friendly_wireframe"));
		chams_friendly_wireframe.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_wireframe, 1);

		chams_friendly_blend.setup("", XOR("chams_friendly_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_friendly_blend.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_blend, 1);

		chams_friendly_double_enable.setup(XOR("enable double material"), XOR("chams_friendly_double_enable"));
		chams_friendly_double_enable.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_double_enable, 1);

		chams_friendly_double_mat.setup(XOR("double material"), XOR("chams_friendly_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_friendly_double_mat.AddShowCallback(callbacks::IsFriendlyChams);
		chams_friendly_double_mat.AddShowCallback(callbacks::IsFriendlyDoubleChams);
		RegisterElement(&chams_friendly_double_mat, 1);

		chams_friendly_double_col.setup(XOR("color"), XOR("chams_friendly_double_col"), { 255, 255, 255 });
		chams_friendly_double_col.AddShowCallback(callbacks::IsFriendlyChams);
		chams_friendly_double_col.AddShowCallback(callbacks::IsFriendlyDoubleChams);
		RegisterElement(&chams_friendly_double_col, 1);

		chams_local_enable.setup(XOR("enable"), XOR("chams_local_enable"));
		chams_local_enable.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_enable, 1);

		chams_local_mat.setup(XOR("chams local material"), XOR("chams_local_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass")});
		chams_local_mat.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_mat, 1);

		chams_local_col.setup(XOR("color"), XOR("chams_local_col"), { 60, 120, 180 });
		chams_local_col.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_col, 1);

		chams_local_wireframe.setup(XOR("wireframe"), XOR("chams_local_wireframe"));
		chams_local_wireframe.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_wireframe, 1);

		chams_local_blend.setup("", XOR("chams_local_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_local_blend.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_blend, 1);

		chams_local_double_enable.setup(XOR("enable double material"), XOR("chams_local_double_enable"));
		chams_local_double_enable.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_double_enable, 1);

		chams_local_double_mat.setup(XOR("double material"), XOR("chams_local_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_local_double_mat.AddShowCallback(callbacks::IsLocalChams);
		chams_local_double_mat.AddShowCallback(callbacks::IsLocalDoubleChams);
		RegisterElement(&chams_local_double_mat, 1);

		chams_local_double_col.setup(XOR("color"), XOR("chams_local_double_col"), { 255, 255, 255 });
		chams_local_double_col.AddShowCallback(callbacks::IsLocalChams);
		chams_local_double_col.AddShowCallback(callbacks::IsLocalDoubleChams);
		RegisterElement(&chams_local_double_col, 1);

		chams_history_enable.setup(XOR("enable history chams"), XOR("chams_history_enable"));
		chams_history_enable.AddShowCallback(callbacks::IsBacktrackChams);
		RegisterElement(&chams_history_enable, 1);

		chams_history_mat.setup(XOR("chams history material"), XOR("chams_history_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass")});
		chams_history_mat.AddShowCallback(callbacks::IsBacktrackChams);
		RegisterElement(&chams_history_mat, 1);

		chams_history_col.setup(XOR("color"), XOR("chams_history_col"), { 255, 255, 200 });
		chams_history_col.AddShowCallback(callbacks::IsBacktrackChams);
		RegisterElement(&chams_history_col, 1);

		chams_history_wireframe.setup(XOR("wireframe"), XOR("chams_history_wireframe"));
		chams_history_wireframe.AddShowCallback(callbacks::IsBacktrackChams);
		RegisterElement(&chams_history_wireframe, 1);

		chams_history_blend.setup("", XOR("chams_history_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_history_blend.AddShowCallback(callbacks::IsBacktrackChams);
		RegisterElement(&chams_history_blend, 1);

		chams_history_double_enable.setup(XOR("enable double material"), XOR("chams_history_double_enable"));
		chams_history_double_enable.AddShowCallback(callbacks::IsBacktrackChams);
		RegisterElement(&chams_history_double_enable, 1);

		chams_history_double_mat.setup(XOR("double material"), XOR("chams_history_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_history_double_mat.AddShowCallback(callbacks::IsBacktrackChams);
		chams_history_double_mat.AddShowCallback(callbacks::IsHistoryDoubleChams);
		RegisterElement(&chams_history_double_mat, 1);

		chams_history_double_col.setup(XOR("color"), XOR("chams_history_double_col"), { 255, 255, 255 });
		chams_history_double_col.AddShowCallback(callbacks::IsBacktrackChams);
		chams_history_double_col.AddShowCallback(callbacks::IsHistoryDoubleChams);
		RegisterElement(&chams_history_double_col, 1);

		chams_weapon_enable.setup(XOR("enable"), XOR("chams_weapon_enable"));
		chams_weapon_enable.AddShowCallback(callbacks::IsWeaponChams);
		RegisterElement(&chams_weapon_enable, 1);

		chams_weapon_mat.setup(XOR("chams weapon material"), XOR("chams_weapon_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass") });
		chams_weapon_mat.AddShowCallback(callbacks::IsWeaponChams);
		RegisterElement(&chams_weapon_mat, 1);

		chams_weapon_vis.setup(XOR("color"), XOR("chams_weapon_vis"), { 150, 200, 60 });
		chams_weapon_vis.AddShowCallback(callbacks::IsWeaponChams);
		RegisterElement(&chams_weapon_vis, 1);

		chams_weapon_wireframe.setup(XOR("wireframe"), XOR("chams_weapon_wireframe"));
		chams_weapon_wireframe.AddShowCallback(callbacks::IsWeaponChams);
		RegisterElement(&chams_weapon_wireframe, 1);

		chams_weapon_blend.setup("", XOR("chams_weapon_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_weapon_blend.AddShowCallback(callbacks::IsWeaponChams);
		RegisterElement(&chams_weapon_blend, 1);

		chams_weapon_double_enable.setup(XOR("enable double material"), XOR("chams_weapon_double_enable"));
		chams_weapon_double_enable.AddShowCallback(callbacks::IsWeaponChams);
		RegisterElement(&chams_weapon_double_enable, 1);

		chams_weapon_double_mat.setup(XOR("double material"), XOR("chams_weapon_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_weapon_double_mat.AddShowCallback(callbacks::IsWeaponChams);
		chams_weapon_double_mat.AddShowCallback(callbacks::IsWeaponDoubleChams);
		RegisterElement(&chams_weapon_double_mat, 1);

		chams_weapon_double_col.setup(XOR("color"), XOR("chams_weapon_double_col"), { 255, 255, 255 });
		chams_weapon_double_col.AddShowCallback(callbacks::IsWeaponChams);
		chams_weapon_double_col.AddShowCallback(callbacks::IsWeaponDoubleChams);
		RegisterElement(&chams_weapon_double_col, 1);

		chams_shot_enable.setup(XOR("enable"), XOR("chams_shot_enable"));
		chams_shot_enable.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_enable, 1);

		chams_shot_time.setup("time", XOR("chams_shot_time"), 1.f, 10.f, true, 0, 4.f, 1.f, XOR(L"s"));
		chams_shot_time.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_time, 1);

		chams_shot_mat.setup(XOR("chams shot material"), XOR("chams_shot_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass")});
		chams_shot_mat.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_mat, 1);

		chams_shot_col.setup(XOR("color"), XOR("chams_shot_vis"), { 150, 200, 60 });
		chams_shot_col.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_col, 1);

		chams_shot_wireframe.setup(XOR("wireframe"), XOR("chams_shot_wireframe"));
		chams_shot_wireframe.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_wireframe, 1);

		chams_shot_blend.setup("", XOR("chams_shot_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_shot_blend.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_blend, 1);

		chams_shot_double_enable.setup(XOR("enable double material"), XOR("chams_shot_double_enable"));
		chams_shot_double_enable.AddShowCallback(callbacks::IsShotChams);
		RegisterElement(&chams_shot_double_enable, 1);

		chams_shot_double_mat.setup(XOR("double material"), XOR("chams_shot_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_shot_double_mat.AddShowCallback(callbacks::IsShotChams);
		chams_shot_double_mat.AddShowCallback(callbacks::IsShotDoubleChams);
		RegisterElement(&chams_shot_double_mat, 1);

		chams_shot_double_col.setup(XOR("color"), XOR("chams_shot_double_col"), { 255, 255, 255 });
		chams_shot_double_col.AddShowCallback(callbacks::IsShotChams);
		chams_shot_double_col.AddShowCallback(callbacks::IsShotDoubleChams);
		RegisterElement(&chams_shot_double_col, 1);




		chams_autopeek_enable.setup(XOR("enable"), XOR("chams_autopeek_enable"));
		chams_autopeek_enable.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_enable, 1);

		chams_autopeek_mat.setup(XOR("chams autopeek material"), XOR("chams_autopeek_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost"), XOR("scroll"), XOR("glass")});
		chams_autopeek_mat.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_mat, 1);

		chams_autopeek_col.setup(XOR("color"), XOR("chams_autopeek_vis"), { 150, 200, 60 });
		chams_autopeek_col.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_col, 1);

		chams_autopeek_wireframe.setup(XOR("wireframe"), XOR("chams_autopeek_wireframe"));
		chams_autopeek_wireframe.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_wireframe, 1);

		chams_autopeek_blend.setup("", XOR("chams_autopeek_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		chams_autopeek_blend.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_blend, 1);

		chams_autopeek_pulsate_alpha.setup(XOR("pulsate alpha"), XOR("chams_autopeek_pulsate_alpha"));
		chams_autopeek_pulsate_alpha.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_pulsate_alpha, 1);

		chams_autopeek_double_enable.setup(XOR("enable double material"), XOR("chams_autopeek_double_enable"));
		chams_autopeek_double_enable.AddShowCallback(callbacks::IsAutoPeekChams);
		RegisterElement(&chams_autopeek_double_enable, 1);

		chams_autopeek_double_mat.setup(XOR("double material"), XOR("chams_autopeek_double_mat"), { XOR("outline"), XOR("ghost"), XOR("scroll") });
		chams_autopeek_double_mat.AddShowCallback(callbacks::IsAutoPeekChams);
		chams_autopeek_double_mat.AddShowCallback(callbacks::IsAutoPeekDoubleChams);
		RegisterElement(&chams_autopeek_double_mat, 1);

		chams_autopeek_double_col.setup(XOR("color"), XOR("chams_autopeek_double_col"), { 255, 255, 255 });
		chams_autopeek_double_col.AddShowCallback(callbacks::IsAutoPeekChams);
		chams_autopeek_double_col.AddShowCallback(callbacks::IsAutoPeekDoubleChams);
		RegisterElement(&chams_autopeek_double_col, 1);
	}
};

class VisualsTab : public Tab {
public:
	Checkbox      distance;
	Checkbox      items;
	Colorpicker   item_color;
	Checkbox      ammo;
	Colorpicker   dropammo_color;
	Checkbox      proj;
	Colorpicker   proj_safe_color;
	Colorpicker   proj_dangerous_color;
	MultiDropdown planted_c4;
	Checkbox      disableteam;
	MultiDropdown world;
	Dropdown      night_color;
	Slider        night_darkness;
	Colorpicker   ambientlight_color;
	Slider        ambientlight_darkness;
	//	  world_color;
	Checkbox      transparent_props;
	Slider		  transparent_props_amount;
	Checkbox      enemy_radar;

	Checkbox	FogOverride; // butt
	Colorpicker	FogColor; // color
	Slider		FogStart; // slider
	Slider		FogEnd; // slider
	Slider		Fogdensity; // slider
	//Colorpicker      glowcolor;

	// col2.
	MultiDropdown vis_removals;
	Checkbox      fov;
	Slider        fov_amt;
	Slider        scope_zoom;
	Checkbox      viewmodel_fov;
	Slider        viewmodel_fov_amt;
	Checkbox      spectators;
	Checkbox      force_xhair;
	Checkbox      spread_xhair;
	Colorpicker   spread_xhair_col;
	Slider        spread_xhair_blend;
	Checkbox      pen_crosshair;
	Checkbox      pen_damage;
	MultiDropdown indicators;
	Colorpicker   indicators_color;
	Checkbox      tracers;
	MultiDropdown impact_beams_select;
	Colorpicker   impact_beams_color;
	Slider        impact_beams_time;
	Keybind       thirdperson;
	Checkbox      thirdperson_interpolate;
	Slider		  thirdperson_distance;

public:
	void init() {
		SetTitle(XOR("visuals"));

		items.setup(XOR("dropped weapons"), XOR("items"));
		RegisterElement(&items);

		distance.setup(XOR("distance weapons"), XOR("distance"));
		RegisterElement(&distance);

		item_color.setup(XOR("color"), XOR("item_color"), colors::white);
		RegisterElement(&item_color);

		ammo.setup(XOR("dropped weapons ammo"), XOR("ammo"));
		RegisterElement(&ammo);

		dropammo_color.setup(XOR("color"), XOR("dropammo_color"), colors::light_blue);
		RegisterElement(&dropammo_color);

		proj.setup(XOR("projectiles"), XOR("proj"));
		RegisterElement(&proj);

		proj_safe_color.setup(XOR("safe projectiles color"), XOR("proj_safe_color"), Color(150, 200, 60));
		RegisterElement(&proj_safe_color);

		proj_dangerous_color.setup(XOR("dangerous projectiles color"), XOR("proj_dangerous_color"), colors::white);
		RegisterElement(&proj_dangerous_color);

		planted_c4.setup(XOR("planted c4"), XOR("planted_c4"), { XOR("on screen (2D)"), XOR("on bomb (3D)"), XOR("bomb timer (2D)"), XOR("bomb rimer (3D)") });
		RegisterElement(&planted_c4);

		disableteam.setup(XOR("do not render teammates"), XOR("disableteam"));
		RegisterElement(&disableteam);

		world.setup(XOR("world"), XOR("world"), { XOR("night"), XOR("fullbright"), XOR("ambient lighting"), XOR("sunset") });
		world.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&world);

		night_color.setup(XOR("night color"), XOR("night_color"), { XOR("red"), XOR("purple"), XOR("custom") });
		night_color.SetCallback(Visuals::ModulateWorld);
		night_color.AddShowCallback(callbacks::IsNightMode);
		RegisterElement(&night_color);

		night_darkness.setup("", XOR("night_darkness"), 0.f, 100.f, false, 0, 10.f, 1.f, XOR(L"%"));
		night_darkness.SetCallback(Visuals::ModulateWorld);
		night_darkness.AddShowCallback(callbacks::IsNightCustom);
		RegisterElement(&night_darkness);

		ambientlight_color.setup(XOR("ambient light color"), XOR("ambientlight_color"), Color(0, 0, 0));
		ambientlight_color.SetCallback(Visuals::ModulateWorld);
		ambientlight_color.AddShowCallback(callbacks::IsAmbientLight);
		RegisterElement(&ambientlight_color);

		ambientlight_darkness.setup(XOR("ambient light nigger"), XOR("ambientlight_darkness"), 1.f, 100.f, false, 0.f, 10.f, 1.f, XOR(L"%"));
		ambientlight_darkness.SetCallback(Visuals::ModulateWorld);
		ambientlight_darkness.AddShowCallback(callbacks::IsAmbientLight);
		RegisterElement(&ambientlight_darkness);
		

		//world_color.setup(XOR("color"), XOR("world_color"), colors::burgundy);
		//RegisterElement(&world_color);

		transparent_props.setup(XOR("transparent props"), XOR("transparent_props"));
		transparent_props.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&transparent_props);

		transparent_props_amount.setup("", XOR("transparent_props_amount"), 0.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		transparent_props_amount.SetCallback(Visuals::ModulateWorld);
		transparent_props_amount.AddShowCallback(callbacks::IsTransparentProps);
		RegisterElement(&transparent_props_amount);

		enemy_radar.setup(XOR("force enemies on radar"), XOR("enemy_radar"));
		RegisterElement(&enemy_radar);

		FogOverride.setup(XOR("Override fog"), XOR("FogOverride"));
		RegisterElement(&FogOverride);

		FogColor.setup("color", XOR("FogColor"), colors::burgundy);
		RegisterElement(&FogColor);

		FogStart.setup(XOR("Start"), XOR("Fog start"), 0.f, 2500.f, false, 0, 100.f, 1.f, XOR(L"%"));
		RegisterElement(&FogStart);

		FogEnd.setup(XOR("End"), XOR("Fog end"), 0.f, 2500.f, false, 0, 100.f, 1.f, XOR(L"%"));
		RegisterElement(&FogEnd);

		Fogdensity.setup(XOR("Density"), XOR("Fog density"), 0.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		RegisterElement(&Fogdensity);

		// col2.
		vis_removals.setup(XOR("removals"), XOR("vis_removals"), { XOR("remove recoil"), XOR("remove smoke"), XOR("remove fog"), XOR("remove flashbangs"), XOR("remove scope"), XOR("remove shadows"), XOR("remove post processing") });
		RegisterElement(&vis_removals, 1);

		fov.setup(XOR("override fov"), XOR("fov"));
		RegisterElement(&fov, 1);

		fov_amt.setup("", XOR("fov_amt"), 60.f, 140.f, false, 0, 90.f, 1.f, XOR(L"°"));
		RegisterElement(&fov_amt, 1);

		scope_zoom.setup("scope zoom", XOR("scope_zoom"), 0.f, 100.f, true, 0, 0.f, 1.f, XOR(L"%"));
		RegisterElement(&scope_zoom, 1);

		viewmodel_fov.setup(XOR("override viewmodel fov"), XOR("viewmodel_fov"));
		RegisterElement(&viewmodel_fov, 1);

		viewmodel_fov_amt.setup("", XOR("viewmodel_fov_amt"), 60.f, 140.f, false, 0, 90.f, 1.f, XOR(L"°"));
		RegisterElement(&viewmodel_fov_amt, 1);

		spectators.setup(XOR("show spectator list"), XOR("spectators"));
		RegisterElement(&spectators, 1);

		force_xhair.setup(XOR("force crosshair"), XOR("force_xhair"));
		RegisterElement(&force_xhair, 1);

		spread_xhair.setup(XOR("visualize spread"), XOR("spread_xhair"));
		RegisterElement(&spread_xhair, 1);

		spread_xhair_col.setup(XOR("visualize spread color"), XOR("spread_xhair_col"), colors::burgundy);
		RegisterElement(&spread_xhair_col, 1);

		spread_xhair_blend.setup("", XOR("spread_xhair_blend"), 10.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		RegisterElement(&spread_xhair_blend, 1);

		pen_crosshair.setup(XOR("penetration crosshair"), XOR("pen_xhair"));
		RegisterElement(&pen_crosshair, 1);

		pen_damage.setup(XOR("penetration crosshair damage"), XOR("pen_damage"));
		RegisterElement(&pen_damage, 1);

		indicators.setup(XOR("indicators"), XOR("indicators"), { XOR("antiaim"), XOR("lag compensation"), XOR("fake latency"), XOR("damage override"), XOR("autopeek"), XOR("force baim") });
		RegisterElement(&indicators, 1);

		indicators_color.setup(XOR("indicators color"), XOR("indicators_color"), colors::white);
		RegisterElement(&indicators_color, 1);

		tracers.setup(XOR("grenade simulation"), XOR("tracers"));
		RegisterElement(&tracers, 1);

		impact_beams_select.setup(XOR("impact beams"), XOR("impact_beams"), { XOR("local"), XOR("enemy") });
		RegisterElement(&impact_beams_select, 1);

		impact_beams_color.setup(XOR("impact beams color"), XOR("impact_beams_color"), { 150, 130, 255 });
		RegisterElement(&impact_beams_color, 1);

		impact_beams_time.setup(XOR("impact beams time"), XOR("impact_beams_time"), 1.f, 10.f, true, 0, 1.f, 1.f, XOR(L"s"));
		RegisterElement(&impact_beams_time, 1);

		thirdperson.setup(XOR("thirdperson"), XOR("thirdperson"));
		thirdperson.SetToggleCallback(callbacks::ToggleThirdPerson);
		RegisterElement(&thirdperson, 1);

		thirdperson_interpolate.setup(XOR("interpolate thirdperson"), XOR("thirdperson_interpolate"));
		RegisterElement(&thirdperson_interpolate, 1);

		thirdperson_distance.setup(XOR(" "), XOR("thirdperson_distance"), 50.f, 300.f, false, 0, 150.f, 1.f, XOR(L"°"));
		RegisterElement(&thirdperson_distance, 1);
	}
};

class MovementTab : public Tab {
public:
	Checkbox bhop;
	Checkbox airduck;
	Checkbox autostrafe;
	Checkbox quickstop;
	Keybind  cstrafe;
	Keybind  astrafe;
	Keybind  zstrafe;
	Slider   z_freq;
	Slider   z_dist;

	Keybind  autopeek;
	Checkbox autopeek_knife;
	Checkbox autopeek_circle;
	Colorpicker autopeek_circle_color;

public:
	void init() {
		SetTitle(XOR("movement"));

		bhop.setup(XOR("automatic jump"), XOR("bhop"));
		RegisterElement(&bhop);

		airduck.setup(XOR("duck in air"), XOR("airduck"));
		RegisterElement(&airduck);

		autostrafe.setup(XOR("automatic strafe"), XOR("autostrafe"));
		RegisterElement(&autostrafe);

		quickstop.setup(XOR("quick stop"), XOR("quickstop"));
		RegisterElement(&quickstop);

		cstrafe.setup(XOR("c-strafe"), XOR("cstrafe"));
		RegisterElement(&cstrafe);

		astrafe.setup(XOR("a-strafe"), XOR("astrafe"));
		RegisterElement(&astrafe);

		zstrafe.setup(XOR("z-strafe"), XOR("zstrafe"));
		RegisterElement(&zstrafe);

		z_freq.setup("", XOR("z_freq"), 1.f, 100.f, false, 0, 50.f, 0.5f, XOR(L"hz"));
		RegisterElement(&z_freq);

		z_dist.setup("", XOR("z_dist"), 1.f, 100.f, false, 0, 20.f, 0.5f, XOR(L"%"));
		RegisterElement(&z_dist);

		autopeek.setup(XOR("automatic peek"), XOR("autopeek"));
		RegisterElement(&autopeek, 1);

		autopeek_knife.setup(XOR("automatic peek equip knife"), XOR("autopeek_knife"));
		RegisterElement(&autopeek_knife, 1);

		autopeek_circle.setup(XOR("automatic peek world circle"), XOR("autopeek_circle"));
		RegisterElement(&autopeek_circle, 1);

		autopeek_circle_color.setup(XOR("circle color"), XOR("autopeek_circle_color"), colors::white);
		RegisterElement(&autopeek_circle_color, 1);
	}
};

class SkinsTab : public Tab {
public:
	Checkbox enable;

	Edit     id_deagle;
	Checkbox stattrak_deagle;
	Slider   quality_deagle;
	Slider	 seed_deagle;

	Edit     id_elite;
	Checkbox stattrak_elite;
	Slider   quality_elite;
	Slider	 seed_elite;

	Edit     id_fiveseven;
	Checkbox stattrak_fiveseven;
	Slider   quality_fiveseven;
	Slider	 seed_fiveseven;

	Edit     id_glock;
	Checkbox stattrak_glock;
	Slider   quality_glock;
	Slider	 seed_glock;

	Edit     id_ak47;
	Checkbox stattrak_ak47;
	Slider   quality_ak47;
	Slider	 seed_ak47;

	Edit     id_aug;
	Checkbox stattrak_aug;
	Slider   quality_aug;
	Slider	 seed_aug;

	Edit     id_awp;
	Checkbox stattrak_awp;
	Slider   quality_awp;
	Slider	 seed_awp;

	Edit     id_famas;
	Checkbox stattrak_famas;
	Slider   quality_famas;
	Slider	 seed_famas;

	Edit     id_g3sg1;
	Checkbox stattrak_g3sg1;
	Slider   quality_g3sg1;
	Slider	 seed_g3sg1;

	Edit     id_galil;
	Checkbox stattrak_galil;
	Slider   quality_galil;
	Slider	 seed_galil;

	Edit     id_m249;
	Checkbox stattrak_m249;
	Slider   quality_m249;
	Slider	 seed_m249;

	Edit     id_m4a4;
	Checkbox stattrak_m4a4;
	Slider   quality_m4a4;
	Slider	 seed_m4a4;

	Edit     id_mac10;
	Checkbox stattrak_mac10;
	Slider   quality_mac10;
	Slider	 seed_mac10;

	Edit     id_p90;
	Checkbox stattrak_p90;
	Slider   quality_p90;
	Slider	 seed_p90;

	Edit     id_ump45;
	Checkbox stattrak_ump45;
	Slider   quality_ump45;
	Slider	 seed_ump45;

	Edit     id_xm1014;
	Checkbox stattrak_xm1014;
	Slider   quality_xm1014;
	Slider	 seed_xm1014;

	Edit     id_bizon;
	Checkbox stattrak_bizon;
	Slider   quality_bizon;
	Slider	 seed_bizon;

	Edit     id_mag7;
	Checkbox stattrak_mag7;
	Slider   quality_mag7;
	Slider	 seed_mag7;

	Edit     id_negev;
	Checkbox stattrak_negev;
	Slider   quality_negev;
	Slider	 seed_negev;

	Edit     id_sawedoff;
	Checkbox stattrak_sawedoff;
	Slider   quality_sawedoff;
	Slider	 seed_sawedoff;

	Edit     id_tec9;
	Checkbox stattrak_tec9;
	Slider   quality_tec9;
	Slider	 seed_tec9;

	Edit     id_p2000;
	Checkbox stattrak_p2000;
	Slider   quality_p2000;
	Slider	 seed_p2000;

	Edit     id_mp7;
	Checkbox stattrak_mp7;
	Slider   quality_mp7;
	Slider	 seed_mp7;

	Edit     id_mp9;
	Checkbox stattrak_mp9;
	Slider   quality_mp9;
	Slider	 seed_mp9;

	Edit     id_nova;
	Checkbox stattrak_nova;
	Slider   quality_nova;
	Slider	 seed_nova;

	Edit     id_p250;
	Checkbox stattrak_p250;
	Slider   quality_p250;
	Slider	 seed_p250;

	Edit     id_scar20;
	Checkbox stattrak_scar20;
	Slider   quality_scar20;
	Slider	 seed_scar20;

	Edit     id_sg553;
	Checkbox stattrak_sg553;
	Slider   quality_sg553;
	Slider	 seed_sg553;

	Edit     id_ssg08;
	Checkbox stattrak_ssg08;
	Slider   quality_ssg08;
	Slider	 seed_ssg08;

	Edit     id_m4a1s;
	Checkbox stattrak_m4a1s;
	Slider   quality_m4a1s;
	Slider	 seed_m4a1s;

	Edit     id_usps;
	Checkbox stattrak_usps;
	Slider   quality_usps;
	Slider	 seed_usps;

	Edit     id_cz75a;
	Checkbox stattrak_cz75a;
	Slider   quality_cz75a;
	Slider	 seed_cz75a;

	Edit     id_revolver;
	Checkbox stattrak_revolver;
	Slider   quality_revolver;
	Slider	 seed_revolver;

	Edit     id_bayonet;
	Checkbox stattrak_bayonet;
	Slider   quality_bayonet;
	Slider	 seed_bayonet;

	Edit     id_flip;
	Checkbox stattrak_flip;
	Slider   quality_flip;
	Slider	 seed_flip;

	Edit     id_gut;
	Checkbox stattrak_gut;
	Slider   quality_gut;
	Slider	 seed_gut;

	Edit     id_karambit;
	Checkbox stattrak_karambit;
	Slider   quality_karambit;
	Slider	 seed_karambit;

	Edit     id_m9;
	Checkbox stattrak_m9;
	Slider   quality_m9;
	Slider	 seed_m9;

	Edit     id_huntsman;
	Checkbox stattrak_huntsman;
	Slider   quality_huntsman;
	Slider	 seed_huntsman;

	Edit     id_falchion;
	Checkbox stattrak_falchion;
	Slider   quality_falchion;
	Slider	 seed_falchion;

	Edit     id_bowie;
	Checkbox stattrak_bowie;
	Slider   quality_bowie;
	Slider	 seed_bowie;

	Edit     id_butterfly;
	Checkbox stattrak_butterfly;
	Slider   quality_butterfly;
	Slider	 seed_butterfly;

	Edit     id_daggers;
	Checkbox stattrak_daggers;
	Slider   quality_daggers;
	Slider	 seed_daggers;

	// col 2.
	Dropdown knife;
	Dropdown glove;
	Edit	 glove_id;

public:
	void init() {
		SetTitle(XOR("skins"));

		enable.setup(XOR("enable"), XOR("skins_enable"));
		enable.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&enable);

		// weapons...
		id_deagle.setup(XOR("paintkit id"), XOR("id_deagle"), 3);
		id_deagle.SetCallback(callbacks::SkinUpdate);
		id_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&id_deagle);

		stattrak_deagle.setup(XOR("stattrak"), XOR("stattrak_deagle"));
		stattrak_deagle.SetCallback(callbacks::SkinUpdate);
		stattrak_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&stattrak_deagle);

		quality_deagle.setup(XOR("quality"), XOR("quality_deagle"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_deagle.SetCallback(callbacks::SkinUpdate);
		quality_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&quality_deagle);

		seed_deagle.setup(XOR("seed"), XOR("seed_deagle"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_deagle.SetCallback(callbacks::SkinUpdate);
		seed_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&seed_deagle);

		id_elite.setup(XOR("paintkit id"), XOR("id_elite"), 3);
		id_elite.SetCallback(callbacks::SkinUpdate);
		id_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&id_elite);

		stattrak_elite.setup(XOR("stattrak"), XOR("stattrak_elite"));
		stattrak_elite.SetCallback(callbacks::SkinUpdate);
		stattrak_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&stattrak_elite);

		quality_elite.setup(XOR("quality"), XOR("quality_elite"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_elite.SetCallback(callbacks::SkinUpdate);
		quality_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&quality_elite);

		seed_elite.setup(XOR("seed"), XOR("seed_elite"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_elite.SetCallback(callbacks::SkinUpdate);
		seed_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&seed_elite);

		id_fiveseven.setup(XOR("paintkit id"), XOR("id_fiveseven"), 3);
		id_fiveseven.SetCallback(callbacks::SkinUpdate);
		id_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&id_fiveseven);

		stattrak_fiveseven.setup(XOR("stattrak"), XOR("stattrak_fiveseven"));
		stattrak_fiveseven.SetCallback(callbacks::SkinUpdate);
		stattrak_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&stattrak_fiveseven);

		quality_fiveseven.setup(XOR("quality"), XOR("quality_fiveseven"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_fiveseven.SetCallback(callbacks::SkinUpdate);
		quality_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&quality_fiveseven);

		seed_fiveseven.setup(XOR("seed"), XOR("seed_fiveseven"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_fiveseven.SetCallback(callbacks::SkinUpdate);
		seed_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&seed_fiveseven);

		id_glock.setup(XOR("paintkit id"), XOR("id_glock"), 3);
		id_glock.SetCallback(callbacks::SkinUpdate);
		id_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&id_glock);

		stattrak_glock.setup(XOR("stattrak"), XOR("stattrak_glock"));
		stattrak_glock.SetCallback(callbacks::SkinUpdate);
		stattrak_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&stattrak_glock);

		quality_glock.setup(XOR("quality"), XOR("quality_glock"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_glock.SetCallback(callbacks::SkinUpdate);
		quality_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&quality_glock);

		seed_glock.setup(XOR("seed"), XOR("seed_glock"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_glock.SetCallback(callbacks::SkinUpdate);
		seed_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&seed_glock);

		id_ak47.setup(XOR("paintkit id"), XOR("id_ak47"), 3);
		id_ak47.SetCallback(callbacks::SkinUpdate);
		id_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&id_ak47);

		stattrak_ak47.setup(XOR("stattrak"), XOR("stattrak_ak47"));
		stattrak_ak47.SetCallback(callbacks::SkinUpdate);
		stattrak_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&stattrak_ak47);

		quality_ak47.setup(XOR("quality"), XOR("quality_ak47"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_ak47.SetCallback(callbacks::SkinUpdate);
		quality_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&quality_ak47);

		seed_ak47.setup(XOR("seed"), XOR("seed_ak47"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_ak47.SetCallback(callbacks::SkinUpdate);
		seed_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&seed_ak47);

		id_aug.setup(XOR("paintkit id"), XOR("id_aug"), 3);
		id_aug.SetCallback(callbacks::SkinUpdate);
		id_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&id_aug);

		stattrak_aug.setup(XOR("stattrak"), XOR("stattrak_aug"));
		stattrak_aug.SetCallback(callbacks::SkinUpdate);
		stattrak_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&stattrak_aug);

		quality_aug.setup(XOR("quality"), XOR("quality_aug"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_aug.SetCallback(callbacks::SkinUpdate);
		quality_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&quality_aug);

		seed_aug.setup(XOR("seed"), XOR("seed_aug"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_aug.SetCallback(callbacks::SkinUpdate);
		seed_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&seed_aug);

		id_awp.setup(XOR("paintkit id"), XOR("id_awp"), 3);
		id_awp.SetCallback(callbacks::SkinUpdate);
		id_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&id_awp);

		stattrak_awp.setup(XOR("stattrak"), XOR("stattrak_awp"));
		stattrak_awp.SetCallback(callbacks::SkinUpdate);
		stattrak_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&stattrak_awp);

		quality_awp.setup(XOR("quality"), XOR("quality_awp"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_awp.SetCallback(callbacks::SkinUpdate);
		quality_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&quality_awp);

		seed_awp.setup(XOR("seed"), XOR("seed_awp"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_awp.SetCallback(callbacks::SkinUpdate);
		seed_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&seed_awp);

		id_famas.setup(XOR("paintkit id"), XOR("id_famas"), 3);
		id_famas.SetCallback(callbacks::SkinUpdate);
		id_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&id_famas);

		stattrak_famas.setup(XOR("stattrak"), XOR("stattrak_famas"));
		stattrak_famas.SetCallback(callbacks::SkinUpdate);
		stattrak_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&stattrak_famas);

		quality_famas.setup(XOR("quality"), XOR("quality_famas"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_famas.SetCallback(callbacks::SkinUpdate);
		quality_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&quality_famas);

		seed_famas.setup(XOR("seed"), XOR("seed_famas"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_famas.SetCallback(callbacks::SkinUpdate);
		seed_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&seed_famas);

		id_g3sg1.setup(XOR("paintkit id"), XOR("id_g3sg1"), 3);
		id_g3sg1.SetCallback(callbacks::SkinUpdate);
		id_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&id_g3sg1);

		stattrak_g3sg1.setup(XOR("stattrak"), XOR("stattrak_g3sg1"));
		stattrak_g3sg1.SetCallback(callbacks::SkinUpdate);
		stattrak_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&stattrak_g3sg1);

		quality_g3sg1.setup(XOR("quality"), XOR("quality_g3sg1"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_g3sg1.SetCallback(callbacks::SkinUpdate);
		quality_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&quality_g3sg1);

		seed_g3sg1.setup(XOR("seed"), XOR("seed_g3sg1"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_g3sg1.SetCallback(callbacks::SkinUpdate);
		seed_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&seed_g3sg1);

		id_galil.setup(XOR("paintkit id"), XOR("id_galil"), 3);
		id_galil.SetCallback(callbacks::SkinUpdate);
		id_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&id_galil);

		stattrak_galil.setup(XOR("stattrak"), XOR("stattrak_galil"));
		stattrak_galil.SetCallback(callbacks::SkinUpdate);
		stattrak_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&stattrak_galil);

		quality_galil.setup(XOR("quality"), XOR("quality_galil"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_galil.SetCallback(callbacks::SkinUpdate);
		quality_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&quality_galil);

		seed_galil.setup(XOR("seed"), XOR("seed_galil"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_galil.SetCallback(callbacks::SkinUpdate);
		seed_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&seed_galil);

		id_m249.setup(XOR("paintkit id"), XOR("id_m249"), 3);
		id_m249.SetCallback(callbacks::SkinUpdate);
		id_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&id_m249);

		stattrak_m249.setup(XOR("stattrak"), XOR("stattrak_m249"));
		stattrak_m249.SetCallback(callbacks::SkinUpdate);
		stattrak_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&stattrak_m249);

		quality_m249.setup(XOR("quality"), XOR("quality_m249"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m249.SetCallback(callbacks::SkinUpdate);
		quality_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&quality_m249);

		seed_m249.setup(XOR("seed"), XOR("seed_m249"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m249.SetCallback(callbacks::SkinUpdate);
		seed_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&seed_m249);

		id_m4a4.setup(XOR("paintkit id"), XOR("id_m4a4"), 3);
		id_m4a4.SetCallback(callbacks::SkinUpdate);
		id_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&id_m4a4);

		stattrak_m4a4.setup(XOR("stattrak"), XOR("stattrak_m4a4"));
		stattrak_m4a4.SetCallback(callbacks::SkinUpdate);
		stattrak_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&stattrak_m4a4);

		quality_m4a4.setup(XOR("quality"), XOR("quality_m4a4"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m4a4.SetCallback(callbacks::SkinUpdate);
		quality_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&quality_m4a4);

		seed_m4a4.setup(XOR("seed"), XOR("seed_m4a4"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m4a4.SetCallback(callbacks::SkinUpdate);
		seed_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&seed_m4a4);

		id_mac10.setup(XOR("paintkit id"), XOR("id_mac10"), 3);
		id_mac10.SetCallback(callbacks::SkinUpdate);
		id_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&id_mac10);

		stattrak_mac10.setup(XOR("stattrak"), XOR("stattrak_mac10"));
		stattrak_mac10.SetCallback(callbacks::SkinUpdate);
		stattrak_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&stattrak_mac10);

		quality_mac10.setup(XOR("quality"), XOR("quality_mac10"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mac10.SetCallback(callbacks::SkinUpdate);
		quality_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&quality_mac10);

		seed_mac10.setup(XOR("seed"), XOR("seed_mac10"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mac10.SetCallback(callbacks::SkinUpdate);
		seed_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&seed_mac10);

		id_p90.setup(XOR("paintkit id"), XOR("id_p90"), 3);
		id_p90.SetCallback(callbacks::SkinUpdate);
		id_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&id_p90);

		stattrak_p90.setup(XOR("stattrak"), XOR("stattrak_p90"));
		stattrak_p90.SetCallback(callbacks::SkinUpdate);
		stattrak_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&stattrak_p90);

		quality_p90.setup(XOR("quality"), XOR("quality_p90"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_p90.SetCallback(callbacks::SkinUpdate);
		quality_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&quality_p90);

		seed_p90.setup(XOR("seed"), XOR("seed_p90"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_p90.SetCallback(callbacks::SkinUpdate);
		seed_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&seed_p90);

		id_ump45.setup(XOR("paintkit id"), XOR("id_ump45"), 3);
		id_ump45.SetCallback(callbacks::SkinUpdate);
		id_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&id_ump45);

		stattrak_ump45.setup(XOR("stattrak"), XOR("stattrak_ump45"));
		stattrak_ump45.SetCallback(callbacks::SkinUpdate);
		stattrak_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&stattrak_ump45);

		quality_ump45.setup(XOR("quality"), XOR("quality_ump45"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_ump45.SetCallback(callbacks::SkinUpdate);
		quality_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&quality_ump45);

		seed_ump45.setup(XOR("seed"), XOR("seed_ump45"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_ump45.SetCallback(callbacks::SkinUpdate);
		seed_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&seed_ump45);

		id_xm1014.setup(XOR("paintkit id"), XOR("id_xm1014"), 3);
		id_xm1014.SetCallback(callbacks::SkinUpdate);
		id_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&id_xm1014);

		stattrak_xm1014.setup(XOR("stattrak"), XOR("stattrak_xm1014"));
		stattrak_xm1014.SetCallback(callbacks::SkinUpdate);
		stattrak_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&stattrak_xm1014);

		quality_xm1014.setup(XOR("quality"), XOR("quality_xm1014"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_xm1014.SetCallback(callbacks::SkinUpdate);
		quality_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&quality_xm1014);

		seed_xm1014.setup(XOR("seed"), XOR("seed_xm1014"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_xm1014.SetCallback(callbacks::SkinUpdate);
		seed_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&seed_xm1014);

		id_bizon.setup(XOR("paintkit id"), XOR("id_bizon"), 3);
		id_bizon.SetCallback(callbacks::SkinUpdate);
		id_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&id_bizon);

		stattrak_bizon.setup(XOR("stattrak"), XOR("stattrak_bizon"));
		stattrak_bizon.SetCallback(callbacks::SkinUpdate);
		stattrak_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&stattrak_bizon);

		quality_bizon.setup(XOR("quality"), XOR("quality_bizon"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_bizon.SetCallback(callbacks::SkinUpdate);
		quality_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&quality_bizon);

		seed_bizon.setup(XOR("seed"), XOR("seed_bizon"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_bizon.SetCallback(callbacks::SkinUpdate);
		seed_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&seed_bizon);

		id_mag7.setup(XOR("paintkit id"), XOR("id_mag7"), 3);
		id_mag7.SetCallback(callbacks::SkinUpdate);
		id_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&id_mag7);

		stattrak_mag7.setup(XOR("stattrak"), XOR("stattrak_mag7"));
		stattrak_mag7.SetCallback(callbacks::SkinUpdate);
		stattrak_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&stattrak_mag7);

		quality_mag7.setup(XOR("quality"), XOR("quality_mag7"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mag7.SetCallback(callbacks::SkinUpdate);
		quality_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&quality_mag7);

		seed_mag7.setup(XOR("seed"), XOR("seed_mag7"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mag7.SetCallback(callbacks::SkinUpdate);
		seed_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&seed_mag7);

		id_negev.setup(XOR("paintkit id"), XOR("id_negev"), 3);
		id_negev.SetCallback(callbacks::SkinUpdate);
		id_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&id_negev);

		stattrak_negev.setup(XOR("stattrak"), XOR("stattrak_negev"));
		stattrak_negev.SetCallback(callbacks::SkinUpdate);
		stattrak_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&stattrak_negev);

		quality_negev.setup(XOR("quality"), XOR("quality_negev"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_negev.SetCallback(callbacks::SkinUpdate);
		quality_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&quality_negev);

		seed_negev.setup(XOR("seed"), XOR("seed_negev"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_negev.SetCallback(callbacks::SkinUpdate);
		seed_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&seed_negev);

		id_sawedoff.setup(XOR("paintkit id"), XOR("id_sawedoff"), 3);
		id_sawedoff.SetCallback(callbacks::SkinUpdate);
		id_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&id_sawedoff);

		stattrak_sawedoff.setup(XOR("stattrak"), XOR("stattrak_sawedoff"));
		stattrak_sawedoff.SetCallback(callbacks::SkinUpdate);
		stattrak_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&stattrak_sawedoff);

		quality_sawedoff.setup(XOR("quality"), XOR("quality_sawedoff"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_sawedoff.SetCallback(callbacks::SkinUpdate);
		quality_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&quality_sawedoff);

		seed_sawedoff.setup(XOR("seed"), XOR("seed_sawedoff"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_sawedoff.SetCallback(callbacks::SkinUpdate);
		seed_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&seed_sawedoff);

		id_tec9.setup(XOR("paintkit id"), XOR("id_tec9"), 3);
		id_tec9.SetCallback(callbacks::SkinUpdate);
		id_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&id_tec9);

		stattrak_tec9.setup(XOR("stattrak"), XOR("stattrak_tec9"));
		stattrak_tec9.SetCallback(callbacks::SkinUpdate);
		stattrak_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&stattrak_tec9);

		quality_tec9.setup(XOR("quality"), XOR("quality_tec9"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_tec9.SetCallback(callbacks::SkinUpdate);
		quality_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&quality_tec9);

		seed_tec9.setup(XOR("seed"), XOR("seed_tec9"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_tec9.SetCallback(callbacks::SkinUpdate);
		seed_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&seed_tec9);

		id_p2000.setup(XOR("paintkit id"), XOR("id_p2000"), 3);
		id_p2000.SetCallback(callbacks::SkinUpdate);
		id_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&id_p2000);

		stattrak_p2000.setup(XOR("stattrak"), XOR("stattrak_p2000"));
		stattrak_p2000.SetCallback(callbacks::SkinUpdate);
		stattrak_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&stattrak_p2000);

		quality_p2000.setup(XOR("quality"), XOR("quality_p2000"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_p2000.SetCallback(callbacks::SkinUpdate);
		quality_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&quality_p2000);

		seed_p2000.setup(XOR("seed"), XOR("seed_p2000"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_p2000.SetCallback(callbacks::SkinUpdate);
		seed_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&seed_p2000);

		id_mp7.setup(XOR("paintkit id"), XOR("id_mp7"), 3);
		id_mp7.SetCallback(callbacks::SkinUpdate);
		id_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&id_mp7);

		stattrak_mp7.setup(XOR("stattrak"), XOR("stattrak_mp7"));
		stattrak_mp7.SetCallback(callbacks::SkinUpdate);
		stattrak_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&stattrak_mp7);

		quality_mp7.setup(XOR("quality"), XOR("quality_mp7"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mp7.SetCallback(callbacks::SkinUpdate);
		quality_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&quality_mp7);

		seed_mp7.setup(XOR("seed"), XOR("seed_mp7"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mp7.SetCallback(callbacks::SkinUpdate);
		seed_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&seed_mp7);

		id_mp9.setup(XOR("paintkit id"), XOR("id_mp9"), 3);
		id_mp9.SetCallback(callbacks::SkinUpdate);
		id_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&id_mp9);

		stattrak_mp9.setup(XOR("stattrak"), XOR("stattrak_mp9"));
		stattrak_mp9.SetCallback(callbacks::SkinUpdate);
		stattrak_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&stattrak_mp9);

		quality_mp9.setup(XOR("quality"), XOR("quality_mp9"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mp9.SetCallback(callbacks::SkinUpdate);
		quality_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&quality_mp9);

		seed_mp9.setup(XOR("seed"), XOR("seed_mp9"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mp9.SetCallback(callbacks::SkinUpdate);
		seed_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&seed_mp9);

		id_nova.setup(XOR("paintkit id"), XOR("id_nova"), 3);
		id_nova.SetCallback(callbacks::SkinUpdate);
		id_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&id_nova);

		stattrak_nova.setup(XOR("stattrak"), XOR("stattrak_nova"));
		stattrak_nova.SetCallback(callbacks::SkinUpdate);
		stattrak_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&stattrak_nova);

		quality_nova.setup(XOR("quality"), XOR("quality_nova"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_nova.SetCallback(callbacks::SkinUpdate);
		quality_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&quality_nova);

		seed_nova.setup(XOR("seed"), XOR("seed_nova"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_nova.SetCallback(callbacks::SkinUpdate);
		seed_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&seed_nova);

		id_p250.setup(XOR("paintkit id"), XOR("id_p250"), 3);
		id_p250.SetCallback(callbacks::SkinUpdate);
		id_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&id_p250);

		stattrak_p250.setup(XOR("stattrak"), XOR("stattrak_p250"));
		stattrak_p250.SetCallback(callbacks::SkinUpdate);
		stattrak_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&stattrak_p250);

		quality_p250.setup(XOR("quality"), XOR("quality_p250"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_p250.SetCallback(callbacks::SkinUpdate);
		quality_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&quality_p250);

		seed_p250.setup(XOR("seed"), XOR("seed_p250"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_p250.SetCallback(callbacks::SkinUpdate);
		seed_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&seed_p250);

		id_scar20.setup(XOR("paintkit id"), XOR("id_scar20"), 3);
		id_scar20.SetCallback(callbacks::SkinUpdate);
		id_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&id_scar20);

		stattrak_scar20.setup(XOR("stattrak"), XOR("stattrak_scar20"));
		stattrak_scar20.SetCallback(callbacks::SkinUpdate);
		stattrak_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&stattrak_scar20);

		quality_scar20.setup(XOR("quality"), XOR("quality_scar20"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_scar20.SetCallback(callbacks::SkinUpdate);
		quality_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&quality_scar20);

		seed_scar20.setup(XOR("seed"), XOR("seed_scar20"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_scar20.SetCallback(callbacks::SkinUpdate);
		seed_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&seed_scar20);

		id_sg553.setup(XOR("paintkit id"), XOR("id_sg553"), 3);
		id_sg553.SetCallback(callbacks::SkinUpdate);
		id_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&id_sg553);

		stattrak_sg553.setup(XOR("stattrak"), XOR("stattrak_sg553"));
		stattrak_sg553.SetCallback(callbacks::SkinUpdate);
		stattrak_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&stattrak_sg553);

		quality_sg553.setup(XOR("quality"), XOR("quality_sg553"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_sg553.SetCallback(callbacks::SkinUpdate);
		quality_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&quality_sg553);

		seed_sg553.setup(XOR("seed"), XOR("seed_sg553"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_sg553.SetCallback(callbacks::SkinUpdate);
		seed_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&seed_sg553);

		id_ssg08.setup(XOR("paintkit id"), XOR("id_ssg08"), 3);
		id_ssg08.SetCallback(callbacks::SkinUpdate);
		id_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&id_ssg08);

		stattrak_ssg08.setup(XOR("stattrak"), XOR("stattrak_ssg08"));
		stattrak_ssg08.SetCallback(callbacks::SkinUpdate);
		stattrak_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&stattrak_ssg08);

		quality_ssg08.setup(XOR("quality"), XOR("quality_ssg08"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_ssg08.SetCallback(callbacks::SkinUpdate);
		quality_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&quality_ssg08);

		seed_ssg08.setup(XOR("seed"), XOR("seed_ssg08"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_ssg08.SetCallback(callbacks::SkinUpdate);
		seed_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&seed_ssg08);

		id_m4a1s.setup(XOR("paintkit id"), XOR("id_m4a1s"), 3);
		id_m4a1s.SetCallback(callbacks::SkinUpdate);
		id_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&id_m4a1s);

		stattrak_m4a1s.setup(XOR("stattrak"), XOR("stattrak_m4a1s"));
		stattrak_m4a1s.SetCallback(callbacks::SkinUpdate);
		stattrak_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&stattrak_m4a1s);

		quality_m4a1s.setup(XOR("quality"), XOR("quality_m4a1s"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m4a1s.SetCallback(callbacks::SkinUpdate);
		quality_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&quality_m4a1s);

		seed_m4a1s.setup(XOR("seed"), XOR("seed_m4a1s"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m4a1s.SetCallback(callbacks::SkinUpdate);
		seed_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&seed_m4a1s);

		id_usps.setup(XOR("paintkit id"), XOR("id_usps"), 3);
		id_usps.SetCallback(callbacks::SkinUpdate);
		id_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&id_usps);

		stattrak_usps.setup(XOR("stattrak"), XOR("stattrak_usps"));
		stattrak_usps.SetCallback(callbacks::SkinUpdate);
		stattrak_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&stattrak_usps);

		quality_usps.setup(XOR("quality"), XOR("quality_usps"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_usps.SetCallback(callbacks::SkinUpdate);
		quality_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&quality_usps);

		seed_usps.setup(XOR("seed"), XOR("seed_usps"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_usps.SetCallback(callbacks::SkinUpdate);
		seed_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&seed_usps);

		id_cz75a.setup(XOR("paintkit id"), XOR("id_cz75a"), 3);
		id_cz75a.SetCallback(callbacks::SkinUpdate);
		id_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&id_cz75a);

		stattrak_cz75a.setup(XOR("stattrak"), XOR("stattrak_cz75a"));
		stattrak_cz75a.SetCallback(callbacks::SkinUpdate);
		stattrak_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&stattrak_cz75a);

		quality_cz75a.setup(XOR("quality"), XOR("quality_cz75a"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_cz75a.SetCallback(callbacks::SkinUpdate);
		quality_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&quality_cz75a);

		seed_cz75a.setup(XOR("seed"), XOR("seed_cz75a"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_cz75a.SetCallback(callbacks::SkinUpdate);
		seed_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&seed_cz75a);

		id_revolver.setup(XOR("paintkit id"), XOR("id_revolver"), 3);
		id_revolver.SetCallback(callbacks::SkinUpdate);
		id_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&id_revolver);

		stattrak_revolver.setup(XOR("stattrak"), XOR("stattrak_revolver"));
		stattrak_revolver.SetCallback(callbacks::SkinUpdate);
		stattrak_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&stattrak_revolver);

		quality_revolver.setup(XOR("quality"), XOR("quality_revolver"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_revolver.SetCallback(callbacks::SkinUpdate);
		quality_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&quality_revolver);

		seed_revolver.setup(XOR("seed"), XOR("seed_revolver"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_revolver.SetCallback(callbacks::SkinUpdate);
		seed_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&seed_revolver);

		id_bayonet.setup(XOR("paintkit id"), XOR("id_bayonet"), 3);
		id_bayonet.SetCallback(callbacks::SkinUpdate);
		id_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&id_bayonet);

		stattrak_bayonet.setup(XOR("stattrak"), XOR("stattrak_bayonet"));
		stattrak_bayonet.SetCallback(callbacks::SkinUpdate);
		stattrak_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&stattrak_bayonet);

		quality_bayonet.setup(XOR("quality"), XOR("quality_bayonet"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_bayonet.SetCallback(callbacks::SkinUpdate);
		quality_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&quality_bayonet);

		seed_bayonet.setup(XOR("seed"), XOR("seed_bayonet"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_bayonet.SetCallback(callbacks::SkinUpdate);
		seed_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&seed_bayonet);

		id_flip.setup(XOR("paintkit id"), XOR("id_flip"), 3);
		id_flip.SetCallback(callbacks::SkinUpdate);
		id_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&id_flip);

		stattrak_flip.setup(XOR("stattrak"), XOR("stattrak_flip"));
		stattrak_flip.SetCallback(callbacks::SkinUpdate);
		stattrak_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&stattrak_flip);

		quality_flip.setup(XOR("quality"), XOR("quality_flip"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_flip.SetCallback(callbacks::SkinUpdate);
		quality_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&quality_flip);

		seed_flip.setup(XOR("seed"), XOR("seed_flip"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_flip.SetCallback(callbacks::SkinUpdate);
		seed_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&seed_flip);

		id_gut.setup(XOR("paintkit id"), XOR("id_gut"), 3);
		id_gut.SetCallback(callbacks::SkinUpdate);
		id_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&id_gut);

		stattrak_gut.setup(XOR("stattrak"), XOR("stattrak_gut"));
		stattrak_gut.SetCallback(callbacks::SkinUpdate);
		stattrak_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&stattrak_gut);

		quality_gut.setup(XOR("quality"), XOR("quality_gut"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_gut.SetCallback(callbacks::SkinUpdate);
		quality_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&quality_gut);

		seed_gut.setup(XOR("seed"), XOR("seed_gut"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_gut.SetCallback(callbacks::SkinUpdate);
		seed_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&seed_gut);

		id_karambit.setup(XOR("paintkit id"), XOR("id_karambit"), 3);
		id_karambit.SetCallback(callbacks::SkinUpdate);
		id_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&id_karambit);

		stattrak_karambit.setup(XOR("stattrak"), XOR("stattrak_karambit"));
		stattrak_karambit.SetCallback(callbacks::SkinUpdate);
		stattrak_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&stattrak_karambit);

		quality_karambit.setup(XOR("quality"), XOR("quality_karambit"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_karambit.SetCallback(callbacks::SkinUpdate);
		quality_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&quality_karambit);

		seed_karambit.setup(XOR("seed"), XOR("seed_karambit"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_karambit.SetCallback(callbacks::SkinUpdate);
		seed_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&seed_karambit);

		id_m9.setup(XOR("paintkit id"), XOR("id_m9"), 3);
		id_m9.SetCallback(callbacks::SkinUpdate);
		id_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&id_m9);

		stattrak_m9.setup(XOR("stattrak"), XOR("stattrak_m9"));
		stattrak_m9.SetCallback(callbacks::SkinUpdate);
		stattrak_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&stattrak_m9);

		quality_m9.setup(XOR("quality"), XOR("quality_m9"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m9.SetCallback(callbacks::SkinUpdate);
		quality_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&quality_m9);

		seed_m9.setup(XOR("seed"), XOR("seed_m9"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m9.SetCallback(callbacks::SkinUpdate);
		seed_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&seed_m9);

		id_huntsman.setup(XOR("paintkit id"), XOR("id_huntsman"), 3);
		id_huntsman.SetCallback(callbacks::SkinUpdate);
		id_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&id_huntsman);

		stattrak_huntsman.setup(XOR("stattrak"), XOR("stattrak_huntsman"));
		stattrak_huntsman.SetCallback(callbacks::SkinUpdate);
		stattrak_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&stattrak_huntsman);

		quality_huntsman.setup(XOR("quality"), XOR("quality_huntsman"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_huntsman.SetCallback(callbacks::SkinUpdate);
		quality_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&quality_huntsman);

		seed_huntsman.setup(XOR("seed"), XOR("seed_huntsman"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_huntsman.SetCallback(callbacks::SkinUpdate);
		seed_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&seed_huntsman);

		id_falchion.setup(XOR("paintkit id"), XOR("id_falchion"), 3);
		id_falchion.SetCallback(callbacks::SkinUpdate);
		id_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&id_falchion);

		stattrak_falchion.setup(XOR("stattrak"), XOR("stattrak_falchion"));
		stattrak_falchion.SetCallback(callbacks::SkinUpdate);
		stattrak_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&stattrak_falchion);

		quality_falchion.setup(XOR("quality"), XOR("quality_falchion"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_falchion.SetCallback(callbacks::SkinUpdate);
		quality_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&quality_falchion);

		seed_falchion.setup(XOR("seed"), XOR("seed_falchion"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_falchion.SetCallback(callbacks::SkinUpdate);
		seed_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&seed_falchion);

		id_bowie.setup(XOR("paintkit id"), XOR("id_bowie"), 3);
		id_bowie.SetCallback(callbacks::SkinUpdate);
		id_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&id_bowie);

		stattrak_bowie.setup(XOR("stattrak"), XOR("stattrak_bowie"));
		stattrak_bowie.SetCallback(callbacks::SkinUpdate);
		stattrak_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&stattrak_bowie);

		quality_bowie.setup(XOR("quality"), XOR("quality_bowie"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_bowie.SetCallback(callbacks::SkinUpdate);
		quality_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&quality_bowie);

		seed_bowie.setup(XOR("seed"), XOR("seed_bowie"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_bowie.SetCallback(callbacks::SkinUpdate);
		seed_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&seed_bowie);

		id_butterfly.setup(XOR("paintkit id"), XOR("id_butterfly"), 3);
		id_butterfly.SetCallback(callbacks::SkinUpdate);
		id_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&id_butterfly);

		stattrak_butterfly.setup(XOR("stattrak"), XOR("stattrak_butterfly"));
		stattrak_butterfly.SetCallback(callbacks::SkinUpdate);
		stattrak_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&stattrak_butterfly);

		quality_butterfly.setup(XOR("quality"), XOR("quality_butterfly"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_butterfly.SetCallback(callbacks::SkinUpdate);
		quality_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&quality_butterfly);

		seed_butterfly.setup(XOR("seed"), XOR("seed_butterfly"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_butterfly.SetCallback(callbacks::SkinUpdate);
		seed_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&seed_butterfly);

		id_daggers.setup(XOR("paintkit id"), XOR("id_daggers"), 3);
		id_daggers.SetCallback(callbacks::SkinUpdate);
		id_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&id_daggers);

		stattrak_daggers.setup(XOR("stattrak"), XOR("stattrak_daggers"));
		stattrak_daggers.SetCallback(callbacks::SkinUpdate);
		stattrak_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&stattrak_daggers);

		quality_daggers.setup(XOR("quality"), XOR("quality_daggers"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_daggers.SetCallback(callbacks::SkinUpdate);
		quality_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&quality_daggers);

		seed_daggers.setup(XOR("seed"), XOR("seed_daggers"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_daggers.SetCallback(callbacks::SkinUpdate);
		seed_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&seed_daggers);

		// col 2.
		knife.setup(XOR("knife model"), XOR("skins_knife_model"), { XOR("off"), XOR("bayonet"), XOR("bowie"), XOR("butterfly"), XOR("falchion"), XOR("flip"), XOR("gut"), XOR("huntsman"), XOR("karambit"), XOR("m9 bayonet"), XOR("daggers") });
		knife.SetCallback(callbacks::SkinUpdate);
		RegisterElement(&knife, 1);

		glove.setup(XOR("glove model"), XOR("skins_glove_model"), { XOR("off"), XOR("bloodhound"), XOR("sport"), XOR("driver"), XOR("handwraps"), XOR("moto"), XOR("specialist") });
		glove.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&glove, 1);

		glove_id.setup(XOR("glove paintkit id"), XOR("skins_glove_id"), 2);
		glove_id.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&glove_id, 1);
	}
};

class MiscTab : public Tab {
public:
	// col1.
	MultiDropdown buy1;
	MultiDropdown buy2;
	MultiDropdown buy3;
	MultiDropdown notifications;
	Keybind       last_tick_defuse;
	Checkbox	  fake_latency_always;
	Keybind       fake_latency;
	Slider		  fake_latency_amt;
	Slider		  sv_maxunlag;

	// col2.
	Checkbox skyboxchange;
	Dropdown skybox;
	Checkbox autoaccept;
	Checkbox unlock;
	MultiDropdown hitmarkers;
	Dropdown hitmarker_style;
	Dropdown hitsound;
	Checkbox bullet_impacts;
	Checkbox ragdoll_force;
	Checkbox killfeed;
	Checkbox clantag;
	Checkbox slide_walk;
	Checkbox watermark;
	Checkbox ranks;
	Button   HiddenCvar;
	Button   ForceUpdate;

public:
	void init() {
		SetTitle(XOR("misc"));

		buy1.setup(XOR("auto buy items"), XOR("auto_buy1"),
			{
				XOR("galilar"),
				XOR("famas"),
				XOR("ak47"),
				XOR("m4a1"),
				XOR("m4a1_silencer"),
				XOR("ssg08"),
				XOR("aug"),
				XOR("sg556"),
				XOR("awp"),
				XOR("scar20"),
				XOR("g3sg1"),
				XOR("nova"),
				XOR("xm1014"),
				XOR("mag7"),
				XOR("m249"),
				XOR("negev"),
				XOR("mac10"),
				XOR("mp9"),
				XOR("mp7"),
				XOR("ump45"),
				XOR("p90"),
				XOR("bizon"),
			});
		RegisterElement(&buy1);

		buy2.setup("", XOR("auto_buy2"),
			{
				XOR("glock"),
				XOR("hkp2000"),
				XOR("usp_silencer"),
				XOR("elite"),
				XOR("p250"),
				XOR("tec9"),
				XOR("fn57"),
				XOR("deagle"),
			}, false);
		RegisterElement(&buy2);

		buy3.setup("", XOR("auto_buy3"),
			{
				XOR("vest"),
				XOR("vesthelm"),
				XOR("taser"),
				XOR("defuser"),
				XOR("heavyarmor"),
				XOR("molotov"),
				XOR("incgrenade"),
				XOR("decoy"),
				XOR("flashbang"),
				XOR("hegrenade"),
				XOR("smokegrenade"),
			}, false);
		RegisterElement(&buy3);

		notifications.setup(XOR("notifications"), XOR("notifications"), { XOR("matchmaking"), XOR("damage"), XOR("harmed"), XOR("purchases"), XOR("bomb"), XOR("defuse") });
		RegisterElement(&notifications);

		last_tick_defuse.setup(XOR("last tick defuse"), XOR("last_tick_defuse"));
		RegisterElement(&last_tick_defuse);

		fake_latency_always.setup(XOR("fake latency always on"), XOR("fake_latency_always"));
		RegisterElement(&fake_latency_always);

		fake_latency.setup(XOR("fake latency"), XOR("fake_latency"));
		fake_latency.SetToggleCallback(callbacks::ToggleFakeLatency);
		RegisterElement(&fake_latency);

		fake_latency_amt.setup("", XOR("fake_latency_amt"), 50.f, 1000.f, false, 0, 200.f, 50.f, XOR(L"ms"));
		RegisterElement(&fake_latency_amt);

		sv_maxunlag.setup("sv_maxunlag", XOR("sv_maxunlag"), 0.f, 1.f, true, 3, 1.f, 0.05f, XOR(L""));
		sv_maxunlag.SetCallback(callbacks::UpdateMaxUnlag);
		RegisterElement(&sv_maxunlag);

		// col2.
		skyboxchange.setup(XOR("skybox change"), XOR("skyboxchange"));
		RegisterElement(&skyboxchange, 1);

		//		mode.setup(XOR("safety mode"), XOR("mode"), { XOR("matchmaking"), XOR("no-spread") });
		//		RegisterElement(&mode, 1);

		skybox.setup(XOR("skyboxes"), XOR("skybox"), { XOR("Tibet"),XOR("Embassy"),XOR("Italy"),XOR("Daylight"),XOR("Cloudy"),XOR("Night 1"),XOR("Night 2"),XOR("Night Flat"),XOR("Day HD"),XOR("Day"),XOR("Rural"),XOR("Vertigo HD"),XOR("Vertigo Blue HD"),XOR("Vertigo"),XOR("Vietnam"),XOR("Dusty Sky"),XOR("Jungle"),XOR("Nuke"),XOR("Office") });
		skybox.AddShowCallback(callbacks::IsSkyBoxChange);

		RegisterElement(&skybox, 1);

		autoaccept.setup(XOR("auto-accept matchmaking"), XOR("autoaccept"));
		RegisterElement(&autoaccept, 1);

		unlock.setup(XOR("unlock inventory in-game"), XOR("unlock_inventory"));
		RegisterElement(&unlock, 1);

		hitmarkers.setup(XOR("hitmarkers"), XOR("hitmarkers"), { XOR("world"), XOR("screen") });
		RegisterElement(&hitmarkers, 1);

		hitmarker_style.setup(XOR("hitmarker style"), XOR("hitmarker_style"), { XOR("animated"), XOR("static") });
		RegisterElement(&hitmarker_style, 1);

		hitsound.setup(XOR("hitsound"), XOR("hitsound"), { XOR("off"), XOR("1"), XOR("2"), XOR("skeet") });
		RegisterElement(&hitsound, 1);

		bullet_impacts.setup(XOR("draw bullet impacts"), XOR("bullet_impacts"));
		RegisterElement(&bullet_impacts, 1);

		ragdoll_force.setup(XOR("ragdoll force"), XOR("ragdoll_force"));
		RegisterElement(&ragdoll_force, 1);

		ranks.setup(XOR("reveal matchmaking ranks"), XOR("ranks"));
		RegisterElement(&ranks, 1);

		killfeed.setup(XOR("preserve killfeed"), XOR("killfeed"));
		killfeed.SetCallback(callbacks::ToggleKillfeed);
		RegisterElement(&killfeed, 1);

		clantag.setup(XOR("clan-tag spammer"), XOR("clantag"));
		RegisterElement(&clantag, 1);

		slide_walk.setup(XOR("slide walk"), XOR("slide_walk"));
		RegisterElement(&slide_walk, 1);

		watermark.setup(XOR("watermark enable"), XOR("watermark"));
		RegisterElement(&watermark, 1);

		HiddenCvar.setup(XOR("unlock hidden cvars"));
		HiddenCvar.SetCallback(callbacks::HiddenCvar);
		RegisterElement(&HiddenCvar, 1);

		ForceUpdate.setup(XOR("force update"));
		ForceUpdate.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&ForceUpdate, 1);
	}
};

class ConfigTab : public Tab {
public:
	Colorpicker menu_color;
	Slider offscreen_mode;
	Slider offscreen_mode1;
	Slider r_aspect_ratio;

	Dropdown mode;
	Dropdown config;
	Keybind  key1;
	Keybind  key2;
	Keybind  key3;
	Keybind  key4;
	Keybind  key5;
	Keybind  key6;
	Button   save;
	Button   load;

public:

	void init() {
		SetTitle(XOR("config"));

		menu_color.setup(XOR("menu color"), XOR("menu_color"), colors::burgundy, &g_gui.m_color);
		RegisterElement(&menu_color);

		offscreen_mode.setup(XOR("offscreen size"), XOR("offscreen_mode"), 20.f, 200.f, true, 0, 100.f, 1.f, XOR(L""));
		RegisterElement(&offscreen_mode);

		offscreen_mode1.setup(XOR("offscreen position"), XOR("offscreen_mode1"), 20.f, 200.f, true, 0, 200.f, 1.f, XOR(L""));
		RegisterElement(&offscreen_mode1);

		r_aspect_ratio.setup(XOR("r_aspectratio"), XOR("r_aspect_ratio"), 0.f, 2.f, true, 0, 0.f, 0.1f, XOR(L""));
		r_aspect_ratio.SetCallback(callbacks::r_aspect_ratio);
		RegisterElement(&r_aspect_ratio);

		mode.setup(XOR("safety mode"), XOR("mode"), { XOR("matchmaking"), XOR("no-spread") });
		RegisterElement(&mode, 1);

		config.setup(XOR("configuration"), XOR("cfg"), { XOR("1"), XOR("2"), XOR("3"), XOR("4"), XOR("5"), XOR("6") });
		config.RemoveFlags(ElementFlags::SAVE);
		RegisterElement(&config, 1);

		key1.setup(XOR("configuration key 1"), XOR("cfg_key1"));
		key1.RemoveFlags(ElementFlags::SAVE);
		key1.SetCallback(callbacks::SaveHotkeys);
		key1.AddShowCallback(callbacks::IsConfig1);
		key1.SetToggleCallback(callbacks::ConfigLoad1);
		RegisterElement(&key1, 1);

		key2.setup(XOR("configuration key 2"), XOR("cfg_key2"));
		key2.RemoveFlags(ElementFlags::SAVE);
		key2.SetCallback(callbacks::SaveHotkeys);
		key2.AddShowCallback(callbacks::IsConfig2);
		key2.SetToggleCallback(callbacks::ConfigLoad2);
		RegisterElement(&key2, 1);

		key3.setup(XOR("configuration key 3"), XOR("cfg_key3"));
		key3.RemoveFlags(ElementFlags::SAVE);
		key3.SetCallback(callbacks::SaveHotkeys);
		key3.AddShowCallback(callbacks::IsConfig3);
		key3.SetToggleCallback(callbacks::ConfigLoad3);
		RegisterElement(&key3, 1);

		key4.setup(XOR("configuration key 4"), XOR("cfg_key4"));
		key4.RemoveFlags(ElementFlags::SAVE);
		key4.SetCallback(callbacks::SaveHotkeys);
		key4.AddShowCallback(callbacks::IsConfig4);
		key4.SetToggleCallback(callbacks::ConfigLoad4);
		RegisterElement(&key4, 1);

		key5.setup(XOR("configuration key 5"), XOR("cfg_key5"));
		key5.RemoveFlags(ElementFlags::SAVE);
		key5.SetCallback(callbacks::SaveHotkeys);
		key5.AddShowCallback(callbacks::IsConfig5);
		key5.SetToggleCallback(callbacks::ConfigLoad5);
		RegisterElement(&key5, 1);

		key6.setup(XOR("configuration key 6"), XOR("cfg_key6"));
		key6.RemoveFlags(ElementFlags::SAVE);
		key6.SetCallback(callbacks::SaveHotkeys);
		key6.AddShowCallback(callbacks::IsConfig6);
		key6.SetToggleCallback(callbacks::ConfigLoad6);
		RegisterElement(&key6, 1);

		save.setup(XOR("save"));
		save.SetCallback(callbacks::ConfigSave);
		RegisterElement(&save, 1);

		load.setup(XOR("load"));
		load.SetCallback(callbacks::ConfigLoad);
		RegisterElement(&load, 1);
	}
};

class MainForm : public Form {
public:
	// aimbot.
	AimbotTab    aimbot;
	AntiAimTab   antiaim;

	// visuals.
	PlayersTab	 players;
	VisualsTab	 visuals;

	// misc.
	MovementTab  movement;
	SkinsTab     skins;
	MiscTab	     misc;
	ConfigTab	 config;

public:
	void init() {
		SetPosition(50, 50);
		SetSize(630, 700);

		// aim.
		RegisterTab(&aimbot);
		aimbot.init();

		RegisterTab(&antiaim);
		antiaim.init();

		// visuals.
		RegisterTab(&players);
		players.init();

		RegisterTab(&visuals);
		visuals.init();

		// misc.
		RegisterTab(&movement);
		movement.init();

		RegisterTab(&skins);
		skins.init();

		RegisterTab(&misc);
		misc.init();

		RegisterTab(&config);
		config.init();
	}
};

class Menu {
public:
	MainForm main;

public:
	void init() {

		Colorpicker::init();	// points here, so this was the second injection crash problem, moved here to fix bound bug, look at colorpicker init

		main.init();

		g_gui.RegisterForm(&main, VK_INSERT);
	}
};

extern Menu g_menu;