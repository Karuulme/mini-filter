#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

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
FLT_PREOP_CALLBACK_STATUS  MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompetionText) {
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION  FileNameInfo;
	WCHAR Name[200] = { 0 };
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Name.MaximumLength < 260) {
				RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				if (wcsstr(Name, L"OPENME.TXT")) {
					KdPrint(("Write file: %ws blocked \r\n", Name));
					Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
					Data->IoStatus.Information = 0;
					FltReleaseFileNameInformation(FileNameInfo);
					return FLT_PREOP_COMPLETE;

				}
			}
		}
		FltReleaseFileNameInformation(FileNameInfo);
	}
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status;
	FLT_REGISTRATION fltregister = { 0 };
	status = FltRegisterFilter(DriverObject, &fltregister, &FltHandle);//filtre sürücüsüne kayýt
	if (NT_SUCCESS(status)) {
		status = FltStartFiltering(FltHandle);
		if (!NT_SUCCESS(status)) {
			FltUnregisterFilter(FltHandle);
		}
	}
	return status;
}
