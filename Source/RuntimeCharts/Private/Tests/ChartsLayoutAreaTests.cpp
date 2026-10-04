#include "ChartsScene.h"
#include "SRuntimeChart.h"
#include "RuntimeChartsWidgets.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Rendering/SlateRenderer.h"
#include "RenderingThread.h"
#include "Slate/WidgetRenderer.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include <limits>

namespace
{
    constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

    FChartsAreaConfig IrregularArea(bool bSmooth = false)
    {
        FChartsAreaConfig Config;
        Config.Axis = {{0, 100}};
        Config.SeriesNames = {TEXT("Output"), TEXT("Baseline")};
        Config.bIsSmooth = bSmooth;
        for (double Value : {90.0, 20.0, 80.0, 15.0, 70.0})
        {
            FChartsData Data;
            Data.DataName = FString::Printf(TEXT("Category%d"), Config.Data.Num());
            FChartsValue Sample;
            Sample.Value = Value;
            Sample.PrimaryColor = FSlateColor(FLinearColor::Blue);
            Data.Values.Add(Sample);
            Config.Data.Add(Data);
        }
        return Config;
    }

    FVector2f SamplePosition(const FChartsScene& Scene, double Category, double Value, double Minimum = 0, double Maximum = 100)
    {
        return {Scene.Plot.Left + (Scene.Plot.Right - Scene.Plot.Left) * float((Category + 0.5) / 5),
            Scene.Plot.Bottom - (Scene.Plot.Bottom - Scene.Plot.Top) * float((Value - Minimum) / (Maximum - Minimum))};
    }

    const FChartsPrimitive* FindText(const FChartsScene& Scene, const FString& Label)
    {
        return Scene.Primitives.FindByPredicate([&](const FChartsPrimitive& Primitive) { return Primitive.Kind == EChartsPrimitive::Text && Primitive.Text == Label; });
    }

