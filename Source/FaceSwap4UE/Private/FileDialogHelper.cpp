// Fill out your copyright notice in the Description page of Project Settings.


#include "FileDialogHelper.h"

#include "Runtime/Engine/Public/UnrealEngine.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/SlateCore/Public/Widgets/SWindow.h"
#include "Runtime/ApplicationCore/Public/GenericPlatform/GenericWindow.h"

#include "Runtime/Core/Public/HAL/Platform.h"
#include "Microsoft/COMPointer.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <string>

#include "Windows/HideWindowsPlatformTypes.h"


bool FileDialogShared(bool bSave, const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, ESelectFlags Flags, TArray<FString>& OutFilenames, int32& OutFilterIndex)
{
	bool bSuccess = false;

	TComPtr<IFileDialog> FileDialog;
	if (SUCCEEDED(::CoCreateInstance(bSave ? CLSID_FileSaveDialog : CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, bSave ? IID_IFileSaveDialog : IID_IFileOpenDialog, IID_PPV_ARGS_Helper(&FileDialog))))
	{
		if (bSave)
		{
			// Set the default "filename"
			if (!DefaultFile.IsEmpty())
			{
				FileDialog->SetFileName(*FPaths::GetCleanFilename(DefaultFile));
			}
		}
		else
		{
			// Set this up as a multi-select picker
			if (Flags==ESelectFlags::EMultiple)
			{
				DWORD dwFlags = 0;
				FileDialog->GetOptions(&dwFlags);
				FileDialog->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);
			}
		}

		// Set up common settings
		FileDialog->SetTitle(*DialogTitle);
		if (!DefaultPath.IsEmpty())
		{
			// SHCreateItemFromParsingName requires the given path be absolute and use \ rather than / as our normalized paths do
			FString DefaultWindowsPath = FPaths::ConvertRelativePathToFull(DefaultPath);
			DefaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);

			TComPtr<IShellItem> DefaultPathItem;
			if (SUCCEEDED(::SHCreateItemFromParsingName(*DefaultWindowsPath, nullptr, IID_PPV_ARGS(&DefaultPathItem))))
			{
				FileDialog->SetFolder(DefaultPathItem);
			}
		}

		// Set-up the file type filters
		TArray<FString> UnformattedExtensions;
		TArray<COMDLG_FILTERSPEC> FileDialogFilters;
		{
			// Split the given filter string (formatted as "Pair1String1|Pair1String2|Pair2String1|Pair2String2") into the Windows specific filter struct
			FileTypes.ParseIntoArray(UnformattedExtensions, TEXT("|"), true);

			if (UnformattedExtensions.Num() % 2 == 0)
			{
				FileDialogFilters.Reserve(UnformattedExtensions.Num() / 2);
				for (int32 ExtensionIndex = 0; ExtensionIndex < UnformattedExtensions.Num();)
				{
					COMDLG_FILTERSPEC& NewFilterSpec = FileDialogFilters[FileDialogFilters.AddDefaulted()];
					NewFilterSpec.pszName = *UnformattedExtensions[ExtensionIndex++];
					NewFilterSpec.pszSpec = *UnformattedExtensions[ExtensionIndex++];
				}
			}
		}
		FileDialog->SetFileTypes(FileDialogFilters.Num(), FileDialogFilters.GetData());

		// Show the picker
		if (SUCCEEDED(FileDialog->Show((HWND)ParentWindowHandle)))
		{
			OutFilterIndex = 0;
			if (SUCCEEDED(FileDialog->GetFileTypeIndex((UINT*)&OutFilterIndex)))
			{
				OutFilterIndex -= 1; // GetFileTypeIndex returns a 1-based index
			}

			auto AddOutFilename = [&OutFilenames](const FString& InFilename)
			{
				FString& OutFilename = OutFilenames[OutFilenames.Add(InFilename)];
				OutFilename = IFileManager::Get().ConvertToRelativePath(*OutFilename);
				FPaths::NormalizeFilename(OutFilename);
			};

			if (bSave)
			{
				TComPtr<IShellItem> Result;
				if (SUCCEEDED(FileDialog->GetResult(&Result)))
				{
					PWSTR pFilePath = nullptr;
					if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
					{
						bSuccess = true;

						// Apply the selected extension if the given filename doesn't already have one
						FString SaveFilePath = pFilePath;
						if (FileDialogFilters.IsValidIndex(OutFilterIndex))
						{
							// May have multiple semi-colon separated extensions in the pattern
							const FString ExtensionPattern = FileDialogFilters[OutFilterIndex].pszSpec;
							TArray<FString> SaveExtensions;
							ExtensionPattern.ParseIntoArray(SaveExtensions, TEXT(";"));

							// Build a "clean" version of the selected extension (without the wildcard)
							FString CleanExtension = SaveExtensions[0];
							if (CleanExtension == TEXT("*.*"))
							{
								CleanExtension.Reset();
							}
							else
							{
								int32 WildCardIndex = INDEX_NONE;
								if (CleanExtension.FindChar(TEXT('*'), WildCardIndex))
								{
									CleanExtension.RightChopInline(WildCardIndex + 1, false);
								}
							}

							// We need to split these before testing the extension to avoid anything within the path being treated as a file extension
							FString SaveFileName = FPaths::GetCleanFilename(SaveFilePath);
							SaveFilePath = FPaths::GetPath(SaveFilePath);

							// Apply the extension if the file name doesn't already have one
							if (FPaths::GetExtension(SaveFileName).IsEmpty() && !CleanExtension.IsEmpty())
							{
								SaveFileName = FPaths::SetExtension(SaveFileName, CleanExtension);
							}

							SaveFilePath /= SaveFileName;
						}
						AddOutFilename(SaveFilePath);

						::CoTaskMemFree(pFilePath);
					}
				}
			}
			else
			{
				IFileOpenDialog* FileOpenDialog = static_cast<IFileOpenDialog*>(FileDialog.Get());

				TComPtr<IShellItemArray> Results;
				if (SUCCEEDED(FileOpenDialog->GetResults(&Results)))
				{
					DWORD NumResults = 0;
					Results->GetCount(&NumResults);
					for (DWORD ResultIndex = 0; ResultIndex < NumResults; ++ResultIndex)
					{
						TComPtr<IShellItem> Result;
						if (SUCCEEDED(Results->GetItemAt(ResultIndex, &Result)))
						{
							PWSTR pFilePath = nullptr;
							if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
							{
								bSuccess = true;
								AddOutFilename(pFilePath);
								::CoTaskMemFree(pFilePath);
							}
						}
					}
				}
			}
		}
	}

	return bSuccess;
}

bool UFileDialogHelper::OpenImageFileDialog(const FString DefaultPath, FString& FilePath)
{
	void* ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
	int32 DummyFilterIndex;
	TArray<FString> OutFilenames;
	if (bool Result = FileDialogShared(false, ParentWindowHandle, TEXT("选择图像文件"), DefaultPath, TEXT(""), TEXT("Image|*.png;*.jpg;*.jpeg;.*.bmp"), ESelectFlags::ENone, OutFilenames, DummyFilterIndex ))
	{
		FilePath = OutFilenames[0];
		return Result;
	}
	FilePath = TEXT("");
	return false;
}

bool UFileDialogHelper::SaveImageFileDialog(const FString DefaultPath, const FString DefaultFileName, FString& FilePath)
{
	void* ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
	int32 DummyFilterIndex;
	TArray<FString> OutFilenames;
	if (bool Result = FileDialogShared(true, ParentWindowHandle, TEXT("保存PNG图像"), DefaultPath, DefaultFileName, TEXT("PNG|*.png"), ESelectFlags::ENone, OutFilenames, DummyFilterIndex ))
	{
		FilePath = OutFilenames[0];
		return Result;
	}
	FilePath = TEXT("");
	return false;
}
