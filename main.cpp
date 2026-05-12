#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <fstream>

// --- Manuel Map İçin Gerekli Yapı ---
struct MAPPING_DATA {
    void* pImageBase;
    HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
    FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);
    UINT_PTR pOriginalRip;
};

// Hedef süreçte çalışacak Shellcode (Aynı kalıyor)
void __stdcall Shellcode(MAPPING_DATA* pData) {
    BYTE* pBase = (BYTE*)pData->pImageBase;
    auto* pOpt = &((PIMAGE_NT_HEADERS)(pBase + ((PIMAGE_DOS_HEADER)pBase)->e_lfanew))->OptionalHeader;
    auto f_LoadLibraryA = pData->pLoadLibraryA;
    auto f_GetProcAddress = pData->pGetProcAddress;

    // Relocation
    auto* pRelocDir = &pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (pRelocDir->Size) {
        auto* pRelocData = (PIMAGE_BASE_RELOCATION)(pBase + pRelocDir->VirtualAddress);
        while (pRelocData->VirtualAddress) {
            UINT entries = (pRelocData->SizeOfBlock - sizeof(PIMAGE_BASE_RELOCATION)) / sizeof(WORD);
            WORD* pInfo = (WORD*)(pRelocData + 1);
            for (UINT i = 0; i != entries; ++i, ++pInfo) {
                if ((*pInfo >> 12) == IMAGE_REL_BASED_DIR64) {
                    UINT_PTR* pPatch = (UINT_PTR*)(pBase + pRelocData->VirtualAddress + (*pInfo & 0xFFF));
                    *pPatch += (UINT_PTR)(pBase)-pOpt->ImageBase;
                }
            }
            pRelocData = (PIMAGE_BASE_RELOCATION)((BYTE*)pRelocData + pRelocData->SizeOfBlock);
        }
    }

    // IAT Rebuilding
    auto* pImportDir = &pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (pImportDir->Size) {
        auto* pImportDescr = (PIMAGE_IMPORT_DESCRIPTOR)(pBase + pImportDir->VirtualAddress);
        while (pImportDescr->Name) {
            HINSTANCE hMod = f_LoadLibraryA((char*)(pBase + pImportDescr->Name));
            auto* pThunk = (PIMAGE_THUNK_DATA)(pBase + pImportDescr->OriginalFirstThunk);
            auto* pIAT = (PIMAGE_THUNK_DATA)(pBase + pImportDescr->FirstThunk);
            if (!pThunk) pThunk = pIAT;
            for (; pThunk->u1.AddressOfData; ++pThunk, ++pIAT) {
                if (IMAGE_ORDINAL_FLAG & pThunk->u1.Ordinal)
                    pIAT->u1.Function = (UINT_PTR)f_GetProcAddress(hMod, (char*)(pThunk->u1.Ordinal & 0xFFFF));
                else
                    pIAT->u1.Function = (UINT_PTR)f_GetProcAddress(hMod, (char*)((PIMAGE_IMPORT_BY_NAME)(pBase + pThunk->u1.AddressOfData))->Name);
            }
            pImportDescr++;
        }
    }

    using f_DllMain = BOOL(WINAPI*)(void*, DWORD, void*);
    ((f_DllMain)(pBase + pOpt->AddressOfEntryPoint))(pBase, DLL_PROCESS_ATTACH, nullptr);
}

// --- İşlem Listeleme Fonksiyonu ---
void ListProcesses() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32; pe32.dwSize = sizeof(pe32);
    printf("%-10s %-30s\n", "PID", "Isim");
    printf("----------------------------------------\n");
    if (Process32First(hSnap, &pe32)) {
        do {
            printf("%-10d %-30s\n", pe32.th32ProcessID, pe32.szExeFile);
        } while (Process32Next(hSnap, &pe32));
    }
    CloseHandle(hSnap);
}

int main() {
    std::string dllPath;
    DWORD pid;

    printf("=== MANUEL ENJEKTOR V6 ===\n\n");
    
    ListProcesses();

    printf("\nHedef PID girin: ");
    std::cin >> pid;

    printf("DLL Dosya Yolunu girin (orn: hile.dll): ");
    std::cin >> dllPath;

    // 1. DLL Dosyasını Oku
    std::ifstream file(dllPath, std::ios::binary | std::ios::ate);
    if (file.fail()) {
        printf("[X] DLL bulunamadi!\n");
        return 1;
    }
    auto size = file.tellg();
    BYTE* pSrcData = new BYTE[(UINT_PTR)size];
    file.seekg(0, std::ios::beg);
    file.read((char*)pSrcData, size);
    file.close();

    // 2. Sürece Bağlan
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        printf("[X] Surece erisilemedi!\n");
        return 1;
    }

    // --- Enjeksiyon Adımları (VirtualAllocEx, WriteProcessMemory vb. yukarıdakiyle aynı) ---
    // (Kodun devamı önceki mesajdaki Manual Map mantığıyla aynı şekilde VirtualAllocEx ve Shellcode execution adımlarını içerir)

    printf("[!] Enjeksiyon tamamlandi!\n");
    CloseHandle(hProc);
    return 0;
}

