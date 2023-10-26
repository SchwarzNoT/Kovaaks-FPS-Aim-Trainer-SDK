#include "engine.h"

uintptr_t UWorld;
uintptr_t Levels;
INT32 levelCount;
uintptr_t persistentLevel;
uintptr_t GameInstance;
uintptr_t GameState;
uintptr_t localPlayers;
uintptr_t localPlayer;
uintptr_t localPlayerController;
uintptr_t localPlayerState;
uintptr_t localPlayerPiece;

std::vector<AActor> targetPawns;
uintptr_t kovGameState;
uintptr_t playerArray;


AActor localPiece;
int currAimPiecIndex;


ImVec2 boxDimensions;

uintptr_t playerCamManager;
CameraCacheEntry cameraCache;

clock_t worldUpdateTimer = -4000;
clock_t localPlayerUpdateTimer = -4000;
clock_t pieceBaseUpdateTimer = -4000;
clock_t pieceUpdateTimer = -2000;


std::string GetNameFromFName(int key)
{
    unsigned int chunkOffset = (unsigned int)((int)(key) >> 16);
    unsigned short nameOffset = (unsigned short)key;
    
    UINT64 namePoolChunk = util::Read<UINT64>(GNames + ((chunkOffset + 2) * 8), sizeof(UINT64));
    UINT64 entryOffset = namePoolChunk + (unsigned long)(2 * nameOffset);

    INT16 nameEntry = util::Read<INT16>(entryOffset, sizeof(INT16));

    UINT16 nameLength = nameEntry >> 6;

    char* buf = new char[nameLength];
    SecureZeroMemory(buf, nameLength);

    ReadProcessMemory(hProc, (BYTE*)(entryOffset + 2), buf, nameLength, NULL);

    std::string name = buf;
    delete[] buf;
    name = name.substr(0, nameLength);
    return name;
}

std::vector<AActor> getAActors(uintptr_t levelBaseAddress) {

    uintptr_t pAActors = util::Read<uintptr_t>((levelBaseAddress + 0x98), sizeof(pAActors));

    int actorCount = util::Read<int>((levelBaseAddress + 0xA0), sizeof(actorCount));
    
    std::vector<AActor> AActors(actorCount);

    for (int i = 0; i < actorCount; i++) {

        AActors[i].baseAddress = util::Read<uintptr_t>((pAActors + (8 * i)), sizeof(AActors[i].baseAddress));

        AActors[i].FNameID = util::Read<INT64>((AActors[i].baseAddress + 0x18), sizeof(AActors[i].FNameID));

        AActors[i].Name = GetNameFromFName(AActors[i].FNameID);

        AActors[i].RootComponent.RelativeLocation = util::Read<Vector3>(((util::Read<uintptr_t>((AActors[i].baseAddress + offsets::AActor::RootComponent), sizeof(uintptr_t))) + offsets::USceneComponent::RelativeLocation), sizeof(AActors[i].RootComponent.RelativeLocation));


    }

    return AActors;

}

bool updateWorld() {
    

    uintptr_t buffer = UWorld;

    clock_t tempTime = clock();
    if (tempTime >= worldUpdateTimer + 4000) {

        UWorld = util::Read<uintptr_t>(GWorld + modBase, sizeof(UWorld));


        if (UWorld != buffer && UWorld != 0x0) {

            Levels = util::Read<uintptr_t>(UWorld + offsets::UWorld::Levels, sizeof(Levels));

            levelCount = util::Read<INT32>(UWorld + 0x140, sizeof(levelCount));

            persistentLevel = util::Read<uintptr_t>((UWorld + offsets::UWorld::PersistentLevel), sizeof(persistentLevel));

            GameInstance = util::Read<uintptr_t>((UWorld + offsets::UWorld::OwningGameInstance), sizeof(GameInstance));

            GameState = util::Read<uintptr_t>((UWorld + offsets::UWorld::GameState), sizeof(GameState));

            localPlayers = util::Read<uintptr_t>((GameInstance + 0x38), sizeof(localPlayers));

            localPlayer = util::Read<uintptr_t>(localPlayers, sizeof(localPlayer));



            worldUpdateTimer = tempTime;

            return true;

        }
       
            
       
    }
    return false;
}



