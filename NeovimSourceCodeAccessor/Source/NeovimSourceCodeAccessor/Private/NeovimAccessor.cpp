//Copyright (c) 2023 Tomi Halko
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "NeovimAccessor.h"
#include "Windows/WindowsPlatformMisc.h"
#include "Misc/CString.h"
#include "Misc/Paths.h"
#include "ISourceCodeAccessModule.h"

#define LOCTEXT_NAMESPACE "FNeovimAccessorModule"

DECLARE_LOG_CATEGORY_EXTERN(LogNeovimAccessor, Log, All);
DEFINE_LOG_CATEGORY(LogNeovimAccessor);

void FNeovimAccessor::RefreshAvailability()
{
    bIsAvailable = false;

    if (FPaths::FileExists(*NeovimPath))
    {
        bIsAvailable = true;
    }
    else
    {
        UE_LOG(LogNeovimAccessor, Warning, TEXT("Neovim executable not found at %s"), *NeovimPath);
    }
}

bool FNeovimAccessor::CanAccessSourceCode() const
{
    return true;
}

FName FNeovimAccessor::GetFName() const
{
    return FName("Neovim");
}

FText FNeovimAccessor::GetNameText() const
{
    return LOCTEXT("NeovimDisplayName", "Neovim");
}

FText FNeovimAccessor::GetDescriptionText() const
{
    return LOCTEXT("NeovimDisplayDesc", "Open source files in Neovim");
}

bool FNeovimAccessor::OpenSolution()
{
    if (!bIsAvailable)
    {
        return false;
    }

    const FString ProjectFileLocation = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    const FString ProjectDirectory = FPaths::GetPath(ProjectFileLocation);

    return OpenSolutionAtPath(ProjectDirectory);
}

bool FNeovimAccessor::OpenSolutionAtPath(const FString& SolutionPath)
{
    FString UProj = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    FString Left, Right;
    UProj.Split(TEXT("."), &Left, &Right);
    const FString Path = Left + TEXT(".sln");
    const FString Params = FString::Printf(TEXT("\"%s\""), *Path);

    // TODO: We should check if there is already a Neovim instance running and use that instead of launching a new one.
    return RunNeovim([this, &Params, &Path]() -> bool
    {
        FProcHandle Proc = FPlatformProcess::CreateProc(*NeovimPath, *Params, true, false, false, nullptr, 0, nullptr, nullptr);
        const bool Success = Proc.IsValid();
        if (!Success)
        {
            UE_LOG(LogNeovimAccessor, Error, TEXT("Failed to launch Neovim with %s, in project file %s."), *Params, *Path);
            FPlatformProcess::CloseProc(Proc);
            return Success;
        }
        return Success;
    });
}

bool FNeovimAccessor::RunNeovim(const TFunction<bool()> Callback) const
{
    ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>("SourceCodeAccess");
    SourceCodeAccessModule.OnLaunchingCodeAccessor().Broadcast();
    const bool Success = Callback();
    SourceCodeAccessModule.OnDoneLaunchingCodeAccessor().Broadcast(Success);
    return Success;
}

bool FNeovimAccessor::DoesSolutionExist() const
{
    const FString SolutionPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    return FPaths::FileExists(SolutionPath);
}

bool FNeovimAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{
    if (!bIsAvailable)
    {
        return false;
    }
    
    const FString Path = FString::Printf(TEXT("\"%s\""), *FullPath);
    const FString Params = FString::Printf(TEXT("+%d %s"), LineNumber, *Path);

    // TODO: We should check if there is already a Neovim instance running and use that instead of launching a new one.
    return RunNeovim([this, &Params, &Path]() -> bool
    {
        FProcHandle Proc = FPlatformProcess::CreateProc(*NeovimPath, *Params, true, false, false, nullptr, 0, nullptr, nullptr);
        const bool Success = Proc.IsValid();
        if (!Success)
        {
            UE_LOG(LogNeovimAccessor, Error, TEXT("Failed to launch Neovim with %s, in project file %s."), *Params, *Path);
            FPlatformProcess::CloseProc(Proc);
            return Success;
        }
        return Success;
    });
}

bool FNeovimAccessor::OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths)
{
    if (!bIsAvailable)
    {
        return false;
    }

    FString SolutionPath = FPaths::GetProjectFilePath();
    FString SourceLocations = "";
    for (const FString& SourcePath : AbsoluteSourcePaths)
    {
        SourceLocations += FString::Printf(TEXT("\"%s\" %s"), *SolutionPath, *SourceLocations);
    }

    const FString Params = FString::Printf(TEXT("\"%s\" %s"), *SolutionPath, *SourceLocations);

    // TODO: We should check if there is already a Neovim instance running and use that instead of launching a new one.
    return RunNeovim([this, &Params, &SourceLocations]() -> bool
    {
        FProcHandle Proc = FPlatformProcess::CreateProc(*NeovimPath, *Params, true, false, false, nullptr, 0, nullptr, nullptr);
        const bool Success = Proc.IsValid();
        if (!Success)
        {
            UE_LOG(LogNeovimAccessor, Error, TEXT("Failed to launch Neovim with %s, in project file %s."), *Params, *SourceLocations);
            FPlatformProcess::CloseProc(Proc);
            return Success;
        }
        return Success;
    });
}

bool FNeovimAccessor::AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules)
{
    return true;
}

bool FNeovimAccessor::SaveAllOpenDocuments() const
{
    return false;
}

void FNeovimAccessor::Tick(const float DeltaTime){}

#undef LOCTEXT_NAMESPACE

