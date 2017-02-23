
#include "../Flare.h"
#include "FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Economy/FlareCargoBay.h"
#include "../Game/AI/FlareCompanyAI.h"


#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareHUD::AFlareHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CombatMouseRadius(100)
	, HUDVisible(true)
	, IsDrawingCockpit(false)
{
	// Load content (general icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDReticleIconObj         (TEXT("/Game/Gameplay/HUD/TX_Reticle.TX_Reticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCombatReticleIconobj   (TEXT("/Game/Gameplay/HUD/TX_CombatReticle.TX_CombatReticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBackReticleIconObj     (TEXT("/Game/Gameplay/HUD/TX_BackReticle.TX_BackReticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimIconObj             (TEXT("/Game/Gameplay/HUD/TX_Aim.TX_Aim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBombAimIconObj         (TEXT("/Game/Gameplay/HUD/TX_BombAim.TX_BombAim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBombMarkerObj          (TEXT("/Game/Gameplay/HUD/TX_BombMarker.TX_BombMarker"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimHelperIconObj       (TEXT("/Game/Gameplay/HUD/TX_AimHelper.TX_AimHelper"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDNoseIconObj            (TEXT("/Game/Gameplay/HUD/TX_Nose.TX_Nose"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDObjectiveIconObj       (TEXT("/Game/Gameplay/HUD/TX_Objective.TX_Objective"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCombatMouseIconObj     (TEXT("/Game/Gameplay/HUD/TX_CombatCursor.TX_CombatCursor"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorCornerObj    (TEXT("/Game/Gameplay/HUD/TX_DesignatorCorner.TX_DesignatorCorner"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectedCornerObj(TEXT("/Game/Gameplay/HUD/TX_DesignatorCornerSelected.TX_DesignatorCornerSelected"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectionObj (TEXT("/Game/Slate/Icons/TX_Icon_TargettingContextButton.TX_Icon_TargettingContextButton"));

	// Load content (status icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDTemperatureIconObj     (TEXT("/Game/Slate/Icons/TX_Icon_Temperature.TX_Icon_Temperature"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPowerIconObj           (TEXT("/Game/Slate/Icons/TX_Icon_Power.TX_Icon_Power"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPropulsionIconObj      (TEXT("/Game/Slate/Icons/TX_Icon_Propulsion.TX_Icon_Propulsion"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDRCSIconObj             (TEXT("/Game/Slate/Icons/TX_Icon_RCS.TX_Icon_RCS"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDHealthIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_LifeSupport.TX_Icon_LifeSupport"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDWeaponIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_Shell.TX_Icon_Shell"));

	// Load content (font)
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontObj                (TEXT("/Game/Slate/Fonts/HudFont.HudFont"));
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontLargeObj           (TEXT("/Game/Slate/Fonts/HudFontLarge.HudFontLarge"));

	// Set content (general icons)
	HUDReticleIcon = HUDReticleIconObj.Object;
	HUDCombatReticleIcon = HUDCombatReticleIconobj.Object;
	HUDBackReticleIcon = HUDBackReticleIconObj.Object;
	HUDAimIcon = HUDAimIconObj.Object;
	HUDBombAimIcon = HUDBombAimIconObj.Object;
	HUDBombMarker = HUDBombMarkerObj.Object;
	HUDAimHelperIcon = HUDAimHelperIconObj.Object;
	HUDNoseIcon = HUDNoseIconObj.Object;
	HUDObjectiveIcon = HUDObjectiveIconObj.Object;
	HUDCombatMouseIcon = HUDCombatMouseIconObj.Object;
	HUDDesignatorCornerTexture = HUDDesignatorCornerObj.Object;
	HUDDesignatorCornerSelectedTexture = HUDDesignatorSelectedCornerObj.Object;
	HUDDesignatorSelectionTexture = HUDDesignatorSelectionObj.Object;

	// Set content (status icons)
	HUDTemperatureIcon = HUDTemperatureIconObj.Object;
	HUDPowerIcon = HUDPowerIconObj.Object;
	HUDPropulsionIcon = HUDPropulsionIconObj.Object;
	HUDRCSIcon = HUDRCSIconObj.Object;
	HUDHealthIcon = HUDHealthIconObj.Object;
	HUDWeaponIcon = HUDWeaponIconObj.Object;

	// Set content (font)
	HUDFont = HUDFontObj.Object;
	HUDFontLarge = HUDFontLargeObj.Object;

	// Settings
	FocusDistance = 10000000;
	IconSize = 24;
	ShadowOffset = FVector2D(1, 1);
	ShadowColor = FLinearColor(0.02, 0.02, 0.02, 1.0f);

	// Cockpit instruments
	TopInstrument =   FVector2D(20, 10);
	LeftInstrument =  FVector2D(20, 165);
	RightInstrument = FVector2D(24, 320);
	InstrumentSize =  FVector2D(380, 115);
	InstrumentLine =  FVector2D(0, 20);

	// Debug
	DistortionGrid = 0;
}

void AFlareHUD::BeginPlay()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get HUD colors
	HudColorNeutral = Theme.NeutralColor;
	HudColorFriendly = Theme.FriendlyColor;
	HudColorEnemy = Theme.EnemyColor;
	HudColorObjective = Theme.ObjectiveColor;
	HudColorNeutral.A = Theme.DefaultAlpha;
	HudColorFriendly.A = Theme.DefaultAlpha;
	HudColorEnemy.A = Theme.DefaultAlpha;
	HudColorObjective.A = Theme.DefaultAlpha;

	Super::BeginPlay();
}

void AFlareHUD::Setup(AFlareMenuManager* NewMenuManager)
{
	MenuManager = NewMenuManager;

	if (GEngine->IsValidLowLevel())
	{
		// Create HUD menus
		SAssignNew(HUDMenu, SFlareHUDMenu).MenuManager(MenuManager);
		SAssignNew(MouseMenu, SFlareMouseMenu).MenuManager(MenuManager);

		// Context menu
		SAssignNew(ContextMenuContainer, SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(ContextMenu, SFlareContextMenu)
				.HUD(this)
				.MenuManager(MenuManager)
			];

		// Register menus at their Z-Index
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(HUDMenu.ToSharedRef()), 0);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MouseMenu.ToSharedRef()), 5);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ContextMenuContainer.ToSharedRef()), 10);
	}
}


/*----------------------------------------------------
	HUD interaction
----------------------------------------------------*/

void AFlareHUD::ToggleHUD()
{
	HUDVisible = !HUDVisible;
	UpdateHUDVisibility();
}

void AFlareHUD::SetInteractive(bool Status)
{
	IsInteractive = Status;
}

void AFlareHUD::SetWheelMenu(bool State)
{
	if (State)
	{
		MouseMenu->Open();
	}
	else
	{
		MouseMenu->Close();
	}
}

void AFlareHUD::SetWheelCursorMove(FVector2D Move)
{
	MouseMenu->SetWheelCursorMove(Move);
}

bool AFlareHUD::IsWheelMenuOpen() const
{
	return MouseMenu->IsOpen();
}

void AFlareHUD::OnTargetShipChanged()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->GetPlayerShip())
	{
		HUDMenu->SetTargetShip(PC->GetPlayerShip());
		UpdateHUDVisibility();
	}
}

void AFlareHUD::UpdateHUDVisibility()
{
	bool NewVisibility = HUDVisible && !MenuManager->IsUIOpen();
	AFlarePlayerController* PC = MenuManager->GetPC();

	FLOGV("AFlareHUD::UpdateHUDVisibility : new state is %d", NewVisibility);
	HUDMenu->SetVisibility((NewVisibility && !PC->UseCockpit) ? EVisibility::Visible : EVisibility::Collapsed);
	ContextMenu->SetVisibility(NewVisibility && !MenuManager->IsSwitchingMenu() ? EVisibility::Visible : EVisibility::Collapsed);
}

