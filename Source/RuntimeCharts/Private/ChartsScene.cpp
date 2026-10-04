#include "ChartsScene.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

namespace
{
    FVector2f RadialPoint(FVector2f Center, float Radius, float Angle)
    {
        return Center + FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
    }

    FString ShortLabel(const FString& Label, int32 MaxCharacters)
    {
        return Label.Len() <= MaxCharacters ? Label : Label.Left(FMath::Max(1, MaxCharacters - 1)) + TEXT("…");
    }

    bool InRect(FVector2f Point, const FSlateRect& Rect)
    {
        return Point.X >= Rect.Left && Point.X <= Rect.Right && Point.Y >= Rect.Top && Point.Y <= Rect.Bottom;
    }

    FVector2f TextSize(const FString& Label, int32 FontSize)
    {
        if (!FSlateApplication::IsInitialized()) return FVector2f(Label.Len() * FontSize * 0.65f, FontSize * 1.4f);
        return FVector2f(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Label, FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize)));
    }

    bool InTriangle(FVector2f Point, FVector2f A, FVector2f B, FVector2f C)
    {
        const float Area = FVector2f::CrossProduct(B - A, C - A);
        if (FMath::Abs(Area) <= UE_SMALL_NUMBER) return false;
        const float U = FVector2f::CrossProduct(B - Point, C - Point) / Area;
        const float V = FVector2f::CrossProduct(C - Point, A - Point) / Area;
        return U >= 0 && V >= 0 && U + V <= 1;
    }
}

bool FChartsHitRegion::Contains(FVector2f Point) const
{
    if (!InRect(Point, Bounds)) return false;
    switch (Shape)
    {
    case EChartsHitShape::Rect: return true;
    case EChartsHitShape::Circle: return (Point - Center).SizeSquared() <= Outer * Outer;
    case EChartsHitShape::Sector: return ChartsMath::InSector(Point, Center, Inner, Outer, Start, Sweep);
    case EChartsHitShape::Polygon: return ChartsMath::PointInPolygon(Point, Points);
    case EChartsHitShape::Mesh:
        for (int32 I = 0; I + 2 < Indices.Num(); I += 3)
            if (InTriangle(Point, Points[Indices[I]], Points[Indices[I + 1]], Points[Indices[I + 2]])) return true;
        return false;
    case EChartsHitShape::Series:
        for (const FChartsHitRegion& Region : Regions) if (Region.Contains(Point)) return true;
        return false;
    case EChartsHitShape::Line:
        for (int32 I = 1; I < Points.Num(); ++I)
            if (ChartsMath::SegmentDistanceSquared(Point, Points[I - 1], Points[I]) <= Tolerance * Tolerance) return true;
        return false;
    }
    return false;
}

bool FChartsHitRegion::HitTest(FVector2f Point, FChartsElement& OutElement) const
{
    if (!InRect(Point, Bounds)) return false;
    if (Shape == EChartsHitShape::Series)
    {
        for (int32 I = Regions.Num() - 1; I >= 0; --I) if (Regions[I].HitTest(Point, OutElement)) return true;
        return false;
    }
    if (!Contains(Point)) return false;
    OutElement = Element;
    return true;
}

bool FChartsScene::HitTest(FVector2f Position, FChartsElement& OutElement) const
{
    OutElement = FChartsElement();
    if (!FMath::IsFinite(Position.X) || !FMath::IsFinite(Position.Y) || !InRect(Position, FSlateRect(0, 0, Size.X, Size.Y))) return false;
    for (int32 Index = Hits.Num() - 1; Index >= 0; --Index)
    {
        if (Hits[Index].HitTest(Position, OutElement)) return true;
    }
    return false;
}

void FChartsScene::Text(FVector2f Position, FString Label, int32 FontSize)
{
    FChartsPrimitive& Primitive = Primitives.AddDefaulted_GetRef();
    Primitive.Kind = EChartsPrimitive::Text;
    Primitive.Points.Add(Position);
    Primitive.Text = MoveTemp(Label);
    Primitive.FontSize = FontSize > 0 ? FontSize : Style.FontSize;
    Primitive.Color = FSlateColor(Style.TextColor);
}

