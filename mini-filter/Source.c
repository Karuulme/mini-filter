#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

FLT_OPERATION_REGISTRATION CallBacks[] = {
	{IRP_MJ_CREATE,0,MiniPreCreate,MiniPostCreate},
	{IRP_MJ_WRITE,0,MiniPreWrite,NULL},
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
};
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath) {
	NTSTATUS status;
	FLT_REGISTRATION fltregister = {0};

	status=FltRegisterFilter(DriverObject,&fltregister,&FltHandle);//filtre sürücüsüne kayýt
	if (NT_SUCCESS(status)) {
		status = FltStartFiltering(FltHandle);
		if (!NT_SUCCESS(status)) {
			FltUnregisterFilter(FltHandle);
		}
	}
	return status;
}