void AFlareHUD::RemoveTarget(AFlareSpacecraft* Spacecraft)
{
	for (int32 Index = 0; Index < ScreenTargets.Num(); Index++)
	{
		if (ScreenTargets[Index].Spacecraft == Spacecraft)
		{
			ScreenTargets.RemoveAt(Index);
			return;
		}
	}
}

void AFlareHUD::RemoveAllTargets()
{
	ScreenTargets.Empty();
}

void AFlareHUD::DrawHUD()
{
	Super::DrawHUD();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	if (PC)
	{
		// Setup data
		CurrentViewportSize = ViewportSize;
		CurrentCanvas = Canvas;
		IsDrawingCockpit = false;
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

		// Initial data and checks
		if (!ShouldDrawHUD())
		{
			return;
		}

		// Draw the general-purpose HUD (no-cockpit version)
		bool IsExternalCamera = PlayerShip->GetStateManager()->IsExternalCamera();
		if (HUDVisible && (!PC->UseCockpit || IsExternalCamera))
		{
			DrawHUDInternal();
		}

		// Draw nose
		if (HUDVisible && !IsExternalCamera)
		{
			DrawHUDIcon(CurrentViewportSize / 2, IconSize, PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN ? HUDAimIcon : HUDNoseIcon, HudColorNeutral, true);
		}

		// Draw combat mouse pointer
		if (HUDVisible && !PlayerShip->GetNavigationSystem()->IsAutoPilot() && (
			PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_NONE
			|| PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN))
		{
			// Compute clamped mouse position
			FVector2D MousePosDelta = CombatMouseRadius * PlayerShip->GetStateManager()->GetPlayerMouseOffset();
			FVector MousePosDelta3D = FVector(MousePosDelta.X, MousePosDelta.Y, 0);
			MousePosDelta3D = MousePosDelta3D.GetClampedToMaxSize(CombatMouseRadius);
			MousePosDelta = FVector2D(MousePosDelta3D.X, MousePosDelta3D.Y);

			// Keep an offset
			FVector2D MinimalOffset = MousePosDelta;
			MinimalOffset.Normalize();
			MousePosDelta += 12 * MinimalOffset;

			// Draw
			FLinearColor PointerColor = HudColorNeutral;
			PointerColor.A = FMath::Clamp((MousePosDelta.Size() / CombatMouseRadius) - 0.1f, 0.0f, PointerColor.A);
			DrawHUDIconRotated(CurrentViewportSize / 2 + MousePosDelta, IconSize, HUDCombatMouseIcon, PointerColor, MousePosDelta3D.Rotation().Yaw);
		}

		// Distorsion grid
		if (DistortionGrid)
		{
			DrawDebugGrid(HudColorEnemy);
		}
	}
}

void AFlareHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());

	// Mouse control
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && !MouseMenu->IsOpen())
	{
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
		MousePos = 2 * ((MousePos - ViewportCenter) / ViewportSize);
		PC->MousePositionInput(MousePos);
	}
}


/*----------------------------------------------------
	Cockpit HUD drawing
----------------------------------------------------*/

void AFlareHUD::DrawCockpitHUD(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->UseCockpit)
	{
		CurrentViewportSize = FVector2D(Width, Height);
		CurrentCanvas = TargetCanvas;
		IsDrawingCockpit = true;

		if (HUDVisible && ShouldDrawHUD())
		{
			DrawHUDInternal();

			if (DistortionGrid)
			{
				DrawDebugGrid(HudColorFriendly);
			}
		}
	}
}

void AFlareHUD::DrawCockpitInstruments(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->UseCockpit)
	{
		// Dynamic data
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		CurrentViewportSize = FVector2D(Width, Height);
		CurrentCanvas = TargetCanvas;

		// Draw instruments
		if (PlayerShip && PlayerShip->IsValidLowLevel() && PlayerShip->GetParent()->GetDamageSystem()->IsAlive())
		{
			// Regular case
			if (!PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage())
			{
				DrawCockpitSubsystems(PlayerShip);
				DrawCockpitEquipment(PlayerShip);
				DrawCockpitTarget(PlayerShip);
			}

			// Power out
			else
			{
				// Power out text
				FText PowerOut = LOCTEXT("PowerOut", "NO POWER");
				FText PowerOutInfo = FText::Format(LOCTEXT("PwBackInFormat", "Power back in {0}..."),
					FText::AsNumber((int32)(PlayerShip->GetParent()->GetDamageSystem()->GetPowerOutageDuration()) + 1));

				// Draw
				FVector2D CurrentPos = RightInstrument;
				const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
				FlareDrawText(PowerOut.ToString(), CurrentPos, Theme.EnemyColor, false, true);
				CurrentPos += 2 * InstrumentLine;
				FlareDrawText(PowerOutInfo.ToString(), CurrentPos, Theme.EnemyColor, false);
			}
		}
	}
}

void AFlareHUD::DrawCockpitSubsystems(AFlareSpacecraft* PlayerShip)
{
	// Data
	float CockpitIconSize = 20;
	FVector2D CurrentPos = LeftInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Temperature = PlayerShip->GetParent()->GetDamageSystem()->GetTemperature();
	FText TemperatureText = FText::Format(LOCTEXT("TemperatureFormat", "Hull Temperature: {0}K"), FText::AsNumber(Temperature));

	// Ship name
	FString FlyingText = PlayerShip->GetParent()->GetImmatriculation().ToString();
	FlareDrawText(FlyingText, CurrentPos, Theme.FriendlyColor, false, true);
	CurrentPos += 2 * InstrumentLine;

	// Ship status
	FlareDrawText(PlayerShip->GetShipStatus().ToString(), CurrentPos, Theme.FriendlyColor, false);
	CurrentPos += InstrumentLine;
	
	// Temperature text
	FLinearColor TemperatureColor = GetTemperatureColor(Temperature, PlayerShip->GetParent()->GetDamageSystem()->GetOverheatTemperature());
	DrawHUDIcon(CurrentPos, CockpitIconSize, HUDTemperatureIcon, TemperatureColor);
	FlareDrawText(TemperatureText.ToString(), CurrentPos + FVector2D(1.5 * CockpitIconSize, 0), TemperatureColor, false);
	CurrentPos += InstrumentLine;

	// Subsystem health
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Temperature, CurrentPos);
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Propulsion, CurrentPos);
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_RCS, CurrentPos);
	CurrentPos += FVector2D(InstrumentSize.X / 2, 0) - 3 * InstrumentLine;
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_LifeSupport, CurrentPos);
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Power, CurrentPos);

	// Weapons
	if (PlayerShip->GetParent()->IsMilitary())
	{
		DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Weapon, CurrentPos);
	}

	// Ship icon
	int32 ShipIconSize = 80;
	UTexture2D* ShipIcon = Cast<UTexture2D>(PlayerShip->GetParent()->GetDescription()->MeshPreviewBrush.GetResourceObject());
	DrawHUDIcon(LeftInstrument + FVector2D(InstrumentSize.X - ShipIconSize, 0), ShipIconSize, ShipIcon, Theme.FriendlyColor);
}

