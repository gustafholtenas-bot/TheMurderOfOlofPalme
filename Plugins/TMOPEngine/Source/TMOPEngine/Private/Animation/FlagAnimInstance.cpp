#include "Animation/FlagAnimInstance.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/Crc.h"

void UFlagAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    StartOffset = 0.0f;
    const USkeletalMeshComponent* Mesh = GetSkelMeshComponent();
    if (!Mesh || !FlagSequence)
    {
        return;
    }

    const float Duration = FlagSequence->GetPlayLength();
    if (Duration <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const FVector2D Direction = WindDirection.GetSafeNormal();
    if (Direction.IsNearlyZero())
    {
        return;
    }

    const FVector Position = Mesh->GetComponentLocation();
    const FVector2D PositionXY(Position.X, Position.Y);
    const double DistanceMeters = FVector2D::DotProduct(PositionXY, Direction) / 100.0;
    const double Speed = FMath::Max(static_cast<double>(WindSpeedMetersPerSecond), 0.01);

    // A wave traveling in +WindDirection has phase (time - distance/speed).
    // Negative delay is intentional: flags farther downwind respond later.
    double PhaseSeconds = -DistanceMeters / Speed;
    PhaseSeconds += DistanceMeters * static_cast<double>(ExtraCyclesPerMeter) * Duration;

    if (JitterSeconds > 0.0f)
    {
        // Stable spatial hash: no per-frame random values and no global RNG state.
        const FIntVector Cell(
            FMath::FloorToInt(Position.X),
            FMath::FloorToInt(Position.Y),
            FMath::FloorToInt(Position.Z));
        const uint32 Hash = FCrc::MemCrc32(&Cell, sizeof(Cell));
        const double Uniform01 = static_cast<double>(Hash) / static_cast<double>(MAX_uint32);
        PhaseSeconds += (Uniform01 * 2.0 - 1.0) * JitterSeconds;
    }

    // Wrap safely even when world coordinates are negative.
    double Wrapped = FMath::Fmod(PhaseSeconds, static_cast<double>(Duration));
    if (Wrapped < 0.0)
    {
        Wrapped += Duration;
    }
    StartOffset = static_cast<float>(Wrapped);
}
