#include "RuntimeChartsWidgets.h"
#include "ChartsScene.h"

void UChartsBarWidget::SetConfig(const FChartsBarConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsBarWidget::SetChartStyle(const FChartsBarStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsBarWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsBarWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}

void UChartsLineWidget::SetConfig(const FChartsLineConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsLineWidget::SetChartStyle(const FChartsLineStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsLineWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsLineWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}

void UChartsAreaWidget::SetConfig(const FChartsAreaConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsAreaWidget::SetChartStyle(const FChartsAreaStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsAreaWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsAreaWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}

void UChartsComboWidget::SetConfig(const FChartsComboConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsComboWidget::SetChartStyle(const FChartsComboStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsComboWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsComboWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}

void UChartsPieWidget::SetConfig(const FChartsPieConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsPieWidget::SetChartStyle(const FChartsPieStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsPieWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsPieWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}

void UChartsPolarAreaWidget::SetConfig(const FChartsPolarAreaConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsPolarAreaWidget::SetChartStyle(const FChartsPolarAreaStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsPolarAreaWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsPolarAreaWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}

void UChartsRadarWidget::SetConfig(const FChartsRadarConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsRadarWidget::SetChartStyle(const FChartsRadarStyle& InStyle)
{
    ChartStyle = InStyle;
    RefreshChart();
}

void UChartsRadarWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

void UChartsRadarWidget::BuildStyle(FChartsRenderStyle& OutStyle) const
{
    OutStyle = FChartsRenderStyle(ChartStyle);
}