void AFlareHUD::DrawCockpitEquipment(AFlareSpacecraft* PlayerShip)
{
	FVector2D CurrentPos = RightInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Fighter version
	if (PlayerShip->GetParent()->IsMilitary() && PlayerShip->GetParent()->GetDescription()->Size == EFlarePartSize::S)
	{
		FText TitleText;
		FText InfoText;
		FLinearColor HealthColor = Theme.FriendlyColor;
		int32 CurrentWeapongroupIndex = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex();
		FFlareWeaponGroup* CurrentWeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
		FText DisarmText = LOCTEXT("WeaponsDisabled", "Standing down");;

		if (CurrentWeaponGroup)
		{
			float ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(CurrentWeapongroupIndex);
			HealthColor = GetHealthColor(ComponentHealth);

			// Get ammo count
			int32 RemainingAmmo = 0;
			for (int32 i = 0; i < CurrentWeaponGroup->Weapons.Num(); i++)
			{
				if (CurrentWeaponGroup->Weapons[i]->GetDamageRatio() <= 0.0f)
				{
					continue;
				}
				RemainingAmmo += CurrentWeaponGroup->Weapons[i]->GetCurrentAmmo();
			}

			// Final strings
			TitleText = CurrentWeaponGroup->Description->Name;
			InfoText = FText::Format(LOCTEXT("WeaponInfoFormat", "{0} - {1}%"),
				FText::Format(LOCTEXT("Rounds", "{0} rounds"), FText::AsNumber(RemainingAmmo)),
				FText::AsNumber((int32)(100 * ComponentHealth)));
		}
		else
		{
			TitleText = DisarmText;
		}

		// Draw text
		FlareDrawText(TitleText.ToString(), CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;
		FlareDrawText(InfoText.ToString(), CurrentPos, HealthColor, false);
		CurrentPos += InstrumentLine;

		// Weapon icon
		if (CurrentWeaponGroup)
		{
			int32 WeaponIconSize = 80;
			UTexture2D* WeaponIcon = Cast<UTexture2D>(CurrentWeaponGroup->Weapons[0]->GetDescription()->MeshPreviewBrush.GetResourceObject());
			DrawHUDIcon(RightInstrument + FVector2D(InstrumentSize.X - WeaponIconSize, 0), WeaponIconSize, WeaponIcon, Theme.FriendlyColor);
		}

		// Weapon list
		TArray<FFlareWeaponGroup*>& WeaponGroupList = PlayerShip->GetWeaponsSystem()->GetWeaponGroupList();
		for (int32 i = WeaponGroupList.Num() - 1; i >= 0; i--)
		{
			float WeaponHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(i);
			FText WeaponText = FText::Format(LOCTEXT("WeaponListInfoFormat", "{0}. {1} - {2}%"),
				FText::AsNumber(i + 2), 
				WeaponGroupList[i]->Description->Name,
				FText::AsNumber((int32)(100 * WeaponHealth)));
			FString WeaponString = ((i == CurrentWeapongroupIndex) ? FString("> ") : FString("   ")) + WeaponText.ToString();

			HealthColor = GetHealthColor(WeaponHealth);
			FlareDrawText(WeaponString, CurrentPos, HealthColor, false);
			CurrentPos += InstrumentLine;
		}

		// No weapon
		FString DisarmedName = FString("1. ") + DisarmText.ToString();
		DisarmedName = ((CurrentWeapongroupIndex == -1) ? FString("> ") : FString("    ")) + DisarmedName;
		FlareDrawText(DisarmedName, CurrentPos, HealthColor, false);
		CurrentPos += InstrumentLine;
	}

	// Capital ship version
	else if (PlayerShip->GetParent()->IsMilitary())
	{
		// Title
		FText CommandGroupText = LOCTEXT("ShipGroups", "Command groups");
		FlareDrawText(CommandGroupText.ToString(), CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;

		int32 CurrentGroupIndex = MenuManager->GetPC()->GetCompany()->GetAI()->GetCurrentShipGroup();

		// Group list
		for (int32 Index = EFlareCombatGroup::AllMilitary; Index <= EFlareCombatGroup::Civilan; Index++)
		{
			EFlareCombatGroup::Type GroupType = static_cast<EFlareCombatGroup::Type>(Index);
			FText GroupName = UFlareGameTypes::GetCombatGroupDescription(GroupType);
			FText GroupText = FText::Format(LOCTEXT("GroupListInfoFormat", "{0}. {1} ({2}) : {3}"),
				FText::AsNumber(Index + 1),
				GroupName,
				FText::AsNumber(MenuManager->GetPC()->GetCompany()->GetAI()->GetShipCountForShipGroup(GroupType)),
				UFlareGameTypes::GetCombatTacticDescription(MenuManager->GetPC()->GetCompany()->GetAI()->GetCurrentTacticForShipGroup(GroupType)));

			FString GroupString = ((Index == CurrentGroupIndex) ? FString("> ") : FString("   ")) + GroupText.ToString();

			FlareDrawText(GroupString, CurrentPos, Theme.FriendlyColor, false);
			CurrentPos += InstrumentLine;
		}
	}

	// Unarmed version
	else
	{
		UFlareCargoBay* CargoBay = PlayerShip->GetParent()->GetCargoBay();
		check(CargoBay);

		// Title
		FText CargoText = LOCTEXT("CargoText", "Cargo bay");
		FlareDrawText(CargoText.ToString(), CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;

		// Subtitle
		FText CargoInfoText = FText::Format(LOCTEXT("CargoInfoFormat", "Current usage : {0} / {1}"),
			FText::AsNumber(CargoBay->GetUsedCargoSpace()), FText::AsNumber(CargoBay->GetCapacity()));
		FlareDrawText(CargoInfoText.ToString(), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += InstrumentLine;

		// Cargo bay slots
		uint32 MaxCargoBayCount = 8;
		TArray<FFlareCargo>& CargoBaySlots = CargoBay->GetSlots();
		for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
		{
			// Create text
			FFlareCargo& Cargo = CargoBaySlots[CargoIndex];
			FText CargoSlotResourceText = Cargo.Resource ? Cargo.Resource->Acronym : LOCTEXT("CargoBaySlotEmpty", "Empty");
			FText CargoBaySlotText = FText::Format(LOCTEXT("CargoBaySlotFormat", "{0} ({1}/{2})"),
				CargoSlotResourceText, FText::AsNumber(Cargo.Quantity), FText::AsNumber(Cargo.Capacity));

			// Go to the right column after n/2 slots
			if (CargoIndex == MaxCargoBayCount / 2)
			{
				CurrentPos += FVector2D(InstrumentSize.X / 2, 0) - (MaxCargoBayCount / 2) * InstrumentLine;
			}

			// Draw
			FlareDrawText(CargoBaySlotText.ToString(), CurrentPos, Theme.FriendlyColor, false);
			CurrentPos += InstrumentLine;
		}
	}
}

void AFlareHUD::DrawCockpitTarget(AFlareSpacecraft* PlayerShip)
{
	FVector2D CurrentPos = TopInstrument + FVector2D(0, InstrumentSize.Y) - InstrumentLine;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Sector info
	UFlareSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector();

	if (CurrentSector)
	{
		// Get text
		FText SectorText;
		if (PlayerShip->GetParent()->GetCurrentFleet()->IsTraveling())
		{
			SectorText = PlayerShip->GetParent()->GetCurrentFleet()->GetStatusInfo();
		}
		else
		{
			SectorText = FText::Format(LOCTEXT("CurrentSectorFormat", "Current sector : {0} ({1})"),
				CurrentSector->GetSimulatedSector()->GetSectorName(),
				CurrentSector->GetSimulatedSector()->GetSectorFriendlynessText(PlayerShip->GetParent()->GetCompany()));
		}

		// Draw
		FlareDrawText(SectorText.ToString(), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += InstrumentLine;

		// Target info
		AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
		if (TargetShip && TargetShip->IsValidLowLevel())
		{
			FText ShipText = FText::Format(LOCTEXT("CurrentTargetFormat", "Current target : {0} ({1})"),
				FText::FromString(TargetShip->GetParent()->GetImmatriculation().ToString()),
				TargetShip->GetParent()->GetCompany()->GetPlayerHostilityText());
			FlareDrawText(ShipText.ToString(), CurrentPos, Theme.FriendlyColor, false);
		}
	}
}

void AFlareHUD::DrawCockpitSubsystemInfo(EFlareSubsystem::Type Subsystem, FVector2D& Position)
{
	AFlareSpacecraft* PlayerShip = MenuManager->GetPC()->GetShipPawn();
	float ComponentHealth = PlayerShip->GetParent()->GetDamageSystem()->GetSubsystemHealth(Subsystem);

	FText SystemText = FText::Format(LOCTEXT("SubsystemInfoFormat", "{0}: {1}%"),
		UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName(Subsystem),
		FText::AsNumber((int32)(100 * ComponentHealth)));

	// Drawing data
	UTexture2D* Icon = NULL;
	float CockpitIconSize = 20;
	FLinearColor HealthColor = GetHealthColor(ComponentHealth);
	
	// Icon
	switch (Subsystem)
	{
		case EFlareSubsystem::SYS_Temperature:
			Icon = HUDTemperatureIcon;
			break;
		case EFlareSubsystem::SYS_Propulsion:
			Icon = HUDPropulsionIcon;
			break;
		case EFlareSubsystem::SYS_RCS:
			Icon = HUDRCSIcon;
			break;
		case EFlareSubsystem::SYS_LifeSupport:
			Icon = HUDHealthIcon;
			break;
		case EFlareSubsystem::SYS_Power:
			Icon = HUDPowerIcon;
			break;
		case EFlareSubsystem::SYS_Weapon:
			Icon = HUDWeaponIcon;
			break;
	}

	DrawHUDIcon(Position, CockpitIconSize, Icon, HealthColor);
	FlareDrawText(SystemText.ToString(), Position + FVector2D(1.5 * CockpitIconSize, 0), HealthColor, false);
	Position += InstrumentLine;
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FLinearColor AFlareHUD::GetTemperatureColor(float Current, float Max)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	float Distance = Current - Max;
	float Ratio = FMath::Clamp(FMath::Abs(Distance) / 10.0f, 0.0f, 1.0f);

	if (Distance < 0)
	{
		Ratio = 0.0f;
	}

	return FMath::Lerp(NormalColor, DamageColor, Ratio);
}

FLinearColor AFlareHUD::GetHealthColor(float Current)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	float Ratio = FMath::Clamp(1 - Current, 0.0f, 1.0f);
	return FMath::Lerp(NormalColor, DamageColor, Ratio);
}

	
/*----------------------------------------------------
	HUD drawing
----------------------------------------------------*/

inline static bool IsCloserToCenter(const FFlareScreenTarget& TargetA, const FFlareScreenTarget& TargetB)
{
	return (TargetA.DistanceFromScreenCenter < TargetB.DistanceFromScreenCenter);
}


bool AFlareHUD::ShouldDrawHUD() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	UFlareSector* ActiveSector = PC->GetGame()->GetActiveSector();
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

	if (!ActiveSector || !PlayerShip || !PlayerShip->GetParent()->GetDamageSystem()->IsAlive() || MenuManager->IsUIOpen() || MenuManager->IsSwitchingMenu() || IsWheelMenuOpen())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AFlareHUD::DrawHUDInternal()
{
	// Initial data and checks
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
	UFlareSector* ActiveSector = PC->GetGame()->GetActiveSector();
		
	// Iterate on all 'other' ships to show designators, markings, etc
	ScreenTargets.Empty();
	ScreenTargetsOwner = PlayerShip->GetParent()->GetImmatriculation();
	FoundTargetUnderMouse = false;
	for (int SpacecraftIndex = 0; SpacecraftIndex < ActiveSector->GetSpacecrafts().Num(); SpacecraftIndex ++)
	{
		AFlareSpacecraft* Spacecraft = ActiveSector->GetSpacecrafts()[SpacecraftIndex];
		if (Spacecraft->IsValidLowLevel() && Spacecraft != PlayerShip)
		{
			// Draw designators
			bool ShouldDrawSearchMarker;
			if (PC->LineOfSightTo(Spacecraft))
			{
				ShouldDrawSearchMarker = DrawHUDDesignator(Spacecraft);
			}
			else
			{
				ShouldDrawSearchMarker = Spacecraft->GetParent()->GetDamageSystem()->IsAlive();
			}

			// Draw search markers
			if (!PlayerShip->GetStateManager()->IsExternalCamera() && ShouldDrawSearchMarker)
			{
				DrawSearchArrow(Spacecraft->GetActorLocation(), GetHostilityColor(PC, Spacecraft), FocusDistance);
			}
		}

		// Hide the context menu if nothing was found
		if (!FoundTargetUnderMouse || !IsInteractive)
		{
			ContextMenu->Hide();
		}
	}

	// Update HUD materials
	if (PlayerShip)
	{
		// Draw inertial vectors
		FVector ShipSmoothedVelocity = PlayerShip->GetSmoothedLinearVelocity() * 100;
		bool Firing = (PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN);

		if (Firing)
		{
			DrawSpeed(PC, PlayerShip, HUDCombatReticleIcon, ShipSmoothedVelocity, FText(), false);
			DrawSpeed(PC, PlayerShip, HUDCombatReticleIcon, -ShipSmoothedVelocity, FText(), true);
		}
		else
		{
			DrawSpeed(PC, PlayerShip, HUDReticleIcon, ShipSmoothedVelocity, LOCTEXT("Forward", "FWD"), false);
			DrawSpeed(PC, PlayerShip, HUDBackReticleIcon, -ShipSmoothedVelocity, LOCTEXT("Backward", "BWD"), true);
		}

		// Draw objective
		if (PC->HasObjective() && PC->GetCurrentObjective()->Data.TargetList.Num() > 0)
		{
			FVector2D ScreenPosition;

			for (int TargetIndex = 0; TargetIndex < PC->GetCurrentObjective()->Data.TargetList.Num(); TargetIndex++)
			{
				const FFlarePlayerObjectiveTarget* Target = &PC->GetCurrentObjective()->Data.TargetList[TargetIndex];
				FVector ObjectiveLocation = Target->Location;
				FLinearColor InactiveColor = HudColorNeutral;

				bool ShouldDrawMarker = false;

				if (ProjectWorldLocationToCockpit(ObjectiveLocation, ScreenPosition))
				{
					if (IsInScreen(ScreenPosition))
					{
						// Draw icon
						DrawHUDIcon(ScreenPosition, IconSize, HUDObjectiveIcon, (Target->Active ? HudColorNeutral : InactiveColor), true);

						float Distance = (ObjectiveLocation - PlayerShip->GetActorLocation()).Size() / 100;
						FString ObjectiveText = FormatDistance(Distance);

						// Draw distance
						FVector2D CenterScreenPosition = ScreenPosition - CurrentViewportSize / 2 + FVector2D(0, IconSize);
						FlareDrawText(ObjectiveText, CenterScreenPosition, (Target->Active ? HudColorObjective : HudColorNeutral));
					}

					// Tell the HUD to draw the search marker only if we are outside this
					ShouldDrawMarker = (FVector2D::Distance(ScreenPosition, CurrentViewportSize / 2) >= (CurrentViewportSize.GetMin() / 3));
				}
				else
				{
					ShouldDrawMarker = true;
				}

				if (ShouldDrawMarker && !PlayerShip->GetStateManager()->IsExternalCamera() && Target->Active)
				{
					DrawSearchArrow(ObjectiveLocation, HudColorObjective);
				}
			}
		}

		// Draw bomb marker
		if (PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB)
		{
			float AmmoVelocity = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup()->Weapons[0]->GetAmmoVelocity();
			FRotator ShipAttitude = PlayerShip->GetActorRotation();
			FVector ShipVelocity = 100.f * PlayerShip->GetLinearVelocity();

			// Bomb velocity
			FVector BombVelocity = ShipAttitude.Vector();
			BombVelocity.Normalize();
			BombVelocity *= 100.f * AmmoVelocity;

			FVector BombDirection = (ShipVelocity + BombVelocity).GetUnsafeNormal();
			FVector BombTarget = PlayerShip->GetActorLocation() + BombDirection * 1000000;

			FVector2D ScreenPosition;
			if (ProjectWorldLocationToCockpit(BombTarget, ScreenPosition))
			{
				// Icon
				DrawHUDIcon(ScreenPosition, IconSize, HUDBombAimIcon, HudColorNeutral, true);
			}
		}

		// Draw bombs
		for (int32 BombIndex = 0; BombIndex < ActiveSector->GetBombs().Num(); BombIndex++)
		{
			FVector2D ScreenPosition;
			AFlareBomb* Bomb = ActiveSector->GetBombs()[BombIndex];
			
			if (Bomb && ProjectWorldLocationToCockpit(Bomb->GetActorLocation(), ScreenPosition))
			{
				if (IsInScreen(ScreenPosition))
				{
					DrawHUDIcon(ScreenPosition, IconSize, HUDBombMarker, GetHostilityColor(PC, Bomb->GetFiringSpacecraft()) , true);
				}
			}
		}
	}

	// Sort screen targets
	ScreenTargets.Sort(&IsCloserToCenter);
}

void AFlareHUD::DrawDebugGrid(FLinearColor Color)
{
	float HPrecision = DistortionGrid;
	float VPrecision = DistortionGrid;
	
	for (int32 HIndex = -HPrecision; HIndex <= HPrecision; HIndex++)
	{
		for (int32 VIndex = -VPrecision; VIndex <= VPrecision; VIndex++)
		{

			FVector2D Screen = FVector2D(0.5 * ViewportSize.X * ((VPrecision + VIndex) / VPrecision),
										 0.5 * ViewportSize.Y * ((HPrecision + HIndex) / HPrecision));
			FVector2D Cockpit = Screen;
			if (!IsDrawingCockpit || ScreenToCockpit(Screen, Cockpit))
			{
				DrawHUDIcon(Cockpit, IconSize, HUDNoseIcon, Color, true);
				FlareDrawText(FText::Format(LOCTEXT("GridLocation", "{0},{1}"),
					FText::AsNumber(VIndex + VPrecision),
					FText::AsNumber(HIndex + HPrecision)).ToString(),
					Cockpit, Color, false, false);
			}
		}
	}
}

FString AFlareHUD::FormatDistance(float Distance)
{
	if (Distance < 1000)
	{
		return FString::FromInt(Distance) + FString("m");
	}
	else
	{
		int Kilometers = ((int) Distance)/1000;
		if (Kilometers < 10)
		{
			int Hectometer = ((int)(Distance - Kilometers * 1000)) / 100;
			return FString::FromInt(Kilometers) + "." + FString::FromInt(Hectometer) + FString("km");
		}
		else
		{
			return FString::FromInt(Kilometers) + FString("km");
		}
	}
}

void AFlareHUD::DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed, FText Designation, bool Invert)
{
	// Get HUD data
	FVector2D ScreenPosition;
	int32 SpeedMS = (Speed.Size() + 10.) / 100.0f;

	// Draw inertial vector
	FVector EndPoint = Object->GetActorLocation() + FocusDistance * Speed;
	if (ProjectWorldLocationToCockpit(EndPoint, ScreenPosition) && SpeedMS > 1)
	{
		// Cap screen pos
		float ScreenBorderDistanceX = 100;
		float ScreenBorderDistanceY = 20;
		ScreenPosition.X = FMath::Clamp(ScreenPosition.X, ScreenBorderDistanceX, CurrentViewportSize.X - ScreenBorderDistanceX);
		ScreenPosition.Y = FMath::Clamp(ScreenPosition.Y, ScreenBorderDistanceY, CurrentViewportSize.Y - ScreenBorderDistanceY);

		// Label
		FString IndicatorText = Designation.ToString();
		FVector2D IndicatorPosition = ScreenPosition - CurrentViewportSize / 2 - FVector2D(60, 0);
		FlareDrawText(IndicatorText, IndicatorPosition, HudColorNeutral);

		// Icon
		DrawHUDIcon(ScreenPosition, IconSize, Icon, HudColorNeutral, true);

		// Speed 
		FString VelocityText = FString::FromInt(Invert ? -SpeedMS : SpeedMS) + FString(" m/s");
		FVector2D VelocityPosition = ScreenPosition - CurrentViewportSize / 2 + FVector2D(60, 0);
		FlareDrawText(VelocityText, VelocityPosition, HudColorNeutral);
	}
}

void AFlareHUD::DrawSearchArrow(FVector TargetLocation, FLinearColor Color, float MaxDistance)
{
	// Data
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* Ship = PC->GetShipPawn();
	FVector Direction = TargetLocation - Ship->GetActorLocation();

	if (Direction.Size() < MaxDistance)
	{
		// Compute position
		FVector LocalDirection = Ship->GetRootComponent()->GetComponentToWorld().GetRotation().Inverse().RotateVector(Direction);
		FVector2D ScreenspacePosition = FVector2D(LocalDirection.Y, -LocalDirection.Z);
		ScreenspacePosition.Normalize();
		ScreenspacePosition *= 1.2 * CombatMouseRadius;

		// Draw
		FVector Position3D = FVector(ScreenspacePosition.X, ScreenspacePosition.Y, 0);
		DrawHUDIconRotated(CurrentViewportSize / 2 + ScreenspacePosition, IconSize, HUDCombatMouseIcon, Color, Position3D.Rotation().Yaw);
	}
}

bool AFlareHUD::DrawHUDDesignator(AFlareSpacecraft* Spacecraft)
{
	// Calculation data
	FVector2D ScreenPosition;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
	FVector TargetLocation = Spacecraft->GetActorLocation();

	if (ProjectWorldLocationToCockpit(TargetLocation, ScreenPosition))
	{
		// Compute apparent size in screenspace
		float ShipSize = 2 * Spacecraft->GetMeshScale();
		float Distance = (TargetLocation - PlayerLocation).Size();
		float ApparentAngle = FMath::RadiansToDegrees(FMath::Atan(ShipSize / Distance));
		float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * CurrentViewportSize.X;
		FVector2D ObjectSize = FMath::Min(0.66f * Size, 300.0f) * FVector2D(1, 1);

		// Add to targets
		FFlareScreenTarget TargetData;
		TargetData.Spacecraft = Spacecraft;
		TargetData.DistanceFromScreenCenter = (ScreenPosition - CurrentViewportSize / 2).Size();
		ScreenTargets.Add(TargetData);

		// Check if the mouse is there
		int ToleranceRange = 3;
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ShipBoxMin = ScreenPosition - ObjectSize / 2;
		FVector2D ShipBoxMax = ScreenPosition + ObjectSize / 2;
		bool Hovering = (MousePos.X + ToleranceRange >= ShipBoxMin.X
		              && MousePos.Y + ToleranceRange >= ShipBoxMin.Y
		              && MousePos.X - ToleranceRange <= ShipBoxMax.X
		              && MousePos.Y - ToleranceRange <= ShipBoxMax.Y);

		// Draw the context menu
		if (Hovering && !FoundTargetUnderMouse && IsInteractive)
		{
			// Update state
			FoundTargetUnderMouse = true;
			ContextMenuPosition = ScreenPosition;

			ContextMenu->SetSpacecraft(Spacecraft);
			if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
			{
				ContextMenu->Show();
			}
		}

		// Draw the HUD designator
		else if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
		{
			float CornerSize = 8;
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
			FVector2D CenterPos = ScreenPosition - ObjectSize / 2;
			FLinearColor Color = GetHostilityColor(PC, Spacecraft);

			// Draw designator corners
			bool Highlighted = (PlayerShip && Spacecraft == PlayerShip->GetCurrentTarget());
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, -1), 0,     Color, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, +1), -90,   Color, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, +1), -180,  Color, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, -1), -270,  Color, Highlighted);

			// Draw the target's distance if selected
			if (Spacecraft == PlayerShip->GetCurrentTarget())
			{
				FString DistanceText = FormatDistance(Distance / 100);
				FVector2D DistanceTextPosition = ScreenPosition - (CurrentViewportSize / 2) + FVector2D(-ObjectSize.X / 2, ObjectSize.Y / 2) + 2 * CornerSize * FVector2D::UnitVector;
				FlareDrawText(DistanceText, DistanceTextPosition, Color);
			}

			// Draw the status
			if (!Spacecraft->GetParent()->IsStation() && ObjectSize.X > IconSize)
			{
				int32 NumberOfIcons = Spacecraft->GetParent()->IsMilitary() ? 3 : 2;
				FVector2D StatusPos = CenterPos;
				StatusPos.X += 0.5 * (ObjectSize.X - NumberOfIcons * IconSize);
				StatusPos.Y -= (IconSize + 0.5 * CornerSize);
				DrawHUDDesignatorStatus(StatusPos, IconSize, Spacecraft);
			}

			// Target selection info
			/*int32 SelectionSize = FMath::Min(48, (int32)ObjectSize.X);
			if (Spacecraft == PlayerShip->GetWeaponsSystem()->GetActiveWeaponTarget())
			{
				DrawHUDIcon(ScreenPosition, SelectionSize, HUDDesignatorSelectionTexture, Color, true);
			}*/
			
			// Combat helper
			if (Spacecraft->GetParent()->GetPlayerWarState() == EFlareHostility::Hostile && PlayerShip && PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
			{
				FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
				if (WeaponGroup)
				{
					float AmmoVelocity = WeaponGroup->Weapons[0]->GetAmmoVelocity();
					FVector AmmoIntersectionLocation;
					float InterceptTime = Spacecraft->GetAimPosition(PlayerShip, AmmoVelocity, 0.0, &AmmoIntersectionLocation);

					if (InterceptTime > 0 && ProjectWorldLocationToCockpit(AmmoIntersectionLocation, ScreenPosition))
					{
						// Get some more data
						FLinearColor HUDAimHelperColor = GetHostilityColor(PC, Spacecraft);
						EFlareWeaponGroupType::Type WeaponType = PlayerShip->GetWeaponsSystem()->GetActiveWeaponType();
						bool FighterTargettingLarge = WeaponType == EFlareWeaponGroupType::WG_GUN && Spacecraft->GetParent()->GetSize() == EFlarePartSize::L;
						bool BomberTargettingSmall = WeaponType == EFlareWeaponGroupType::WG_BOMB && Spacecraft->GetParent()->GetSize() == EFlarePartSize::S;
						bool BomberTargettingLarge = WeaponType == EFlareWeaponGroupType::WG_BOMB && Spacecraft->GetParent()->GetSize() == EFlarePartSize::L;

						// Draw helper if it makes sense
						if (!FighterTargettingLarge && !BomberTargettingSmall)
						{
							DrawHUDIcon(ScreenPosition, IconSize, HUDAimHelperIcon, HUDAimHelperColor, true);
						}

						// Bomber UI
						if (BomberTargettingLarge)
						{
							// Time display
							FString TimeText = FString::FromInt(InterceptTime) + FString(".") + FString::FromInt( (InterceptTime - (int) InterceptTime ) *10) + FString(" s");
							FVector2D TimePosition = ScreenPosition - CurrentViewportSize / 2 - FVector2D(42,0);
							FlareDrawText(TimeText, TimePosition, HUDAimHelperColor);
						}
					}
				}
			}

			// Tell the HUD to draw the search marker only if we are outside this
			return (FVector2D::Distance(ScreenPosition, CurrentViewportSize / 2) >= (CurrentViewportSize.GetMin() / 3));
		}
	}

	// Dead ship
	if (!Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		return false;
	}

	return true;
}