bool updateLocalPlayer() {

    clock_t tempTime = clock();

    if (tempTime >= localPlayerUpdateTimer + 3000) {

        localPlayerController = util::Read<uintptr_t>(localPlayer + 0x30, sizeof(localPlayerController));

        localPlayerState = util::Read<uintptr_t>((localPlayerController + 0x228), sizeof(localPlayerState));

        localPlayerPiece = util::Read<uintptr_t>((localPlayerState + 0x280), sizeof(localPlayerPiece));

        localPlayerUpdateTimer = tempTime;
        return true;
    }
    return false;

}



bool updateCameraCache() {

    playerCamManager = util::Read<uintptr_t>(localPlayerController + 0x2B8, sizeof(playerCamManager));

    uintptr_t cameraCachePrivate = playerCamManager + 0x1AE0;

    cameraCache.POV = util::Read<FMinimalViewInfo>(cameraCachePrivate + 0x10, sizeof(FMinimalViewInfo));

    return true;

}

float getDistance(Vector3 localPos, Vector3 enemyPos) {


    float xDiff = (localPos.x - enemyPos.x);
    float yDiff = (localPos.y - enemyPos.y);
    float zDiff = (localPos.z - enemyPos.z);
  
    return sqrt((xDiff * xDiff) + (yDiff * yDiff) + (zDiff * zDiff));



}

float get2DDistance(Vector3 W2S, Vector3 screenCenter) {


    float xDiff = (W2S.x - screenCenter.x);
    float yDiff = (W2S.y - screenCenter.y);
    return sqrt((xDiff * xDiff) + (yDiff * yDiff));

}

std::string getPlayerName(PieceChar pieceChar) {

    uintptr_t buf = pieceChar.playerState;
    pieceChar.playerState = util::Read<uintptr_t>((pieceChar.baseAddress + 0x240), sizeof(pieceChar.playerState));

    if (buf == pieceChar.playerState) {
        return pieceChar.playerName;
    }

    int nameLength = util::Read<int>((pieceChar.playerState + 0x308), sizeof(nameLength));

    if (nameLength <= 0) {

        return pieceChar.playerName;
    }

    wchar_t* buffer = new wchar_t[nameLength*2];
    SecureZeroMemory(buffer, nameLength*2);

    uintptr_t nameAddr = util::Read<uintptr_t>((pieceChar.playerState + 0x300), sizeof(nameAddr));

    ReadProcessMemory(hProc, (BYTE*)(nameAddr), buffer, nameLength*2, NULL);

    std::wstring ws(buffer);
    std::string str(ws.begin(), ws.end());
    delete[] buffer;
    
    return str;



}

void aimbot(int vKey, float smoothX, float smoothY, AActor enemy) {
    
   

    if (GetAsyncKeyState(vKey)) {
 /*
     

        float atanYX;
        float asinXY;
       

      


            atanYX = atan2(enemy.RootComponent.RelativeLocation.y - localPiece.RootComponent.RelativeLocation.y, enemy.RootComponent.RelativeLocation.x - localPiece.RootComponent.RelativeLocation.x);
            asinXY = asin((enemy.RootComponent.RelativeLocation.z - localPiece.RootComponent.RelativeLocation.z) / getDistance(localPiece.RootComponent.RelativeLocation, enemy.RootComponent.RelativeLocation));

            */

        float currValY = util::Read<float>(localPlayerController + 0x288 + 0x4, sizeof(currValY));
        float currValX = util::Read<float>(localPlayerController + 0x288, sizeof(currValX));
        float atanYX;
        float asinXY;





        atanYX = atan2(enemy.RootComponent.RelativeLocation.y - cameraCache.POV.Location.y, enemy.RootComponent.RelativeLocation.x - cameraCache.POV.Location.x);
        asinXY = asin((enemy.RootComponent.RelativeLocation.z - cameraCache.POV.Location.z) / getDistance(cameraCache.POV.Location, enemy.RootComponent.RelativeLocation));

        

        float yChange = atanYX * (180.f / 3.1415926535897932384626433832795);
        float xChange = asinXY * (180.f / 3.1415926535897932384626433832795);


        float bufferY = yChange - currValY;

        float bufferX = xChange - currValX;


        if (abs(bufferY) >= 180.f) {

            bufferY = 360 - abs(bufferY);

        }
        if (abs(bufferX) >= 250) {

            bufferX = 360 - abs(bufferX);
        }


        util::Write<float>(localPlayerController + 0x420, bufferX/smoothX);
        util::Write<float>(localPlayerController + 0x420 + 0x4, bufferY/smoothY);
    }

}

