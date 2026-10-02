#include "ChartsExamples.h"
#include "RuntimeChartsWidgets.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

namespace
{
    const FLinearColor Blue(0.07f, 0.48f, 0.95f), Teal(0.02f, 0.78f, 0.58f), Amber(1.0f, 0.5f, 0.07f);

    TArray<FChartsData> MonthlyData()
    {
        const double Values[][2] = {{22, 35}, {38, 29}, {31, 48}, {56, 42}, {49, 62}, {73, 57}};
        const TCHAR* Names[] = {TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"), TEXT("May"), TEXT("Jun")};
        TArray<FChartsData> Data;
        for (int32 I = 0; I < 6; ++I)
        {
            FChartsData Row;
            Row.DataName = Names[I];
            FChartsValue A, B;
            A.Value = Values[I][0]; A.Color = FSlateColor(Blue);
            B.Value = Values[I][1]; B.Color = FSlateColor(Teal);
            Row.Values = {A, B};
            Data.Add(MoveTemp(Row));
        }
        return Data;
    }

    TArray<FChartsData> CategoryData()
    {
        const TCHAR* Names[] = {TEXT("Design"), TEXT("Build"), TEXT("Support"), TEXT("Research"), TEXT("Operations")};
        const double Values[] = {30, 25, 20, 15, 10};
        const FLinearColor Colors[] = {Blue, Teal, Amber, FLinearColor(0.65f, 0.26f, 0.95f), FLinearColor(0.97f, 0.19f, 0.35f)};
        TArray<FChartsData> Data;
        for (int32 I = 0; I < 5; ++I)
        {
            FChartsData Row;
            Row.DataName = Names[I];
            FChartsValue Value;
            Value.Value = Values[I]; Value.Color = FSlateColor(Colors[I]);
            Row.Values.Add(Value);
            Data.Add(MoveTemp(Row));
        }
        return Data;
    }
}

FChartsBarConfig UChartsExampleLibrary::MakeExampleBarConfig(bool bHorizontal)
{
    FChartsBarConfig Config;
    Config.ChartName = bHorizontal ? TEXT("Horizontal bars · grouped series") : TEXT("Bar · grouped series");
    Config.bIsHorizontal = bHorizontal;
    Config.Data = MonthlyData();
    Config.SeriesNames = {TEXT("Product A"), TEXT("Product B")};
    return Config;
}

FChartsLineConfig UChartsExampleLibrary::MakeExampleLineConfig()
{
    FChartsLineConfig Config;
    Config.ChartName = TEXT("Line · smooth with data points");
    Config.Data = MonthlyData();
    Config.SeriesNames = {TEXT("Product A"), TEXT("Product B")};
    Config.bShowDataPoints = true;
    Config.bIsSmooth = true;
    return Config;
}

FChartsAreaConfig UChartsExampleLibrary::MakeExampleAreaConfig()
{
    FChartsAreaConfig Config;
    static_cast<FChartsLineConfig&>(Config) = MakeExampleLineConfig();
    Config.ChartName = TEXT("Area · translucent overlays");
    Config.bShowDataPoints = false;
    return Config;
}

FChartsComboConfig UChartsExampleLibrary::MakeExampleComboConfig()
{
    FChartsComboConfig Config;
    static_cast<FChartsLineConfig&>(Config) = MakeExampleLineConfig();
    Config.ChartName = TEXT("Combo · independent value axes");
    Config.SeriesNames = {TEXT("Units"), TEXT("Rate %")};
    Config.Axis = {{0, 100}, {0, 1}};
    Config.SeriesTypes = {EChartsSeriesType::Bar, EChartsSeriesType::Line};
    for (FChartsData& Row : Config.Data) Row.Values[1].Value /= 100.0;
    return Config;
}

FChartsPieConfig UChartsExampleLibrary::MakeExamplePieConfig()
{
    FChartsPieConfig Config;
    Config.ChartName = TEXT("Pie · precise segment input");
    Config.Data = CategoryData();
    return Config;
}

FChartsPolarAreaConfig UChartsExampleLibrary::MakeExamplePolarAreaConfig()
{
    FChartsPolarAreaConfig Config;
    Config.ChartName = TEXT("Polar area · area represents value");
    Config.Data = CategoryData();
    return Config;
}

FChartsRadarConfig UChartsExampleLibrary::MakeExampleRadarConfig()
{
    FChartsRadarConfig Config;
    Config.ChartName = TEXT("Radar · overlapping shapes");
    Config.bIsFillInside = true;
    Config.AxisLabels = {TEXT("Speed"), TEXT("Quality"), TEXT("Capacity"), TEXT("Efficiency"), TEXT("Reliability")};
    Config.Axis = {{0, 100}};
    const double Values[][5] = {{85, 62, 92, 55, 78}, {58, 88, 64, 90, 63}};
    for (int32 I = 0; I < 2; ++I)
    {
        FChartsData Row;
        Row.DataName = I == 0 ? TEXT("Product A") : TEXT("Product B");
        for (double Number : Values[I])
        {
            FChartsValue Value;
            Value.Value = Number; Value.Color = FSlateColor(I == 0 ? Blue : Teal);
            Row.Values.Add(Value);
        }
        Config.Data.Add(MoveTemp(Row));
    }
    return Config;
}

TSharedRef<SWidget> UChartsDemoWidget::RebuildWidget()
{
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
        WidgetTree->RootWidget = Root;
        StatusText = WidgetTree->ConstructWidget<UTextBlock>();
        StatusText->SetText(FText::FromString(TEXT("Runtime Charts | Hover or click a chart element")));
        StatusText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 16));
        Root->AddChildToVerticalBox(StatusText)->SetPadding(FMargin(16));
        UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
        Root->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>();
        Grid->SetSlotPadding(FMargin(6));
        Scroll->AddChild(Grid);
        TArray<UChartWidget*> Charts;
        UChartsBarWidget* Bar = WidgetTree->ConstructWidget<UChartsBarWidget>();
        Bar->SetConfig(UChartsExampleLibrary::MakeExampleBarConfig()); Charts.Add(Bar);
        UChartsBarWidget* Horizontal = WidgetTree->ConstructWidget<UChartsBarWidget>();
        Horizontal->SetConfig(UChartsExampleLibrary::MakeExampleBarConfig(true)); Charts.Add(Horizontal);
        UChartsLineWidget* LineChart = WidgetTree->ConstructWidget<UChartsLineWidget>();
        LineChart->SetConfig(UChartsExampleLibrary::MakeExampleLineConfig()); Charts.Add(LineChart);
        UChartsAreaWidget* Area = WidgetTree->ConstructWidget<UChartsAreaWidget>();
        Area->SetConfig(UChartsExampleLibrary::MakeExampleAreaConfig()); Charts.Add(Area);
        UChartsComboWidget* Combo = WidgetTree->ConstructWidget<UChartsComboWidget>();
        Combo->SetConfig(UChartsExampleLibrary::MakeExampleComboConfig()); Charts.Add(Combo);
        UChartsPieWidget* Pie = WidgetTree->ConstructWidget<UChartsPieWidget>();
        Pie->SetConfig(UChartsExampleLibrary::MakeExamplePieConfig()); Charts.Add(Pie);
        UChartsPolarAreaWidget* Polar = WidgetTree->ConstructWidget<UChartsPolarAreaWidget>();
        Polar->SetConfig(UChartsExampleLibrary::MakeExamplePolarAreaConfig()); Charts.Add(Polar);
        UChartsRadarWidget* RadarChart = WidgetTree->ConstructWidget<UChartsRadarWidget>();
        RadarChart->SetConfig(UChartsExampleLibrary::MakeExampleRadarConfig()); Charts.Add(RadarChart);
        for (int32 I = 0; I < Charts.Num(); ++I)
        {
            Charts[I]->OnElementHovered.AddDynamic(this, &UChartsDemoWidget::ShowHover);
            Charts[I]->OnElementClicked.AddDynamic(this, &UChartsDemoWidget::ShowClick);
            USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>();
            Box->SetWidthOverride(560); Box->SetHeightOverride(340);
            Box->AddChild(Charts[I]);
            Grid->AddChildToUniformGrid(Box, I / 2, I % 2);
        }
    }
    return Super::RebuildWidget();
}

void UChartsDemoWidget::ShowHover(const FString& DataName, double Value, FSlateColor Color)
{
    if (StatusText) StatusText->SetText(FText::FromString(FString::Printf(TEXT("Hover: %s | %.3f"), *DataName, Value)));
}

void UChartsDemoWidget::ShowClick(const FString& DataName, double Value, FSlateColor Color)
{
    if (StatusText) StatusText->SetText(FText::FromString(FString::Printf(TEXT("Click: %s | %.3f"), *DataName, Value)));
}