void AFlareHUD::DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float DesignatorIconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor, bool Highlighted)
{
	float ScaledDesignatorIconSize = DesignatorIconSize * (Highlighted ? 2 : 1);
	ObjectSize = FMath::Max(ObjectSize, ScaledDesignatorIconSize * FVector2D::UnitVector);

	FlareDrawTexture(Highlighted ? HUDDesignatorCornerSelectedTexture : HUDDesignatorCornerTexture,
		Position.X + (ObjectSize.X + DesignatorIconSize) * MainOffset.X / 2,
		Position.Y + (ObjectSize.Y + DesignatorIconSize) * MainOffset.Y / 2,
		ScaledDesignatorIconSize, ScaledDesignatorIconSize, 0, 0, 1, 1,
		HudColor,
		BLEND_Translucent, 1.0f, false,
		Rotation);
}

void AFlareHUD::DrawHUDDesignatorStatus(FVector2D Position, float DesignatorIconSize, AFlareSpacecraft* Ship)
{
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion), HUDPropulsionIcon);
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport), HUDHealthIcon);

	if (Ship->GetParent()->IsMilitary())
	{
		DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon), HUDWeaponIcon);
	}
}

FVector2D AFlareHUD::DrawHUDDesignatorStatusIcon(FVector2D Position, float DesignatorIconSize, float Health, UTexture2D* Texture)
{
	if (Health < 0.95f)
	{
		FLinearColor Color = FFlareStyleSet::GetHealthColor(Health);
		Color.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
		DrawHUDIcon(Position, DesignatorIconSize, Texture, Color);
	}

	return Position + DesignatorIconSize * FVector2D(1, 0);
}

