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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsBarStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsBarConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsBarStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "Line Chart"))
class RUNTIMECHARTS_API UChartsLineWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsLineConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsLineStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsLineConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsLineStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "Area Chart"))
class RUNTIMECHARTS_API UChartsAreaWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsAreaConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsAreaStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsAreaConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsAreaStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "Combo Chart"))
class RUNTIMECHARTS_API UChartsComboWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsComboConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsComboStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsComboConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsComboStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "Pie Chart"))
class RUNTIMECHARTS_API UChartsPieWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsPieConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsPieStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsPieConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsPieStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "PolarArea Chart"))
class RUNTIMECHARTS_API UChartsPolarAreaWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsPolarAreaConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsPolarAreaStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsPolarAreaConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsPolarAreaStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "Radar Chart"))
class RUNTIMECHARTS_API UChartsRadarWidget : public UChartWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetConfig, Category = "Charts")
    FChartsRadarConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetChartStyle, Category = "Charts")
    FChartsRadarStyle ChartStyle;

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetConfig(const FChartsRadarConfig& InConfig);

    UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Charts")
    void SetChartStyle(const FChartsRadarStyle& InStyle);

protected:
    virtual void BuildModel(FChartsModel& OutModel) const override;
    virtual void BuildStyle(FChartsRenderStyle& OutStyle) const override;
};


