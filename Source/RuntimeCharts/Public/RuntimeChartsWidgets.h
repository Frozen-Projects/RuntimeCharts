#pragma once

#include "ChartWidget.h"
#include "RuntimeChartsWidgets.generated.h"
UCLASS(BlueprintType, meta = (DisplayName = "Bar Chart"))
class RUNTIMECHARTS_API UChartsBarWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsBarConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsBarConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};
UCLASS(BlueprintType, meta = (DisplayName = "Line Chart"))
class RUNTIMECHARTS_API UChartsLineWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsLineConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsLineConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};
UCLASS(BlueprintType, meta = (DisplayName = "Area Chart"))
class RUNTIMECHARTS_API UChartsAreaWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsAreaConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsAreaConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};
UCLASS(BlueprintType, meta = (DisplayName = "Combo Chart"))
class RUNTIMECHARTS_API UChartsComboWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsComboConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsComboConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};
UCLASS(BlueprintType, meta = (DisplayName = "Pie Chart"))
class RUNTIMECHARTS_API UChartsPieWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsPieConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsPieConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};
UCLASS(BlueprintType, meta = (DisplayName = "PolarArea Chart"))
class RUNTIMECHARTS_API UChartsPolarAreaWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsPolarAreaConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsPolarAreaConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};
UCLASS(BlueprintType, meta = (DisplayName = "Radar Chart"))
class RUNTIMECHARTS_API UChartsRadarWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsRadarConfig Config;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsRadarConfig& InConfig);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
};

