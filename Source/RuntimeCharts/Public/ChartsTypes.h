#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "ChartsTypes.generated.h"

UENUM(BlueprintType)
enum class EChartsElementPart : uint8
{
    Data,
    Bar,
    Line,
    Area
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    double Value = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FSlateColor PrimaryColor = FSlateColor(FLinearColor(0.12f, 0.55f, 1.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FSlateColor SecondaryColor = FSlateColor(FLinearColor(1.0f, 0.7f, 0.12f));
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FString DataName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FChartsValue> Values;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsBarConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bIsHorizontal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FString ChartName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FChartsData> Data;

    /** Empty: shared automatic range. One: shared explicit range. Many: Values index -> axis. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FVector2D> Axis;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FString> SeriesNames;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsLineConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bShowDataPoints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bIsSmooth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FString ChartName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FChartsData> Data;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FVector2D> Axis;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FString> SeriesNames;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsAreaConfig : public FChartsLineConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "1"))
    float FillOpacity = 0.3f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsComboConfig : public FChartsBarConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bShowDataPoints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bIsSmooth = false;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsPieConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FString ChartName;

    /** Each Values index is a concentric ring; Data entries are slices. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FChartsData> Data;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "0.9"))
    float InnerRadiusRatio = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    float StartAngleDegrees = -90.0f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsPolarAreaConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FString ChartName;

    /** Equal-angle sectors; each category reserves one sector per series. Area represents value. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FChartsData> Data;

    /** Zero selects the largest positive value automatically. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0"))
    double MaxValue = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    float StartAngleDegrees = -90.0f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsRadarConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bIsFillInside = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FString ChartName;

    /** Each Data entry is a shape, and each Values index is a spoke. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FChartsData> Data;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FString> AxisLabels;

    /** Optional shared range or one range per spoke; empty uses a shared automatic range. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    TArray<FVector2D> Axis;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "1"))
    float FillOpacity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bShowDataPoints = true;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsStyle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FVector2D DesiredSize = FVector2D(560.0, 340.0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FLinearColor BackgroundColor = FLinearColor(0.018f, 0.025f, 0.042f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FLinearColor TextColor = FLinearColor(0.8f, 0.86f, 0.94f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "8", ClampMax = "48"))
    int32 FontSize = 11;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bShowLegend = true;

    /** Space between the plot/axis labels and the series legend, in Slate units. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "200"))
    float LegendPadding = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bShowTooltips = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bHighlightHovered = true;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsGridStyle : public FChartsStyle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    FLinearColor GridColor = FLinearColor(0.19f, 0.24f, 0.32f, 0.6f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "2", ClampMax = "12"))
    int32 GridDivisions = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts")
    bool bShowGrid = true;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsAxisStyle : public FChartsGridStyle
{
    GENERATED_BODY()

    /** Space from the plot to axis/category names and tick labels, in Slate units. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "200"))
    float AxisLabelPadding = 8.0f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsBarStyle : public FChartsAxisStyle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "0.8"))
    float BarGapRatio = 0.25f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsLineStyle : public FChartsAxisStyle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0.5", ClampMax = "20"))
    float LineThickness = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "1", ClampMax = "20"))
    float PointRadius = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "2", ClampMax = "30"))
    float HitTolerance = 7.0f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsAreaStyle : public FChartsLineStyle
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsComboStyle : public FChartsLineStyle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charts", meta = (ClampMin = "0", ClampMax = "0.8"))
    float BarGapRatio = 0.25f;
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsPieStyle : public FChartsStyle
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsPolarAreaStyle : public FChartsGridStyle
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsRadarStyle : public FChartsLineStyle
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct RUNTIMECHARTS_API FChartsElement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    FString DataName;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    double Value = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    FSlateColor Color;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    FSlateColor PrimaryColor;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    FSlateColor SecondaryColor;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    EChartsElementPart Part = EChartsElementPart::Data;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    int32 DataIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    int32 ValueIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Charts")
    FString SeriesName;

    bool IsSameTarget(const FChartsElement& Other) const
    {
        return DataIndex != INDEX_NONE && Other.DataIndex != INDEX_NONE && ValueIndex == Other.ValueIndex && Part == Other.Part
            && (Part == EChartsElementPart::Area || DataIndex == Other.DataIndex);
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChartsElementEvent, const FString&, DataName, double, Value, FSlateColor, Color);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChartsDetailedElementEvent, const FChartsElement&, Element);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChartsHoverEndedEvent);
