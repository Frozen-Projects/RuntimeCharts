#include "RuntimeChartsWidgets.h"
#include "ChartsScene.h"
void UChartsBarWidget::SetConfig(const FChartsBarConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsBarWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}
void UChartsLineWidget::SetConfig(const FChartsLineConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsLineWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}
void UChartsAreaWidget::SetConfig(const FChartsAreaConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsAreaWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}
void UChartsComboWidget::SetConfig(const FChartsComboConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsComboWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}
void UChartsPieWidget::SetConfig(const FChartsPieConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsPieWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}
void UChartsPolarAreaWidget::SetConfig(const FChartsPolarAreaConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsPolarAreaWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}
void UChartsRadarWidget::SetConfig(const FChartsRadarConfig& InConfig)
{
    Config = InConfig;
    RefreshChart();
}

void UChartsRadarWidget::BuildModel(FChartsModel& OutModel) const
{
    OutModel = FChartsModel::From(Config);
}

