// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UE4DuinoBPLibrary.h"
#include "UE4Duino.h"
#include "WinBase.h"

#define BOOL2bool(B) B == 0 ? false : true

DEFINE_LOG_CATEGORY(UE4Duino);


UUE4DuinoBPLibrary::UUE4DuinoBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer),multi_handleIDCom(NULL),m_Port(-1),m_BaudRate(-1), WriteLineEnd(ELineEnd::n)
{
	FMemory::Memset(&multi_OverlappedRead, 0, sizeof(OVERLAPPED));
	FMemory::Memset(&multi_OverlappedWrite, 0, sizeof(OVERLAPPED));
}

UUE4DuinoBPLibrary::~UUE4DuinoBPLibrary() {
	Close();
}

bool UUE4DuinoBPLibrary::Open(int32 nPort, int32 nBoudRate)
{
	if (nPort < 0)
	{
		UE_LOG(UE4Duino, Error, TEXT("Geçersiz Port Numarasý: %d"), nPort);

		return false;
	}

	if (multi_handleIDCom)
	{
		UE_LOG(UE4Duino, Warning, TEXT("Açik Serial Portu deneniyor " "Geçerli açýk Serial portu: %d | Port denendi: %d"), m_Port, nPort);
		return false;
	}

	FString szPort = FString::Printf(nPort < 10 ? TEXT("COM%d") : TEXT("\\\\.\\COM%d"), nPort);

	/*
	*DCB structure = Seri iletiþim cihazý için kontrol ayarýný tanýmlar. Daha fazla bilgi için :
	*https://msdn.microsoft.com/en-us/library/windows/desktop/aa363214(v=vs.85).aspx
	*/
	DCB dcb;

	multi_handleIDCom = CreateFile(*szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (multi_handleIDCom == NULL)
	{
		unsigned long dwError = GetLastError();
		//UE_LOG(UE4Duino, Error, TEXT("Failed to open port COM%d (%s). Error: %08X"), nPort, c_szport, dwError);
		return false;
	}
	
	FMemory::Memset(&multi_OverlappedRead, 0, sizeof(OVERLAPPED));
	FMemory::Memset(&multi_OverlappedWrite, 0, sizeof(OVERLAPPED));

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 10;
	SetCommTimeouts(multi_handleIDCom, &CommTimeOuts);


	multi_OverlappedRead.hEvent = CreateEvent(NULL, true, false, NULL);
	multi_OverlappedWrite.hEvent = CreateEvent(NULL, true, false, NULL);

	dcb.DCBlength = sizeof(DCB);
	//Bayt cinsinden yapýnýn uzunluðu. Arayan bu üyeyi sizeof (DCB) olarak ayarlamalýdýr.
	GetCommState(multi_handleIDCom, &dcb);
	//Belirtilen bir iletiþim cihazý için geçerli kontrol ayarlarýný alýr.
	dcb.BaudRate = nBoudRate;
	/* 
	*Ýletiþim cihazýnýn çalýþtýðý baud hýzý.Bu üye gerçek bir baud hýzý deðeri veya aþaðýdaki dizinlerden biri olabilir.
	* https://msdn.microsoft.com/en-us/library/windows/desktop/aa363214(v=vs.85).aspx
	*/
	dcb.ByteSize = 8;
	//Gönderilen ve alýnan baytlardaki bit sayýsý.

	if (!SetCommState(multi_handleIDCom, &dcb) || !SetupComm(multi_handleIDCom, 10000, 10000) || multi_OverlappedRead.hEvent == NULL || multi_OverlappedWrite.hEvent == NULL)
	{
		unsigned long dwError = GetLastError();
		if (multi_OverlappedRead.hEvent != NULL)
		{
			CloseHandle(multi_OverlappedRead.hEvent);
		}
		if (multi_OverlappedWrite.hEvent != NULL)
		{
			CloseHandle(multi_OverlappedWrite.hEvent);
		}

		CloseHandle(multi_handleIDCom),
		multi_handleIDCom = NULL;

		UE_LOG(UE4Duino, Error, TEXT("Baðlantý noktasý COM ayarlanamadý%d. Error: %08X"), nPort, dwError);

		return false;
	}

	AddToRoot();
	m_Port = nPort;
	m_BaudRate = nBoudRate;
	m_BaudRate = nBoudRate;
	return true;
}


UUE4DuinoBPLibrary* UUE4DuinoBPLibrary::OpenSerialPort(bool &bOpened, int32 Port, int32 BoundRate) 
{
	UUE4DuinoBPLibrary* Serial = NewObject<UUE4DuinoBPLibrary>();
	bOpened = Serial->Open(Port, BoundRate);
	return Serial;
}

void UUE4DuinoBPLibrary::Close() {
	if (!multi_handleIDCom)
	{
		return;
	}
	if (multi_OverlappedRead.hEvent != NULL)
	{
		CloseHandle(multi_OverlappedRead.hEvent);
	}
	if (multi_OverlappedWrite.hEvent != NULL)
	{
		CloseHandle(multi_OverlappedWrite.hEvent);
	}

	CloseHandle(multi_handleIDCom);

	multi_handleIDCom = NULL;

	RemoveFromRoot();
}

void UUE4DuinoBPLibrary::Clear() 
{
	if (!multi_handleIDCom)
	{
		return;
	}

	TArray<uint8> Data;

	do
	{
		Data = ReadBytes(8192);
	} 
	while (Data.Num() > 0);
}

/*
- Read Byte
*/

uint8 UUE4DuinoBPLibrary::ReadByte(bool &bSuccess)
{
	bSuccess = false;
	if (!multi_handleIDCom)
	{
		return 0x0;
	}

	uint8 Byte = 0x0;

	bool bReadStatus;
	unsigned long dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError(multi_handleIDCom, &dwErrorFlags, &ComStat);

	if (!ComStat.cbInQue)
	{
		return 0x0;
	}

	bReadStatus = BOOL2bool(ReadFile(
		multi_handleIDCom,
		&Byte,
		1,
		&dwBytesRead,
		&multi_OverlappedRead));

	if (!bReadStatus)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(multi_OverlappedRead.hEvent, 2000);
		}
		else
		{
			return 0x0;
		}
	}

	bSuccess = dwBytesRead > 0;
	return Byte;
}

