// Fill out your copyright notice in the Description page of Project Settings.


#include "MixedRealityMathUtilsFunctionLibrary.h"

FRotator UMixedRealityMathUtilsFunctionLibrary::GetRotationBetweenVectors(const FVector &Vector1, const FVector &Vector2)
{
	return FQuat::FindBetween(Vector1, Vector2).Rotator();
}

void UMixedRealityMathUtilsFunctionLibrary::SwingTwistDecompose(const FRotator &Rotation, const FVector &TwistAxis, FRotator &Swing, FRotator &Twist)
{
	FQuat qSwing, qTwist;
	FQuat(Rotation).ToSwingTwist(TwistAxis, qSwing, qTwist);
	Swing = qSwing.Rotator();
	Twist = qTwist.Rotator();
}

FTransform UMixedRealityMathUtilsFunctionLibrary::RotateAboutPivotPoint(const FTransform &Transform, const FRotator &Rotation, const FVector &Pivot)
{
	FTransform result = Transform;
	result.SetLocation(result.GetLocation() - Pivot);
	result *= FTransform(Rotation);
	result.SetLocation(result.GetLocation() + Pivot);
	return result;
}