D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
    float radPitch = (rot.x * float(3.1415926535897932384626433832795) / 180.f);
    float radYaw = (rot.y * float(3.1415926535897932384626433832795) / 180.f);
    float radRoll = (rot.z * float(3.1415926535897932384626433832795) / 180.f);

    float SP = sinf(radPitch);
    float CP = cosf(radPitch);
    float SY = sinf(radYaw);
    float CY = cosf(radYaw);
    float SR = sinf(radRoll);
    float CR = cosf(radRoll);

    D3DMATRIX matrix;
    matrix.m[0][0] = CP * CY;
    matrix.m[0][1] = CP * SY;
    matrix.m[0][2] = SP;
    matrix.m[0][3] = 0.f;

    matrix.m[1][0] = SR * SP * CY - CR * SY;
    matrix.m[1][1] = SR * SP * SY + CR * CY;
    matrix.m[1][2] = -SR * CP;
    matrix.m[1][3] = 0.f;

    matrix.m[2][0] = -(CR * SP * CY + SR * SY);
    matrix.m[2][1] = CY * SR - CR * SP * SY;
    matrix.m[2][2] = CR * CP;
    matrix.m[2][3] = 0.f;

    matrix.m[3][0] = origin.x;
    matrix.m[3][1] = origin.y;
    matrix.m[3][2] = origin.z;
    matrix.m[3][3] = 1.f;

    return matrix;
}


Vector3 WorldToScreen(Vector3 WorldLocation, CameraCacheEntry CameraCacheL)
{
    Vector3 Screenlocation = Vector3(0, 0, 0);

    auto POV = CameraCacheL.POV;

    Vector3 Rotation = POV.Rotation; // FRotator

    D3DMATRIX tempMatrix = Matrix(Rotation); // Matrix

    Vector3 vAxisX, vAxisY, vAxisZ;

    vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
    vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
    vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

    Vector3 vDelta = WorldLocation - POV.Location;
    Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

    if (vTransformed.z < 1.f)
        vTransformed.z = 1.f;

    float FovAngle = POV.FOV;

    float ScreenCenterX =  (winProperties.width/ 2.0f);
    float ScreenCenterY =  (winProperties.height/ 2.0f);
  
    Screenlocation.x = (ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)3.1415926535897932384626433832795/ 360.f)) / vTransformed.z)+winProperties.x;
    Screenlocation.y = (ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)3.1415926535897932384626433832795 / 360.f)) / vTransformed.z)+winProperties.y;
    return Screenlocation;
}