void FChartsScene::Line(TArray<FVector2f> Points, FSlateColor Color, float Thickness, const FChartsElement* Element)
{
    if (Points.Num() < 2) return;
    FChartsPrimitive& Primitive = Primitives.AddDefaulted_GetRef();
    Primitive.Kind = EChartsPrimitive::Line;
    Primitive.Points = MoveTemp(Points);
    Primitive.Color = Color;
    Primitive.Thickness = Thickness;
    if (Element)
    {
        Primitive.DataIndex = Element->DataIndex;
        Primitive.ValueIndex = Element->ValueIndex;
        Primitive.Part = Element->Part;
        FChartsHitRegion& Hit = Hits.AddDefaulted_GetRef();
        Hit.Shape = EChartsHitShape::Line;
        Hit.Element = *Element;
        Hit.Points = Primitive.Points;
        Hit.Tolerance = FMath::Max(Style.HitTolerance, Thickness * 0.5f);
        Hit.Bounds = FSlateRect(0, 0, Size.X, Size.Y);
    }
}

void FChartsScene::Mesh(TArray<FVector2f> Points, TArray<uint32> Indices, FSlateColor Color, float Opacity, const FChartsElement* Element)
{
    if (Points.IsEmpty() || Indices.IsEmpty()) return;
    FChartsPrimitive& Primitive = Primitives.AddDefaulted_GetRef();
    Primitive.Points = MoveTemp(Points);
    Primitive.Indices = MoveTemp(Indices);
    Primitive.Color = Color;
    Primitive.Opacity = Opacity;
    if (Element)
    {
        Primitive.DataIndex = Element->DataIndex;
        Primitive.ValueIndex = Element->ValueIndex;
        Primitive.Part = Element->Part;
    }
}

void FChartsScene::Rect(const FSlateRect& Bounds, FSlateColor Color, const FChartsElement* Element)
{
    if (Bounds.Right <= Bounds.Left || Bounds.Bottom <= Bounds.Top) return;
    Mesh({{Bounds.Left, Bounds.Top}, {Bounds.Right, Bounds.Top}, {Bounds.Right, Bounds.Bottom}, {Bounds.Left, Bounds.Bottom}}, {0, 1, 2, 0, 2, 3}, Color, 1.0f, Element);
    if (Element)
    {
        FChartsHitRegion& Hit = Hits.AddDefaulted_GetRef();
        Hit.Element = *Element;
        Hit.Bounds = Bounds;
    }
}

void FChartsScene::Sector(FVector2f Center, float Inner, float Outer, float Start, float Sweep, const FChartsElement& Element)
{
    if (Outer <= Inner || Sweep <= 0.0f) return;
    const int32 Steps = FMath::Clamp(FMath::CeilToInt(Sweep * FMath::Sqrt(FMath::Max(Outer, 1.0f)) * 1.8f), 2, 512);
    TArray<FVector2f> Points;
    TArray<uint32> Indices;
    Points.Reserve(2 * (Steps + 1));
    Indices.Reserve(6 * Steps);
    for (int32 Step = 0; Step <= Steps; ++Step)
    {
        const float Angle = Start + Sweep * static_cast<float>(Step) / Steps;
        Points.Add(RadialPoint(Center, Inner, Angle));
        Points.Add(RadialPoint(Center, Outer, Angle));
        if (Step < Steps)
        {
            const uint32 I = 2 * Step;
            Indices.Append({I, I + 1, I + 3, I, I + 3, I + 2});
        }
    }
    Mesh(MoveTemp(Points), MoveTemp(Indices), Element.Color, 1.0f, &Element);
    FChartsHitRegion& Hit = Hits.AddDefaulted_GetRef();
    Hit.Shape = EChartsHitShape::Sector;
    Hit.Element = Element;
    Hit.Center = Center;
    Hit.Inner = Inner;
    Hit.Outer = Outer;
    Hit.Start = Start;
    Hit.Sweep = Sweep;
    Hit.Bounds = FSlateRect(Center.X - Outer, Center.Y - Outer, Center.X + Outer, Center.Y + Outer);
}

