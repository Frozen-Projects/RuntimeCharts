// Fill out your copyright notice in the Description page of Project Settings.

#include "SliceWidget.h"

#include "SlateOptMacros.h"
#include "Kismet/KismetMathLibrary.h"
#include "Input/HittestGrid.h"

#define LOCTEXT_NAMESPACE "UMG"

void USliceWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	this->MySlice->SetBrush(&this->Brush);
	this->MySlice->SetAngle(this->Angle);
	this->MySlice->SetArcSize(this->ArcSize);
	this->MySlice->SetSmoothness(this->Smoothness);
	this->MySlice->SetParent(this);
}

void USliceWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	this->MySlice.Reset();
}

void USliceWidget::SetAngle(double InAngle)
{
	const double Temp_Angle = FMath::Clamp(InAngle, 0, 360);
	
	if (this->MySlice)
	{
		this->Angle = Temp_Angle;
		this->MySlice->SetAngle(Temp_Angle);
	}
}

void USliceWidget::SetArcSize(double InArcSize)
{
	const double Temp_ArcSize = FMath::Clamp(InArcSize, 0, 360);
	
	if (this->MySlice)
	{
		this->ArcSize = Temp_ArcSize;
		this->MySlice->SetArcSize(Temp_ArcSize);
	}
}

void USliceWidget::SetSmoothness(double InSmoothness)
{
	const double Temp_Smoothness = FMath::Clamp(InSmoothness, 0.5, 10);

	if (this->MySlice)
	{
		this->Smoothness = Temp_Smoothness;
		this->MySlice->SetArcSize(this->Smoothness);
	}
}

void USliceWidget::SetParent()
{
	if (this->MySlice)
	{
		this->MySlice->SetParent(this);
	}
}

bool USliceWidget::IsMouseOnPie(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D CursorPos = MouseEvent.GetScreenSpacePosition();
	const FVector2D Size = MyGeometry.GetAbsoluteSize();
	const FVector2D Center = MyGeometry.GetAbsolutePosition() + 0.5 * Size;
	const double Radius = FMath::Min(Size.X, Size.Y) * 0.5f;
	const double CursorDistance = UKismetMathLibrary::Distance2D(CursorPos, Center);

	if (CursorDistance > Radius)
	{
		return false;
	}

	const FVector2D Fixed_CursorPos = FVector2D(CursorPos.X, MyGeometry.GetRenderBoundingRect().GetBottomLeft2f().Y - CursorPos.Y);
	const FVector2D Fixed_Center = FVector2D(Center.X, MyGeometry.GetRenderBoundingRect().GetBottomLeft2f().Y - Center.Y);
	const double Yaw = -1 * UKismetMathLibrary::FindLookAtRotation(FVector(Fixed_Center.X, Fixed_Center.Y, double(0)), FVector(Fixed_CursorPos.X, Fixed_CursorPos.Y, double(0))).Yaw;
	double Angle_Cursor = Yaw >= 0 ? Yaw : 360 + Yaw;
	const double Difference = this->Angle - (360 - this->ArcSize);

	double Range_Min = 0;
	double Range_Max = 0;

	if (Difference < 0)
	{
		Range_Min = this->Angle;
		Range_Max = 360 + Difference;

		if (FMath::IsWithin(Angle_Cursor, Range_Min, Range_Max))
		{
			return true;
		}

		else
		{
			return false;
		}
	}

	else
	{
		Range_Min = Difference;
		Range_Max = this->Angle;

		if (!FMath::IsWithin(Angle_Cursor, Range_Min, Range_Max))
		{
			return true;
		}

		else
		{
			return false;
		}
	}
}

#if WITH_EDITOR
const FText USliceWidget::GetPaletteCategory()
{
	return LOCTEXT("FF Charts : Pie", "FF Charts");
}
#endif

TSharedRef<SWidget> USliceWidget::RebuildWidget()
{
	MySlice = SNew(SSlateSlice)
		.Brush(&this->Brush)
		.Angle(FMath::Clamp(this->Angle, 0, 360))
		.ArcSize(FMath::Clamp(this->ArcSize, 0, 360))
		.Smoothness(FMath::Clamp(this->ArcSize, 0.5, 10));
	
	return MySlice.ToSharedRef();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSlateSlice::Construct(const FArguments& InArgs)
{
	this->Brush = (InArgs._Brush);
	this->Angle = FMath::Clamp(InArgs._Angle, 0, 360);
	this->ArcSize = FMath::Clamp(InArgs._ArcSize, 0, 360);
	this->Smoothness = FMath::Clamp(InArgs._Smoothness, 0.5, 10);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SSlateSlice::CallbackCursorHit(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FDelegateSlice Delegate)
{
	const FVector2D CursorPos = MouseEvent.GetScreenSpacePosition();
	const FVector2D Size = MyGeometry.GetAbsoluteSize();
	const FVector2D Center = MyGeometry.GetAbsolutePosition() + 0.5 * Size;
	const double Radius = FMath::Min(Size.X, Size.Y) * 0.5f;
	const double CursorDistance = UKismetMathLibrary::Distance2D(CursorPos, Center);

	if (CursorDistance > Radius)
	{
		return FReply::Unhandled();
	}

	const FVector2D Fixed_CursorPos = FVector2D(CursorPos.X, MyGeometry.GetRenderBoundingRect().GetBottomLeft2f().Y - CursorPos.Y);
	const FVector2D Fixed_Center = FVector2D(Center.X, MyGeometry.GetRenderBoundingRect().GetBottomLeft2f().Y - Center.Y);
	const double Yaw = -1 * UKismetMathLibrary::FindLookAtRotation(FVector(Fixed_Center.X, Fixed_Center.Y, double(0)), FVector(Fixed_CursorPos.X, Fixed_CursorPos.Y, double(0))).Yaw;
	double Angle_Cursor = Yaw >= 0 ? Yaw : 360 + Yaw;
	const double Difference = Angle - (360 - ArcSize);

	double Range_Min = 0;
	double Range_Max = 0;
	
	if (Difference < 0)
	{
		Range_Min = Angle;
		Range_Max = 360 + Difference;

		if (FMath::IsWithin(Angle_Cursor, Range_Min, Range_Max))
		{
			if (this->ParentBuffer)
			{
				Delegate.Broadcast(MyGeometry, MouseEvent);
			}

			return FReply::Handled();
		}

		else
		{
			return FReply::Unhandled();
		}
	}

	else
	{
		Range_Min = Difference;
		Range_Max = Angle;

		if (!FMath::IsWithin(Angle_Cursor, Range_Min, Range_Max))
		{
			if (this->ParentBuffer)
			{
				Delegate.Broadcast(MyGeometry, MouseEvent);
			}

			return FReply::Handled();
		}

		else
		{
			return FReply::Unhandled();
		}
	}
}

FReply SSlateSlice::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!ParentBuffer)
	{
		return FReply::Unhandled();
	}

	USliceWidget* ParentWidget = Cast<USliceWidget>((USliceWidget*)this->ParentBuffer);

	if (!IsValid(ParentWidget))
	{
		return FReply::Unhandled();
	}

	return this->CallbackCursorHit(MyGeometry, MouseEvent, ParentWidget->Delegate_Mouse_Move);
}

