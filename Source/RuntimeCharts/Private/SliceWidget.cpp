#include "SliceWidget.h"

#include "Framework/Application/SlateApplication.h"
#include "Layout/ArrangedChildren.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Types/PaintArgs.h"
#include "Widgets/SLeafWidget.h"

namespace SliceWidgetPrivate
{
	double ClampFinite(double Value, double Min, double Max)
	{
		return FMath::IsFinite(Value) ? FMath::Clamp(Value, Min, Max) : Min;
	}

	int32 SegmentCount(double ArcSize, double Smoothness)
	{
		return ArcSize > 0.0 ? FMath::Max(1, FMath::RoundToInt(ArcSize * Smoothness)) : 0;
	}

	bool Contains(const FGeometry& Geometry, const FVector2D& ScreenPosition, double Angle, double ArcSize, double Smoothness)
	{
		const FVector2f Size = Geometry.GetLocalSize();
		const double Radius = FMath::Min(Size.X, Size.Y) * 0.5;
		const int32 NumSegments = SegmentCount(ArcSize, Smoothness);
		if (Radius <= 0.0 || NumSegments == 0) return false;

		const FVector2D Offset = FVector2D(Geometry.AbsoluteToLocal(ScreenPosition)) - FVector2D(Size) * 0.5;
		const double DistanceSquared = Offset.SizeSquared();
		if (!FMath::IsFinite(DistanceSquared) || DistanceSquared > Radius * Radius) return false;
		if (DistanceSquared == 0.0) return true;

		const double CursorAngle = FMath::RadiansToDegrees(FMath::Atan2(Offset.Y, Offset.X));
		const double RelativeAngle = FMath::Fmod(CursorAngle - Angle + 720.0, 360.0);
		if (RelativeAngle > ArcSize) return false;

		const double Step = ArcSize / NumSegments;
		const int32 Segment = FMath::Min(FMath::FloorToInt(RelativeAngle / Step), NumSegments - 1);
		const double MidAngle = FMath::DegreesToRadians(Angle + (Segment + 0.5) * Step);
		const double ChordDistance = Radius * FMath::Cos(FMath::DegreesToRadians(Step * 0.5));
		return Offset.X * FMath::Cos(MidAngle) + Offset.Y * FMath::Sin(MidAngle) <= ChordDistance;
	}

	class SSliceHitRegion : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SSliceHitRegion) {} SLATE_END_ARGS()
		void Construct(const FArguments&) {}
		virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
		virtual int32 OnPaint(const FPaintArgs&, const FGeometry&, const FSlateRect&, FSlateWindowElementList&, int32 LayerId,
			const FWidgetStyle&, bool) const override { return LayerId; }
	};

	void AddHalfPlane(FSlateClippingState& State, const FGeometry& Geometry, const FVector2f& A, const FVector2f& B, float Extent, bool bRadialEdge = false)
	{
		const FVector2f Tangent = (B - A).GetSafeNormal();
		const FVector2f Inward(-Tangent.Y, Tangent.X);
		// Slate excludes exact clip edges; retain the center and shared radial seams.
		const FVector2f EdgeBias = bRadialEdge ? Inward * 0.001f : FVector2f::ZeroVector;
		const FSlateRenderTransform& Transform = Geometry.GetAccumulatedRenderTransform();
		// Keep fractional edges: Slate's axis-aligned constructor rounds to whole pixels.
		FSlateClippingZone Zone(FVector2f(0, 0), FVector2f(2, 1), FVector2f(-1, 2), FVector2f(1, 3));
		Zone.TopLeft = Transform.TransformPoint(A - Tangent * Extent - EdgeBias);
		Zone.TopRight = Transform.TransformPoint(B + Tangent * Extent - EdgeBias);
		Zone.BottomLeft = Transform.TransformPoint(A - Tangent * Extent + Inward * Extent - EdgeBias);
		Zone.BottomRight = Transform.TransformPoint(B + Tangent * Extent + Inward * Extent - EdgeBias);
		State.StencilQuads.Add(Zone);
	}
}

SSlateSlice::SSlateSlice() : HitRegions(this) {}

void SSlateSlice::Construct(const FArguments& InArgs)
{
	Brush = InArgs._Brush;
	Angle = SliceWidgetPrivate::ClampFinite(InArgs._Angle, 0.0, 360.0);
	ArcSize = SliceWidgetPrivate::ClampFinite(InArgs._ArcSize, 0.0, 360.0);
	Smoothness = SliceWidgetPrivate::ClampFinite(InArgs._Smoothness, 0.5, 10.0);
	for (int32 Index = 0; Index < 3; ++Index) HitRegions.Add(SNew(SliceWidgetPrivate::SSliceHitRegion));
	SetVisibility(GetVisibility());
	UpdateArc();
}

