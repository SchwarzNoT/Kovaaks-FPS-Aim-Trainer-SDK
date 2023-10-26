#pragma once
#include "precomp.h"

std::string GetNameFromFName(int key);

std::vector<AActor> getAActors(uintptr_t levelBaseAddress);

bool updateWorld();

bool updateLocalPlayer();

bool refreshPawns();

bool updateCameraCache();

void setPriorityTarget();

void aimbot(int vKey, float smoothX, float smoothY, AActor enemy);

float getDistance();

std::string getPlayerName(PieceChar pieceChar);

D3DMATRIX Matrix(FVector rot, FVector origin = FVector(0, 0, 0));


Vector3 WorldToScreen(Vector3 WorldLocation, CameraCacheEntry CameraCacheL);


void mainLoop();
