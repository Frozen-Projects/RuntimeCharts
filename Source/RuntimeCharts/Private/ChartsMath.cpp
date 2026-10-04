#include "ChartsScene.h"

FChartsModel FChartsModel::From(const FChartsBarConfig& Config)
{
    FChartsModel Model;
    Model.Title = Config.ChartName;
    Model.Data = Config.Data;
    Model.Axis = Config.Axis;
    Model.Names = Config.SeriesNames;
    Model.bHorizontal = Config.bIsHorizontal;
    return Model;
}

FChartsModel FChartsModel::From(const FChartsLineConfig& Config)
{
    FChartsModel Model;
    Model.Kind = EChartsKind::Line;
    Model.Title = Config.ChartName;
    Model.Data = Config.Data;
    Model.Axis = Config.Axis;
    Model.Names = Config.SeriesNames;
    Model.bPoints = Config.bShowDataPoints;
    Model.bSmooth = Config.bIsSmooth;
    return Model;
}

FChartsModel FChartsModel::From(const FChartsAreaConfig& Config)
{
    FChartsModel Model = From(static_cast<const FChartsLineConfig&>(Config));
    Model.Kind = EChartsKind::Area;
    Model.bFill = true;
    Model.FillOpacity = FMath::IsFinite(Config.FillOpacity) ? FMath::Clamp(Config.FillOpacity, 0.0f, 1.0f) : 0.3f;
    return Model;
}

FChartsModel FChartsModel::From(const FChartsComboConfig& Config)
{
    FChartsModel Model = From(static_cast<const FChartsBarConfig&>(Config));
    Model.Kind = EChartsKind::Combo;
    Model.bPoints = Config.bShowDataPoints;
    Model.bSmooth = Config.bIsSmooth;
    return Model;
}

FChartsModel FChartsModel::From(const FChartsPieConfig& Config)
{
    FChartsModel Model;
    Model.Kind = EChartsKind::Pie;
    Model.Title = Config.ChartName;
    Model.Data = Config.Data;
    Model.InnerRadius = FMath::IsFinite(Config.InnerRadiusRatio) ? FMath::Clamp(Config.InnerRadiusRatio, 0.0f, 0.9f) : 0.0f;
    Model.StartAngle = FMath::IsFinite(Config.StartAngleDegrees) ? FMath::Fmod(Config.StartAngleDegrees, 360.0f) : -90.0f;
    return Model;
}

FChartsModel FChartsModel::From(const FChartsPolarAreaConfig& Config)
{
    FChartsModel Model;
    Model.Kind = EChartsKind::PolarArea;
    Model.Title = Config.ChartName;
    Model.Data = Config.Data;
    Model.MaxValue = FMath::IsFinite(Config.MaxValue) ? FMath::Max(0.0, Config.MaxValue) : 0.0;
    Model.StartAngle = FMath::IsFinite(Config.StartAngleDegrees) ? FMath::Fmod(Config.StartAngleDegrees, 360.0f) : -90.0f;
    return Model;
}

FChartsModel FChartsModel::From(const FChartsRadarConfig& Config)
{
    FChartsModel Model;
    Model.Kind = EChartsKind::Radar;
    Model.Title = Config.ChartName;
    Model.Data = Config.Data;
    Model.Axis = Config.Axis;
    Model.Names = Config.AxisLabels;
    Model.bFill = Config.bIsFillInside;
    Model.bPoints = Config.bShowDataPoints;
    Model.FillOpacity = FMath::IsFinite(Config.FillOpacity) ? FMath::Clamp(Config.FillOpacity, 0.0f, 1.0f) : 0.2f;
    return Model;
}

int32 FChartsModel::NumSeries() const
{
    int32 Count = 0;
    for (const FChartsData& Row : Data) Count = FMath::Max(Count, Row.Values.Num());
    return Count;
}

bool FChartsModel::HasBars() const
{
    return Kind == EChartsKind::Bar || Kind == EChartsKind::Combo;
}

const FChartsValue* FChartsModel::GetValue(int32 DataIndex, int32 ValueIndex) const
{
    if (!Data.IsValidIndex(DataIndex) || !Data[DataIndex].Values.IsValidIndex(ValueIndex)) return nullptr;
    const FChartsValue& Value = Data[DataIndex].Values[ValueIndex];
    return FMath::IsFinite(Value.Value) ? &Value : nullptr;
}

