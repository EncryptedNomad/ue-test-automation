#include "RecordedInput/DaeTestReplayComponent.h"
#include "RecordedInput/DaeTestRecordInterface.h"
#include "DaeTestInputBlueprintFunctionLibrary.h"
#include "DaeTestLogCategory.h"
#include <EngineUtils.h>
#include <Serialization/ArchiveLoadCompressedProxy.h>
#include <PlatformFeatures.h>
#include <SaveGameSystem.h>
#include <GameFramework/PlayerInput.h>

void UDaeTestReplayComponent::LoadRecording()
{
    Initialize();
    LoadBinaryArchive();
    bIsPlayingRecording = true;
}

void UDaeTestReplayComponent::TickComponent(float DeltaTime, ELevelTick Tick,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, Tick, ThisTickFunction);
    
    if (bIsPlayingRecording)
    {
        ProcessNextActionInput();
        ProcessNextAxisInput();
        UpdateTransforms();
    }
}

void UDaeTestReplayComponent::UpdateTransforms()
{
    if (RecordedActorsIndex >= RecordedActors.Num())
    {
        return;
    }

    const float UnpausedTimeSeconds = GetWorld()->GetUnpausedTimeSeconds();
    FDaeTestRecordedActor& NextActor = RecordedActors[RecordedActorsIndex];
    while (NextActor.UnpausedTimeSeconds <= UnpausedTimeSeconds)
    {
        for (FActorIterator It(GetWorld()); It; ++It)
        {
            AActor* Actor = *It;
            if (!IsValid(Actor) || !HasRecordInterface(Actor)
                || BytesFromString(GetFullActorName(Actor)) != NextActor.Name)
            {
                continue;
            }

            TArray<USceneComponent*> SceneComponents;
            IDaeTestRecordInterface::Execute_ComponentsToRecord(Actor, SceneComponents);
            for (auto& RecordedComponentData : NextActor.ComponentsSaveData)
            {
                for (const auto& SceneComponent : SceneComponents)
                {
                    if (!SceneComponent->IsRegistered())
                    {
                        continue;
                    }

                    if (RecordedComponentData.GetName()
                        == BytesFromString(SceneComponent->GetName()))
                    {
                        SceneComponent->SetWorldLocationAndRotation(
                            RecordedComponentData.GetLocation(), RecordedComponentData.GetQuat(),
                            false, nullptr, ETeleportType::TeleportPhysics);

                        break;
                    }
                }
            }
        }

        RecordedActorsIndex++;
        if (RecordedActorsIndex < RecordedActors.Num())
        {
            NextActor = RecordedActors[RecordedActorsIndex];
        }
        else
        {
            break;
        }
    }
}

void UDaeTestReplayComponent::ProcessNextActionInput()
{
    if (RecordedActionInputsIndex < RecordedActionInputs.Num())
    {
        const FDaeTestActorRecordedActionInput& NextAction =
            RecordedActionInputs[RecordedActionInputsIndex];
        if (NextAction.UnpausedTimeSeconds <= GetWorld()->GetUnpausedTimeSeconds())
        {
            const FName ActionName(StringFromBytes(NextAction.ActionName));
            UDaeTestInputBlueprintFunctionLibrary::ApplyInputAction(this, ActionName,
                                                                    NextAction.InputEvent);

            /*UE_LOG(LogDaeTest, Log, TEXT("ActionName: %s | InputEvent: %d"), *ActionName.ToString(),
                   static_cast<int32>(NextAction.InputEvent));*/

            RecordedActionInputsIndex++;
        }
    }
}

void UDaeTestReplayComponent::ProcessNextAxisInput()
{
    if (RecordedAxisInputsIndex < RecordedAxisInputs.Num())
    {
        const FDaeTestActorRecordedAxisInput& NextAxis =
            RecordedAxisInputs[RecordedAxisInputsIndex];
        if (NextAxis.UnpausedTimeSeconds <= GetWorld()->GetUnpausedTimeSeconds())
        {
            const FName AxisName(StringFromBytes(NextAxis.AxisName));
            UDaeTestInputBlueprintFunctionLibrary::ApplyInputAxis(this, AxisName,
                                                                  NextAxis.AxisValue);

            /*UE_LOG(LogDaeTest, Log, TEXT("AxisName: %s | AxisValue: %f"), *AxisName.ToString(),
                   NextAxis.AxisValue);*/

            RecordedAxisInputsIndex++;
        }
    }
}

bool UDaeTestReplayComponent::LoadBinaryArchive()
{
    ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
    if (!SaveSystem->DoesSaveGameExist(*FullSavePath, PlayerIndex))
    {
        return false;
    }

    TArray<uint8> BinaryData;
    if (!SaveSystem->LoadGame(false, *FullSavePath, PlayerIndex, BinaryData))
    {
        UE_LOG(LogDaeTest, Warning, TEXT("%s could not be loaded"), *FullSavePath);
        return false;
    }

    if (IsArrayEmpty(BinaryData))
    {
        UE_LOG(LogDaeTest, Warning, TEXT("No binary data found for %s"), *FullSavePath);
        return false;
    }

    //Decompress and load

    FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(BinaryData, NAME_Zlib);

    if (Decompressor.GetError())
    {
        UE_LOG(LogDaeTest, Error, TEXT("Cannot load, file might not be compressed: %s"),
               *FullSavePath);
        return false;
    }

    FBufferArchive DecompressedBinary;
    Decompressor << DecompressedBinary;

    FMemoryReader FromBinary = FMemoryReader(DecompressedBinary, true);
    FromBinary.Seek(0);

    //Unpack archive and do stuff
    bool bSuccess = UnpackBinaryArchive(FromBinary);

    Decompressor.FlushCache();
    Decompressor.Close();

    DecompressedBinary.Empty();
    DecompressedBinary.Close();

    BinaryData.Empty();

    FromBinary.FlushCache();
    FromBinary.Close();

    return bSuccess;
}

bool UDaeTestReplayComponent::UnpackBinaryArchive(FMemoryReader FromBinary)
{
    FDaeTestRecordedSaveData RecordedSaveData;
    FromBinary << RecordedSaveData;

    RecordedActionInputs = RecordedSaveData.ActionInputs;
    RecordedAxisInputs = RecordedSaveData.AxisInputs;
    RecordedActors = RecordedSaveData.RecordedActors;

    return true;
}
