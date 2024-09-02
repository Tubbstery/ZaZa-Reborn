#pragma once

class AimPlayer;

class LagCompensation {
public:
	enum Lagfix_Modes : size_t {
		LAGCOMP_NOT_BROKEN,
		LAGCOMP_DONT_PREDICT,
		LAGCOMP_PREDICT,
		LAGCOMP_WAIT,
	};

public:
	void Update();
	int StartPrediction(AimPlayer* player);
	void AirAccelerate(LagRecord* record, ang_t angle, float fmove, float smove);
	void SimulateMovement(Player* pEntity, vec3_t& vecOrigin, vec3_t& vecVelocity, int& fFlags, bool bOnGround);
};

extern LagCompensation g_lagcomp;