#pragma once

#include "CoreMinimal.h"

namespace CodeRescueCollision
{
inline constexpr ECollisionChannel PlayerPawnObject = ECC_GameTraceChannel1;
inline constexpr ECollisionChannel ZombiePawnObject = ECC_GameTraceChannel2;
inline constexpr ECollisionChannel CoverObject = ECC_GameTraceChannel3;
inline constexpr ECollisionChannel PickupObject = ECC_GameTraceChannel4;
inline constexpr ECollisionChannel WeaponTrace = ECC_GameTraceChannel5;
inline constexpr ECollisionChannel AISightTrace = ECC_GameTraceChannel6;
inline constexpr ECollisionChannel InteractionTrace = ECC_GameTraceChannel7;
}