    FVector2f MeasureText(const FString& Text, int32 FontSize)
    {
        return FVector2f(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Text, FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize)));
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsPaddingTest, "RuntimeCharts.Geometry.LabelPadding", TestFlags)
bool FChartsPaddingTest::RunTest(const FString& Parameters)
{
    const TArray<UScriptStruct*> AxisStyles{FChartsBarStyle::StaticStruct(), FChartsLineStyle::StaticStruct(), FChartsAreaStyle::StaticStruct(), FChartsComboStyle::StaticStruct(), FChartsRadarStyle::StaticStruct()};
    for (UScriptStruct* Struct : AxisStyles) TestNotNull(TEXT("Axis styles expose label padding"), Struct->FindPropertyByName(TEXT("AxisLabelPadding")));
    TestNull(TEXT("Pie has no irrelevant axis padding"), FChartsPieStyle::StaticStruct()->FindPropertyByName(TEXT("AxisLabelPadding")));
    TestNull(TEXT("Polar area has no external axis names"), FChartsPolarAreaStyle::StaticStruct()->FindPropertyByName(TEXT("AxisLabelPadding")));
    TestNotNull(TEXT("Legend padding is common to every chart style"), FChartsStyle::StaticStruct()->FindPropertyByName(TEXT("LegendPadding")));
    for (bool bHorizontal : {false, true})
    for (int32 AxisCount : {1, 2, 3})
    for (float Padding : {0.0f, 28.0f})
    {
        FChartsModel Model = FChartsModel::From(IrregularArea());
        Model.Kind = EChartsKind::Bar;
        Model.bHorizontal = bHorizontal;
        Model.Names = {TEXT("Output"), TEXT("Rate"), TEXT("Power")};
        Model.Axis.SetNum(AxisCount);
        for (FVector2D& Range : Model.Axis) Range = {0, 100};
        for (FChartsData& Data : Model.Data)
            while (Data.Values.Num() < AxisCount) Data.Values.Add(FChartsValue(Data.Values[0]));
        FChartsBarStyle Style;
        Style.AxisLabelPadding = Padding;
        Style.LegendPadding = 35.0f;
        FChartsScene Scene;
        Scene.Build(Model, Style, {1000, 600});
        const FChartsPrimitive* Category = FindText(Scene, TEXT("Category0"));
        if (!TestNotNull(TEXT("Category label exists"), Category)) continue;
        const FVector2f LabelSize = MeasureText(Category->Text, Style.FontSize);
        const float ActualGap = bHorizontal ? Scene.Plot.Left - Category->Points[0].X - LabelSize.X : Category->Points[0].Y - Scene.Plot.Bottom;
        TestTrue(TEXT("Measured category-label gap matches requested padding"), FMath::IsNearlyEqual(ActualGap, Padding, 0.01f));
        const FChartsPrimitive* FirstTick = FindText(Scene, TEXT("0"));
        if (TestNotNull(TEXT("Value-axis label exists"), FirstTick))
        {
            const float TickGap = bHorizontal ? FirstTick->Points[0].Y - Scene.Plot.Bottom
                : Scene.Plot.Left - FirstTick->Points[0].X - MeasureText(FirstTick->Text, Style.FontSize).X;
            TestTrue(TEXT("Value-axis gap matches requested padding"), FMath::IsNearlyEqual(TickGap, Padding, 0.01f));
        }
        const FChartsPrimitive& Legend = Scene.Primitives.Last();
        const float AxisBlockBottom = Scene.Plot.Bottom + (bHorizontal ? AxisCount : 1) * (LabelSize.Y + Padding);
        TestTrue(TEXT("Legend is separated from the complete axis label block"), FMath::IsNearlyEqual(Legend.Points[0].Y - AxisBlockBottom, Style.LegendPadding, 0.01f));
        Style.bShowLegend = false;
        FChartsScene WithoutLegend, HiddenPadding;
        WithoutLegend.Build(Model, Style, {1000, 600});
        Style.LegendPadding = 150;
        HiddenPadding.Build(Model, Style, {1000, 600});
        TestEqual(TEXT("Hidden legend reserves no padding"), WithoutLegend.Plot.Bottom, HiddenPadding.Plot.Bottom);
    }
    FChartsRadarConfig Radar;
    Radar.AxisLabels = {TEXT("North"), TEXT("East"), TEXT("South"), TEXT("West")};
    Radar.Axis = {{0, 100}};
    FChartsData Shape;
    for (int32 I = 0; I < 4; ++I) { FChartsValue Value; Value.Value = 100; Shape.Values.Add(Value); }
    Radar.Data.Add(Shape);
    for (float Padding : {0.0f, 30.0f})
    {
        FChartsRadarStyle Style;
        Style.AxisLabelPadding = Padding;
        FChartsScene Scene;
        Scene.Build(FChartsModel::From(Radar), Style, {700, 500});
        const FChartsPrimitive* North = FindText(Scene, TEXT("North"));
        const FChartsHitRegion* NorthPoint = Scene.Hits.FindByPredicate([](const FChartsHitRegion& Hit) { return Hit.Shape == EChartsHitShape::Circle && Hit.Element.ValueIndex == 0; });
        if (TestNotNull(TEXT("Radar axis name exists"), North) && TestNotNull(TEXT("Radar tip exists"), NorthPoint))
            TestTrue(TEXT("Radar axis name is separated from its spoke tip"), FMath::IsNearlyEqual(NorthPoint->Center.Y - North->Points[0].Y - MeasureText(North->Text, Style.FontSize).Y, Padding, 0.01f));
    }
    FChartsRenderStyle Invalid;
    Invalid.AxisLabelPadding = std::numeric_limits<float>::quiet_NaN();
    Invalid.LegendPadding = -10;
    TestEqual(TEXT("Non-finite axis padding uses default"), ChartsMath::SanitizeStyle(Invalid).AxisLabelPadding, 8.0f);
    TestEqual(TEXT("Negative legend padding is clamped"), ChartsMath::SanitizeStyle(Invalid).LegendPadding, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsAreaMaskTest, "RuntimeCharts.Geometry.AreaSeriesMasks", TestFlags)
bool FChartsAreaMaskTest::RunTest(const FString& Parameters)
{
    FChartsAreaStyle Style;
    Style.bShowLegend = false;
    Style.HitTolerance = 2;
    for (bool bSmooth : {false, true})
    {
        FChartsAreaConfig Config = IrregularArea(bSmooth);
        FChartsScene Scene;
        Scene.Build(FChartsModel::From(Config), Style, {800, 500});
        TestEqual(TEXT("One compound hit target per area series"), Scene.Hits.Num(), 1);
        TestTrue(TEXT("Area target is a series"), Scene.Hits[0].Shape == EChartsHitShape::Series);
        FChartsElement First, Last, Outside;
        TestTrue(TEXT("Left lobe interior hits"), Scene.HitTest(SamplePosition(Scene, 0, 50), First));
        TestTrue(TEXT("Right lobe interior hits"), Scene.HitTest(SamplePosition(Scene, 4, 40), Last));
        TestTrue(TEXT("Distant lobes have the same hover identity"), First.IsSameTarget(Last));
        TestTrue(TEXT("Area target reports its part"), First.Part == EChartsElementPart::Area);
        TestEqual(TEXT("Area exposes the series name"), First.SeriesName, FString(TEXT("Output")));
        TestEqual(TEXT("Sample context still follows the pointer"), Last.DataIndex, 4);
        TestFalse(TEXT("Concave notch inside bounds is not part of the shape"), Scene.HitTest(SamplePosition(Scene, 1, 60), Outside));
        for (const FChartsHitRegion& Region : Scene.Hits[0].Regions)
        {
            if (Region.Shape != EChartsHitShape::Mesh) continue;
            for (int32 I = 0; I < Region.Indices.Num(); I += 3)
            {
                const FVector2f Center = (Region.Points[Region.Indices[I]] + Region.Points[Region.Indices[I + 1]] + Region.Points[Region.Indices[I + 2]]) / 3.0f;
                FChartsElement Element;
                TestTrue(TEXT("Rendered fill triangle interior belongs to the series"), Scene.HitTest(Center, Element));
                TestTrue(TEXT("Every fill patch shares the same target"), Element.IsSameTarget(First));
            }
        }
        for (FChartsData& Data : Config.Data) { FChartsValue Value; Value.Value = 10; Data.Values.Add(Value); }
        Scene.Build(FChartsModel::From(Config), Style, {800, 500});
        TestEqual(TEXT("Overlaid data creates two series targets"), Scene.Hits.Num(), 2);
        FChartsElement Element;
        TestTrue(TEXT("Overlap is interactive"), Scene.HitTest(SamplePosition(Scene, 2, 5), Element));
        TestEqual(TEXT("Last painted series wins in overlap"), Element.ValueIndex, 1);
        TestTrue(TEXT("Exposed underlying area stays interactive"), Scene.HitTest(SamplePosition(Scene, 2, 40), Element));
        TestEqual(TEXT("Exposed area identifies underlying series"), Element.ValueIndex, 0);
    }
    FChartsAreaConfig Config = IrregularArea();
    Config.Data[2].Values.Reset();
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Config), Style, {800, 500});
    FChartsElement Element;
    TestEqual(TEXT("Disconnected runs retain a single series identity"), Scene.Hits.Num(), 1);
    TestFalse(TEXT("Missing samples do not fill or hit the gap"), Scene.HitTest(SamplePosition(Scene, 2, 5), Element));
    Config = IrregularArea();
    Config.Axis = {{-100, 100}};
    Config.Data[1].Values[0].Value = -60;
    Scene.Build(FChartsModel::From(Config), Style, {800, 500});
    TestTrue(TEXT("Negative lobe interior hits"), Scene.HitTest(SamplePosition(Scene, 1, -30, -100, 100), Element));
    TestFalse(TEXT("No false wedge beyond baseline crossing"), Scene.HitTest(SamplePosition(Scene, 0.5, -30, -100, 100), Element));
    Config.FillOpacity = 0;
    Scene.Build(FChartsModel::From(Config), Style, {800, 500});
    TestFalse(TEXT("Invisible fill has no interior hit target"), Scene.HitTest(SamplePosition(Scene, 1, -30, -100, 100), Element));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsAreaHoverTest, "RuntimeCharts.Input.AreaSeriesHover", TestFlags)
