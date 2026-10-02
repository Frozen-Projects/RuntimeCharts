#pragma once

#include "Blueprint/UserWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChartsTypes.h"
#include "ChartsExamples.generated.h"

class UTextBlock;

UCLASS()
class RUNTIMECHARTS_API UChartsExampleLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsBarConfig MakeExampleBarConfig(bool bHorizontal = false);

    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsLineConfig MakeExampleLineConfig();

    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsAreaConfig MakeExampleAreaConfig();

    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsComboConfig MakeExampleComboConfig();

    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsPieConfig MakeExamplePieConfig();

    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsPolarAreaConfig MakeExamplePolarAreaConfig();

    UFUNCTION(BlueprintPure, Category = "Charts|Examples")
    static FChartsRadarConfig MakeExampleRadarConfig();
};

/** Create Widget -> Charts Demo Widget -> Add to Viewport. */
UCLASS(BlueprintType, Blueprintable)
class RUNTIMECHARTS_API UChartsDemoWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UFUNCTION()
    void ShowHover(const FString& DataName, double Value, FSlateColor Color);

    UFUNCTION()
    void ShowClick(const FString& DataName, double Value, FSlateColor Color);

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> StatusText;
};