/*
- Read Bytes
*/
TArray<uint8>  UUE4DuinoBPLibrary::ReadBytes(int32 Limit)
{
	TArray<uint8> Data;

	if (!multi_handleIDCom)
	{
		return Data;
	}

	Data.Empty(Limit);

	uint8* Buffer = new uint8[Limit];
	bool bReadStatus;
	unsigned long dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError(multi_handleIDCom, &dwErrorFlags, &ComStat);

	if (!ComStat.cbInQue)
	{
		return Data;
	}

	bReadStatus = BOOL2bool(ReadFile(
		multi_handleIDCom,
		Buffer,
		Limit,
		&dwBytesRead,
		&multi_OverlappedRead));

	if (!bReadStatus)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(multi_OverlappedRead.hEvent, 2000);
		}
		else
		{
			return Data;
		}
	}
	Data.Append(Buffer, dwBytesRead);
	return Data;
}

FString UUE4DuinoBPLibrary::ReadStringUntil(bool &bSuccess, uint8 Terminator) {

	bSuccess = false;
	
	if (!multi_handleIDCom)
	{
		return TEXT("");
	}

	TArray<uint8> Chars;
	uint8 Byte = 0x0;
	bool bReadStatus;
	unsigned long dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError(multi_handleIDCom, &dwErrorFlags, &ComStat);
	if (!ComStat.cbInQue)
	{
		return TEXT("");
	}

	do {
		bReadStatus = BOOL2bool(ReadFile(
			multi_handleIDCom,
			&Byte,
			1,
			&dwBytesRead,
			&multi_OverlappedRead));

		if (!bReadStatus)
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				WaitForSingleObject(multi_OverlappedRead.hEvent, 2000);
			}
			else
			{
				Chars.Add(0x0);
				break;
			}
		}

		if (Byte == Terminator || dwBytesRead == 0)
		{
			// when Terminator is \n, we know we're expecting lines from Arduino. But those
			// are ended in \r\n. That means that if we found the line Terminator (\n), our previous
			// character could be \r. If it is, we remove that from the array.
			if (Chars.Num() > 0 && Terminator == '\n' && Chars.Top() == '\r') Chars.Pop(false);

			Chars.Add(0x0);
			break;
		}
		else Chars.Add(Byte);
	} 
	
	while (Byte != 0x0 && Byte != Terminator);

	bSuccess = true;
	auto Convert = FUTF8ToTCHAR((ANSICHAR*)Chars.GetData());
	return FString(Convert.Get());
}

/*
- Read String
*/

