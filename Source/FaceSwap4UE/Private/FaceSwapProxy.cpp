// Fill out your copyright notice in the Description page of Project Settings.


#include "FaceSwapProxy.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Base64.h"

DEFINE_LOG_CATEGORY(LogFaceSwapProxy);

UFaceSwapProxy* UFaceSwapProxy::FaceSwap(const FString& Url, const TArray<uint8>& InRefImageData, const TArray<uint8>& InTargetImageData)
{
	UFaceSwapProxy* Node = NewObject<UFaceSwapProxy>();
	Node->Url = Url;
	Node->RefImageData = InRefImageData;
	Node->TargetImageData = InTargetImageData;
	return Node;
}

void UFaceSwapProxy::Activate()
{
	// 检查参数
	FString Message;
	if (RefImageData.IsEmpty())
	{
		Message = TEXT("The Ref Image Data is empty!");
		UE_LOG(LogFaceSwapProxy,Error,TEXT("%s"), *Message);
		if (OnFailure.IsBound())
		{
			OnFailure.Broadcast(400, {}, Message);
		}
		return;
	}
	if (TargetImageData.IsEmpty())
	{
		Message = TEXT("The Target Image Data is empty!");
		UE_LOG(LogFaceSwapProxy,Error,TEXT("%s"), *Message);
		if (OnFailure.IsBound())
		{
			OnFailure.Broadcast(400, {}, Message);
		}
		return;
	}
	
	SendRequest();
}

void UFaceSwapProxy::SendRequest()
{
	AddToRoot();

	// 网络请求
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	// 连接地址 eg. http://127.0.0.1:8000
	FString RequestUrl = Url + TEXT("/face_swap");

	// 设置请求
	Request->SetURL(RequestUrl);

	Request->SetVerb("POST");

	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString Body = MakeRequestBody(RefImageData, TargetImageData);
	
	Request->SetContentAsString(Body);

	Request->OnProcessRequestComplete().BindUObject(this, &UFaceSwapProxy::OnHttpResponse);
	Request->ProcessRequest();

	// 开始处理
	if (OnProgress.IsBound())
	{
		FString Message = TEXT("Created");
		UE_LOG(LogFaceSwapProxy, Log, TEXT("%s"), *Message)
		OnProgress.Broadcast(201,{}, Message);
	}
}

void UFaceSwapProxy::OnHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	bool bSuccess = false;
	FString Message;
	TArray<uint8> OutputImageData;
	
	if (bWasSuccessful && Response.IsValid())
	{
		FString Data = Response->GetContentAsString();
		int32 Code = Response->GetResponseCode();
	
		if (Code == 200)
		{
			// 解析响应
			if (ParseResponse(Data, bSuccess, Message, OutputImageData))
			{
				if (OnSuccess.IsBound())
				{
					OnSuccess.Broadcast(200, OutputImageData, TEXT("OK"));
				}
			}
			else
			{
				if (OnFailure.IsBound())
				{
					OnFailure.Broadcast(500, OutputImageData, TEXT("Internal Server Error"));
				}
			}
		}
		else
		{
			Message = TEXT("Please check the document.");
			UE_LOG(LogFaceSwapProxy, Error, TEXT("%s"), *Message)
			if (OnFailure.IsBound())
			{
				OnFailure.Broadcast(Code, OutputImageData, Message);
			}
		}
	}
	else
	{
		Message = TEXT("Not Found");
		UE_LOG(LogFaceSwapProxy, Error, TEXT("%s"), *Message)
		if (OnFailure.IsBound())
		{
			OnFailure.Broadcast(404, OutputImageData, Message);
		}
	}

	RemoveFromRoot();
}

FString UFaceSwapProxy::MakeRequestBody(const TArray<uint8>& InRefImageData, const TArray<uint8>& InTargetImageData)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject);

	// 转换成Base64字符串
	FString Base64_RefImageData = FBase64::Encode(InRefImageData);
	Body->SetStringField(TEXT("source_image"), Base64_RefImageData);
	
	FString Base64_TargetImageData = FBase64::Encode(InTargetImageData);
	Body->SetStringField(TEXT("target_image"), Base64_TargetImageData);

	FString OutJsonData;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonData);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	return OutJsonData;
}

bool UFaceSwapProxy::ParseResponse(const FString& InData, bool& Success, FString& Message, TArray<uint8>& Output)
{
	// FString 转 JsonObj
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InData);
	TSharedPtr<FJsonObject> RootJsonObj = MakeShareable(new FJsonObject);
		
	if (FJsonSerializer::Deserialize(JsonReader, RootJsonObj))
	{
		FString OutputString;
		if (RootJsonObj->TryGetBoolField(TEXT("success"), Success) && RootJsonObj->TryGetStringField(TEXT("message"), Message) && RootJsonObj->TryGetStringField(TEXT("output"), OutputString))
		{
			// 解码Base64
			FBase64::Decode(OutputString, Output);
			return true;
		}
		UE_LOG(LogFaceSwapProxy, Error, TEXT("Parse Response Fail : json is wrong."))
		return false;
	}
	UE_LOG(LogFaceSwapProxy, Error, TEXT("Parse Response Fail : Cannot deserialize json."))
	return false;
}