bool FChartsAreaHoverTest::RunTest(const FString& Parameters)
{
    FChartsAreaConfig Config = IrregularArea(true);
    for (FChartsData& Data : Config.Data) { FChartsValue Value; Value.Value = 10; Data.Values.Add(Value); }
    FChartsAreaStyle Style;
    Style.bShowTooltips = false;
    int32 Hovers = 0, Leaves = 0, Clicks = 0;
    FChartsElement Last;
    TSharedRef<SRuntimeChart> Chart = SNew(SRuntimeChart)
        .OnHovered(FOnRuntimeChartElement::CreateLambda([&](const FChartsElement& Element) { ++Hovers; Last = Element; }))
        .OnClicked(FOnRuntimeChartElement::CreateLambda([&](const FChartsElement& Element) { ++Clicks; Last = Element; }))
        .OnHoverEnded(FSimpleDelegate::CreateLambda([&]() { ++Leaves; }));
    Chart->SetModel(FChartsModel::From(Config), Style);
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Config), Style, {800, 500});
    const FGeometry Geometry = FGeometry::MakeRoot(FVector2f(800, 500), FSlateLayoutTransform(1.5f, FVector2f(40, 60)));
    auto EventAt = [&](double Row, double Value)
    {
        const FVector2D Absolute = Geometry.LocalToAbsolute(FVector2D(SamplePosition(Scene, Row, Value)));
        return FPointerEvent(0, Absolute, Absolute, TSet<FKey>{EKeys::LeftMouseButton}, EKeys::LeftMouseButton, 0, FModifierKeysState());
    };
    const FPointerEvent Left = EventAt(0, 50), Right = EventAt(4, 40), Other = EventAt(2, 5), Notch = EventAt(1, 60);
    Chart->OnMouseMove(Geometry, Left);
    Chart->OnMouseMove(Geometry, Right);
    TestEqual(TEXT("Moving between area samples does not emit another enter"), Hovers, 1);
    TestEqual(TEXT("Moving within the shape does not emit leave"), Leaves, 0);
    TestTrue(TEXT("Area remains hovered"), Chart->GetHovered(Last));
    TestEqual(TEXT("Hover snapshot tracks latest sample"), Last.DataIndex, 4);
    Chart->OnMouseButtonDown(Geometry, Left);
    Chart->OnMouseButtonUp(Geometry, Right);
    TestEqual(TEXT("Press and release on different parts of one area clicks"), Clicks, 1);
    Chart->OnMouseButtonDown(Geometry, Left);
    Chart->OnMouseButtonUp(Geometry, Other);
    TestEqual(TEXT("Different series does not complete a click"), Clicks, 1);
    Chart->OnMouseMove(Geometry, Other);
    TestEqual(TEXT("Entering another area emits a new hover"), Hovers, 2);
    TestEqual(TEXT("Hover identifies the other series"), Last.ValueIndex, 1);
    Chart->OnMouseMove(Geometry, Notch);
    TestEqual(TEXT("Leaving the irregular shape emits leave"), Leaves, 1);
    TestFalse(TEXT("Notch clears hover"), Chart->GetHovered(Last));
    Chart->OnMouseMove(Geometry, Left);
    TestEqual(TEXT("Reentering the shape emits hover"), Hovers, 3);
    Chart->SetModel(FChartsModel::From(Config), Style);
    TestFalse(TEXT("Changing data clears series hover"), Chart->GetHovered(Last));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsAreaPaddingRenderTest, "RuntimeCharts.Rendering.AreaHoverAndPadding", TestFlags)
