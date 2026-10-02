#pragma once

#include "ChartsScene.h"
#include "Widgets/SLeafWidget.h"

DECLARE_DELEGATE_OneParam(FOnRuntimeChartElement, const FChartsElement&);

class SRuntimeChart : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SRuntimeChart) {}
        SLATE_EVENT(FOnRuntimeChartElement, OnHovered)
        SLATE_EVENT(FOnRuntimeChartElement, OnClicked)
        SLATE_EVENT(FSimpleDelegate, OnHoverEnded)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);
    void SetModel(FChartsModel InModel, const FChartsStyle& InStyle);
    bool HitTest(FVector2f Position, FChartsElement& OutElement) const;
    bool GetHovered(FChartsElement& OutElement) const;

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullingRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const override;
    virtual FReply OnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event) override;
    virtual void OnMouseLeave(const FPointerEvent& Event) override;
    virtual FReply OnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;
    virtual FReply OnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& Event) override;
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& Event) override;
    virtual FReply OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Event) override;
    virtual FReply OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Event) override;
    virtual FReply OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Event) override;

private:
    void EnsureScene(FVector2f Size) const;
    void ClearHover();
    FReply Press(const FGeometry& Geometry, const FPointerEvent& Event);
    FReply Release(const FGeometry& Geometry, const FPointerEvent& Event);
    FChartsModel Model;
    FChartsStyle Style;
    mutable FChartsScene Scene;
    mutable bool bSceneDirty = true;
    FChartsElement Hovered;
    FChartsElement Pressed;
    FOnRuntimeChartElement OnHovered;
    FOnRuntimeChartElement OnClicked;
    FSimpleDelegate OnHoverEnded;
};