void FChartsScene::Point(FVector2f Position, const FChartsElement& Element, bool bDraw)
{
    if (bDraw)
    {
        TArray<FVector2f> Points{Position};
        TArray<uint32> Indices;
        for (int32 I = 0; I <= 24; ++I)
        {
            Points.Add(RadialPoint(Position, Style.PointRadius, 2.0f * PI * I / 24.0f));
            if (I < 24) Indices.Append({0, static_cast<uint32>(I + 1), static_cast<uint32>(I + 2)});
        }
        Mesh(MoveTemp(Points), MoveTemp(Indices), Element.Color, 1.0f, &Element);
    }
    FChartsHitRegion& Hit = Hits.AddDefaulted_GetRef();
    Hit.Shape = EChartsHitShape::Circle;
    Hit.Element = Element;
    Hit.Center = Position;
    Hit.Outer = FMath::Max(Style.HitTolerance, bDraw ? Style.PointRadius : 0.0f);
    Hit.Bounds = FSlateRect(Position.X - Hit.Outer, Position.Y - Hit.Outer, Position.X + Hit.Outer, Position.Y + Hit.Outer);
}

void FChartsScene::Build(const FChartsModel& Model, const FChartsRenderStyle& InStyle, FVector2f InSize)
{
    Style = ChartsMath::SanitizeStyle(InStyle);
    Size = InSize;
    Primitives.Reset();
    Hits.Reset();
    if (!FMath::IsFinite(Size.X) || !FMath::IsFinite(Size.Y) || Size.X <= 0 || Size.Y <= 0) return;
    Rect(FSlateRect(0, 0, Size.X, Size.Y), FSlateColor(Style.BackgroundColor));
    const float Top = Model.Title.IsEmpty() ? 16.0f : Style.FontSize + 34.0f;
    const float Bottom = Style.bShowLegend ? TextSize(TEXT("Ag"), Style.FontSize).Y + 16.0f + Style.LegendPadding : 16.0f;
    Plot = FSlateRect(18, Top, Size.X - 18, Size.Y - Bottom);
    if (!Model.Title.IsEmpty()) Text({18, 12}, ShortLabel(Model.Title, FMath::FloorToInt(Size.X / (Style.FontSize * 0.7f))), Style.FontSize + 3);
    if (Size.X < 120 || Size.Y < 100) return;
    if (Model.Data.IsEmpty() || Model.NumSeries() == 0)
    {
        Text({18, Top + 24}, TEXT("No chart data"));
        return;
    }
    if (Model.Kind == EChartsKind::Pie || Model.Kind == EChartsKind::PolarArea) Radial(Model);
    else if (Model.Kind == EChartsKind::Radar) Radar(Model);
    else Cartesian(Model);
    if (Style.bShowLegend) Legend(Model);
}

void FChartsScene::Legend(const FChartsModel& Model)
{
    const bool bByRow = Model.Kind == EChartsKind::Pie || Model.Kind == EChartsKind::PolarArea || Model.Kind == EChartsKind::Radar;
    const int32 Count = bByRow ? Model.Data.Num() : Model.NumSeries();
    float X = 18.0f;
    const float Y = Size.Y - TextSize(TEXT("Ag"), Style.FontSize).Y - 16.0f;
    for (int32 I = 0; I < Count; ++I)
    {
        FString Name = bByRow ? Model.Data[I].DataName : (Model.Names.IsValidIndex(I) ? Model.Names[I] : FString::Printf(TEXT("Series %d"), I + 1));
        Name = ShortLabel(Name, 22);
        const float Width = 25.0f + Name.Len() * Style.FontSize * 0.63f;
        if (X + Width > Size.X - 18)
        {
            if (X + 20 < Size.X) Text({X, Y}, TEXT("…"));
            break;
        }
        const FChartsValue* Value = nullptr;
        for (int32 J = 0; J < (bByRow ? Model.Data[I].Values.Num() : Model.Data.Num()); ++J)
        {
            Value = bByRow ? Model.GetValue(I, J) : Model.GetValue(J, I);
            if (Value) break;
        }
        if (Value)
        {
            Rect(FSlateRect(X, Y + 3, X + 9, Y + 12), Value->PrimaryColor);
            if (Model.Kind == EChartsKind::Combo) Line({{X, Y + 8}, {X + 9, Y + 8}}, Value->SecondaryColor, Style.LineThickness);
        }
        Text({X + 15, Y}, Name);
        X += Width;
    }
}