bool refreshPawns() {


    clock_t tempTime = clock();
    if (tempTime >= pieceUpdateTimer + 4000) {

        SecureZeroMemory(&targetPawns, sizeof(targetPawns));

        int playerSize = util::Read<INT32>(GameState + 0x2f8, sizeof(playerSize));
        playerArray = util::Read<uintptr_t>(GameState + 0x2f0, sizeof(playerArray));


        targetPawns = std::vector<AActor>(playerSize);


        for (int i = 0; i < targetPawns.size(); i++) {




            targetPawns[i].baseAddress = util::Read<uintptr_t>(playerArray + (i * 0x8), sizeof(targetPawns[i].baseAddress));


            targetPawns[i].FNameID = util::Read<INT64>((targetPawns[i].baseAddress + 0x18), sizeof(targetPawns[i].FNameID));

            targetPawns[i].Name = GetNameFromFName(targetPawns[i].FNameID);


            targetPawns[i].RootComponent.RelativeLocation = util::Read<Vector3>(((util::Read<uintptr_t>((targetPawns[i].baseAddress + offsets::AActor::RootComponent), sizeof(uintptr_t))) + offsets::USceneComponent::RelativeLocation), sizeof(targetPawns[i].RootComponent.RelativeLocation));

            targetPawns[i].W2S = WorldToScreen(targetPawns[i].RootComponent.RelativeLocation, cameraCache);




            if (targetPawns[i].baseAddress == localPlayerPiece) {

                localPiece = targetPawns[i];

            }

            targetPawns[i].Distance = getDistance(localPiece.RootComponent.RelativeLocation, targetPawns[i].RootComponent.RelativeLocation);


        }

        pieceUpdateTimer = tempTime;
        return true;
    }
    else {

        float distBuffer = 999999999.f;
        int currBest = 0;
        for (int i = 0; i < targetPawns.size(); i++) {

            targetPawns[i].RootComponent.RelativeLocation = util::Read<Vector3>(((util::Read<uintptr_t>((targetPawns[i].baseAddress + offsets::AActor::RootComponent), sizeof(uintptr_t))) + offsets::USceneComponent::RelativeLocation), sizeof(targetPawns[i].RootComponent.RelativeLocation));

            targetPawns[i].W2S = WorldToScreen(targetPawns[i].RootComponent.RelativeLocation, cameraCache);

            if (targetPawns[i].baseAddress == localPlayerPiece) {

                localPiece = targetPawns[i];
                
            }


            targetPawns[i].Distance = getDistance(localPiece.RootComponent.RelativeLocation, targetPawns[i].RootComponent.RelativeLocation);
            targetPawns[i].closest = false;

           if (aimPriority == 0) {
                float tempDist = get2DDistance(targetPawns[i].W2S, Vector3(winProperties.width / 2 + winProperties.x, winProperties.height / 2 + winProperties.y, 0));

                if (tempDist < distBuffer) {

                    distBuffer = tempDist;
                    currBest = i;

                }

            }
            else if (aimPriority == 1) {

               if (targetPawns[i].baseAddress != localPiece.baseAddress) {
                   float tempDist = targetPawns[i].Distance;
                   if (tempDist < distBuffer) {
                       distBuffer = tempDist;
                       currBest = i;

                   }
               }


            }
            

        }
        if (currBest) {
            targetPawns[currBest].closest = true;
            currAimPiecIndex = currBest;

        }
        
        
        
       
    }
    return false;

}



    void mainLoop(){

        

        util::updateWindow();
       

       

        if (updateWorld()) {


        }
        


        
            if (UWorld) {


                updateCameraCache();
                
                
                if (refreshPawns()) {
                    updateLocalPlayer();


                }
              //  setPriorityTarget();
                

               
               for (int i = 0; i < targetPawns.size(); i++) {

              

                   if (bAimbot && currAimPiecIndex < targetPawns.size()) {

                       aimbot(aimKey, smoothX, smoothY, targetPawns[currAimPiecIndex]);

                   }
                   
                   if (targetPawns[i].baseAddress != localPiece.baseAddress) {
                    
                           
             

                       boxDimensions = { (30 / (targetPawns[i].Distance / 3500.f)) * (winProperties.width / 1920) , (30 / (targetPawns[i].Distance / 3500.f)) * (winProperties.height / 1080) };



                      
                       if (boxESP) {
                           if (bRounded) {

                               overlay::drawBox(targetPawns[i].W2S, rounding);
                           }
                           else {
                               overlay::drawBox(targetPawns[i].W2S);
                           }

                       }

                       if (lineESP) {

                           overlay::drawLine(targetPawns[i].W2S);

                       }

                       if (quishaESP) {

                           ImGui::GetBackgroundDrawList()->AddImage(quishaImg, { targetPawns[i].W2S.x - boxDimensions.x / 2, targetPawns[i].W2S.y - boxDimensions.y / 2 }, { targetPawns[i].W2S.x + boxDimensions.x / 2, targetPawns[i].W2S.y + boxDimensions.y / 2 });


                       }

                       if (bKillChanger) {

                           util::Write<int>(localPiece.baseAddress + offsets::AMetaCharacter::KillCount, killCount);


                       }

                       if (bDamageDone) {


                           util::Write<float>(localPiece.baseAddress + offsets::AMetaCharacter::DamageDone, damageDone);


                       }




                   }
               }

            }
        }
       

    
       
        


        
    






        