void AFlareHUD::DrawHUDIcon(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color, bool Center)
{
	if (Center)
	{
		Position -= (DesignatorIconSize / 2) * FVector2D::UnitVector;
	}
	FlareDrawTexture(Texture, Position.X, Position.Y, DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1, Color);
}

void AFlareHUD::DrawHUDIconRotated(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color, float Rotation)
{
	Position -= (DesignatorIconSize / 2) * FVector2D::UnitVector;
	FlareDrawTexture(Texture, Position.X, Position.Y, DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1, Color,
		EBlendMode::BLEND_Translucent, 1.0f, false, Rotation, FVector2D::UnitVector / 2);
}

void AFlareHUD::FlareDrawText(FString Text, FVector2D Position, FLinearColor Color, bool Center, bool Large)
{
	if (CurrentCanvas)
	{
		float X, Y;
		UFont* Font = Large ? HUDFontLarge : HUDFont;

		// Optional centering
		if (Center)
		{
			float XL, YL;
			CurrentCanvas->TextSize(Font, Text, XL, YL);
			X = CurrentCanvas->ClipX / 2.0f - XL / 2.0f + Position.X;
			Y = CurrentCanvas->ClipY / 2.0f - YL / 2.0f + Position.Y;
		}
		else
		{
			X = Position.X;
			Y = Position.Y;
		}

		// Drawing
		{
			FCanvasTextItem ShadowItem(FVector2D(X, Y) + ShadowOffset, FText::FromString(Text), Font, ShadowColor);
			ShadowItem.Scale = FVector2D(1, 1);
			CurrentCanvas->DrawItem(ShadowItem);
		}
		{
			FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
			TextItem.Scale = FVector2D(1, 1);
			CurrentCanvas->DrawItem(TextItem);
		}
	}
}

