// Fill out your copyright notice in the Description page of Project Settings.


#include "FaceSwapUtilsBPLibrary.h"
#include "Misc/Base64.h"
#include "ImageUtils.h"

bool UFaceSwapUtilsBPLibrary::SaveBytesToFile(const FString& FilePath,const TArray<uint8>& OutFileData)
{
	if (OutFileData.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("The Data is empty."))
		return false;
	}
	return FFileHelper::SaveArrayToFile(OutFileData, *FilePath);
}

void UFaceSwapUtilsBPLibrary::LoadFileToBytes(const FString& FilePath, bool& bSuccess, TArray<uint8>& OutData)
{
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		bSuccess = true;
		FFileHelper::LoadFileToArray(OutData,*FilePath);
		return;
	}
	bSuccess = false;
	UE_LOG(LogTemp, Error, TEXT("File is not exist."))
}

void UFaceSwapUtilsBPLibrary::BinaryImageToTexture2D(TArray<uint8> BinaryImage, UTexture2D*& texture_out)
{
	if (BinaryImage.IsEmpty()) texture_out = nullptr;
	else
	{
		texture_out = FImageUtils::ImportBufferAsTexture2D(BinaryImage);
	}
}
