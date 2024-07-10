// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CaptureTexture2D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateTextureDelegate, UTexture2D*, OutTexture, const TArray<uint8>&, OutBinaryImage);
/**
 * 
 */
UCLASS()
class FACESWAP4UE_API UCaptureTexture2D : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	/*运行时截图，不捕获UI*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Keywords = "capture,image,texture,captureimage,capturetexture,screenshot"), Category = "Face Swap|Utils|Capture", DisplayName = "Async Capture Texture2D")
	static UCaptureTexture2D* CaptureTexture2D(UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable)
	FCreateTextureDelegate OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FCreateTextureDelegate OnFail;
	
	void ScreenCaptureCompleted(int32 InWidth, int32 InHeight, const TArray<FColor>& InColor);
	
private:
	FDelegateHandle CaptureHandle;
};
