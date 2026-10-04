#include "SRuntimeChart.h"

#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Rendering/DrawElementTypes.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

void SRuntimeChart::Construct(const FArguments& Args)
{
    OnHovered = Args._OnHovered;
    OnClicked = Args._OnClicked;
    OnHoverEnded = Args._OnHoverEnded;
    SetCanTick(false);
    SetClipping(EWidgetClipping::ClipToBounds);
}

void SRuntimeChart::SetModel(FChartsModel InModel, const FChartsRenderStyle& InStyle)
{
    Model = MoveTemp(InModel);
    Style = ChartsMath::SanitizeStyle(InStyle);
    bSceneDirty = true;
    Pressed = FChartsElement();
    Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
    ClearHover();
}

void SRuntimeChart::EnsureScene(FVector2f Size) const
{
    if (bSceneDirty || !Scene.Size.Equals(Size, 0.01f))
    {
        Scene.Build(Model, Style, Size);
        bSceneDirty = false;
    }
}

FVector2D SRuntimeChart::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return Style.DesiredSize;
}

bool SRuntimeChart::HitTest(FVector2f Position, FChartsElement& OutElement) const
{
    const FVector2f Size(GetCachedGeometry().GetLocalSize());
    EnsureScene(Size);
    return Scene.HitTest(Position, OutElement);
}

bool SRuntimeChart::GetHovered(FChartsElement& OutElement) const
{
    OutElement = Hovered;
    return Hovered.DataIndex != INDEX_NONE;
}

int32 SRuntimeChart::OnPaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const
{
    EnsureScene(FVector2f(Geometry.GetLocalSize()));
    const bool bEnabled = ShouldBeEnabled(bParentEnabled);
    const ESlateDrawEffect Effect = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
    const FSlateBrush* Brush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
    const FSlateResourceHandle Resource = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);
    const FSlateRenderTransform Transform = Geometry.GetAccumulatedRenderTransform();
    TArray<FSlateVertex> Vertices;
    TArray<SlateIndex> Indices;
    int32 Layer = LayerId;
    auto FlushMesh = [&]()
    {
        if (Indices.IsEmpty()) return;
        FSlateDrawElement::MakeCustomVerts(OutDrawElements, Layer++, Resource, Vertices, Indices, nullptr, 0, 0, Effect);
        Vertices.Reset();
        Indices.Reset();
    };
    for (const FChartsPrimitive& Primitive : Scene.Primitives)
    {
        FLinearColor Color = Primitive.Color.GetColor(WidgetStyle) * WidgetStyle.GetColorAndOpacityTint();
        Color.A *= Primitive.Opacity;
        if (bEnabled && Style.bHighlightHovered && Hovered.DataIndex != INDEX_NONE && Primitive.ValueIndex == Hovered.ValueIndex
            && Primitive.Part == Hovered.Part && (Hovered.Part == EChartsElementPart::Area || Primitive.DataIndex == Hovered.DataIndex))
        {
            const float Alpha = Color.A;
            Color = FMath::Lerp(Color, FLinearColor::White, 0.2f);
            Color.A = Alpha;
        }
        if (Primitive.Kind == EChartsPrimitive::Mesh)
        {
            // Keep every batch valid on platforms using 16-bit Slate indices.
            if (Vertices.Num() + Primitive.Points.Num() > 60000) FlushMesh();
            const uint32 Base = Vertices.Num();
            const FColor VertexColor = Color.ToFColor(true);
            for (const FVector2f& Position : Primitive.Points)
                Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(Transform, Position, FVector2f(0.5f), VertexColor));
            for (uint32 Index : Primitive.Indices) Indices.Add(static_cast<SlateIndex>(Base + Index));
        }
        else
        {
            FlushMesh();
            if (Primitive.Kind == EChartsPrimitive::Line)
                FSlateDrawElement::MakeLines(OutDrawElements, Layer++, Geometry.ToPaintGeometry(), Primitive.Points, Effect, Color, true, Primitive.Thickness);
            else
                FSlateDrawElement::MakeText(OutDrawElements, Layer++, Geometry.ToPaintGeometry(FVector2f(Geometry.GetLocalSize()), FSlateLayoutTransform(Primitive.Points[0])),
                    Primitive.Text, FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Primitive.FontSize), Effect, Color);
        }
    }
    FlushMesh();
    return Layer;
}

