// CodeRescueMessageReaderWidget.cpp -- see header for design intent and the Mac-compile DoD note.

#include "CodeRescueMessageReaderWidget.h"
#include "CodeRescueCharacter.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

TWeakObjectPtr<UCodeRescueMessageReaderWidget> UCodeRescueMessageReaderWidget::ActiveReader = nullptr;

void UCodeRescueMessageReaderWidget::OpenReader(UObject* WorldContext, const FString& InMessageId,
                                                const FString& InTitle, const FString& InBody)
{
    if (!WorldContext) { return; }
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!World) { return; }

    CloseActiveReader();

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) { return; }

    UCodeRescueMessageReaderWidget* Reader = CreateWidget<UCodeRescueMessageReaderWidget>(PC, UCodeRescueMessageReaderWidget::StaticClass());
    if (!Reader) { return; }

    Reader->SetMessage(InMessageId, InTitle, InBody);
    Reader->AddToViewport(1000);
    ActiveReader = Reader;
}

void UCodeRescueMessageReaderWidget::CloseActiveReader()
{
    if (ActiveReader.IsValid())
    {
        ActiveReader->Close();
    }
    ActiveReader = nullptr;
}

bool UCodeRescueMessageReaderWidget::IsReaderOpen()
{
    // Programmatically constructed widgets can briefly report false from
    // IsInViewport while their Slate resource is being rebuilt. The weak
    // pointer is the authoritative modal-ownership signal.
    return ActiveReader.IsValid();
}

void UCodeRescueMessageReaderWidget::SetMessage(const FString& InMessageId, const FString& InTitle, const FString& InBody)
{
    MessageId = InMessageId;
    Title = InTitle;
    Body = InBody;

    if (IdText)    { IdText->SetText(FText::FromString(MessageId)); }
    if (TitleText) { TitleText->SetText(FText::FromString(Title)); }
    if (BodyText)  { BodyText->SetText(FText::FromString(Body)); }
}

void UCodeRescueMessageReaderWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueMessageReaderWidget::RebuildWidget()
{
    // 2026-07-01 ROOT FIX for invisible UMG: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueMessageReaderWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    if (!RootBorder && WidgetTree)
    {
        RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ReaderRoot"));
        RootBorder->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.90f));
        RootBorder->SetPadding(FMargin(140.0f, 90.0f, 140.0f, 90.0f));
        WidgetTree->RootWidget = RootBorder;

        Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReaderColumn"));
        RootBorder->SetContent(Column);

        IdText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderId"));
        IdText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
        IdText->SetColorAndOpacity(FSlateColor(FLinearColor(0.35f, 0.85f, 1.0f)));
        Column->AddChildToVerticalBox(IdText);

        TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderTitle"));
        TitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 30));
        TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        TitleText->SetAutoWrapText(true);
        Column->AddChildToVerticalBox(TitleText);

        BodyScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ReaderScroll"));
        if (UVerticalBoxSlot* ScrollSlot = Column->AddChildToVerticalBox(BodyScroll))
        {
            ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            ScrollSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 18.0f));
        }

        BodyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderBody"));
        BodyText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 20));
        BodyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.93f, 0.97f)));
        BodyText->SetAutoWrapText(true);
        BodyScroll->AddChild(BodyText);

        HintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderHint"));
        HintText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 15));
        HintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.72f, 0.80f)));
        HintText->SetText(FText::FromString(TEXT("Scroll to read  -  press E, Enter or Esc to close")));
        Column->AddChildToVerticalBox(HintText);

        CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ReaderCloseButton"));
        CloseButton->SetBackgroundColor(FLinearColor(0.10f, 0.30f, 0.38f, 1.0f));
        CloseButton->SetClickMethod(EButtonClickMethod::MouseDown);
        CloseButton->SetTouchMethod(EButtonTouchMethod::DownAndUp);
        UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderCloseLabel"));
        CloseLabel->SetText(FText::FromString(TEXT("CLOSE")));
        CloseLabel->SetJustification(ETextJustify::Center);
        CloseLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
        CloseLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        CloseLabel->SetVisibility(ESlateVisibility::HitTestInvisible);
        CloseButton->AddChild(CloseLabel);
        CloseButton->OnClicked.AddDynamic(this, &UCodeRescueMessageReaderWidget::OnCloseClicked);
        if (UVerticalBoxSlot* CloseSlot = Column->AddChildToVerticalBox(CloseButton))
        {
            CloseSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
        }
    }

    // Apply any message set before the tree existed.
    SetMessage(MessageId, Title, Body);

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
        PC->FlushPressedKeys();
        SetKeyboardFocus();
    }
    ACodeRescueCharacter::SetUIOpen(true);
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

FReply UCodeRescueMessageReaderWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Escape || Key == EKeys::E || Key == EKeys::Enter ||
        Key == EKeys::Tab || Key == EKeys::Gamepad_FaceButton_Right)
    {
        Close();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCodeRescueMessageReaderWidget::Close()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = false;
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    RemoveFromParent();
    if (ActiveReader.Get() == this)
    {
        ActiveReader = nullptr;
    }
}

void UCodeRescueMessageReaderWidget::OnCloseClicked()
{
    Close();
}
