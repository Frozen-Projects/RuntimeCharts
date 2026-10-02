#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "ChartsTypes.h"
#include "ChartWidget.generated.h"

class SRuntimeChart;
struct FChartsModel;

UCLASS(Abstract, BlueprintType)
class RUNTIMECHARTS_API UChartWidget : public UWidget
{
    GENERATED_BODY()

public:
    UChartWidget(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsStyle ChartStyle;

    UPROPERTY(BlueprintAssignable, Category = "Charts|Events")
    FChartsElementEvent OnElementHovered;

    UPROPERTY(BlueprintAssignable, Category = "Charts|Events")
    FChartsElementEvent OnElementClicked;

    UPROPERTY(BlueprintAssignable, Category = "Charts|Events")
    FChartsDetailedElementEvent OnElementHoveredDetailed;

    UPROPERTY(BlueprintAssignable, Category = "Charts|Events")
    FChartsDetailedElementEvent OnElementClickedDetailed;

    UPROPERTY(BlueprintAssignable, Category = "Charts|Events")
    FChartsHoverEndedEvent OnElementHoverEnded;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsStyle& InStyle);

    UFUNCTION(BlueprintCallable, Category = "Charts")
    void RefreshChart();

    /** LocalPosition uses widget-local Slate units, not screen pixels. */
    UFUNCTION(BlueprintCallable, Category = "Charts")
    bool GetElementAtLocalPosition(FVector2D LocalPosition, FChartsElement& OutElement) const;

    UFUNCTION(BlueprintPure, Category = "Charts")
    bool GetHoveredElement(FChartsElement& OutElement) const;

    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
    virtual const FText GetPaletteCategory() override;
#endif

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void BuildModel(FChartsModel& OutModel) const;
    TSharedPtr<SRuntimeChart> MyChart;

private:
    void HandleHover(const FChartsElement& Element);
    void HandleClick(const FChartsElement& Element);
    void HandleHoverEnded();
};