FString UUE4DuinoBPLibrary::ReadString(bool &bSuccess)
{
	return ReadStringUntil(bSuccess, '\0');
}

/*
- Read Line
*/

FString UUE4DuinoBPLibrary::Readln(bool &bSuccess) {
	return ReadStringUntil(bSuccess, '\n');
}

/*
- Read a Float
*/

float UUE4DuinoBPLibrary::ReadFloat(bool &bSuccess)
{
	bSuccess = false;

	TArray<uint8> Bytes = ReadBytes(4);

	if (Bytes.Num() == 0)
	{
		return 0;
	}

	bSuccess = true;
	return *(reinterpret_cast<int32*>(Bytes.GetData()));
}

/*
- Read an Integer
*/

int32 UUE4DuinoBPLibrary::ReadInt(bool &bSuccess)
{
	bSuccess = false;

	TArray<uint8> Bytes = ReadBytes(4);
	if (Bytes.Num() == 0) return 0;

	bSuccess = true;
	return *(reinterpret_cast<int32*>(Bytes.GetData()));
}


/*
- Print
*/

bool UUE4DuinoBPLibrary::Print(FString String)
{
	auto Convert = FTCHARToUTF8(*String);
	TArray<uint8> Data;
	Data.Append((uint8*)Convert.Get(), Convert.Length());
	return WriteBytes(Data);
}

bool UUE4DuinoBPLibrary::Println(FString String)
{
	return Print(String + LineEndToStr(WriteLineEnd));
}

bool UUE4DuinoBPLibrary::WriteByte(uint8 Value)
{
	TArray<uint8> Buffer({ Value });
	return WriteBytes(Buffer);
}

bool UUE4DuinoBPLibrary::WriteBytes(TArray<uint8> Buffer) 
{
	if (!multi_handleIDCom)
	{
		false;
	}

	bool bWriteStat;
	unsigned long dwBytesWritten;

	bWriteStat = BOOL2bool(WriteFile(multi_handleIDCom, Buffer.GetData(), Buffer.Num(), &dwBytesWritten, &multi_OverlappedWrite));

	if (!bWriteStat && (GetLastError() == ERROR_IO_PENDING))
	{
		if (WaitForSingleObject(multi_OverlappedWrite.hEvent, 1000))
		{
			dwBytesWritten = 0;
			return false;
		}
		else
		{
			GetOverlappedResult(multi_handleIDCom, &multi_OverlappedWrite, &dwBytesWritten, false);
			multi_OverlappedWrite.Offset += dwBytesWritten;
			return true;
		}
	}
	return true;
}

bool UUE4DuinoBPLibrary::WriteFloat(float Value)
{
	TArray<uint8> Buffer;
	Buffer.Append(reinterpret_cast<uint8*>(&Value), 4);
	return WriteBytes(Buffer);
}

bool UUE4DuinoBPLibrary::WriteInt(int32 Value)
{
	TArray<uint8> Buffer;
	Buffer.Append(reinterpret_cast<uint8*>(&Value), 4);
	return WriteBytes(Buffer);
}

int32 UUE4DuinoBPLibrary::BytesToInt(TArray<uint8> Bytes) 
{
	if (Bytes.Num() != 4)
	{
		return 0;
	}

	return *reinterpret_cast<int32*>(Bytes.GetData());
}

TArray<uint8> UUE4DuinoBPLibrary::IntToBytes(const int32 &Int)
{
	TArray<uint8> Bytes;
	Bytes.Append(reinterpret_cast<const uint8*>(&Int), 4);
	return Bytes;
}

float UUE4DuinoBPLibrary::BytesToFloat(TArray<uint8> Bytes)
{
	if (Bytes.Num() != 4)
	{
		return 0;
	}

	return *reinterpret_cast<float*>(Bytes.GetData());
}

TArray<uint8> UUE4DuinoBPLibrary::FloatToBytes(const float &Float)
{
	TArray<uint8> Bytes;
	Bytes.Append(reinterpret_cast<const uint8*>(&Float), 4);
	return Bytes;
}

FString UUE4DuinoBPLibrary::LineEndToStr(ELineEnd LineEnd)
{
	switch (LineEnd)
	{
	case ELineEnd::rn:
		return TEXT("\r\n");
	case ELineEnd::n:
		return TEXT("\n");
	case ELineEnd::r:
		return TEXT("\r");
	case ELineEnd::nr:
		return TEXT("\n\r");
	default:
		return TEXT("null");
	}
}