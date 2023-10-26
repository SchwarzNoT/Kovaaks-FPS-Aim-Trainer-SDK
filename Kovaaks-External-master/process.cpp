#include "process.h"

DWORD procID;
uintptr_t modBase;
HANDLE hProc;
HWND targetWindow;
RECT windowRect;
windowProperties winProperties;




uintptr_t GNames = 0x4E993E4;
uintptr_t GObjects = 0x4EC0EE8;

clock_t winUpdateTimer = -6000;
clock_t checkProcActiveTime = -6000;

namespace util {


    

    DWORD getProcID(const char* procName) {

        DWORD procID = 0;
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(PROCESSENTRY32);
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (Process32First(hSnap, &procEntry)) {


            do {


                if (!strcmp(procName, procEntry.szExeFile)) {


                    procID = procEntry.th32ProcessID;
                    break;

                }


            } while (Process32Next(hSnap, &procEntry));


        }
        if (!procID) {


            printf("Failed To Locate Process With Name \"%s\" ", procName);
        }


        return procID;

    }

    uintptr_t GetModuleBaseAddress(DWORD ProcID, const char* procName) {

        uintptr_t modBaseAddr = 0;

        HANDLE hSnap = 0;

        hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ProcID);

        MODULEENTRY32 modEntry;

        modEntry.dwSize = sizeof(modEntry);

        if (Module32First(hSnap, &modEntry)) {

            do {

                if (!strcmp(modEntry.szModule, procName)) {


                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;

                }
            } while (Module32Next(hSnap, &modEntry));


        }
        CloseHandle(hSnap);
        return modBaseAddr;

    }
    template<typename T> T Read(uintptr_t address, int size) {


        T buffer;

        ReadProcessMemory(hProc, (BYTE*)address, &buffer, size, NULL);

        return buffer;


    }

    template<typename T> bool Write(uintptr_t address, T toWrite) {


        return WriteProcessMemory(hProc, (BYTE*)address, &toWrite, sizeof(toWrite), NULL);

    }


    bool updateWindow() {

      //  targetWindow = FindWindow("UnrealWindow", "FPS Chess  ");

        clock_t tempTime = clock();
        if (tempTime >= winUpdateTimer + 5000) {

            if (targetWindow) {


                LONG_PTR windowStyle = GetWindowLongPtr(targetWindow, GWL_STYLE);

                GetWindowRect(targetWindow, &windowRect);

                if (windowStyle & WS_CAPTION) {

                    winProperties.bWindowed = true;
                    winProperties.width = (float)(windowRect.right - windowRect.left );
                    winProperties.height = float(windowRect.bottom - windowRect.top+19.5f);
                    winProperties.x = (float)(windowRect.left);
                    winProperties.y = (float)windowRect.top;
                }
                else {

                    winProperties.bWindowed = false;
                    winProperties.width = (float)(windowRect.right - windowRect.left);
                    winProperties.height = float(windowRect.bottom - windowRect.top);
                    winProperties.x = (float)(windowRect.left);
                    winProperties.y = (float)windowRect.top;

                }




                winUpdateTimer = tempTime;

                return true;
            }

        }
        return false;

    }


    bool init() {

        procID = getProcID("FPSAimTrainer-Win64-Shipping.exe");
        if (procID) {

            modBase = GetModuleBaseAddress(procID, "FPSAimTrainer-Win64-Shipping.exe");
            hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);

            GNames += modBase;
            GObjects += modBase;
            targetWindow = FindWindow("UnrealWindow", "KovaaK’s  ");



            return true;
        }
        else { return false; }
    }




    void toRead() {

        Read<uintptr_t>(1, 0x0);
        Read<INT32>(1, 0x0);
        Read<int>(1, 0x0);
        Read<float>(1, 0x0);
        Read<FVector>(1, 0x0);
        Read<INT64>(1, 0x0);
        Read<short>(1, 0x0);
        Read<bool>(1, 0x0);
        Read<FMinimalViewInfo>(1, 0x0);
        Read<Vector3>(1, 0x0);

        Write<float>(0x0, 1);
        Write<int>(0x0, 1);



    }

    int setAimKey() {


        int key = -1;
        while (key == -1) {


            for (int i = 0; i < 256; i++) {


                if (GetAsyncKeyState(i) & 0x8000) {


                    return i;

                }



            }
        }
        return -1;


    }
    bool isActive() {

        clock_t tempTime = clock();

        if (tempTime >= checkProcActiveTime + 6000) {


            DWORD active;
            GetExitCodeProcess(hProc, &active);

            if (!active) {
                return false;
            }
            else {
                return true;
            }
        }
        else return true;

    }



}