void FChartsScene::Cartesian(const FChartsModel& Model)
{
    const int32 SeriesCount = Model.NumSeries(), RowCount = Model.Data.Num();
    const int32 AxisCount = FMath::Max(1, FMath::Min(Model.Axis.Num(), SeriesCount));
    const float LabelHeight = TextSize(TEXT("Ag"), Style.FontSize).Y, Padding = Style.AxisLabelPadding;
    TArray<FChartsRange> Ranges;
    for (int32 S = 0; S < SeriesCount; ++S) Ranges.Add(ChartsMath::Range(Model, S));
    TArray<float> AxisWidths;
    TArray<FString> AxisNames;
    for (int32 Axis = 0; Axis < AxisCount; ++Axis)
    {
        AxisNames.Add(ShortLabel(Model.Names.IsValidIndex(Axis) ? Model.Names[Axis] : FString::Printf(TEXT("S%d"), Axis + 1), 8));
        float LabelWidth = AxisCount > 1 ? TextSize(AxisNames.Last(), Style.FontSize).X : 0.0f;
        for (int32 Tick = 0; Tick <= Style.GridDivisions; ++Tick)
            LabelWidth = FMath::Max(LabelWidth, TextSize(ChartsMath::Number(Ranges[Axis].At(static_cast<double>(Tick) / Style.GridDivisions)), Style.FontSize).X);
        AxisWidths.Add(FMath::Max(32.0f, LabelWidth) + Padding);
    }
    if (Model.bHorizontal)
    {
        float CategoryWidth = 32.0f;
        for (const FChartsData& Data : Model.Data) CategoryWidth = FMath::Max(CategoryWidth, TextSize(ShortLabel(Data.DataName, 12), Style.FontSize).X);
        if (AxisCount > 1)
            for (const FString& Name : AxisNames) CategoryWidth = FMath::Max(CategoryWidth, TextSize(Name, Style.FontSize).X);
        Plot.Left += CategoryWidth + Padding;
        Plot.Bottom -= AxisCount * (LabelHeight + Padding);
    }
    else
    {
        Plot.Left += AxisWidths[0];
        for (int32 Axis = 1; Axis < AxisCount; ++Axis) Plot.Right -= AxisWidths[Axis];
        Plot.Bottom -= LabelHeight + Padding;
        if (AxisCount > 1) Plot.Top += LabelHeight + Padding;
    }
    if (Plot.Right - Plot.Left < 35 || Plot.Bottom - Plot.Top < 35)
    {
        Text({18, Plot.Top + 20}, TEXT("Increase chart size to display these axes"));
        return;
    }
    const float Width = Plot.Right - Plot.Left, Height = Plot.Bottom - Plot.Top;
    float RightAxisOffset = 0.0f;
    for (int32 Axis = 0; Axis < AxisCount; ++Axis)
    {
        const FChartsRange Range = Ranges[Axis];
        for (int32 Tick = 0; Tick <= Style.GridDivisions; ++Tick)
        {
            const float T = static_cast<float>(Tick) / Style.GridDivisions;
            const float X = Plot.Left + T * Width, Y = Plot.Bottom - T * Height;
            if (Axis == 0 && Style.bShowGrid)
            {
                if (Model.bHorizontal) Line({{X, Plot.Top}, {X, Plot.Bottom}}, FSlateColor(Style.GridColor));
                else Line({{Plot.Left, Y}, {Plot.Right, Y}}, FSlateColor(Style.GridColor));
            }
            const FString Label = ChartsMath::Number(Range.At(T));
            const FVector2f LabelSize = TextSize(Label, Style.FontSize);
            if (Model.bHorizontal) Text({X - LabelSize.X * 0.5f, Plot.Bottom + Padding + Axis * (LabelHeight + Padding)}, Label);
            else Text({Axis == 0 ? Plot.Left - Padding - LabelSize.X : Plot.Right + Padding + RightAxisOffset, Y - LabelSize.Y * 0.5f}, Label);
        }
        if (AxisCount > 1)
        {
            const FString& Name = AxisNames[Axis];
            const float NameWidth = TextSize(Name, Style.FontSize).X;
            if (Model.bHorizontal) Text({Plot.Left - Padding - NameWidth, Plot.Bottom + Padding + Axis * (LabelHeight + Padding)}, Name);
            else Text({Axis == 0 ? Plot.Left - Padding - NameWidth : Plot.Right + Padding + RightAxisOffset, Plot.Top - LabelHeight - Padding}, Name);
        }
        if (Axis > 0) RightAxisOffset += AxisWidths[Axis];
    }
    const float CategoryStep = (Model.bHorizontal ? Height : Width) / RowCount;
    const int32 LabelStride = FMath::Max(1, FMath::CeilToInt((Model.bHorizontal ? Style.FontSize + 8.0f : Style.FontSize * 5.5f) / CategoryStep));
    for (int32 Row = 0; Row < RowCount; Row += LabelStride)
    {
        const FString Label = ShortLabel(Model.Data[Row].DataName, Model.bHorizontal ? 12 : FMath::Max(2, FMath::FloorToInt(CategoryStep * LabelStride / (Style.FontSize * 0.65f))));
        const FVector2f LabelSize = TextSize(Label, Style.FontSize);
        if (Model.bHorizontal) Text({Plot.Left - Padding - LabelSize.X, Plot.Top + (Row + 0.5f) * CategoryStep - LabelSize.Y * 0.5f}, Label);
        else Text({Plot.Left + (Row + 0.5f) * CategoryStep - LabelSize.X * 0.5f, Plot.Bottom + Padding}, Label);
    }
    Line({{Plot.Left, Plot.Top}, {Plot.Left, Plot.Bottom}, {Plot.Right, Plot.Bottom}}, FSlateColor(Style.GridColor), 1.5f);
    const float Band = CategoryStep * (1.0f - Style.BarGapRatio), BarSize = Band / SeriesCount;
    auto BarStart = [CategoryStep, Band, BarSize](int32 Row, int32 Series)
    {
        return Row * CategoryStep + (CategoryStep - Band) * 0.5f + Series * BarSize;
    };
    auto CategoryCenter = [&](int32 Row, int32 Series)
    {
        return Model.HasBars() ? BarStart(Row, Series) + BarSize * 0.47f : (Row + 0.5f) * CategoryStep;
    };
    auto ToLocal = [&Model](FVector2f Position) { return Model.bHorizontal ? FVector2f(Position.Y, Position.X) : Position; };
    for (int32 S = 0; S < SeriesCount; ++S)
    {
        if (!Model.HasBars()) break;
        const FChartsRange Range = Ranges[S];
        for (int32 Row = 0; Row < RowCount; ++Row)
        {
            const FChartsValue* Value = Model.GetValue(Row, S);
            if (!Value) continue;
            const FChartsElement Element = Model.Element(Row, S, EChartsElementPart::Bar);
            const float T = static_cast<float>(Range.Normalize(Value->Value)), Zero = static_cast<float>(Range.Normalize(0.0));
            const float Start = BarStart(Row, S);
            if (Model.bHorizontal)
            {
                const float X0 = Plot.Left + Zero * Width, X1 = Plot.Left + T * Width;
                Rect(FSlateRect(FMath::Min(X0, X1), Plot.Top + Start, FMath::Max(X0, X1), Plot.Top + Start + BarSize * 0.94f), Value->PrimaryColor, &Element);
            }
            else
            {
                const float Y0 = Plot.Bottom - Zero * Height, Y1 = Plot.Bottom - T * Height;
                Rect(FSlateRect(Plot.Left + Start, FMath::Min(Y0, Y1), Plot.Left + Start + BarSize * 0.94f, FMath::Max(Y0, Y1)), Value->PrimaryColor, &Element);
            }
        }
    }
    for (int32 S = 0; S < SeriesCount; ++S)
    {
        if (Model.Kind == EChartsKind::Bar) break;
        const int32 FirstHit = Hits.Num();
        const EChartsElementPart Part = Model.Kind == EChartsKind::Area ? EChartsElementPart::Area : EChartsElementPart::Line;
        const FChartsRange Range = Ranges[S];
        const float Baseline = Plot.Bottom - static_cast<float>(Range.Normalize(0.0)) * Height;
        int32 Row = 0;
        while (Row < RowCount)
        {
            while (Row < RowCount && !Model.GetValue(Row, S)) ++Row;
            const int32 StartRow = Row;
            TArray<FVector2f> Points;
            while (Row < RowCount)
            {
                const FChartsValue* Value = Model.GetValue(Row, S);
                if (!Value) break;
                const float Category = CategoryCenter(Row, S), Normalized = static_cast<float>(Range.Normalize(Value->Value));
                Points.Add(Model.bHorizontal ? FVector2f(Plot.Top + Category, Plot.Left + Normalized * Width)
                    : FVector2f(Plot.Left + Category, Plot.Bottom - Normalized * Height));
                ++Row;
            }
            for (int32 I = 1; I < Points.Num(); ++I)
            {
                TArray<FVector2f> Path = ChartsMath::Curve(Points, I - 1, Model.bSmooth);
                const FChartsElement Left = Model.Element(StartRow + I - 1, S, Part);
                const FChartsElement Right = Model.Element(StartRow + I, S, Part);
                const float MidX = (Points[I - 1].X + Points[I].X) * 0.5f;
                for (int32 P = 1; P < Path.Num(); ++P)
                {
                    const FVector2f A = Path[P - 1], B = Path[P];
                    if (A.X < MidX && B.X > MidX)
                    {
                        Path.Insert(FVector2f(MidX, FMath::Lerp(A.Y, B.Y, (MidX - A.X) / (B.X - A.X))), P);
                        break;
                    }
                }
                for (int32 P = 1; P < Path.Num(); ++P)
                {
                    const FVector2f A = Path[P - 1], B = Path[P];
                    const FChartsElement& Element = (A.X + B.X) * 0.5f <= MidX ? Left : Right;
                    if (Model.bFill && Model.FillOpacity > 0)
                    {
                        TArray<FVector2f> Patch{A, B, {B.X, Baseline}, {A.X, Baseline}};
                        if ((A.Y - Baseline) * (B.Y - Baseline) < 0.0f)
                        {
                            const float CrossX = FMath::Lerp(A.X, B.X, (Baseline - A.Y) / (B.Y - A.Y));
                            Mesh({A, {CrossX, Baseline}, {A.X, Baseline}, B, {B.X, Baseline}}, {0, 1, 2, 1, 3, 4}, Element.Color, Model.FillOpacity, &Element);
                        }
                        else Mesh(Patch, {0, 1, 2, 0, 2, 3}, Element.Color, Model.FillOpacity, &Element);
                        FChartsHitRegion& Hit = Hits.AddDefaulted_GetRef();
                        Hit.Shape = EChartsHitShape::Mesh;
                        Hit.Element = Element;
                        Hit.Points = Primitives.Last().Points;
                        Hit.Indices = Primitives.Last().Indices;
                        Hit.Bounds = Plot;
                    }
                }
                // Split the hit path at its midpoint so events report the closest sample.
                TArray<FVector2f> LeftPath, RightPath;
                for (const FVector2f& Position : Path)
                {
                    if (Position.X <= MidX) LeftPath.Add(Position);
                    if (Position.X >= MidX) RightPath.Add(Position);
                }
                for (FVector2f& Position : LeftPath) Position = ToLocal(Position);
                for (FVector2f& Position : RightPath) Position = ToLocal(Position);
                Line(MoveTemp(LeftPath), Left.Color, Style.LineThickness, &Left);
                Line(MoveTemp(RightPath), Right.Color, Style.LineThickness, &Right);
            }
            for (int32 I = 0; I < Points.Num(); ++I)
                Point(ToLocal(Points[I]), Model.Element(StartRow + I, S, Part), Model.bPoints || Points.Num() == 1);
            if (Row == StartRow) ++Row;
        }
        if (Part == EChartsElementPart::Area && Hits.Num() > FirstHit)
        {
            FChartsHitRegion Series;
            Series.Shape = EChartsHitShape::Series;
            Series.Element = Hits[FirstHit].Element;
            Series.Bounds = FSlateRect(0, 0, Size.X, Size.Y);
            Series.Regions.Reserve(Hits.Num() - FirstHit);
            for (int32 I = FirstHit; I < Hits.Num(); ++I) Series.Regions.Add(MoveTemp(Hits[I]));
            Hits.SetNum(FirstHit, EAllowShrinking::No);
            Hits.Add(MoveTemp(Series));
        }
    }
}

