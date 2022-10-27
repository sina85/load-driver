#include <ntddk.h>
#include <process.h>

#define DEV_SEND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x837, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)
#define DEV_REC CTL_CODE(FILE_DEVICE_UNKNOWN, 0x838, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)

UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\MyDev123");
UNICODE_STRING sym_link = RTL_CONSTANT_STRING(L"\\??\\MySimLink123");
PDEVICE_OBJECT device_object = { 0 };

void Unload(PDRIVER_OBJECT dobject) {
	IoDeleteDevice(device_object);
	IoDeleteSymbolicLink(&sym_link);
	KdPrint(("Unload.\n"));
}

NTSTATUS DevControl(PDEVICE_OBJECT deviceOb, PIRP irp) {
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS DevPassThrough(PDEVICE_OBJECT deviceOb, PIRP irp) {
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
	ULONG info;
	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case DEV_SEND:
		KdPrint(("DEV_SEND called, sending from driver. %u - %p\n", irp->UserBuffer, irp->UserBuffer));
		memcpy(irp->UserBuffer, "Send from driver sample.\n", 26);
		info = 26;
		break;
	case DEV_REC:
		KdPrint(("DEV_REC called, receiving from driver. %u - %p\n", stack->Parameters.DeviceIoControl.Type3InputBuffer, stack->Parameters.DeviceIoControl.Type3InputBuffer));
		info = 0;
		break;
	}
	irp->IoStatus.Information = info;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT dObject, PUNICODE_STRING regPath) {
	NTSTATUS status = STATUS_SUCCESS;
	dObject->DriverUnload = Unload;
	status = IoCreateDevice(dObject, NULL, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Creating device failed.\n"));
		Unload(dObject);
	}
	status = IoCreateSymbolicLink(&sym_link, &device_name);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Creating symbolic-link failed.\n"));
		Unload(dObject);
	}
	for (int n = 0; n < IRP_MJ_MAXIMUM_FUNCTION; ++n) dObject->MajorFunction[n] = DevPassThrough;
	dObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DevControl;
	KdPrint(("Driver loaded.\n"));
	return status;
}