FChartsElement FChartsModel::Element(int32 DataIndex, int32 ValueIndex, EChartsElementPart Part) const
{
    FChartsElement Result;
    if (const FChartsValue* Value = GetValue(DataIndex, ValueIndex))
    {
        Result.DataName = Data[DataIndex].DataName;
        Result.Value = Value->Value;
        Result.PrimaryColor = Value->PrimaryColor;
        Result.SecondaryColor = Value->SecondaryColor;
        Result.Color = Kind == EChartsKind::Combo && Part == EChartsElementPart::Line ? Value->SecondaryColor : Value->PrimaryColor;
        Result.Part = Part;
        Result.DataIndex = DataIndex;
        Result.ValueIndex = ValueIndex;
        Result.SeriesName = Kind == EChartsKind::Radar || Kind == EChartsKind::Pie || Kind == EChartsKind::PolarArea
            ? Data[DataIndex].DataName : (Names.IsValidIndex(ValueIndex) ? Names[ValueIndex] : FString::Printf(TEXT("Series %d"), ValueIndex + 1));
    }
    return Result;
}

double FChartsRange::Normalize(double Value) const
{
    if (!FMath::IsFinite(Value)) return 0.0;
    const double Scale = FMath::Max(FMath::Abs(Min), FMath::Abs(Max));
    if (Scale == 0.0 || Min == Max) return 0.0;
    return FMath::Clamp((FMath::Clamp(Value, Min, Max) / Scale - Min / Scale) / (Max / Scale - Min / Scale), 0.0, 1.0);
}

double FChartsRange::At(double Fraction) const
{
    return (1.0 - Fraction) * Min + Fraction * Max;
}

FChartsRange ChartsMath::Range(const FChartsModel& Model, int32 Series)
{
    FChartsRange Result{0.0, 0.0};
    const bool bShared = Model.Axis.Num() <= 1;
    const auto AxisFor = [&Model, bShared](int32 Index)
    {
        if (bShared || (Model.Kind != EChartsKind::Radar && !Model.Axis.IsValidIndex(Index))) return 0;
        return Index;
    };
    const int32 AxisIndex = AxisFor(Series);
    for (int32 Row = 0; Row < Model.Data.Num(); ++Row)
    {
        for (int32 Index = 0; Index < Model.Data[Row].Values.Num(); ++Index)
        {
            if (AxisFor(Index) != AxisIndex) continue;
            if (const FChartsValue* Value = Model.GetValue(Row, Index))
            {
                Result.Min = FMath::Min(Result.Min, Value->Value);
                Result.Max = FMath::Max(Result.Max, Value->Value);
            }
        }
    }
    if (Model.Axis.IsValidIndex(AxisIndex))
    {
        const FVector2D& Limits = Model.Axis[AxisIndex];
        if (FMath::IsFinite(Limits.X) && FMath::IsFinite(Limits.Y))
        {
            Result.Min = FMath::Min(Limits.X, Limits.Y);
            Result.Max = FMath::Max(Limits.X, Limits.Y);
        }
    }
    if (Result.Min == Result.Max)
    {
        if (Result.Min > 0.0) Result.Min = 0.0;
        else if (Result.Max < 0.0) Result.Max = 0.0;
        else Result.Max = 1.0;
    }
    return Result;
}

float ChartsMath::SegmentDistanceSquared(FVector2f Point, FVector2f A, FVector2f B)
{
    const FVector2f Delta = B - A;
    const float Length = Delta.SizeSquared();
    const float T = Length > UE_SMALL_NUMBER ? FMath::Clamp(FVector2f::DotProduct(Point - A, Delta) / Length, 0.0f, 1.0f) : 0.0f;
    return (Point - (A + T * Delta)).SizeSquared();
}

bool ChartsMath::PointInPolygon(FVector2f Point, const TArray<FVector2f>& Polygon)
{
    bool bInside = false;
    for (int32 I = 0, J = Polygon.Num() - 1; I < Polygon.Num(); J = I++)
    {
        const FVector2f A = Polygon[I], B = Polygon[J];
        if (SegmentDistanceSquared(Point, A, B) < 0.0001f) return true;
        if ((A.Y > Point.Y) != (B.Y > Point.Y) && Point.X < (B.X - A.X) * (Point.Y - A.Y) / (B.Y - A.Y) + A.X) bInside = !bInside;
    }
    return bInside;
}

