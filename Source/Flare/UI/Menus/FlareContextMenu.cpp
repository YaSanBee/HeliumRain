
#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "FlareContextMenu.h"

#define LOCTEXT_NAMESPACE "FlareContextMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareContextMenu::Construct(const FArguments& InArgs)
{
	// Data
	TargetShip = NULL;
	TargetStation = NULL;
	OwnerHUD = InArgs._OwnerHUD;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

	// Structure
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(this, &SFlareContextMenu::GetContextMenuPosition)
		[
			SNew(SFlareRoundButton)
			.OnClicked(this, &SFlareContextMenu::OpenTargetMenu)
			.Icon(FFlareStyleSet::GetIcon("DesignatorContextButton"))
			.Text(this, &SFlareContextMenu::GetText)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareContextMenu::SetStation(IFlareSpacecraftInterface* Target)
{
	// TODO M4 GWENN : ONLY ONE MENU FOR BOTH STATIONS AND SHIPS
	TargetStation = Target;
	TargetShip = NULL;
}

void SFlareContextMenu::SetShip(IFlareSpacecraftInterface* Target)
{
	TargetShip = Target;
	TargetStation = NULL;
}

void SFlareContextMenu::Show()
{
	SetVisibility(EVisibility::Visible);
}

void SFlareContextMenu::Hide()
{
	SetVisibility(EVisibility::Hidden);
}

void SFlareContextMenu::OpenTargetMenu()
{
	if (TargetShip)
	{
		OwnerHUD->OpenMenu(EFlareMenu::MENU_Ship, TargetShip);
	}
	else if (TargetStation)
	{
		OwnerHUD->OpenMenu(EFlareMenu::MENU_Station, TargetStation);
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

FMargin SFlareContextMenu::GetContextMenuPosition() const
{
	FVector2D Pos = OwnerHUD->GetContextMenuLocation();

	Pos.X -= 48;
	Pos.Y -= 48;

	return FMargin(Pos.X, Pos.Y, 0, 0);
}

FText SFlareContextMenu::GetText() const
{
	FText Result;
	AFlareSpacecraft* Candidate = NULL;

	if (TargetShip)
	{
		Candidate = Cast<AFlareSpacecraft>(TargetShip);
	}
	else if (TargetStation)
	{
		Candidate = Cast<AFlareSpacecraft>(TargetStation);
	}

	if (Candidate)
	{
		Result = FText::FromString(Candidate->GetName());
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
