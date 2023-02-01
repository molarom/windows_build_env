#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>

#define TRUE 1
#define FALSE 0

#define EXECUTE L"C:\Windows\System32\notepad.exe"

int main(int argc, char** argv)
{
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, (DWORD)NULL);
  if (Process32First(snapshot, &entry) == FALSE) {
    printf("Process32First: GetLastError = %#08lx", GetLastError()); 
    exit(1);
  }

  while (Process32Next(snapshot, &entry) == TRUE) {
    if (stricmp(entry.szExeFile, "explorer.exe") == 0) {
      HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, entry.th32ProcessID);
      
      HANDLE TokenHandle = {0};
      if (OpenProcessToken(hProcess, TOKEN_DUPLICATE, &TokenHandle) == FALSE)
        printf("OpenProcessToken: GetLastError = %#08lx", GetLastError()); 

      
      PROCESS_INFORMATION pi;
      STARTUPINFO si = { sizeof(STARTUPINFO) };
      si.lpDesktop = L"winsta\\default";
      if (CreateProcessAsUserA(TokenHandle, EXECUTE, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) == FALSE)
        printf("CreateProcessAsUserA: GetLastError = %#08lx", GetLastError()); 
      
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
      CloseHandle(hProcess);
    }
  }

  CloseHandle(snapshot);
  
  return 0;
}