bool ChartsMath::InSector(FVector2f Point, FVector2f Center, float Inner, float Outer, float Start, float Sweep)
{
    const FVector2f Delta = Point - Center;
    const float RadiusSquared = Delta.SizeSquared();
    if (Sweep <= 0.0f || Outer <= Inner || RadiusSquared < Inner * Inner || RadiusSquared > Outer * Outer) return false;
    float Angle = FMath::Fmod(FMath::Atan2(Delta.Y, Delta.X) - Start, 2.0f * PI);
    if (Angle < 0.0f) Angle += 2.0f * PI;
    return Sweep >= 2.0f * PI - 0.00001f || Angle <= Sweep;
}

TArray<FVector2f> ChartsMath::Curve(const TArray<FVector2f>& Points, int32 Segment, bool bSmooth)
{
    const FVector2f A = Points[Segment], B = Points[Segment + 1];
    if (!bSmooth) return { A, B };
    const float Slope = B.Y - A.Y;
    auto Tangent = [](float Before, float After)
    {
        if (Before * After <= 0.0f) return 0.0f;
        return 2.0f * Before * After / (Before + After);
    };
    const float M0 = Segment > 0 ? Tangent(A.Y - Points[Segment - 1].Y, Slope) : Slope;
    const float M1 = Segment + 2 < Points.Num() ? Tangent(Slope, Points[Segment + 2].Y - B.Y) : Slope;
    const int32 Steps = FMath::Clamp(FMath::CeilToInt((B - A).Size() / 5.0f), 4, 96);
    TArray<FVector2f> Result;
    Result.Reserve(Steps + 1);
    for (int32 Step = 0; Step <= Steps; ++Step)
    {
        const float T = static_cast<float>(Step) / Steps, T2 = T * T, T3 = T2 * T;
        const float Y = (2 * T3 - 3 * T2 + 1) * A.Y + (T3 - 2 * T2 + T) * M0 + (-2 * T3 + 3 * T2) * B.Y + (T3 - T2) * M1;
        Result.Emplace(FMath::Lerp(A.X, B.X, T), FMath::Clamp(Y, FMath::Min(A.Y, B.Y), FMath::Max(A.Y, B.Y)));
    }
    return Result;
}

FString ChartsMath::Number(double Value)
{
    const double Magnitude = FMath::Abs(Value);
    if (Magnitude >= 1.0e6 || (Magnitude > 0.0 && Magnitude < 0.001)) return FString::Printf(TEXT("%.2g"), Value);
    FNumberFormattingOptions Options;
    Options.SetMaximumFractionalDigits(2);
    return FText::AsNumber(Value, &Options).ToString();
}

FChartsRenderStyle ChartsMath::SanitizeStyle(const FChartsRenderStyle& Input)
{
    FChartsRenderStyle Style = Input;
    auto Safe = [](float Value, float Fallback, float Min, float Max) { return FMath::IsFinite(Value) ? FMath::Clamp(Value, Min, Max) : Fallback; };
    Style.DesiredSize.X = Safe(static_cast<float>(Style.DesiredSize.X), 560.0f, 64.0f, 16384.0f);
    Style.DesiredSize.Y = Safe(static_cast<float>(Style.DesiredSize.Y), 340.0f, 64.0f, 16384.0f);
    Style.FontSize = FMath::Clamp(Style.FontSize, 8, 48);
    Style.GridDivisions = FMath::Clamp(Style.GridDivisions, 2, 12);
    Style.LineThickness = Safe(Style.LineThickness, 2.0f, 0.5f, 20.0f);
    Style.PointRadius = Safe(Style.PointRadius, 3.5f, 1.0f, 20.0f);
    Style.HitTolerance = Safe(Style.HitTolerance, 7.0f, 2.0f, 30.0f);
    Style.BarGapRatio = Safe(Style.BarGapRatio, 0.25f, 0.0f, 0.8f);
    Style.AxisLabelPadding = Safe(Style.AxisLabelPadding, 8.0f, 0.0f, 200.0f);
    Style.LegendPadding = Safe(Style.LegendPadding, 12.0f, 0.0f, 200.0f);
    return Style;
}
