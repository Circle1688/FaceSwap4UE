// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FaceSwapUtilsBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class FACESWAP4UE_API UFaceSwapUtilsBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*
	 * 保存二进制数据到本地硬盘路径
	 */
	UFUNCTION(BlueprintCallable,Category="Face Swap|Utils|File")
	static bool SaveBytesToFile(const FString& FilePath,const TArray<uint8>& InFileData);

	/*
	 * 从文件路径读取二进制数据
	 */
	UFUNCTION(BlueprintCallable,Category="Face Swap|Utils|File")
	static void LoadFileToBytes(const FString& FilePath, bool& bSuccess, TArray<uint8>& OutData);

	/*
	 * 读取图像二进制数据到Texture2D
	 */
	UFUNCTION(BlueprintCallable, Category = "Face Swap|Utils|Convert")
	static void BinaryImageToTexture2D(TArray<uint8> BinaryImage, UPARAM(DisplayName = "Texture2D") UTexture2D*& texture_out);
	
};
