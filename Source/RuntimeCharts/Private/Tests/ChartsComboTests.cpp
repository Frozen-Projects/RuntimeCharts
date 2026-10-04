#include "ChartsScene.h"
#include "RuntimeChartsWidgets.h"
#include "SRuntimeChart.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "InputCoreTypes.h"
#include "Misc/AutomationTest.h"
#include "UObject/CoreRedirects.h"
#include "UObject/UnrealType.h"

namespace
{
    constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

    FChartsComboConfig ComboData()
    {
        FChartsComboConfig Config;
        Config.Axis = {{-20, 100}, {-200, 1000}};
        Config.bShowDataPoints = true;
        for (int32 Row = 0; Row < 3; ++Row)
        {
            FChartsData Data;
            Data.DataName = FString::Printf(TEXT("Category %d"), Row);
            for (int32 Series = 0; Series < 2; ++Series)
            {
                FChartsValue Value;
                Value.Value = (Row == 0 ? -10.0 : 30.0 * Row) * (Series == 0 ? 1.0 : 10.0);
                Value.PrimaryColor = FSlateColor(Series == 0 ? FLinearColor::Blue : FLinearColor::Green);
                Value.SecondaryColor = FSlateColor(Series == 0 ? FLinearColor::Yellow : FLinearColor::Red);
                Data.Values.Add(Value);
            }
            Config.Data.Add(Data);
        }
        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsTypedStylesTest, "RuntimeCharts.Blueprint.TypedStylesAndColorRedirect", TestFlags)
bool FChartsTypedStylesTest::RunTest(const FString& Parameters)
{
    const TArray<UClass*> Classes{UChartsBarWidget::StaticClass(), UChartsLineWidget::StaticClass(), UChartsAreaWidget::StaticClass(),
        UChartsComboWidget::StaticClass(), UChartsPieWidget::StaticClass(), UChartsPolarAreaWidget::StaticClass(), UChartsRadarWidget::StaticClass()};
    const TArray<UScriptStruct*> Styles{FChartsBarStyle::StaticStruct(), FChartsLineStyle::StaticStruct(), FChartsAreaStyle::StaticStruct(),
        FChartsComboStyle::StaticStruct(), FChartsPieStyle::StaticStruct(), FChartsPolarAreaStyle::StaticStruct(), FChartsRadarStyle::StaticStruct()};
    for (int32 I = 0; I < Classes.Num(); ++I)
    {
        const FStructProperty* Property = FindFProperty<FStructProperty>(Classes[I], TEXT("ChartStyle"));
        TestTrue(TEXT("Widget exposes its own style type"), Property && Property->Struct == Styles[I]);
        const UFunction* Setter = Classes[I]->FindFunctionByName(TEXT("SetChartStyle"));
        const FStructProperty* Parameter = Setter ? FindFProperty<FStructProperty>(Setter, TEXT("InStyle")) : nullptr;
        TestTrue(TEXT("Blueprint style setter accepts the matching type"), Parameter && Parameter->Struct == Styles[I]);
        const bool bLines = I == 1 || I == 2 || I == 3 || I == 6;
        for (const TCHAR* Name : {TEXT("LineThickness"), TEXT("PointRadius"), TEXT("HitTolerance")})
            TestEqual(TEXT("Line controls exist only on charts with data lines"), Styles[I]->FindPropertyByName(Name) != nullptr, bLines);
        TestEqual(TEXT("Bar spacing exists only on bar and combo"), Styles[I]->FindPropertyByName(TEXT("BarGapRatio")) != nullptr, I == 0 || I == 3);
        for (const TCHAR* Name : {TEXT("GridColor"), TEXT("GridDivisions"), TEXT("bShowGrid")})
            TestEqual(TEXT("Pie has no irrelevant grid controls"), Styles[I]->FindPropertyByName(Name) != nullptr, I != 4);
        TestNotNull(TEXT("Every chart retains common styling"), Styles[I]->FindPropertyByName(TEXT("DesiredSize")));
    }
    TestNull(TEXT("Old value color is no longer exposed"), FChartsValue::StaticStruct()->FindPropertyByName(TEXT("Color")));
    TestNotNull(TEXT("Primary color is reflected"), FChartsValue::StaticStruct()->FindPropertyByName(TEXT("PrimaryColor")));
    TestNotNull(TEXT("Secondary color is reflected"), FChartsValue::StaticStruct()->FindPropertyByName(TEXT("SecondaryColor")));
    TestNull(TEXT("Combo no longer partitions values into bar or line series"), FChartsComboConfig::StaticStruct()->FindPropertyByName(TEXT("SeriesTypes")));
    const FCoreRedirectObjectName OldName(FString(TEXT("/Script/RuntimeCharts.ChartsValue.Color")));
    const FCoreRedirectObjectName NewName(FString(TEXT("/Script/RuntimeCharts.ChartsValue.PrimaryColor")));
    TestTrue(TEXT("Saved Color property redirects to PrimaryColor"), FCoreRedirects::GetRedirectedName(ECoreRedirectFlags::Type_Property, OldName) == NewName);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsComboGeometryTest, "RuntimeCharts.Geometry.ComboBarsLinesAndStyles", TestFlags)
bool FChartsComboGeometryTest::RunTest(const FString& Parameters)
{
    for (bool bHorizontal : {false, true})
    for (bool bSmooth : {false, true})
    for (float Gap : {0.1f, 0.6f})
    for (float Thickness : {1.0f, 8.0f})
    {
        FChartsComboConfig Config = ComboData();
        Config.bIsHorizontal = bHorizontal;
        Config.bIsSmooth = bSmooth;
        FChartsComboStyle Style;
        Style.bShowLegend = false;
        Style.BarGapRatio = Gap;
        Style.LineThickness = Thickness;
        Style.PointRadius = Thickness + 2.0f;
        FChartsBarStyle BarStyle;
        BarStyle.bShowLegend = false;
        BarStyle.BarGapRatio = Gap;
        FChartsScene Combo, Bars;
        Combo.Build(FChartsModel::From(Config), Style, {800, 500});
        Bars.Build(FChartsModel::From(static_cast<const FChartsBarConfig&>(Config)), BarStyle, {800, 500});
        int32 BarCount = 0, PointCount = 0, LineCount = 0;
        bool bSawLine = false;
        for (const FChartsPrimitive& Primitive : Combo.Primitives)
        {
            if (Primitive.DataIndex == INDEX_NONE) continue;
            const FChartsValue& Value = Config.Data[Primitive.DataIndex].Values[Primitive.ValueIndex];
            if (Primitive.Part == EChartsElementPart::Bar)
            {
                ++BarCount;
                TestFalse(TEXT("All bars are painted before all lines"), bSawLine);
                TestEqual(TEXT("Bar uses PrimaryColor"), Primitive.Color.GetSpecifiedColor(), Value.PrimaryColor.GetSpecifiedColor());
                const FChartsPrimitive* Bar = Bars.Primitives.FindByPredicate([&](const FChartsPrimitive& Item)
                {
                    return Item.Part == EChartsElementPart::Bar && Item.DataIndex == Primitive.DataIndex && Item.ValueIndex == Primitive.ValueIndex;
                });
                TestTrue(TEXT("Combo preserves standalone grouped bar geometry"), Bar && Bar->Points == Primitive.Points);
                continue;
            }
            bSawLine = true;
            TestEqual(TEXT("Line and marker use SecondaryColor"), Primitive.Color.GetSpecifiedColor(), Value.SecondaryColor.GetSpecifiedColor());
            if (Primitive.Kind == EChartsPrimitive::Line)
            {
                ++LineCount;
                TestEqual(TEXT("Line thickness reaches actual geometry"), Primitive.Thickness, Thickness);
                TestEqual(TEXT("Smooth flag controls curve subdivision"), Primitive.Points.Num() > 3, bSmooth);
                const FChartsHitRegion* Marker = Combo.Hits.FindByPredicate([&](const FChartsHitRegion& Hit)
                {
                    return Hit.Shape == EChartsHitShape::Circle && Hit.Element.DataIndex == Primitive.DataIndex && Hit.Element.ValueIndex == Primitive.ValueIndex;
                });
                TestTrue(TEXT("Each half-line meets its own corresponding bar marker"), Marker &&
                    (Primitive.Points[0].Equals(Marker->Center, 0.01f) || Primitive.Points.Last().Equals(Marker->Center, 0.01f)));
            }
            else
            {
                ++PointCount;
                const FChartsHitRegion* Bar = Bars.Hits.FindByPredicate([&](const FChartsHitRegion& Hit)
                {
                    return Hit.Element.DataIndex == Primitive.DataIndex && Hit.Element.ValueIndex == Primitive.ValueIndex;
                });
                if (!TestNotNull(TEXT("Every marker has a matching bar"), Bar)) continue;
                const FVector2f Expected = bHorizontal
                    ? FVector2f(Value.Value < 0 ? Bar->Bounds.Left : Bar->Bounds.Right, (Bar->Bounds.Top + Bar->Bounds.Bottom) * 0.5f)
                    : FVector2f((Bar->Bounds.Left + Bar->Bounds.Right) * 0.5f, Value.Value < 0 ? Bar->Bounds.Bottom : Bar->Bounds.Top);
                TestTrue(TEXT("Point lands at the center of its own signed bar tip"), Primitive.Points[0].Equals(Expected, 0.01f));
                TestTrue(TEXT("Point radius reaches actual marker mesh"), FMath::IsNearlyEqual((Primitive.Points[1] - Primitive.Points[0]).Size(), Style.PointRadius, 0.01f));
            }
        }
        TestEqual(TEXT("Two values per category create six bars"), BarCount, 6);
        TestEqual(TEXT("Each bar has its own point"), PointCount, 6);
        TestEqual(TEXT("Two matching lines span all three categories"), LineCount, 8);
    }
    FChartsComboConfig Gaps = ComboData();
    Gaps.Data[1].Values.SetNum(1);
    Gaps.bShowDataPoints = false;
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Gaps), FChartsComboStyle(), {800, 500});
    int32 DisconnectedPoints = 0;
    for (const FChartsPrimitive& Primitive : Scene.Primitives)
    {
        if (Primitive.ValueIndex != 1 || Primitive.Part != EChartsElementPart::Line) continue;
        TestTrue(TEXT("Missing samples leave disconnected markers, never a bridge"), Primitive.Kind == EChartsPrimitive::Mesh);
        ++DisconnectedPoints;
    }
    TestEqual(TEXT("Both isolated combo samples remain visible"), DisconnectedPoints, 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsComboInputTest, "RuntimeCharts.Input.ComboPartColors", TestFlags)