bool FChartsAreaPaddingRenderTest::RunTest(const FString& Parameters)
{
    if (!FApp::CanEverRender()) { AddError(TEXT("This test needs a rendering RHI; do not use -NullRHI.")); return false; }
    TStrongObjectPtr<UChartsAreaWidget> Normal(NewObject<UChartsAreaWidget>()), Hovered(NewObject<UChartsAreaWidget>());
    TStrongObjectPtr<UChartsBarWidget> Compact(NewObject<UChartsBarWidget>()), Padded(NewObject<UChartsBarWidget>());
    FChartsAreaConfig Config = IrregularArea();
    Config.FillOpacity = 1;
    Config.bShowDataPoints = true;
    for (FChartsData& Data : Config.Data) { FChartsValue Value; Value.Value = 10; Value.PrimaryColor = FSlateColor(FLinearColor::Green); Data.Values.Add(Value); }
    Config.ChartName = TEXT("Area: before hover");
    Normal->SetConfig(Config);
    Config.ChartName = TEXT("Area: entire Output series hovered");
    Hovered->SetConfig(Config);
    FChartsAreaStyle AreaStyle;
    AreaStyle.bShowTooltips = false;
    Hovered->SetChartStyle(AreaStyle);
    FChartsBarConfig Bars;
    Bars.Data = Config.Data;
    Bars.Axis = Config.Axis;
    Bars.SeriesNames = Config.SeriesNames;
    Bars.ChartName = TEXT("Bar: axis padding 8 / legend padding 12");
    Compact->SetConfig(Bars);
    Bars.ChartName = TEXT("Bar: axis padding 32 / legend padding 40");
    Padded->SetConfig(Bars);
    const TArray<UChartWidget*> Widgets{Normal.Get(), Hovered.Get(), Compact.Get(), Padded.Get()};
    TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(6));
    for (int32 I = 0; I < Widgets.Num(); ++I)
        Grid->AddSlot(I % 2, I / 2)[SNew(SBox).WidthOverride(560).HeightOverride(340)[Widgets[I]->TakeWidget()]];
    constexpr int32 Width = 1144, Height = 704;
    FWidgetRenderer* Renderer = new FWidgetRenderer(false);
    TStrongObjectPtr<UTextureRenderTarget2D> Target(NewObject<UTextureRenderTarget2D>());
    Target->ClearColor = FLinearColor::Transparent;
    Target->TargetGamma = 1.0f;
    Target->InitCustomFormat(Width, Height, PF_FloatRGBA, true);
    Target->UpdateResourceImmediate(true);
    auto Capture = [&]()
    {
        Renderer->DrawWidget(Target.Get(), Grid, FVector2D(Width, Height), 0.0f);
        FlushRenderingCommands();
        TArray<FLinearColor> Pixels;
        FReadSurfaceDataFlags Flags(RCM_UNorm);
        Flags.SetLinearToGamma(false);
        TestTrue(TEXT("Native preview pixels read"), Target->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(Pixels, Flags));
        return Pixels;
    };
    const TArray<FLinearColor> Before = Capture();
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Config), AreaStyle, {560, 340});
    const FGeometry Geometry = Hovered->TakeWidget()->GetCachedGeometry();
    const FVector2f Left = SamplePosition(Scene, 0.15, 50), Right = SamplePosition(Scene, 3.85, 40);
    const FVector2D Absolute = Geometry.LocalToAbsolute(FVector2D(Left));
    const FPointerEvent Pointer(0, Absolute, Absolute, TSet<FKey>(), EKeys::Invalid, 0, FModifierKeysState());
    TestTrue(TEXT("Native UMG area accepts interior hover"), Hovered->TakeWidget()->OnMouseMove(Geometry, Pointer).IsEventHandled());
    FChartsBarStyle BarStyle;
    BarStyle.AxisLabelPadding = 32;
    BarStyle.LegendPadding = 40;
    Padded->SetChartStyle(BarStyle);
    const TArray<FLinearColor> After = Capture();
    auto PixelIndex = [](FVector2f Local) { return (FMath::RoundToInt(Local.Y) + 6) * Width + FMath::RoundToInt(Local.X) + 578; };
    if (Before.Num() == Width * Height && After.Num() == Width * Height)
    {
        for (const FVector2f Position : {Left, Right})
        {
            const int32 Index = PixelIndex(Position);
            TestTrue(TEXT("Hover highlights both distant lobes of the same series"), After[Index].R > Before[Index].R + 0.1f);
        }
        for (const FVector2f Position : {SamplePosition(Scene, 1, 60), SamplePosition(Scene, 2, 5)})
        {
            const int32 Index = PixelIndex(Position);
            TestTrue(TEXT("Hover leaves concave gaps and another opaque series unchanged"), Before[Index].Equals(After[Index], 0.001f));
        }
        FChartsScene PaddedScene;
        PaddedScene.Build(FChartsModel::From(Bars), BarStyle, {560, 340});
        const FSlateRect& Bounds = PaddedScene.Hits[0].Bounds;
        FChartsElement Element;
        TestTrue(TEXT("Runtime padding updates actual UMG hit geometry"), Padded->GetElementAtLocalPosition(FVector2D((Bounds.Left + Bounds.Right) * 0.5f, (Bounds.Top + Bounds.Bottom) * 0.5f), Element));
        TestEqual(TEXT("Padded geometry still selects the matching series"), Element.ValueIndex, 0);
        TArray<FColor> Pixels;
        Pixels.Reserve(After.Num());
        for (const FLinearColor& Pixel : After) Pixels.Add(Pixel.ToFColor(true));
        FString Output = FPaths::ProjectSavedDir() / TEXT("RuntimeCharts");
        FParse::Value(FCommandLine::Get(), TEXT("ChartsOutput="), Output);
        IFileManager::Get().MakeDirectory(*Output, true);
        TestTrue(TEXT("Native hover and padding preview saved"), FImageUtils::SaveImageByExtension(*(Output / TEXT("RuntimeCharts-AreaHoverAndPadding.png")), FImageView(Pixels.GetData(), Width, Height)));
    }
    else AddError(TEXT("Unexpected render target dimensions."));
    BeginCleanup(Renderer);
    FlushRenderingCommands();
    return true;
}

#endif