FReply SSlateSlice::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!ParentBuffer)
	{
		return FReply::Unhandled();
	}

	USliceWidget* ParentWidget = Cast<USliceWidget>((USliceWidget*)this->ParentBuffer);

	if (!IsValid(ParentWidget))
	{
		return FReply::Unhandled();
	}

	return this->CallbackCursorHit(MyGeometry, MouseEvent, ParentWidget->Delegate_Mouse_Down);
}

int32 SSlateSlice::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	auto CalculateUV = [](const FVector2D& Point, const FVector2D& Center, double Radius) -> FVector2D
		{
			// Normalize direction
			const FVector2D Direction = (Point - Center) / Radius;
			const float U = 0.5f + Direction.X * 0.5f;
			const float V = 0.5f + Direction.Y * 0.5f;

			return FVector2D(U, V);
		};

	const FColor VertexColor = Brush->GetTint(InWidgetStyle).ToFColor(true);

	const FVector2D Pos = AllottedGeometry.GetAbsolutePosition();
	const FVector2D Size = AllottedGeometry.GetAbsoluteSize();
	const FVector2D Center = Pos + 0.5 * Size;
	const double Radius = FMath::Min(Size.X, Size.Y) * 0.5;
	const FVector2D CenterUV = CalculateUV(Center, Center, Radius);

	const int32 NumSegments = FMath::RoundToInt(this->ArcSize * this->Smoothness);
	TArray<FSlateVertex> Vertices;
	Vertices.Reserve(NumSegments + 3);

	// Add center vertex
	Vertices.AddZeroed();
	FSlateVertex& CenterVertex = Vertices.Last();
	CenterVertex.Position = (FVector2f)Center;
	CenterVertex.TexCoords[0] = CenterUV.X;
	CenterVertex.TexCoords[1] = CenterUV.Y;
	CenterVertex.Color = VertexColor;

	// Add edge vertices
	for (int i = 0; i < NumSegments + 2; ++i)
	{
		const double CurrentAngle = FMath::DegreesToRadians(this->ArcSize * i / NumSegments + this->Angle);
		const FVector2D EndPoint = (FVector2D)(Center + FVector2D(Radius * FVector2D(FMath::Cos(CurrentAngle), FMath::Sin(CurrentAngle))));
		const FVector2D EndPointUV = CalculateUV(EndPoint, Center, Radius);

		Vertices.AddZeroed();
		FSlateVertex& OuterVert = Vertices.Last();
		OuterVert.Position = (FVector2f)EndPoint;
		OuterVert.TexCoords[0] = EndPointUV.X;
		OuterVert.TexCoords[1] = EndPointUV.Y;
		OuterVert.Color = VertexColor;
	}

	TArray<SlateIndex> Indices;
	for (int i = 0; i <= NumSegments; ++i)
	{
		Indices.Add(0);
		Indices.Add(i);
		Indices.Add(i + 1);
	}

	const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);
	FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Vertices, Indices, nullptr, 0, 0, ESlateDrawEffect::PreMultipliedAlpha);
	FHittestGrid& PreviousGrid = Args.GetHittestGrid();

	return LayerId;
}

void SSlateSlice::SetBrush(FSlateBrush* InBrush)
{
	if (InBrush)
	{
		this->Brush = InBrush;
	}
}

void SSlateSlice::SetAngle(double InAngle)
{
	this->Angle = FMath::Clamp(InAngle, 0, 360);
}

void SSlateSlice::SetArcSize(double InArcSize)
{
	this->ArcSize = FMath::Clamp(InArcSize, 0, 360);
}

void SSlateSlice::SetSmoothness(double InSmoothness)
{
	this->Smoothness = InSmoothness;
}

void SSlateSlice::SetParent(void* InBuffer)
{
	if (InBuffer)
	{
		this->ParentBuffer = InBuffer;
	}
}