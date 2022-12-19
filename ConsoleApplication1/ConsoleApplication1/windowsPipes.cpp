#include <windows.h>
#include <cstdio>
#include <strsafe.h>
#include <tchar.h>
#include <iostream>

#define BUFSIZE 4096

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

void CreateChildProcess(void);
void WriteToPipe(void);
void ReadFromPipe(void);
void ErrorExit(PTSTR);

int _tmain(int argc, TCHAR* argv[])
{
	SECURITY_ATTRIBUTES saAttr;
	printf("\n->Start of parent execution.\n");
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ErrorExit(const_cast<LPTSTR>(TEXT("StdoutRd CreatePipe")));
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(const_cast<LPTSTR>(TEXT("Stdout SetHandleInformation")));
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit(const_cast<LPTSTR>(TEXT("Stdin CreatePipe")));
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(const_cast<LPTSTR>(TEXT("Stdin SetHandleInformation")));
	CreateChildProcess();
	if (argc == 1)
		ErrorExit(const_cast<LPTSTR>(TEXT("Please specify an input file.\n")));
	g_hInputFile = CreateFile(
		argv[1],
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);
	if (g_hInputFile == INVALID_HANDLE_VALUE)
		ErrorExit(const_cast<LPTSTR>(TEXT("CreateFile")));
	WriteToPipe();
	
	ReadFromPipe();
	printf("\n->End of parent execution.\n");
	return 0;
}

void CreateChildProcess()
{
	TCHAR szCmdline[] = TEXT("child");
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	bSuccess = CreateProcess(NULL,
		szCmdline,     
		NULL,          
		NULL,          
		TRUE,          
		0,             
		NULL,          
		NULL,          
		&siStartInfo,  
		&piProcInfo);  
	if (!bSuccess)
		ErrorExit(const_cast<LPTSTR>(TEXT("CreateProcess")));
	else
	{
		
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}

void WriteToPipe(void)

{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	for (;;)
	{
		bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;
		bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}
	if (!CloseHandle(g_hChildStd_IN_Wr))
		ErrorExit(const_cast<LPTSTR>(TEXT("StdInWr CloseHandle")));
}

void ReadFromPipe(void)

{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(hParentStdOut, chBuf,
			dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}
}

void ErrorExit(PTSTR lpszFunction)

{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}


#include <cstdio>
#include <windows.h>
#include <iostream>

#define BUFSIZE 4096

int main(void)
{
	CHAR chBuf[BUFSIZE];
	DWORD dwRead, dwWritten;
	HANDLE hStdin, hStdout;
	BOOL bSuccess;

	using namespace std;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (
		(hStdout == INVALID_HANDLE_VALUE) ||
		(hStdin == INVALID_HANDLE_VALUE)
		)
		ExitProcess(1);

	for (int i = 1; i <= 10000; i++)
	{
		bool ok = 1;
		for (int j = 2; j <= i / 2; j++)
			if (i % j == 0)
				ok = 0;
		if (ok)
			cout << i << " ";
	}

	

	for (;;)
	{
		bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, NULL);

		if (!bSuccess || dwRead == 0)
			break;

		bSuccess = WriteFile(hStdout, chBuf, dwRead, &dwWritten, NULL);

		if (!bSuccess)
			break;
	}
	return 0;
}

