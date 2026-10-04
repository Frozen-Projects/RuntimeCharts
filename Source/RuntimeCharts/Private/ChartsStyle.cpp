#include "ChartsScene.h"

FChartsRenderStyle::FChartsRenderStyle(const FChartsStyle& InStyle) : FChartsStyle(InStyle) {}

FChartsRenderStyle::FChartsRenderStyle(const FChartsGridStyle& InStyle) : FChartsRenderStyle(static_cast<const FChartsStyle&>(InStyle))
{
    GridColor = InStyle.GridColor;
    GridDivisions = InStyle.GridDivisions;
    bShowGrid = InStyle.bShowGrid;
}

FChartsRenderStyle::FChartsRenderStyle(const FChartsAxisStyle& InStyle) : FChartsRenderStyle(static_cast<const FChartsGridStyle&>(InStyle))
{
    AxisLabelPadding = InStyle.AxisLabelPadding;
}

FChartsRenderStyle::FChartsRenderStyle(const FChartsBarStyle& InStyle) : FChartsRenderStyle(static_cast<const FChartsAxisStyle&>(InStyle))
{
    BarGapRatio = InStyle.BarGapRatio;
}

FChartsRenderStyle::FChartsRenderStyle(const FChartsLineStyle& InStyle) : FChartsRenderStyle(static_cast<const FChartsAxisStyle&>(InStyle))
{
    LineThickness = InStyle.LineThickness;
    PointRadius = InStyle.PointRadius;
    HitTolerance = InStyle.HitTolerance;
}

FChartsRenderStyle::FChartsRenderStyle(const FChartsComboStyle& InStyle) : FChartsRenderStyle(static_cast<const FChartsLineStyle&>(InStyle))
{
    BarGapRatio = InStyle.BarGapRatio;
}
