#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

PFLT_PORT port = NULL;
PFLT_PORT clientPort = NULL;

FLT_PREOP_CALLBACK_STATUS  MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText);
FLT_PREOP_CALLBACK_STATUS  MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText);
FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText, FLT_POST_OPERATION_FLAGS Flags);
NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

FLT_OPERATION_REGISTRATION CallBacks[] = {
	//{IRP_MJ_CREATE,0,MiniPreCreate,MiniPostCreate},
	//{IRP_MJ_WRITE,0,MiniPreWrite,NULL},
	{IRP_MJ_OPERATION_END}
};
PFLT_FILTER FltHandle = NULL;
const FLT_REGISTRATION fltregister = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	 0,
	 NULL,
	 CallBacks,
	 MiniUnload,
	 NULL,
	 NULL,
	 NULL,
	 NULL,
	 NULL,
	 NULL,
	 NULL,
	 NULL
};
NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
	KdPrint(("Driver unload \r\n"));
	FltCloseCommunicationPort(port);
	FltUnregisterFilter(FltHandle);
	return STATUS_SUCCESS;
}
FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText, FLT_POST_OPERATION_FLAGS Flags) {
	KdPrint(("Post Create is Runnig \r\n"));
	return FLT_POSTOP_FINISHED_PROCESSING;
}
FLT_PREOP_CALLBACK_STATUS  MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText) {
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION  FileNameInfo;
	WCHAR Name[200] = { 0 };
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Name.MaximumLength < 260) {
				RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				KdPrint(("Create File: %ws \r\n", Name));
			}
		}
		FltReleaseFileNameInformation(FileNameInfo);
	}
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}
FLT_PREOP_CALLBACK_STATUS  MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText);
NTSTATUS MiniConnect(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext,PVOID* ConnectionPortCookie) {
	clientPort = ClientPort;
	KdPrint(("connect \r\n"));
	return STATUS_SUCCESS;
}
VOID MiniDisconnect(PVOID connectionCookie) {
	KdPrint(("disconnect \r\n"));
	FltCloseClientPort(FltHandle,&clientPort);
}
NTSTATUS MiniSendRec(PVOID PortCookie, PVOID InputBuffer OPTIONAL, ULONG InputBufferLength, PVOID OutputBuffer OPTIONAL, ULONG OutputBufferLength, PULONG ReturnOutputBufferLength0) {

	PCHAR msg = "kernel msg";
	KdPrint(("user msg is: &s \r\n"),(PCHAR)InputBuffer);
	strcpy((PCHAR)OutputBufferLength, msg);
	return STATUS_SUCCESS;
}
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status;
	FLT_REGISTRATION fltregister = { 0 };
	PSECURITY_DESCRIPTOR sd;
	OBJECT_ATTRIBUTES oa = { 0 };
	UNICODE_STRING name = RTL_CONSTANT_STRING(L"\\mf");

	status = FltRegisterFilter(DriverObject, &fltregister, &FltHandle);//filtre sürücüsüne kayýt
	if (NT_SUCCESS(status)) {
		return status;
	}
	status = FltBuildDefaultSecurityDescriptor(&sd,FLT_PORT_ALL_ACCESS);
	if (NT_SUCCESS(status)) {
		InitializeObjectAttributes(&oa,&name,OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,NULL,sd);
		status=FltCreateCommunicationPort(&FltHandle,&port,&oa,NULL,MiniConnect,MiniDisconnect,MiniSendRec,1);

		FltFreeSecurityDescriptor(&sd);
		if (NT_SUCCESS(status)) {
			status = FltStartFiltering(FltHandle);
			if (NT_SUCCESS(status)) {
				return status;
			}
			FltCloseCommunicationPort(port);
		}
		FltUnregisterFilter(FltHandle);

	}
	return status;
}
