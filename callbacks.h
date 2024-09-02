#pragma once

namespace callbacks {
	void SkinUpdate();
	void ForceFullUpdate();
	void ToggleThirdPerson();
	void ToggleFakeLatency();
	void ToggleKillfeed();
	void SaveHotkeys();
	void ConfigLoad1();
	void ConfigLoad2();
	void ConfigLoad3();
	void ConfigLoad4();
	void ConfigLoad5();
	void ConfigLoad6();
	void ConfigLoad();
	void HiddenCvar();
	void ConfigSave();
	void r_aspect_ratio();
	void UpdateMaxUnlag();

	void UpdateWhitelist();

	void ManualForward();
	void ManualLeft();
	void ManualRight();
	void ManualBack();

	void ToggleDamageOverride();
	bool IsForceBaimAfterMisses();

	bool IsDesyncEnabled();
	bool IsYawModifierJitter();
	bool IsYawModifierRotate();
	bool IsLbyBreakerOn();
	bool IsLbyBreakAndNotToLastMoveLol();
	bool IsRetardModeOn();

	bool IsMultipointOn();
	bool IsMultipointBodyOn();

	bool IsNightMode();
	bool IsNightCustom();
	bool IsAmbientLight();

	bool IsConfigMM();
	bool IsConfigNS();
	bool IsConfig1();
	bool IsConfig2();
	bool IsConfig3();
	bool IsConfig4();
	bool IsConfig5();
	bool IsConfig6();

	bool IsEnemyChams();
	bool IsFriendlyChams();
	bool IsLocalChams();
	bool IsBacktrackChams();
	bool IsWeaponChams();
	bool IsShotChams();
	bool IsAutoPeekChams();

	bool IsEnemyDoubleChams();
	bool IsFriendlyDoubleChams();
	bool IsLocalDoubleChams();
	bool IsHistoryDoubleChams();
	bool IsWeaponDoubleChams();
	bool IsShotDoubleChams();
	bool IsAutoPeekDoubleChams();

	bool IsTransparentProps();
	bool IsSkyBoxChange();

	// weapon cfgs.
	bool DEAGLE();
	bool ELITE();
	bool FIVESEVEN();
	bool GLOCK();
	bool AK47();
	bool AUG();
	bool AWP();
	bool FAMAS();
	bool G3SG1();
	bool GALIL();
	bool M249();
	bool M4A4();
	bool MAC10();
	bool P90();
	bool UMP45();
	bool XM1014();
	bool BIZON();
	bool MAG7();
	bool NEGEV();
	bool SAWEDOFF();
	bool TEC9();
	bool P2000();
	bool MP7();
	bool MP9();
	bool NOVA();
	bool P250();
	bool SCAR20();
	bool SG553();
	bool SSG08();
	bool M4A1S();
	bool USPS();
	bool CZ75A();
	bool REVOLVER();
	bool KNIFE_BAYONET();
	bool KNIFE_FLIP();
	bool KNIFE_GUT();
	bool KNIFE_KARAMBIT();
	bool KNIFE_M9_BAYONET();
	bool KNIFE_HUNTSMAN();
	bool KNIFE_FALCHION();
	bool KNIFE_BOWIE();
	bool KNIFE_BUTTERFLY();
	bool KNIFE_SHADOW_DAGGERS();
}