// CodeRescueBeaconMarkerActor.cpp -- see header. 2026-07-04 word-competition fix.

#include "CodeRescueBeaconMarkerActor.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueMaterialUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    // 2026-07-04 (top-50 item 49): colorblind-safe beacon accents. Category is
    // already shape-coded by the glyph; under a colorblind mode the accent
    // additionally snaps to the nearest Okabe-Ito safe anchor so beam colors
    // stay distinguishable. Tritanopes keep red/green but lose blue/yellow
    // contrast, so their anchor set swaps accordingly.
    FLinearColor SnapToColorblindSafeAccent(const FLinearColor& In, EColorblindMode Mode)
    {
        if (Mode == EColorblindMode::None)
        {
            return In;
        }
        static const FLinearColor DeuterProtAnchors[] = {
            FLinearColor(0.00f, 0.45f, 0.70f),   // strong blue
            FLinearColor(0.90f, 0.62f, 0.00f),   // orange
            FLinearColor(0.34f, 0.71f, 0.91f),   // sky blue
            FLinearColor(0.94f, 0.89f, 0.26f),   // yellow
        };
        static const FLinearColor TritanAnchors[] = {
            FLinearColor(0.84f, 0.16f, 0.16f),   // red
            FLinearColor(0.00f, 0.62f, 0.45f),   // bluish green
            FLinearColor(0.80f, 0.47f, 0.65f),   // reddish purple
            FLinearColor(0.95f, 0.95f, 0.95f),   // near-white
        };
        const FLinearColor* Anchors = (Mode == EColorblindMode::Tritanope) ? TritanAnchors : DeuterProtAnchors;
        const int32 AnchorCount = 4;
        float BestDist = TNumericLimits<float>::Max();
        FLinearColor Best = In;
        for (int32 i = 0; i < AnchorCount; ++i)
        {
            const FLinearColor& A = Anchors[i];
            const float Dist = FMath::Square(In.R - A.R) + FMath::Square(In.G - A.G) + FMath::Square(In.B - A.B);
            if (Dist < BestDist)
            {
                BestDist = Dist;
                Best = A;
            }
        }
        return Best;
    }
}

ACodeRescueBeaconMarkerActor::ACodeRescueBeaconMarkerActor()
{
    // Vertical beam: a thin emissive cylinder. Centered on the actor so half of it reaches
    // down toward the marked object and half rises above the glyph (a "beam from the sky").
    Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconBeam"));
    Beam->SetupAttachment(GetRootComponent());
    Beam->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    Beam->SetRelativeScale3D(FVector(0.16f, 0.16f, BeamHeight / 100.0f));
    Beam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Beam->SetCastShadow(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylMesh.Succeeded())
    {
        Beam->SetStaticMesh(CylMesh.Object);
    }

    // Single large category glyph, always camera-facing (parent Tick already yaws the actor).
    Glyph = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BeaconGlyph"));
    Glyph->SetupAttachment(GetRootComponent());
    Glyph->SetRelativeLocation(FVector(0.0f, 0.0f, 46.0f));
    Glyph->SetHorizontalAlignment(EHTA_Center);
    Glyph->SetVerticalAlignment(EVRTA_TextCenter);
    Glyph->SetWorldSize(64.0f);
    Glyph->SetText(FText::FromString(TEXT("!")));
    Glyph->SetCastShadow(false);

    Tags.Add(FName("BeaconMarker"));
}

void ACodeRescueBeaconMarkerActor::ConfigureBeacon(const FString& InGlyph, const FLinearColor& InAccent)
{
    // 2026-07-04 (top-50 item 49): honor the player's colorblind setting.
    FLinearColor Accent = InAccent;
    if (const UWorld* World = GetWorld())
    {
        if (const UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>())
        {
            Accent = SnapToColorblindSafeAccent(InAccent, GI->ColorblindMode);
        }
    }
    if (Glyph)
    {
        Glyph->SetText(FText::FromString(InGlyph));
        Glyph->SetTextRenderColor(Accent.ToFColor(true));
    }
    if (Beam)
    {
        Beam->SetRelativeScale3D(FVector(0.16f, 0.16f, BeamHeight / 100.0f));
        // Strong emissive tint so the beam blooms and stays legible day and night,
        // without any content-pack material dependency.
        CodeRescueMaterials::ApplyTintedMaterial(Beam, 0, this, Accent * 4.5f, 0.55f);
    }
}

void ACodeRescueBeaconMarkerActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);   // parent: face player, prompt visibility, gentle bob
    PulseTime += DeltaSeconds;

    // 2026-07-16 freeze pass: distant markers skip presentation work (see
    // ACodeRescueMessageMarkerActor::Tick) — the beam holds its last width.
    if (bMarkerAnimationCulled)
    {
        return;
    }

    // Slow breathing pulse on the beam width; near-static under reduced motion.
    const float Rate = bReducedMotion ? 0.6f : 2.0f;
    const float Amount = bReducedMotion ? 0.02f : 0.07f;
    const float S = 0.16f * (1.0f + Amount * FMath::Sin(PulseTime * Rate));
    if (Beam)
    {
        Beam->SetRelativeScale3D(FVector(S, S, BeamHeight / 100.0f));
    }
}