void FChartsScene::Radial(const FChartsModel& Model)
{
    const FVector2f Center((Plot.Left + Plot.Right) * 0.5f, (Plot.Top + Plot.Bottom) * 0.5f);
    const float Radius = FMath::Min(Plot.Right - Plot.Left, Plot.Bottom - Plot.Top) * 0.5f - 10.0f;
    if (Radius <= 1) return;
    const int32 SeriesCount = Model.NumSeries();
    const float Start = FMath::DegreesToRadians(Model.StartAngle);
    if (Model.Kind == EChartsKind::Pie)
    {
        const float Hole = Radius * Model.InnerRadius, RingWidth = (Radius - Hole) / SeriesCount;
        for (int32 S = 0; S < SeriesCount; ++S)
        {
            double Scale = 0.0, Sum = 0.0;
            for (int32 R = 0; R < Model.Data.Num(); ++R) if (const FChartsValue* Value = Model.GetValue(R, S)) Scale = FMath::Max(Scale, Value->Value);
            if (Scale <= 0) continue;
            for (int32 R = 0; R < Model.Data.Num(); ++R) if (const FChartsValue* Value = Model.GetValue(R, S)) Sum += FMath::Max(0.0, Value->Value / Scale);
            double Fraction = 0.0;
            for (int32 R = 0; R < Model.Data.Num(); ++R)
            {
                const FChartsValue* Value = Model.GetValue(R, S);
                if (!Value || Value->Value <= 0) continue;
                const double Slice = (Value->Value / Scale) / Sum;
                const float Inner = Hole + S * RingWidth + (S > 0 ? FMath::Min(2.0f, RingWidth * 0.1f) : 0.0f);
                Sector(Center, Inner, Hole + (S + 1) * RingWidth, Start + static_cast<float>(Fraction * 2.0 * PI), static_cast<float>(Slice * 2.0 * PI), Model.Element(R, S));
                Fraction += Slice;
            }
        }
    }
    else
    {
        double Maximum = Model.MaxValue;
        if (Maximum <= 0)
            for (const FChartsData& Row : Model.Data) for (const FChartsValue& Value : Row.Values)
                if (FMath::IsFinite(Value.Value)) Maximum = FMath::Max(Maximum, Value.Value);
        if (Maximum <= 0) { Text({Plot.Left + 10, Plot.Top + 20}, TEXT("No positive values")); return; }
        if (Style.bShowGrid)
        {
            for (int32 Ring = 1; Ring <= Style.GridDivisions; ++Ring)
            {
                const float T = static_cast<float>(Ring) / Style.GridDivisions;
                TArray<FVector2f> Circle;
                for (int32 I = 0; I <= 96; ++I) Circle.Add(RadialPoint(Center, Radius * FMath::Sqrt(T), 2.0f * PI * I / 96.0f));
                Line(MoveTemp(Circle), FSlateColor(Style.GridColor));
                Text({Center.X + 3, Center.Y - Radius * FMath::Sqrt(T)}, ChartsMath::Number(Maximum * T));
            }
        }
        const float Sweep = 2.0f * PI / (static_cast<float>(Model.Data.Num()) * SeriesCount);
        for (int32 R = 0; R < Model.Data.Num(); ++R)
            for (int32 S = 0; S < SeriesCount; ++S)
                if (const FChartsValue* Value = Model.GetValue(R, S); Value && Value->Value > 0)
                    Sector(Center, 0, Radius * static_cast<float>(FMath::Sqrt(FMath::Min(Value->Value / Maximum, 1.0))), Start + (static_cast<float>(R) * SeriesCount + S) * Sweep, Sweep, Model.Element(R, S));
    }
    if (Hits.IsEmpty()) Text({Plot.Left + 10, Plot.Top + 20}, TEXT("No positive values"));
}

