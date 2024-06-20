// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController():
	HudOverlay(nullptr)
{
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Check Overlay
	if(HudOverlayClass)
	{
		HudOverlay = CreateWidget<UUserWidget>(this,HudOverlayClass);
		if(HudOverlay)
		{
			HudOverlay->AddToViewport();
			HudOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
