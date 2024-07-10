// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Http.h"
#include "FaceSwapProxy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFaceSwapProxy, Log, All);

// 字节数组为图像二进制数据，可以直接保存到本地或是通过网络传输
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGenImageDelegate, int32, Code, const TArray<uint8>&, TextureRawDataBytes, const FString&, Message);

/**
 * 换脸服务，输入一个头像和目标图像，即可进行换脸
 */
UCLASS()
class FACESWAP4UE_API UFaceSwapProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/*
	 * 换脸服务
	 * 输入字节数组为原始图像二进制数据，请勿使用Texture2D转换的字节数组
	 * 
	 * Url: 请输入正确的Url地址，不需要路由地址，连接地址 eg. http://127.0.0.1:8000
	 * InRefImageData: 输入的头像二进制数据
	 * InTargetImageData: 输入的目标图像二进制数据
	 * 
	 * return: FGenImageDelegate委托返回图像二进制数据和处理消息
	 *
	 * 状态码:
	 * 200 请求成功
	 * 201 请求已创建
	 * 400 请求参数错误，服务器拒绝处理
	 * 404 无法找到服务器
	 * 500 服务器内部错误，无法完成请求
	 * 502 作为网关或代理角色的服务器，从上游服务器中接收到的响应是无效的
	 * 其他状态码查看网络上的资料
	 */
	UFUNCTION(BlueprintCallable, Category = "Face Swap", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Face Swap"))
	static UFaceSwapProxy* FaceSwap(const FString& Url, const TArray<uint8>& InRefImageData, const TArray<uint8>& InTargetImageData);

	// 生成中
	UPROPERTY(BlueprintAssignable)
	FGenImageDelegate OnProgress;

	// 生成成功
	UPROPERTY(BlueprintAssignable)
	FGenImageDelegate OnSuccess;

	// 失败
	UPROPERTY(BlueprintAssignable)
	FGenImageDelegate OnFailure;

	virtual void Activate() override;

private:
	FString Url;
	TArray<uint8> RefImageData;
	TArray<uint8> TargetImageData;

private:
	// 发送数据
	void SendRequest();
	
	// 请求响应
	void OnHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// 构造请求体
	static FString MakeRequestBody(const TArray<uint8>& InRefImageData, const TArray<uint8>& InTargetImageData);

	// 解析响应
	static bool ParseResponse(const FString& InData, bool& Success, FString& Message,TArray<uint8>& Output);
};
