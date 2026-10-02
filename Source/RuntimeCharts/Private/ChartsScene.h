#pragma once

#include "ChartsTypes.h"
#include "Layout/SlateRect.h"

enum class EChartsKind : uint8 { Bar, Line, Area, Combo, Pie, PolarArea, Radar };

struct FChartsModel
{
    EChartsKind Kind = EChartsKind::Bar;
    FString Title;
    TArray<FChartsData> Data;
    TArray<FVector2D> Axis;
    TArray<FString> Names;
    TArray<EChartsSeriesType> SeriesTypes;
    bool bHorizontal = false;
    bool bPoints = false;
    bool bSmooth = false;
    bool bFill = false;
    float FillOpacity = 0.3f;
    float InnerRadius = 0.0f;
    float StartAngle = -90.0f;
    double MaxValue = 0.0;

    static FChartsModel From(const FChartsBarConfig& Config);
    static FChartsModel From(const FChartsLineConfig& Config);
    static FChartsModel From(const FChartsAreaConfig& Config);
    static FChartsModel From(const FChartsComboConfig& Config);
    static FChartsModel From(const FChartsPieConfig& Config);
    static FChartsModel From(const FChartsPolarAreaConfig& Config);
    static FChartsModel From(const FChartsRadarConfig& Config);
    int32 NumSeries() const;
    bool IsBarSeries(int32 Series) const;
    const FChartsValue* GetValue(int32 DataIndex, int32 ValueIndex) const;
    FChartsElement Element(int32 DataIndex, int32 ValueIndex) const;
};

struct FChartsRange
{
    double Min = 0.0;
    double Max = 1.0;
    double Normalize(double Value) const;
    double At(double Fraction) const;
};

namespace ChartsMath
{
    FChartsRange Range(const FChartsModel& Model, int32 Series);
    float SegmentDistanceSquared(FVector2f Point, FVector2f A, FVector2f B);
    bool PointInPolygon(FVector2f Point, const TArray<FVector2f>& Polygon);
    bool InSector(FVector2f Point, FVector2f Center, float Inner, float Outer, float Start, float Sweep);
    TArray<FVector2f> Curve(const TArray<FVector2f>& Points, int32 Segment, bool bSmooth);
    FString Number(double Value);
    FChartsStyle SanitizeStyle(const FChartsStyle& Style);
}

enum class EChartsPrimitive : uint8 { Mesh, Line, Text };

struct FChartsPrimitive
{
    EChartsPrimitive Kind = EChartsPrimitive::Mesh;
    TArray<FVector2f> Points;
    TArray<uint32> Indices;
    FSlateColor Color = FSlateColor(FLinearColor::White);
    float Opacity = 1.0f;
    float Thickness = 1.0f;
    FString Text;
    int32 FontSize = 11;
    int32 DataIndex = INDEX_NONE;
    int32 ValueIndex = INDEX_NONE;
};

enum class EChartsHitShape : uint8 { Rect, Circle, Sector, Line, Polygon };

struct FChartsHitRegion
{
    EChartsHitShape Shape = EChartsHitShape::Rect;
    FChartsElement Element;
    TArray<FVector2f> Points;
    FSlateRect Bounds;
    FVector2f Center = FVector2f::ZeroVector;
    float Inner = 0.0f;
    float Outer = 0.0f;
    float Start = 0.0f;
    float Sweep = 0.0f;
    float Tolerance = 0.0f;
    bool Contains(FVector2f Point) const;
};

struct FChartsScene
{
    FVector2f Size = FVector2f::ZeroVector;
    FSlateRect Plot;
    TArray<FChartsPrimitive> Primitives;
    TArray<FChartsHitRegion> Hits;

    void Build(const FChartsModel& Model, const FChartsStyle& InStyle, FVector2f InSize);
    bool HitTest(FVector2f Point, FChartsElement& OutElement) const;

private:
    FChartsStyle Style;
    void Text(FVector2f Position, FString Label, int32 FontSize = 0);
    void Line(TArray<FVector2f> Points, FSlateColor Color, float Thickness = 1.0f, const FChartsElement* Element = nullptr);
    void Mesh(TArray<FVector2f> Points, TArray<uint32> Indices, FSlateColor Color, float Opacity = 1.0f, const FChartsElement* Element = nullptr);
    void Rect(const FSlateRect& Bounds, FSlateColor Color, const FChartsElement* Element = nullptr);
    void Sector(FVector2f Center, float Inner, float Outer, float Start, float Sweep, const FChartsElement& Element);
    void Point(FVector2f Position, const FChartsElement& Element, bool bDraw);
    void Legend(const FChartsModel& Model);
    void Cartesian(const FChartsModel& Model);
    void Radial(const FChartsModel& Model);
    void Radar(const FChartsModel& Model);
};