void SSlateSlice::SetVisibility(TAttribute<EVisibility> InVisibility)
{
	auto MapVisibility = [](EVisibility Value) { return Value == EVisibility::Visible ? EVisibility::SelfHitTestInvisible : Value; };
	if (InVisibility.IsBound())
	{
		SWidget::SetVisibility(TAttribute<EVisibility>::CreateLambda([InVisibility, MapVisibility]() { return MapVisibility(InVisibility.Get()); }));
	}
	else
	{
		SWidget::SetVisibility(MapVisibility(InVisibility.Get(EVisibility::Visible)));
	}
}

FChildren* SSlateSlice::GetChildren() { return &HitRegions; }
FVector2D SSlateSlice::ComputeDesiredSize(float) const { return FVector2D::ZeroVector; }

void SSlateSlice::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	for (int32 Index = 0; Index < HitRegions.Num(); ++Index)
	{
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(ConstCastSharedRef<SWidget>(HitRegions.GetChildAt(Index)), AllottedGeometry.GetLocalSize(), FSlateLayoutTransform()));
	}
}

void SSlateSlice::UpdateArc()
{
	const int32 NumSegments = SliceWidgetPrivate::SegmentCount(ArcSize, Smoothness);
	ArcPoints.Reset(NumSegments + 1);
	if (NumSegments > 0)
	{
		const double Step = ArcSize / NumSegments;
		SegmentsPerRegion = FMath::Max(1, FMath::FloorToInt(180.0 / Step));
		for (int32 Index = 0; Index <= NumSegments; ++Index)
		{
			const double Radians = FMath::DegreesToRadians(Angle + Step * Index);
			ArcPoints.Emplace(FMath::Cos(Radians), FMath::Sin(Radians));
		}
	}
	Invalidate(EInvalidateWidgetReason::Paint);
}

int32 SSlateSlice::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2f Size = AllottedGeometry.GetLocalSize();
	const FVector2f Center = Size * 0.5f;
	const float Radius = FMath::Min(Size.X, Size.Y) * 0.5f;
	const int32 NumSegments = FMath::Max(0, ArcPoints.Num() - 1);
	const FLinearColor Tint = Brush ? Brush->GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint() : FLinearColor::Transparent;
	const bool bHasShape = Radius > 0.0f && NumSegments > 0 && Tint.A > 0.0f;
	const FSlateRenderTransform& Transform = AllottedGeometry.GetAccumulatedRenderTransform();

	if (bHasShape)
	{
		TArray<FSlateVertex> Vertices;
		TArray<SlateIndex> Indices;
		Vertices.Reserve(NumSegments + 2);
		Indices.Reserve(NumSegments * 3);
		const FColor VertexColor = Tint.ToFColor(true);
		Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(Transform, Center, FVector2f(0.5f, 0.5f), VertexColor));
		for (const FVector2f& Point : ArcPoints)
		{
			Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(Transform, Center + Point * Radius, Point * 0.5f + FVector2f(0.5f), VertexColor));
		}
		for (int32 Index = 0; Index < NumSegments; ++Index)
		{
			Indices.Append({0, static_cast<SlateIndex>(Index + 1), static_cast<SlateIndex>(Index + 2)});
		}
		const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);
		const ESlateDrawEffect Effects = ShouldBeEnabled(bParentEnabled) ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Vertices, Indices, nullptr, 0, 0, Effects);
	}

	for (int32 RegionIndex = 0; RegionIndex < HitRegions.Num(); ++RegionIndex)
	{
		const int32 First = RegionIndex * SegmentsPerRegion;
		const int32 Last = FMath::Min(First + SegmentsPerRegion, NumSegments);
		FSlateClippingState Clip;
		if (bHasShape && First < Last)
		{
			const TOptional<FSlateClippingState> ParentClip = OutDrawElements.GetClippingState();
			if (ParentClip.IsSet())
			{
				Clip.StencilQuads = ParentClip->StencilQuads;
				if (ParentClip->ScissorRect.IsSet()) Clip.StencilQuads.Add(ParentClip->ScissorRect.GetValue());
			}
			const float Extent = Radius * 8.0f;
			SliceWidgetPrivate::AddHalfPlane(Clip, AllottedGeometry, Center, Center + ArcPoints[First] * Radius, Extent, true);
			for (int32 Index = First; Index < Last; ++Index)
			{
				SliceWidgetPrivate::AddHalfPlane(Clip, AllottedGeometry, Center + ArcPoints[Index] * Radius, Center + ArcPoints[Index + 1] * Radius, Extent);
			}
			SliceWidgetPrivate::AddHalfPlane(Clip, AllottedGeometry, Center + ArcPoints[Last] * Radius, Center, Extent, true);
		}
		else
		{
			Clip.ScissorRect = FSlateClippingZone(FSlateRect(-1.0f, -1.0f, -1.0f, -1.0f));
		}

		// These children draw nothing; their inherited masks restrict Slate's hit grid.
		OutDrawElements.GetClippingManager().PushClippingState(Clip);
		HitRegions.GetChildAt(RegionIndex)->Paint(Args.WithNewParent(this), AllottedGeometry, MyCullingRect, OutDrawElements,
			LayerId, InWidgetStyle, ShouldBeEnabled(bParentEnabled));
		OutDrawElements.PopClip();
	}
	return LayerId;
}

