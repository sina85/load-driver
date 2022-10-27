#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>

#define DEV_REC CTL_CODE(FILE_DEVICE_UNKNOWN,0x838,METHOD_NEITHER,FILE_WRITE_DATA|FILE_READ_DATA)
#define DEV_SEND CTL_CODE(FILE_DEVICE_UNKNOWN,0x837,METHOD_NEITHER,FILE_WRITE_DATA|FILE_READ_DATA)

#define Size 512

int main(){
	int d;
	void* buff;
	buff = malloc(Size);
	DWORD ret;
	HANDLE file;
	file = CreateFile("\\\\.\\MySymLink123", GENERIC_ALL, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM,NULL);
	if (!file)printf("Cannot open sym-link.\n");
	else {
		printf("buff address is at: %p\n", buff);
		while (1) {
			printf("1-Send Data\n2-Recieve Data\n");
			scanf("%d", &d);
			memset(buff, 0, Size);
			switch (d) {
			case 1:
				break;
				memcpy(buff, "Send From User-Mode", 20);
				if (DeviceIoControl(file, DEV_REC, NULL, NULL, buff, Size, &ret, NULL) == 0)printf("Error case 1, %u", GetLastError());
			case 2:
				if (DeviceIoControl(file, DEV_SEND, buff, Size, NULL, NULL, &ret, NULL) == 0)printf("Error case 2, %u", GetLastError());
				else printf("Got from driver %s", buff);
				break;
			case 3:
				goto exit;
			}
		}
		exit:
		CloseHandle(file);
	}
}