void AFlareHUD::FlareDrawTexture(UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor Color, EBlendMode BlendMode, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot)
{
	if (CurrentCanvas && Texture)
	{
		// Setup texture (in dark)
		FCanvasTileItem TileItem(FVector2D(ScreenX, ScreenY),
			Texture->Resource,
			FVector2D(ScreenW, ScreenH) * Scale,
			FVector2D(TextureU, TextureV),
			FVector2D(TextureU + TextureUWidth, TextureV + TextureVHeight),
			Color);

		// More setup
		TileItem.Rotation = FRotator(0, Rotation, 0);
		TileItem.PivotPoint = RotPivot;
		if (bScalePosition)
		{
			TileItem.Position *= Scale;
		}
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(BlendMode);
		
		// Draw texture
		TileItem.Position -= ShadowOffset;
		TileItem.SetColor(Color);
		CurrentCanvas->DrawItem(TileItem);
	}
}

bool AFlareHUD::IsInScreen(FVector2D ScreenPosition) const
{
	int32 ScreenBorderDistance = 100;

	if (ScreenPosition.X > CurrentViewportSize.X - ScreenBorderDistance || ScreenPosition.X < ScreenBorderDistance
	 || ScreenPosition.Y > CurrentViewportSize.Y - ScreenBorderDistance || ScreenPosition.Y < ScreenBorderDistance)
	{
		return false;
	}
	else
	{
		return true;
	}
}

