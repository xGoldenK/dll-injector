#include <stdio.h>
#include <windows.h>
#include <synchapi.h>

int main() {
	HANDLE process_handle, thread_handle;
	HMODULE kernel32;
	LPVOID buffer_start;
	DWORD PID, TID;
	const char* dll_path = "C:\\Users\\NicolaC\\Desktop\\nostale_packet_logger.dll";

	printf("Hey! Remember to set the path of the dll you want to inject in main.c!\n");
	printf("Enter the PID you want to inject into.\n");
	scanf_s("%d", &PID);

	process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if(!process_handle) {
		printf("Could not find process with PID [%d]. Do you need admin rights?\n", PID);
		return;
	}

	buffer_start = VirtualAllocEx(process_handle, NULL, strlen(dll_path), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if(!buffer_start) {
		printf("Could not allocate memory for process. %ld\n", GetLastError());
		return;
	}

	BOOL w = WriteProcessMemory(process_handle, buffer_start, dll_path, strlen(dll_path), NULL);
	if(!w) {
		printf("Could not write dll in reserved memory space. %ld\n", GetLastError());
		return;
	}

	kernel32 = GetModuleHandle(TEXT("kernel32.dll"));
	if(!kernel32) {
		printf("Could not get Kernel32.dll module. %ld\n", GetLastError());
		CloseHandle(process_handle);
		return;
	}

	LPTHREAD_START_ROUTINE thread_start = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryA");
	printf("Got address for LoadLibrary function [0x%p]\n", thread_start);

	thread_handle = CreateRemoteThread(process_handle, NULL, 0, thread_start, buffer_start, 0, &TID);
	if(!thread_handle) {
		printf("Could not create remote thread. %ld\n", GetLastError());
		CloseHandle(process_handle);
		return;
	}

	printf("Thread started successfully! TID: [%i]\n", TID);
	printf("Waiting for execution to finish...");
	WaitForSingleObject(thread_handle, INFINITE);

	CloseHandle(thread_handle);
	CloseHandle(process_handle);

	return EXIT_SUCCESS;
}