void FChartsScene::Radar(const FChartsModel& Model)
{
    const int32 Count = Model.NumSeries();
    if (Count < 3) { Text({Plot.Left + 10, Plot.Top + 20}, TEXT("Radar needs at least three spokes")); return; }
    const FVector2f Center((Plot.Left + Plot.Right) * 0.5f, (Plot.Top + Plot.Bottom) * 0.5f);
    TArray<FString> Labels;
    FVector2f MaxLabelSize = FVector2f::ZeroVector;
    for (int32 I = 0; I < Count; ++I)
    {
        Labels.Add(Model.Names.IsValidIndex(I) ? ShortLabel(Model.Names[I], 14) : FString::Printf(TEXT("Axis %d"), I + 1));
        const FVector2f LabelSize = TextSize(Labels.Last(), Style.FontSize);
        MaxLabelSize.X = FMath::Max(MaxLabelSize.X, LabelSize.X);
        MaxLabelSize.Y = FMath::Max(MaxLabelSize.Y, LabelSize.Y);
    }
    const float Radius = FMath::Min((Plot.Right - Plot.Left) * 0.5f - MaxLabelSize.X, (Plot.Bottom - Plot.Top) * 0.5f - MaxLabelSize.Y) - Style.AxisLabelPadding;
    if (Radius <= 1) return;
    auto Angle = [Count](int32 Index) { return -0.5f * PI + 2.0f * PI * Index / Count; };
    if (Style.bShowGrid)
    {
        for (int32 Ring = 1; Ring <= Style.GridDivisions; ++Ring)
        {
            TArray<FVector2f> Polygon;
            for (int32 I = 0; I <= Count; ++I) Polygon.Add(RadialPoint(Center, Radius * Ring / Style.GridDivisions, Angle(I)));
            Line(MoveTemp(Polygon), FSlateColor(Style.GridColor));
        }
    }
    for (int32 I = 0; I < Count; ++I)
    {
        const FVector2f Tip = RadialPoint(Center, Radius, Angle(I));
        if (Style.bShowGrid) Line({Center, Tip}, FSlateColor(Style.GridColor));
        const FVector2f Direction(FMath::Cos(Angle(I)), FMath::Sin(Angle(I)));
        const FVector2f LabelSize = TextSize(Labels[I], Style.FontSize);
        FVector2f Label = RadialPoint(Center, Radius + Style.AxisLabelPadding, Angle(I));
        Label.X -= Direction.X < -0.15f ? LabelSize.X : (Direction.X > 0.15f ? 0.0f : LabelSize.X * 0.5f);
        Label.Y -= Direction.Y < -0.15f ? LabelSize.Y : (Direction.Y > 0.15f ? 0.0f : LabelSize.Y * 0.5f);
        Text(Label, Labels[I]);
    }
    const FChartsRange MainRange = ChartsMath::Range(Model, 0);
    if (Model.Axis.Num() <= 1)
        for (int32 Ring = 0; Ring <= Style.GridDivisions; ++Ring)
            Text({Center.X + 5, Center.Y - Radius * Ring / Style.GridDivisions}, ChartsMath::Number(MainRange.At(static_cast<double>(Ring) / Style.GridDivisions)), FMath::Max(8, Style.FontSize - 2));
    for (int32 Row = 0; Row < Model.Data.Num(); ++Row)
    {
        TArray<FVector2f> Points;
        for (int32 I = 0; I < Count; ++I)
        {
            const FChartsValue* Value = Model.GetValue(Row, I);
            if (!Value) break;
            Points.Add(RadialPoint(Center, Radius * static_cast<float>(ChartsMath::Range(Model, I).Normalize(Value->Value)), Angle(I)));
        }
        if (Points.Num() != Count) continue;
        for (int32 I = 0; I < Count; ++I)
        {
            const int32 Next = (I + 1) % Count;
            const FChartsElement A = Model.Element(Row, I), B = Model.Element(Row, Next);
            const FVector2f Mid = (Points[I] + Points[Next]) * 0.5f;
            if (Model.bFill && Model.FillOpacity > 0)
            {
                for (int32 Half = 0; Half < 2; ++Half)
                {
                    const FChartsElement& Element = Half == 0 ? A : B;
                    TArray<FVector2f> Triangle = Half == 0 ? TArray<FVector2f>{Center, Points[I], Mid} : TArray<FVector2f>{Center, Mid, Points[Next]};
                    Mesh(Triangle, {0, 1, 2}, Element.Color, Model.FillOpacity, &Element);
                    FChartsHitRegion& Hit = Hits.AddDefaulted_GetRef();
                    Hit.Shape = EChartsHitShape::Polygon;
                    Hit.Element = Element;
                    Hit.Points = MoveTemp(Triangle);
                    Hit.Bounds = Plot;
                }
            }
            Line({Points[I], Mid}, A.Color, Style.LineThickness, &A);
            Line({Mid, Points[Next]}, B.Color, Style.LineThickness, &B);
        }
        for (int32 I = 0; I < Count; ++I) Point(Points[I], Model.Element(Row, I), Model.bPoints);
    }
    if (Hits.IsEmpty()) Text({Plot.Left + 10, Plot.Top + 20}, TEXT("Each radar shape needs a value for every spoke"));
}