void SRuntimeChart::ClearHover()
{
    const bool bHadHover = Hovered.DataIndex != INDEX_NONE;
    Hovered = FChartsElement();
    SetToolTipText(FText::GetEmpty());
    if (bHadHover)
    {
        Invalidate(EInvalidateWidgetReason::Paint);
        OnHoverEnded.ExecuteIfBound();
    }
}

FReply SRuntimeChart::OnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event)
{
    if (!IsEnabled()) { ClearHover(); return FReply::Unhandled(); }
    EnsureScene(FVector2f(Geometry.GetLocalSize()));
    FChartsElement Element;
    if (!Scene.HitTest(FVector2f(Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition())), Element))
    {
        ClearHover();
        return FReply::Unhandled();
    }
    const bool bChangedTarget = !Hovered.IsSameTarget(Element);
    const bool bChangedSample = bChangedTarget || Hovered.DataIndex != Element.DataIndex;
    Hovered = Element;
    if (Style.bShowTooltips && bChangedSample)
    {
        const FString Prefix = Element.Part == EChartsElementPart::Area ? Element.SeriesName + TEXT(" | ") : FString();
        SetToolTipText(FText::FromString(Prefix + Element.DataName + TEXT(": ") + ChartsMath::Number(Element.Value)));
    }
    if (bChangedTarget)
    {
        Invalidate(EInvalidateWidgetReason::Paint);
        OnHovered.ExecuteIfBound(Element);
    }
    return FReply::Handled();
}

void SRuntimeChart::OnMouseLeave(const FPointerEvent& Event)
{
    SLeafWidget::OnMouseLeave(Event);
    ClearHover();
}

FReply SRuntimeChart::Press(const FGeometry& Geometry, const FPointerEvent& Event)
{
    if (!IsEnabled()) return FReply::Unhandled();
    EnsureScene(FVector2f(Geometry.GetLocalSize()));
    if (Scene.HitTest(FVector2f(Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition())), Pressed))
        return FReply::Handled().CaptureMouse(AsShared());
    return FReply::Unhandled();
}

FReply SRuntimeChart::Release(const FGeometry& Geometry, const FPointerEvent& Event)
{
    const FChartsElement Previous = Pressed;
    Pressed = FChartsElement();
    if (Previous.DataIndex == INDEX_NONE) return HasMouseCapture() ? FReply::Handled().ReleaseMouseCapture() : FReply::Unhandled();
    EnsureScene(FVector2f(Geometry.GetLocalSize()));
    FChartsElement Element;
    if (IsEnabled() && Scene.HitTest(FVector2f(Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition())), Element)
        && Element.IsSameTarget(Previous))
        OnClicked.ExecuteIfBound(Element);
    return FReply::Handled().ReleaseMouseCapture();
}

FReply SRuntimeChart::OnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
    return Event.GetEffectingButton() == EKeys::LeftMouseButton ? Press(Geometry, Event) : FReply::Unhandled();
}

FReply SRuntimeChart::OnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& Event)
{
    return Event.GetEffectingButton() == EKeys::LeftMouseButton ? Release(Geometry, Event) : FReply::Unhandled();
}

void SRuntimeChart::OnMouseCaptureLost(const FCaptureLostEvent& Event)
{
    Pressed = FChartsElement();
    SLeafWidget::OnMouseCaptureLost(Event);
}

FReply SRuntimeChart::OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Event) { return Press(Geometry, Event); }
FReply SRuntimeChart::OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Event) { return OnMouseMove(Geometry, Event); }
FReply SRuntimeChart::OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Event)
{
    FReply Reply = Release(Geometry, Event);
    ClearHover();
    return Reply;
}
