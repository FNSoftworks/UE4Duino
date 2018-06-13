// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#include "AllowWindowsPlatformTypes.h"
#include "Windows.h"
#include "HideWindowsPlatformTypes.h"
#include "CoreTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UE4DuinoBPLibrary.generated.h"

/* 
*	UE4Duino BP Function library class.
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

/* UE4 Log kayitlar� kategori ismi */

DECLARE_LOG_CATEGORY_EXTERN(UE4Duino, Log, All);

/** PrintLn ile seri ba�lant� noktas�na biten sat�r� belirler. */

UENUM(BlueprintType, Category = "UE4Duino")
enum class ELineEnd : uint8
{
	rn	UMETA(DisplayName = "\r\n"),
	n	UMETA(DisplayName = "\n"),
	r	UMETA(DisplayName = "\r"),
	nr	UMETA(DisplayName = "\n\r")
};

UCLASS(BlueprintType, meta = (Keywords = "UE4Duino Arduino Serial Port"), Category = "UE4Duino")
class UUE4DuinoBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "UE4Duino | String")
	ELineEnd WriteLineEnd;

	UUE4DuinoBPLibrary();
	~UUE4DuinoBPLibrary();

	/*
	- Bir seri ba�lant� noktas� a�ar. Programdan ��kmadan �nce port kapatmay� unutma.
	- E�er zaten a�ik bir serial port var ise false de�erini d�nd�r�r ve a��lan port numaras�n� de�i�tirmez.
	- @param Port = A��lacak seri port.
	- @param BaudRate = A��lacak port i�in baund h�z�.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Open Port", Keywords = "UE4Duino Open Port"), Category = "UE4Duino | Port")
		bool Open(int32 Port = 2, int32 BoundRate = 9600);

	/*
	- Bir serial ba�lant� noktas�n� a�ar ve olu�turulan serial ba�lat�y� d�nd�r�r.
	- Programdan ��kmadan �nce serial port kapatmay� unutmay�n.
	- @param bOpened = E�er Seri ba�lant� noktas� ba�ar�yla a��ld�ysa "True" De�eri D�ner.
	- @param Port = A��lacak seri port.
	- @param BaudRate = A��lacak port i�in baund h�z�.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Open Serial Port", Keywords = "UE4Duino Open Serial Port"), Category = "UE4Duino | Port")
		static UUE4DuinoBPLibrary* OpenSerialPort(bool &bOpened, int32 Port = 1, int32 BoundRate = 9600);

	/*
	- Seri port ile ileti�imi kapat�r ve sonland�r�r. A��k de�ilse, hi�bir �ey yapmaz.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Close Port", Keywords = "UE4Duino Close Serial Port"), Category = "UE4Duino | Port")
		void Close();

	/*
	- Seri ba�lant� noktas�n� temizler.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Flush Port", Keywords = "UE4Duino Close Serial Port"), Category = "UE4Duino | Port")
		void Clear();
	/*
	- Seri ba�lant� noktas�ndan bir bayt okur.
	- @param bSuccess = Okumak i�in 4 bayt veri var ise true de�er d�nderir.
	- @return = Byte de�er d�nderir.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read a Byte", Keywords = "UE4Duino Serial Port Read Byte"), Category = "UE4Duino | Read")
		uint8 ReadByte(bool &bSuccess);

	/*
	- Limit bayta kadar okur. Limitin alt�ndaysa okur ve true d�nd�r�r.
	- @return Okunan baytlar� i�eren bir dizi
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Bytes", Keywords = "UE4Duino Serial Port Read Bytes"), Category = "UE4Duino | Read")
		TArray<uint8> ReadBytes(int32 Limit = 256);

	/*
	- Belirli bir char kar��lanana kadar dizeyi okur.
	- Terminat�r char, sonu� dizesine eklenmez.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read String Until", Keywords = "UE4Duino Serial Port Read String Until"), Category = "UE4Duino | Read")
		FString ReadStringUntil(bool &bSuccess, uint8 Terminator);

	/*
	- Seri ba�lant� noktas�ndan string bulunana kadar karakterleri okur
	- @param bSuccess = Okunacak string de�er var ise true de�er d�ner.
	- @return Okunan stringleri d�nderir.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read String", Keywords = "UE4Duino Serial Port Read String"), Category = "UE4Duino | Read")
		FString ReadString(bool &bSuccess);

	/*
	- Seri ba�lant� noktas�ndan \r \n (Arduino println sat�r sonu) bulunana kadar karakterleri okuyacakt�r.
	- @param bSuccess = okunacak string de�er var ise true de�er d�ner.
	- @return Okunan stringleri d�nderir.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Line", Keywords = "UE4Duino Serial Port Read In"), Category = "UE4Duino | Read")
		FString Readln(bool &bSuccess);

	/*
	- Seri porttan bir float okur (4 bayt olarak g�nderilir).
	- @param bSuccess = Okumak i�in 4 bayt var ise true de�er d�ner.
	- @param return = Okuma Float  de�eri d�ner.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read a Float", Keywords = "UE4Duino Serial Port Read Float"), Category = "UE4Duino | Read")
		float ReadFloat(bool &bSuccess);

	/*
	- Seri ba�lant� noktas�ndan bir tam say� okur (4 bayt olarak g�nderilir).
	-  @param bSuccess = Okumak i�in 4 bayt var ise true de�er d�ner.
	-  @param return = Okuma Integer de�eri d�ner.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read an Int", Keywords = "UE4Duino Serial Port Read Int"), Category = "UE4Duino | Read")
		int32 ReadInt(bool &bSuccess);

	/*
	- Seri ba�lant� noktas�na yeni sat�r olmayan bir string yazar.
	- @param String = Seri ba�lant� noktas�na g�nderilecek string.
	- @return = string de�er g�nderilmi� ise True de�er d�nderilir.
	*/

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Print", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Write")
		bool Print(FString String);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Print Line", Keywords = "UE4Duino Serial Port Write Print Line"), Category = "UE4Duino | Write")
		bool Println(FString String);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Write a Byte", Keywords = "UE4Duino Serial Port Write Byte"), Category = "UE4Duino | Write")
		bool WriteByte(uint8 Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Write Bytes", Keywords = "UE4Duino Serial Port Write Byte"), Category = "UE4Duino | Write")
		bool WriteBytes(TArray<uint8> Buffer);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Write a Float", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Write")
		bool WriteFloat(float Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Write an Int", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Write")
		bool WriteInt(int32 Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bytes to Int", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Convert")
		static int32 BytesToInt(TArray<uint8> Bytes);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Int to Bytes", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Convert")
		static TArray<uint8> IntToBytes(const int32 &Int);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Bytes to Float", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Convert")
		static float BytesToFloat(TArray<uint8> Bytes);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Float to Bytes", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Convert")
		static TArray<uint8> FloatToBytes(const float &Float);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Line End to String", Keywords = "UE4Duino Serial Port Write Print"), Category = "UE4Duino | Convert")
		FString LineEndToStr(ELineEnd LineEnd);

protected:
	void* multi_handleIDCom;
	OVERLAPPED multi_OverlappedRead, multi_OverlappedWrite;
	int32 m_Port;
	int32 m_BaudRate;
};
