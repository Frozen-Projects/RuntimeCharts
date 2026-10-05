#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Layout/Children.h"
#include "Widgets/SWidget.h"
#include "SliceWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateSlice, const FGeometry&, MyGeometry, const FPointerEvent&, MouseEvent);

class USliceWidget;

class RUNTIMECHARTS_API SSlateSlice : public SWidget
{
public:

	SLATE_BEGIN_ARGS(SSlateSlice) : _Brush(nullptr), _Angle(0.0), _ArcSize(0.0), _Smoothness(0.5) {}
		SLATE_ARGUMENT(FSlateBrush*, Brush)
		SLATE_ARGUMENT(double, Angle)
		SLATE_ARGUMENT(double, ArcSize)
		SLATE_ARGUMENT(double, Smoothness)
	SLATE_END_ARGS()

	SSlateSlice();
	void Construct(const FArguments& InArgs);
	virtual void SetVisibility(TAttribute<EVisibility> InVisibility) override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SetBrush(FSlateBrush* InBrush);
	void SetAngle(double InAngle);
	void SetArcSize(double InArcSize);
	void SetSmoothness(double InSmoothness);
	void SetParent(USliceWidget* InParent);
	bool ContainsPoint(const FGeometry& MyGeometry, const FVector2D& ScreenPosition) const;

private:

	void UpdateArc();
	FSlateBrush* Brush = nullptr;
	double Angle = 0.0;
	double ArcSize = 0.0;
	double Smoothness = 0.5;
	TWeakObjectPtr<USliceWidget> ParentWidget;
	TSlotlessChildren<SWidget> HitRegions;
	TArray<FVector2f> ArcPoints;
	int32 SegmentsPerRegion = 1;
};

UCLASS()
class RUNTIMECHARTS_API USliceWidget : public UWidget
{
	GENERATED_BODY()

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;
	TSharedPtr<SSlateSlice> MySlice;

public:

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frozen Forest | RuntimeCharts | Slice Widget", meta = (ClampMin = "0", ClampMax = "360"))
	double Angle = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frozen Forest | RuntimeCharts | Slice Widget", meta = (ClampMin = "0", ClampMax = "360"))
	double ArcSize = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frozen Forest | RuntimeCharts | Slice Widget", meta = (ClampMin = "0.5", ClampMax = "10"))
	double Smoothness = 0.5;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	FDelegateSlice Delegate_Mouse_Move;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	FDelegateSlice Delegate_Mouse_Down;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	FDelegateSlice Delegate_Mouse_Enter;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	FDelegateSlice Delegate_Mouse_Leave;

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	void SetAngle(double InAngle);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	void SetArcSize(double InArcSize);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	void SetSmoothness(double InSmoothness);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	void SetParent();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | RuntimeCharts | Slice Widget")
	virtual bool IsMouseOnPie(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
};