FLinearColor AFlareHUD::GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraft* Target)
{
	EFlareHostility::Type Hostility = Target->GetParent()->GetPlayerWarState();
	switch (Hostility)
	{
		case EFlareHostility::Hostile:
			return HudColorEnemy;

		case EFlareHostility::Owned:
			return HudColorFriendly;

		case EFlareHostility::Neutral:
		case EFlareHostility::Friendly:
		default:
			return HudColorNeutral;
	}
}

bool AFlareHUD::ProjectWorldLocationToCockpit(FVector World, FVector2D& Cockpit)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	FVector2D Screen;
	if (PC->ProjectWorldLocationToScreen(World, Screen))
	{
		if (IsDrawingCockpit)
		{
			return ScreenToCockpit(Screen, Cockpit);
		}
		else
		{
			Cockpit = Screen;
			return true;
		}
	}
	else
	{
		return false;
	}

}


/*----------------------------------------------------
	Distorsion
----------------------------------------------------*/

#define GRID_H_SIZE 11
#define GRID_V_SIZE 11

static float FighterHorizontalDistortionMap[] = {
	1.000f, 0.960f, 0.910f, 0.940f, 1.000f, 1.000f, 1.000f, 1.030f, 1.020f, 1.005f, 1.000f,
	1.000f, 0.910f, 0.900f, 0.930f, 1.000f, 1.000f, 1.030f, 1.030f, 1.025f, 1.007f, 1.000f,
	1.000f, 0.870f, 0.880f, 0.920f, 0.960f, 1.000f, 1.025f, 1.035f, 1.030f, 1.013f, 1.000f,
	1.000f, 0.820f, 0.860f, 0.910f, 0.960f, 1.000f, 1.025f, 1.035f, 1.035f, 1.018f, 1.000f,
	1.000f, 0.800f, 0.850f, 0.905f, 0.955f, 1.000f, 1.026f, 1.040f, 1.037f, 1.022f, 1.000f,
	1.000f, 0.795f, 0.850f, 0.905f, 0.960f,   1.f , 1.030f, 1.040f, 1.037f, 1.022f, 1.000f,
	1.000f, 0.800f, 0.850f, 0.905f, 0.955f, 1.000f, 1.026f, 1.040f, 1.037f, 1.022f, 1.000f,
	1.000f, 0.820f, 0.860f, 0.910f, 0.960f, 1.000f, 1.025f, 1.035f, 1.035f, 1.018f, 1.000f,
	1.000f, 0.870f, 0.880f, 0.920f, 0.960f, 1.000f, 1.025f, 1.035f, 1.030f, 1.013f, 1.000f,
	1.000f, 0.910f, 0.900f, 0.930f, 1.000f, 1.000f, 1.030f, 1.030f, 1.025f, 1.008f, 1.000f,
	1.000f, 0.960f, 0.910f, 0.940f, 1.000f, 1.000f, 1.000f, 1.030f, 1.020f, 1.005f, 1.000f };

static float FighterVerticalDistortionMap[] = {
	1.000f, 0.500f, 0.100f, 0.000f, 1.000f, 1.000f, 1.000f, 0.000f, 0.100f, 0.100f, 1.000f,
	1.000f, 0.680f, 0.500f, 0.350f, 0.350f, 0.350f, 0.350f, 0.350f, 0.500f, 0.680f, 1.000f,
	1.000f, 0.850f, 0.770f, 0.720f, 0.680f, 0.680f, 0.680f, 0.720f, 0.770f, 0.850f, 1.000f,
	1.000f, 0.920f, 0.890f, 0.860f, 0.840f, 0.840f, 0.840f, 0.860f, 0.890f, 0.920f, 1.000f,
	1.000f, 0.970f, 0.955f, 0.950f, 0.940f, 0.940f, 0.940f, 0.950f, 0.955f, 0.970f, 1.000f,
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f,   1.f , 1.000f, 1.000f, 1.000f, 1.000f, 1.000f,
	1.000f, 1.020f, 1.030f, 1.035f, 1.040f, 1.040f, 1.040f, 1.035f, 1.030f, 1.020f, 1.000f,
	1.000f, 1.030f, 1.045f, 1.060f, 1.070f, 1.070f, 1.070f, 1.060f, 1.045f, 1.030f, 1.000f,
	1.000f, 1.040f, 1.055f, 1.070f, 1.070f, 1.070f, 1.070f, 1.070f, 1.055f, 1.040f, 1.000f,
	1.000f, 1.040f, 1.065f, 1.080f, 1.080f, 1.080f, 1.080f, 1.080f, 1.065f, 1.040f, 1.000f,
	1.000f, 1.040f, 1.075f, 1.090f, 1.090f, 1.090f, 1.090f, 1.090f, 1.075f, 1.040f, 1.000f };