bool FChartsComboInputTest::RunTest(const FString& Parameters)
{
    FChartsComboConfig Config = ComboData();
    FChartsComboStyle Style;
    Style.bShowTooltips = false;
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Config), Style, {800, 500});
    const FChartsHitRegion* Bar = Scene.Hits.FindByPredicate([](const FChartsHitRegion& Hit)
    {
        return Hit.Element.DataIndex == 2 && Hit.Element.ValueIndex == 0 && Hit.Shape == EChartsHitShape::Rect;
    });
    const FChartsHitRegion* Point = Scene.Hits.FindByPredicate([](const FChartsHitRegion& Hit)
    {
        return Hit.Element.DataIndex == 2 && Hit.Element.ValueIndex == 0 && Hit.Shape == EChartsHitShape::Circle;
    });
    if (!TestNotNull(TEXT("Combo bar hit region exists"), Bar) || !TestNotNull(TEXT("Combo marker hit region exists"), Point)) return false;
    int32 Hovers = 0, Clicks = 0;
    FChartsElement Last;
    TSharedRef<SRuntimeChart> Chart = SNew(SRuntimeChart)
        .OnHovered(FOnRuntimeChartElement::CreateLambda([&](const FChartsElement& Element) { ++Hovers; Last = Element; }))
        .OnClicked(FOnRuntimeChartElement::CreateLambda([&](const FChartsElement& Element) { ++Clicks; Last = Element; }));
    Chart->SetModel(FChartsModel::From(Config), Style);
    const FGeometry Geometry = FGeometry::MakeRoot(FVector2f(800, 500), FSlateLayoutTransform(1.5f, FVector2f(40, 60)));
    auto EventAt = [&](FVector2f Local)
    {
        const FVector2D Absolute = Geometry.LocalToAbsolute(FVector2D(Local));
        return FPointerEvent(0, Absolute, Absolute, TSet<FKey>{EKeys::LeftMouseButton}, EKeys::LeftMouseButton, 0, FModifierKeysState());
    };
    const FPointerEvent BarEvent = EventAt({(Bar->Bounds.Left + Bar->Bounds.Right) * 0.5f, (Bar->Bounds.Top + Bar->Bounds.Bottom) * 0.5f});
    const FPointerEvent LineEvent = EventAt(Point->Center);
    Chart->OnMouseMove(Geometry, BarEvent);
    TestEqual(TEXT("Bar hover reports primary color"), Last.Color.GetSpecifiedColor(), Config.Data[2].Values[0].PrimaryColor.GetSpecifiedColor());
    TestTrue(TEXT("Bar hover identifies the part"), Last.Part == EChartsElementPart::Bar);
    Chart->OnMouseMove(Geometry, LineEvent);
    TestEqual(TEXT("Moving to line at same data index emits a new hover"), Hovers, 2);
    TestTrue(TEXT("Line hover identifies the part"), Last.Part == EChartsElementPart::Line);
    TestEqual(TEXT("Line hover reports secondary color"), Last.Color.GetSpecifiedColor(), Config.Data[2].Values[0].SecondaryColor.GetSpecifiedColor());
    TestEqual(TEXT("Detailed event retains primary color"), Last.PrimaryColor.GetSpecifiedColor(), Config.Data[2].Values[0].PrimaryColor.GetSpecifiedColor());
    TestEqual(TEXT("Detailed event retains secondary color"), Last.SecondaryColor.GetSpecifiedColor(), Config.Data[2].Values[0].SecondaryColor.GetSpecifiedColor());
    Chart->OnMouseButtonDown(Geometry, BarEvent);
    Chart->OnMouseButtonUp(Geometry, LineEvent);
    TestEqual(TEXT("Pressing bar and releasing line is not a click"), Clicks, 0);
    Chart->OnMouseButtonDown(Geometry, LineEvent);
    Chart->OnMouseButtonUp(Geometry, LineEvent);
    TestEqual(TEXT("Line click emitted"), Clicks, 1);
    TestEqual(TEXT("Line click reports secondary color"), Last.Color.GetSpecifiedColor(), Config.Data[2].Values[0].SecondaryColor.GetSpecifiedColor());
    Chart->OnMouseButtonDown(Geometry, BarEvent);
    Chart->OnMouseButtonUp(Geometry, BarEvent);
    TestEqual(TEXT("Bar click emitted"), Clicks, 2);
    TestEqual(TEXT("Bar click reports primary color"), Last.Color.GetSpecifiedColor(), Config.Data[2].Values[0].PrimaryColor.GetSpecifiedColor());
    return true;
}

#endif
