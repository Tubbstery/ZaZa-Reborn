#pragma once

class Resolver {
public:
	enum Modes : size_t {
		RESOLVE_STAND_LBY_DELTA = 0,
		RESOLVE_STAND_FAKE_JITTER,
		RESOLVE_STAND_FAKE_STATIC,
		RESOLVE_STAND_LBY_DELTA_FIRST_TICK,
		RESOLVE_STAND_STOPPED_MOVING,
		RESOLVE_BODY,
		RESOLVE_WALK,
		RESOLVE_NONE,
	};

	enum Fakes : size_t {
		FAKE_UNKNOWN = 0,
		FAKE_STATIC,
		FAKE_JITTER,
		FAKE_RANDOM,
	};

public:
	void FixOnshot(AnimationRecord* record, AimPlayer* data);
	void OnBodyUpdate(Player* player, float value);

	float AntiFreestand(AnimationRecord* record, std::vector<AdaptiveAngle> alternate_angles = {}, bool use_alternate_angles = false);

	bool IsYawSideways(Player* entity, float yaw);
	float GetAwayAngle(AnimationRecord* record);
	float SnapToNearestYaw(float yaw, std::vector<float> options);

	void DetectFake(AnimationRecord* record, AimPlayer* data);
	void ValidateAngle(AnimationRecord* record, AimPlayer* data);

	void LogShot(LagRecord* record, bool miss);

	float GetDeltaChance(AimPlayer* data, float delta);
	float GetMostAccurateLBYDelta(AimPlayer* data);

	void ResolvePlayer(AnimationRecord* record, bool& was_dormant);
	void ResolveStand(AnimationRecord* record, AimPlayer* data, bool fuckniggadormant);
	void ResolveWalk(AnimationRecord* record, AimPlayer* data);
	void ResolveAir(AnimationRecord* record, AimPlayer* data);
};

extern Resolver g_resolver;