bool SSlateSlice::ContainsPoint(const FGeometry& MyGeometry, const FVector2D& ScreenPosition) const
{
	return Brush && SliceWidgetPrivate::Contains(MyGeometry, ScreenPosition, Angle, ArcSize, Smoothness);
}

void SSlateSlice::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);
	if (USliceWidget* Owner = ParentWidget.Get()) Owner->Delegate_Mouse_Enter.Broadcast(MyGeometry, MouseEvent);
}

void SSlateSlice::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);
	if (USliceWidget* Owner = ParentWidget.Get()) Owner->Delegate_Mouse_Leave.Broadcast(GetCachedGeometry(), MouseEvent);
}

FReply SSlateSlice::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!ContainsPoint(MyGeometry, MouseEvent.GetScreenSpacePosition())) return FReply::Unhandled();
	if (USliceWidget* Owner = ParentWidget.Get()) Owner->Delegate_Mouse_Move.Broadcast(MyGeometry, MouseEvent);
	return FReply::Handled();
}

FReply SSlateSlice::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!ContainsPoint(MyGeometry, MouseEvent.GetScreenSpacePosition())) return FReply::Unhandled();
	if (USliceWidget* Owner = ParentWidget.Get()) Owner->Delegate_Mouse_Down.Broadcast(MyGeometry, MouseEvent);
	return FReply::Handled();
}

void SSlateSlice::SetBrush(FSlateBrush* InBrush)
{
	Brush = InBrush;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SSlateSlice::SetAngle(double InAngle)
{
	Angle = SliceWidgetPrivate::ClampFinite(InAngle, 0.0, 360.0);
	UpdateArc();
}

void SSlateSlice::SetArcSize(double InArcSize)
{
	ArcSize = SliceWidgetPrivate::ClampFinite(InArcSize, 0.0, 360.0);
	UpdateArc();
}

void SSlateSlice::SetSmoothness(double InSmoothness)
{
	Smoothness = SliceWidgetPrivate::ClampFinite(InSmoothness, 0.5, 10.0);
	UpdateArc();
}

void SSlateSlice::SetParent(USliceWidget* InParent) { ParentWidget = InParent; }

TSharedRef<SWidget> USliceWidget::RebuildWidget()
{
	MySlice = SNew(SSlateSlice).Brush(&Brush).Angle(Angle).ArcSize(ArcSize).Smoothness(Smoothness);
	MySlice->SetParent(this);
	return MySlice.ToSharedRef();
}

void USliceWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	if (!MySlice.IsValid())
	{
		return;
	}
	
	MySlice->SetBrush(&Brush);
	MySlice->SetAngle(Angle);
	MySlice->SetArcSize(ArcSize);
	MySlice->SetSmoothness(Smoothness);
	MySlice->SetParent(this);
}

void USliceWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	if (MySlice.IsValid()) MySlice->SetParent(nullptr);
	MySlice.Reset();
}

void USliceWidget::SetAngle(double InAngle)
{
	Angle = SliceWidgetPrivate::ClampFinite(InAngle, 0.0, 360.0);
	if (MySlice.IsValid()) MySlice->SetAngle(Angle);
}

void USliceWidget::SetArcSize(double InArcSize)
{
	ArcSize = SliceWidgetPrivate::ClampFinite(InArcSize, 0.0, 360.0);
	if (MySlice.IsValid()) MySlice->SetArcSize(ArcSize);
}

void USliceWidget::SetSmoothness(double InSmoothness)
{
	Smoothness = SliceWidgetPrivate::ClampFinite(InSmoothness, 0.5, 10.0);
	if (MySlice.IsValid()) MySlice->SetSmoothness(Smoothness);
}

void USliceWidget::SetParent()
{
	if (MySlice.IsValid()) MySlice->SetParent(this);
}

bool USliceWidget::IsMouseOnPie(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return SliceWidgetPrivate::Contains(MyGeometry, MouseEvent.GetScreenSpacePosition(),
		SliceWidgetPrivate::ClampFinite(Angle, 0.0, 360.0), SliceWidgetPrivate::ClampFinite(ArcSize, 0.0, 360.0),
		SliceWidgetPrivate::ClampFinite(Smoothness, 0.5, 10.0));
}

#if WITH_EDITOR
const FText USliceWidget::GetPaletteCategory()
{
	return NSLOCTEXT("RuntimeCharts", "Palette", "Runtime Charts");
}
#endif