static float FreighterHorizontalDistortionMap[] = {
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f,
	1.000f, 0.980f, 0.928f, 0.940f, 0.980f, 1.000f, 1.014f, 1.024f, 1.018f, 1.002f, 1.000f,
	1.000f, 0.941f, 0.917f, 0.935f, 0.975f, 1.000f, 1.016f, 1.026f, 1.021f, 1.005f, 1.000f,
	1.000f, 0.915f, 0.903f, 0.930f, 0.970f, 1.000f, 1.018f, 1.028f, 1.023f, 1.005f, 1.000f,
	1.000f, 0.899f, 0.897f, 0.930f, 0.969f, 1.000f, 1.020f, 1.029f, 1.025f, 1.011f, 1.000f,
	1.000f, 0.895f, 0.895f, 0.930f, 0.970f,   1.f , 1.020f, 1.030f, 1.026f, 1.011f, 1.000f,
	1.000f, 0.900f, 0.895f, 0.930f, 0.970f, 1.000f, 1.020f, 1.029f, 1.024f, 1.010f, 1.000f,
	1.000f, 0.920f, 0.905f, 0.935f, 0.970f, 1.000f, 1.020f, 1.026f, 1.022f, 1.008f, 1.000f,
	1.000f, 0.940f, 0.915f, 0.940f, 0.970f, 1.000f, 1.020f, 1.023f, 1.019f, 1.005f, 1.000f,
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f,
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f };

static float FreighterVerticalDistortionMap[] = {
	0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.620f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
	0.750f, 0.600f, 0.400f, 0.240f, 0.300f, 0.620f, 0.300f, 0.190f, 0.350f, 0.540f, 0.750f,
	0.940f, 0.830f, 0.750f, 0.670f, 0.635f, 0.620f, 0.635f, 0.670f, 0.750f, 0.830f, 0.940f,
	0.950f, 0.905f, 0.865f, 0.840f, 0.820f, 0.820f, 0.820f, 0.840f, 0.865f, 0.905f, 0.950f,
	0.970f, 0.962f, 0.945f, 0.933f, 0.930f, 0.930f, 0.930f, 0.933f, 0.945f, 0.962f, 0.970f,
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f,   1.f , 1.000f, 1.000f, 1.000f, 1.000f, 1.000f,
	1.010f, 1.023f, 1.031f, 1.037f, 1.043f, 1.050f, 1.045f, 1.051f, 1.035f, 1.024f, 1.010f,
	1.020f, 1.039f, 1.053f, 1.065f, 1.074f, 1.075f, 1.075f, 1.069f, 1.056f, 1.039f, 1.020f,
	1.020f, 1.045f, 1.059f, 1.071f, 1.080f, 1.100f, 1.080f, 1.071f, 1.059f, 1.045f, 1.020f,
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.100f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f,
	1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.100f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f };

bool AFlareHUD::IsFlyingMilitaryShip() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (PC && PC->GetPlayerShip())
	{
		if (PC->GetPlayerShip()->IsMilitary())
		{
			return true;
		}
	}

	return false;
}

float* AFlareHUD::GetCurrentHorizontalGrid() const
{
	return IsFlyingMilitaryShip() ? FighterHorizontalDistortionMap : FreighterHorizontalDistortionMap;
}

float* AFlareHUD::GetCurrentVerticalGrid() const
{
	return IsFlyingMilitaryShip() ? FighterVerticalDistortionMap : FreighterVerticalDistortionMap;
}

void AFlareHUD::SetDistortion(uint32 Axis, uint32 X, uint32 Y, float Value)
{
	if (X < GRID_H_SIZE && Y < GRID_V_SIZE)
	{
		if (Axis == 0)
		{
			GetCurrentHorizontalGrid()[X  + Y * GRID_H_SIZE] = Value;
		}
		else
		{
			GetCurrentVerticalGrid()[X + Y * GRID_V_SIZE] = Value;
		}
	}
}

bool AFlareHUD::ScreenToCockpit(FVector2D Screen, FVector2D& Cockpit)
{
	float AspectRatio = CurrentViewportSize.X/CurrentViewportSize.Y;
	float ExtraHeight = ViewportSize.Y - ViewportSize.X / AspectRatio;

	// Find for near point of the map
	float XRelativeLocation = (Screen.X / ViewportSize.X)  * (GRID_H_SIZE - 1);
	float YRelativeLocation = (Screen.Y - (ExtraHeight / 2)) / (ViewportSize.Y - ExtraHeight) * (GRID_V_SIZE - 1);
	
	//FLOGV("Screen=%s XRelativeLocation=%f YRelativeLocation=%f AspectRatio=%f ExtraHeight=%f", *Screen.ToString(), XRelativeLocation, YRelativeLocation, AspectRatio, ExtraHeight);

	if(XRelativeLocation < 0.f || XRelativeLocation > (GRID_H_SIZE - 1) || YRelativeLocation < 0.f || YRelativeLocation > (GRID_V_SIZE - 1))
	{
		return false;
	}

	int32 LeftIndex = FMath::FloorToInt(XRelativeLocation);
	int32 RightIndex = FMath::CeilToInt(XRelativeLocation);
	int32 TopIndex = FMath::FloorToInt(YRelativeLocation);
	int32 BottomIndex = FMath::CeilToInt(YRelativeLocation);

	float LocalX = XRelativeLocation - LeftIndex;
	float LocalY = YRelativeLocation - TopIndex;
	
	float TopLeftXDistorsion = GetCurrentHorizontalGrid()[LeftIndex + TopIndex * GRID_H_SIZE];
	float TopRightXDistorsion = GetCurrentHorizontalGrid()[RightIndex + TopIndex * GRID_H_SIZE];
	float BottomLeftXDistorsion = GetCurrentHorizontalGrid()[LeftIndex + BottomIndex * GRID_H_SIZE];
	float BottomRightXDistorsion = GetCurrentHorizontalGrid()[RightIndex + BottomIndex * GRID_H_SIZE];
	
	float TopMeanXDistorsion = LocalX * TopRightXDistorsion +  (1 - LocalX) * TopLeftXDistorsion;
	float BottomMeanXDistorsion = LocalX * BottomRightXDistorsion +  (1 - LocalX) * BottomLeftXDistorsion;
	float MeanXDistorsion = LocalY * BottomMeanXDistorsion +  (1 - LocalY) * TopMeanXDistorsion;

	float TopLeftYDistorsion = GetCurrentVerticalGrid()[LeftIndex + TopIndex * GRID_V_SIZE];
	float TopRightYDistorsion = GetCurrentVerticalGrid()[RightIndex + TopIndex * GRID_V_SIZE];
	float BottomLeftYDistorsion = GetCurrentVerticalGrid()[LeftIndex + BottomIndex * GRID_V_SIZE];
	float BottomRightYDistorsion = GetCurrentVerticalGrid()[RightIndex + BottomIndex * GRID_V_SIZE];
	
	float TopMeanYDistorsion = LocalX * TopRightYDistorsion +  (1 - LocalX) * TopLeftYDistorsion;
	float BottomMeanYDistorsion = LocalX * BottomRightYDistorsion +  (1 - LocalX) * BottomLeftYDistorsion;
	float MeanYDistorsion = LocalY * BottomMeanYDistorsion +  (1 - LocalY) * TopMeanYDistorsion;
	
	Cockpit = FVector2D(XRelativeLocation / (GRID_H_SIZE - 1) * MeanXDistorsion * CurrentViewportSize.X,
						YRelativeLocation / (GRID_H_SIZE - 1) * MeanYDistorsion * CurrentViewportSize.Y);

	//FLOGV("Cockpit=%s MeanXDistorsion=%f MeanYDistorsion=%f", *Cockpit.ToString(), MeanXDistorsion, MeanYDistorsion);
	
	if (Cockpit.X < 0.f || Cockpit.X > CurrentViewportSize.X || Cockpit.Y < 0.f || Cockpit.Y > CurrentViewportSize.Y)
	{
		return false;
	}
	else
	{
		return true;
	}
}



#undef LOCTEXT_NAMESPACE

