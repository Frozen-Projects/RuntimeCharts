#include "ChartsScene.h"
#include "ChartsExamples.h"
#include "RuntimeChartsWidgets.h"
#include "SRuntimeChart.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "HAL/FileManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "ImageUtils.h"
#include "InputCoreTypes.h"
#include "RenderingThread.h"
#include "Slate/WidgetRenderer.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include <limits>

namespace
{
    constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

    FChartsData Row(const TCHAR* Name, std::initializer_list<double> Values)
    {
        FChartsData Data;
        Data.DataName = Name;
        for (double Number : Values)
        {
            FChartsValue Value;
            Value.Value = Number;
            Data.Values.Add(Value);
        }
        return Data;
    }

    FVector2f SectorMiddle(const FChartsHitRegion& Hit)
    {
        const float Angle = Hit.Start + Hit.Sweep * 0.5f;
        return Hit.Center + FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)) * ((Hit.Inner + Hit.Outer) * 0.5f);
    }

    TArray<FChartsModel> ExampleModels()
    {
        return {FChartsModel::From(UChartsExampleLibrary::MakeExampleBarConfig()), FChartsModel::From(UChartsExampleLibrary::MakeExampleBarConfig(true)),
            FChartsModel::From(UChartsExampleLibrary::MakeExampleLineConfig()), FChartsModel::From(UChartsExampleLibrary::MakeExampleAreaConfig()),
            FChartsModel::From(UChartsExampleLibrary::MakeExampleComboConfig()), FChartsModel::From(UChartsExampleLibrary::MakeExamplePieConfig()),
            FChartsModel::From(UChartsExampleLibrary::MakeExamplePolarAreaConfig()), FChartsModel::From(UChartsExampleLibrary::MakeExampleRadarConfig())};
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsRangesTest, "RuntimeCharts.Math.Ranges", Flags)
bool FChartsRangesTest::RunTest(const FString& Parameters)
{
    FChartsModel Model;
    Model.Data = {Row(TEXT("A"), {-10, 500}), Row(TEXT("B"), {20, 1000})};
    TestEqual(TEXT("Empty axes share minimum"), ChartsMath::Range(Model, 1).Min, -10.0);
    TestEqual(TEXT("Empty axes share maximum"), ChartsMath::Range(Model, 0).Max, 1000.0);
    Model.Axis = {{-50, 50}};
    TestEqual(TEXT("Single explicit range applies to every series"), ChartsMath::Range(Model, 1).Max, 50.0);
    Model.Axis = {{-20, 30}, {0, 2000}};
    TestEqual(TEXT("First axis maps first value"), ChartsMath::Range(Model, 0).Max, 30.0);
    TestEqual(TEXT("Second axis maps second value"), ChartsMath::Range(Model, 1).Max, 2000.0);
    Model.Data[0].Values.Add(FChartsValue());
    Model.Data[0].Values[2].Value = 17;
    TestEqual(TEXT("Extra Cartesian series uses the first configured range"), ChartsMath::Range(Model, 2).Max, 30.0);
    Model.Kind = EChartsKind::Radar;
    TestEqual(TEXT("Missing radar spoke range remains automatic"), ChartsMath::Range(Model, 2).Max, 17.0);
    Model.Kind = EChartsKind::Bar;
    Model.Axis = {{50, -50}};
    TestEqual(TEXT("Reversed range is ordered"), ChartsMath::Range(Model, 0).Min, -50.0);
    Model.Axis = {{42, 42}};
    TestEqual(TEXT("Equal positive bounds include zero"), ChartsMath::Range(Model, 0).Min, 0.0);
    Model.Axis = {{0, 0}};
    TestEqual(TEXT("Zero range has finite extent"), ChartsMath::Range(Model, 0).Max, 1.0);
    Model.Axis = {{std::numeric_limits<double>::quiet_NaN(), 1}};
    TestEqual(TEXT("Invalid range falls back to auto"), ChartsMath::Range(Model, 0).Max, 1000.0);
    FChartsRange Extreme{-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    TestEqual(TEXT("Extreme mixed signs normalize without overflow"), Extreme.Normalize(0), 0.5);
    TestEqual(TEXT("Extreme midpoint stays finite"), Extreme.At(0.5), 0.0);
    TestEqual(TEXT("Lower values clamp"), FChartsRange{0, 1}.Normalize(-100), 0.0);
    TestEqual(TEXT("Upper values clamp"), FChartsRange{0, 1}.Normalize(100), 1.0);
    Model.Data[0].Values[0].Value = std::numeric_limits<double>::infinity();
    TestNull(TEXT("Non-finite sample is omitted"), Model.GetValue(0, 0));
    TestNull(TEXT("Missing sample is omitted"), Model.GetValue(1, 9));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsAxisCountTest, "RuntimeCharts.Geometry.ConfiguredAxisCount", Flags)
bool FChartsAxisCountTest::RunTest(const FString& Parameters)
{
    FChartsRenderStyle Style;
    Style.bShowLegend = false;
    FChartsModel Model;
    Model.Data = {Row(TEXT("A"), {-5, 50, 15}), Row(TEXT("B"), {10, 100, 20})};
    Model.Names = {TEXT("Scale0"), TEXT("Scale1"), TEXT("Scale2")};
    const TArray<FVector2D> Limits{{0, 40}, {0, 400}, {0, 80}};
    const EChartsKind Kinds[] = {EChartsKind::Bar, EChartsKind::Bar, EChartsKind::Line, EChartsKind::Area, EChartsKind::Combo};
    for (int32 Case = 0; Case < UE_ARRAY_COUNT(Kinds); ++Case)
    {
        Model.Kind = Kinds[Case];
        Model.bHorizontal = Case == 1;
        for (int32 ConfiguredCount = 0; ConfiguredCount <= 3; ++ConfiguredCount)
        {
            Model.Axis.Reset();
            for (int32 I = 0; I < ConfiguredCount; ++I) Model.Axis.Add(Limits[I]);
            FChartsScene Scene;
            Scene.Build(Model, Style, {760, 440});
            int32 NumericLabels = 0, AxisNames = 0;
            for (const FChartsPrimitive& Primitive : Scene.Primitives)
            {
                if (Primitive.Kind != EChartsPrimitive::Text) continue;
                if (Primitive.Text.IsNumeric()) ++NumericLabels;
                if (Model.Names.Contains(Primitive.Text)) ++AxisNames;
            }
            const int32 ExpectedAxes = FMath::Max(1, ConfiguredCount);
            TestEqual(TEXT("Only configured value axes have tick labels"), NumericLabels, ExpectedAxes * (Style.GridDivisions + 1));
            TestEqual(TEXT("Only configured multiple axes have names"), AxisNames, ConfiguredCount > 1 ? ConfiguredCount : 0);
            if (ConfiguredCount != 2) continue;
            TestEqual(TEXT("Third series shares first displayed axis minimum"), ChartsMath::Range(Model, 2).Min, 0.0);
            TestEqual(TEXT("Third series shares first displayed axis maximum"), ChartsMath::Range(Model, 2).Max, 40.0);
            if (Model.Kind == EChartsKind::Bar)
            {
                const FChartsHitRegion* Hit = Scene.Hits.FindByPredicate([](const FChartsHitRegion& Region) { return Region.Element.DataIndex == 1 && Region.Element.ValueIndex == 2; });
                TestNotNull(TEXT("Extra bar series remains visible and interactive"), Hit);
                if (Hit)
                {
                    const float Extent = Model.bHorizontal ? Hit->Bounds.Right - Scene.Plot.Left : Scene.Plot.Bottom - Hit->Bounds.Top;
                    const float HalfPlot = (Model.bHorizontal ? Scene.Plot.Right - Scene.Plot.Left : Scene.Plot.Bottom - Scene.Plot.Top) * 0.5f;
                    TestTrue(TEXT("Extra series bar uses the displayed range"), FMath::IsNearlyEqual(Extent, HalfPlot, 0.01f));
                }
            }
        }
    }
    Model.Kind = EChartsKind::Bar;
    Model.Axis = {{std::numeric_limits<double>::quiet_NaN(), 1}, {0, 400}};
    TestEqual(TEXT("Automatic fallback includes every series assigned to the primary axis"), ChartsMath::Range(Model, 0).Max, 20.0);
    TestEqual(TEXT("Extra series uses the same automatic fallback"), ChartsMath::Range(Model, 2).Max, 20.0);
    TestEqual(TEXT("Automatic fallback preserves assigned negative values"), ChartsMath::Range(Model, 2).Min, -5.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsMasksTest, "RuntimeCharts.Geometry.SectorMasks", Flags)
bool FChartsMasksTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Quarter circle inside"), ChartsMath::InSector({5, 5}, {0, 0}, 0, 10, 0, PI * 0.5f));
    TestFalse(TEXT("Rectangle corner outside circle"), ChartsMath::InSector({9, 9}, {0, 0}, 0, 10, 0, PI * 0.5f));
    TestFalse(TEXT("Wrong angular segment rejected"), ChartsMath::InSector({-5, 5}, {0, 0}, 0, 10, 0, PI * 0.5f));
    TestFalse(TEXT("Donut hole rejected"), ChartsMath::InSector({1, 1}, {0, 0}, 5, 10, 0, PI * 2));
    TestTrue(TEXT("Angle wraps across zero"), ChartsMath::InSector({7, 0}, {0, 0}, 5, 10, -0.2f, 0.4f));
    FChartsPieConfig Pie;
    Pie.Data = {Row(TEXT("A"), {1, 3}), Row(TEXT("B"), {3, 1}), Row(TEXT("Ignored"), {-1, 0})};
    Pie.InnerRadiusRatio = 0.25f;
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Pie), FChartsStyle(), {560, 340});
    TestEqual(TEXT("Two valid slices in each ring"), Scene.Hits.Num(), 4);
    for (const FChartsHitRegion& Hit : Scene.Hits)
    {
        FChartsElement Element;
        TestTrue(TEXT("Each wedge midpoint hits"), Scene.HitTest(SectorMiddle(Hit), Element));
        TestEqual(TEXT("Correct category"), Element.DataIndex, Hit.Element.DataIndex);
        TestEqual(TEXT("Correct ring"), Element.ValueIndex, Hit.Element.ValueIndex);
        TestEqual(TEXT("Correct value"), Element.Value, Hit.Element.Value);
    }
    FChartsElement Element;
    TestFalse(TEXT("Pie center hole is not interactive"), Scene.HitTest(Scene.Hits[0].Center, Element));
    TestFalse(TEXT("Pie widget corner is not interactive"), Scene.HitTest({20, 40}, Element));
    FChartsPolarAreaConfig Polar;
    Polar.Data = {Row(TEXT("A"), {25}), Row(TEXT("B"), {100})};
    Scene.Build(FChartsModel::From(Polar), FChartsStyle(), {560, 340});
    TestEqual(TEXT("Polar equal-angle sectors"), Scene.Hits[0].Sweep, Scene.Hits[1].Sweep);
    TestTrue(TEXT("Polar area encodes value, not radius"), FMath::IsNearlyEqual(Scene.Hits[0].Outer / Scene.Hits[1].Outer, 0.5f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsCurveTest, "RuntimeCharts.Math.MonotoneCurves", Flags)
bool FChartsCurveTest::RunTest(const FString& Parameters)
{
    const TArray<FVector2f> Points{{0, 50}, {80, 0}, {160, 100}, {240, 90}};
    for (int32 I = 0; I < Points.Num() - 1; ++I)
    {
        const TArray<FVector2f> Curve = ChartsMath::Curve(Points, I, true);
        TestTrue(TEXT("Smooth curve has subdivisions"), Curve.Num() > 2);
        TestEqual(TEXT("Curve starts at data point"), Curve[0], Points[I]);
        TestEqual(TEXT("Curve ends at data point"), Curve.Last(), Points[I + 1]);
        for (const FVector2f& Point : Curve)
            TestTrue(TEXT("Curve does not overshoot data interval"), Point.Y >= FMath::Min(Points[I].Y, Points[I + 1].Y) && Point.Y <= FMath::Max(Points[I].Y, Points[I + 1].Y));
        TestEqual(TEXT("Sharp curve has only endpoints"), ChartsMath::Curve(Points, I, false).Num(), 2);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsSceneTest, "RuntimeCharts.Geometry.AllTypes", Flags)
bool FChartsSceneTest::RunTest(const FString& Parameters)
{
    for (const FChartsModel& Model : ExampleModels())
    {
        for (const FVector2f Size : {FVector2f(560, 340), FVector2f(900, 550), FVector2f(280, 220)})
        {
            FChartsScene Scene;
            Scene.Build(Model, FChartsStyle(), Size);
            TestTrue(TEXT("Example creates drawable geometry"), Scene.Primitives.Num() > 2);
            TestTrue(TEXT("Example creates hit regions"), !Scene.Hits.IsEmpty());
            for (const FChartsPrimitive& Primitive : Scene.Primitives)
            {
                for (const FVector2f& Point : Primitive.Points) TestTrue(TEXT("Finite geometry"), FMath::IsFinite(Point.X) && FMath::IsFinite(Point.Y));
                for (uint32 Index : Primitive.Indices) TestTrue(TEXT("Valid mesh index"), Index < static_cast<uint32>(Primitive.Points.Num()));
            }
            FChartsElement Element;
            TestFalse(TEXT("Outside widget never hits"), Scene.HitTest({-1, -1}, Element));
        }
    }
    FChartsModel Empty;
    FChartsScene Scene;
    Scene.Build(Empty, FChartsStyle(), {560, 340});
    TestTrue(TEXT("Empty charts have no hit regions"), Scene.Hits.IsEmpty());
    FChartsBarConfig Bar;
    Bar.Data = {Row(TEXT("Negative"), {-5}), Row(TEXT("Zero"), {0}), Row(TEXT("Positive"), {10})};
    Scene.Build(FChartsModel::From(Bar), FChartsStyle(), {560, 340});
    TestEqual(TEXT("Zero-size bars do not have rectangular click targets"), Scene.Hits.Num(), 2);
    for (const FChartsHitRegion& Hit : Scene.Hits)
    {
        FChartsElement Element;
        TestTrue(TEXT("Both signed bars are selectable"), Scene.HitTest({(Hit.Bounds.Left + Hit.Bounds.Right) * 0.5f, (Hit.Bounds.Top + Hit.Bounds.Bottom) * 0.5f}, Element));
    }
    FChartsLineConfig Gaps;
    Gaps.Data = {Row(TEXT("A"), {5}), Row(TEXT("Missing"), {}), Row(TEXT("B"), {8})};
    Scene.Build(FChartsModel::From(Gaps), FChartsStyle(), {560, 340});
    TestEqual(TEXT("Missing values create disconnected points"), Scene.Hits.Num(), 2);
    FChartsAreaConfig Area;
    Area.Data = {Row(TEXT("Left"), {4}), Row(TEXT("Right"), {8})};
    Scene.Build(FChartsModel::From(Area), FChartsStyle(), {560, 340});
    FChartsElement AreaElement;
    const float RightHalfX = Scene.Plot.Left + (Scene.Plot.Right - Scene.Plot.Left) * 0.625f;
    TestTrue(TEXT("Sharp area interior is interactive"), Scene.HitTest({RightHalfX, Scene.Plot.Bottom - 10}, AreaElement));
    TestEqual(TEXT("Sharp area right half maps to the right sample"), AreaElement.DataIndex, 1);
    FChartsRadarConfig Radar;
    Radar.Data = {Row(TEXT("A"), {10, 10, 10}), Row(TEXT("B"), {5, 5, 5})};
    Radar.Axis = {{0, 10}};
    Radar.bShowDataPoints = false;
    Scene.Build(FChartsModel::From(Radar), FChartsStyle(), {560, 340});
    FChartsElement Element;
    const FVector2f Center((Scene.Plot.Left + Scene.Plot.Right) / 2, (Scene.Plot.Top + Scene.Plot.Bottom) / 2);
    TestFalse(TEXT("Outline radar interior does not hit"), Scene.HitTest(Center, Element));
    Radar.bIsFillInside = true;
    Scene.Build(FChartsModel::From(Radar), FChartsStyle(), {560, 340});
    TestTrue(TEXT("Filled radar interior hits"), Scene.HitTest(Center + FVector2f(0, -10), Element));
    TestEqual(TEXT("Topmost radar shape wins"), Element.DataIndex, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsInputTest, "RuntimeCharts.Input.MouseAndTouch", Flags)
bool FChartsInputTest::RunTest(const FString& Parameters)
{
    FChartsPieConfig Config;
    Config.Data = {Row(TEXT("A"), {1}), Row(TEXT("B"), {1})};
    Config.InnerRadiusRatio = 0.3f;
    FChartsRenderStyle Style;
    Style.bShowTooltips = false;
    int32 HoverCount = 0, ClickCount = 0, LeaveCount = 0;
    FChartsElement LastElement;
    TSharedRef<SRuntimeChart> Chart = SNew(SRuntimeChart)
        .OnHovered(FOnRuntimeChartElement::CreateLambda([&](const FChartsElement& Element) { ++HoverCount; LastElement = Element; }))
        .OnClicked(FOnRuntimeChartElement::CreateLambda([&](const FChartsElement& Element) { ++ClickCount; LastElement = Element; }))
        .OnHoverEnded(FSimpleDelegate::CreateLambda([&]() { ++LeaveCount; }));
    Chart->SetModel(FChartsModel::From(Config), Style);
    FChartsScene Scene;
    Scene.Build(FChartsModel::From(Config), Style, {560, 340});
    const FGeometry Geometry = FGeometry::MakeRoot(FVector2f(560, 340), FSlateLayoutTransform(1.5f, FVector2f(40, 60)));
    auto EventAt = [&](FVector2f Local)
    {
        const FVector2D Absolute = Geometry.LocalToAbsolute(FVector2D(Local));
        return FPointerEvent(0, Absolute, Absolute, TSet<FKey>{EKeys::LeftMouseButton}, EKeys::LeftMouseButton, 0, FModifierKeysState());
    };
    const FPointerEvent A = EventAt(SectorMiddle(Scene.Hits[0]));
    const FPointerEvent B = EventAt(SectorMiddle(Scene.Hits[1]));
    const FPointerEvent Hole = EventAt(Scene.Hits[0].Center);
    TestTrue(TEXT("DPI transformed hover handled"), Chart->OnMouseMove(Geometry, A).IsEventHandled());
    Chart->OnMouseMove(Geometry, A);
    TestEqual(TEXT("Hover emitted only on element change"), HoverCount, 1);
    TestEqual(TEXT("Hover contains data name"), LastElement.DataName, FString(TEXT("A")));
    TestEqual(TEXT("Hover contains value"), LastElement.Value, 1.0);
    TestEqual(TEXT("Hover contains supplied color"), LastElement.Color.GetSpecifiedColor(), Config.Data[0].Values[0].PrimaryColor.GetSpecifiedColor());
    Chart->OnMouseButtonDown(Geometry, A);
    Chart->OnMouseButtonUp(Geometry, A);
    TestEqual(TEXT("Same element press and release clicks"), ClickCount, 1);
    Chart->OnMouseButtonDown(Geometry, A);
    Chart->OnMouseButtonUp(Geometry, B);
    TestEqual(TEXT("Dragging across slices does not click"), ClickCount, 1);
    TestFalse(TEXT("Donut hole does not consume mouse down"), Chart->OnMouseButtonDown(Geometry, Hole).IsEventHandled());
    Chart->OnMouseMove(Geometry, Hole);
    TestEqual(TEXT("Leaving slice emits hover ended"), LeaveCount, 1);
    Chart->OnTouchStarted(Geometry, B);
    Chart->OnTouchEnded(Geometry, B);
    TestEqual(TEXT("Touch tap clicks"), ClickCount, 2);
    TestEqual(TEXT("Touch reports the tapped slice"), LastElement.DataName, FString(TEXT("B")));
    Chart->OnMouseButtonDown(Geometry, A);
    Chart->SetModel(FChartsModel::From(Config), Style);
    Chart->OnMouseButtonUp(Geometry, A);
    TestEqual(TEXT("Updating data cancels a pending click"), ClickCount, 2);
    Chart->SetEnabled(false);
    TestFalse(TEXT("Disabled chart ignores mouse down"), Chart->OnMouseButtonDown(Geometry, A).IsEventHandled());
    Chart->SetEnabled(true);
    Chart->OnMouseMove(Geometry, A);
    Chart->OnMouseLeave(A);
    FChartsElement OutElement;
    TestFalse(TEXT("Mouse leave clears hover"), Chart->GetHovered(OutElement));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsReflectionTest, "RuntimeCharts.Blueprint.ReflectionAndLifecycle", Flags)
bool FChartsReflectionTest::RunTest(const FString& Parameters)
{
    const TArray<UClass*> Classes{UChartsBarWidget::StaticClass(), UChartsLineWidget::StaticClass(), UChartsAreaWidget::StaticClass(),
        UChartsComboWidget::StaticClass(), UChartsPieWidget::StaticClass(), UChartsPolarAreaWidget::StaticClass(), UChartsRadarWidget::StaticClass()};
    for (UClass* Class : Classes)
    {
        TStrongObjectPtr<UChartWidget> Widget(NewObject<UChartWidget>(GetTransientPackage(), Class));
        TestNotNull(TEXT("Config reflected"), Class->FindPropertyByName(TEXT("Config")));
        UFunction* Setter = Class->FindFunctionByName(TEXT("SetConfig"));
        TestNotNull(TEXT("SetConfig reflected"), Setter);
        TestTrue(TEXT("SetConfig Blueprint callable"), Setter && Setter->HasAnyFunctionFlags(FUNC_BlueprintCallable));
        const FProperty* Hover = Class->FindPropertyByName(TEXT("OnElementHovered"));
        const FProperty* Click = Class->FindPropertyByName(TEXT("OnElementClicked"));
        TestTrue(TEXT("Hover Blueprint assignable"), Hover && Hover->HasAnyPropertyFlags(CPF_BlueprintAssignable));
        TestTrue(TEXT("Click Blueprint assignable"), Click && Click->HasAnyPropertyFlags(CPF_BlueprintAssignable));
        TestTrue(TEXT("Widget constructs Slate"), Widget->TakeWidget()->GetTypeAsString().Len() > 0);
        Widget->SynchronizeProperties();
        Widget->ReleaseSlateResources(true);
        TestTrue(TEXT("Widget rebuilds after release"), Widget->TakeWidget()->GetTypeAsString().Len() > 0);
    }
    TStrongObjectPtr<UChartsBarWidget> Bar(NewObject<UChartsBarWidget>());
    Bar->SetConfig(UChartsExampleLibrary::MakeExampleBarConfig());
    TestEqual(TEXT("Config setter updates data"), Bar->Config.Data.Num(), 6);
    FChartsBarStyle Style;
    Style.DesiredSize = {700, 420};
    Bar->SetChartStyle(Style);
    Bar->TakeWidget()->SlatePrepass();
    TestEqual(TEXT("Style setter changes desired size"), FVector2f(Bar->TakeWidget()->GetDesiredSize()), FVector2f(700, 420));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsRenderTest, "RuntimeCharts.Rendering.UMGGallery", Flags)
bool FChartsRenderTest::RunTest(const FString& Parameters)
{
    if (!FApp::CanEverRender()) { AddError(TEXT("This test needs a rendering RHI; do not use -NullRHI.")); return false; }
    TArray<TStrongObjectPtr<UChartWidget>> Widgets;
    auto Add = [&](UChartWidget* Widget) { Widgets.Emplace(Widget); };
    UChartsBarWidget* Bar = NewObject<UChartsBarWidget>();
    Bar->SetConfig(UChartsExampleLibrary::MakeExampleBarConfig()); Add(Bar);
    UChartsBarWidget* Horizontal = NewObject<UChartsBarWidget>();
    Horizontal->SetConfig(UChartsExampleLibrary::MakeExampleBarConfig(true)); Add(Horizontal);
    UChartsLineWidget* Line = NewObject<UChartsLineWidget>();
    Line->SetConfig(UChartsExampleLibrary::MakeExampleLineConfig()); Add(Line);
    UChartsAreaWidget* Area = NewObject<UChartsAreaWidget>();
    Area->SetConfig(UChartsExampleLibrary::MakeExampleAreaConfig()); Add(Area);
    UChartsComboWidget* Combo = NewObject<UChartsComboWidget>();
    Combo->SetConfig(UChartsExampleLibrary::MakeExampleComboConfig()); Add(Combo);
    UChartsPieWidget* Pie = NewObject<UChartsPieWidget>();
    Pie->SetConfig(UChartsExampleLibrary::MakeExamplePieConfig()); Add(Pie);
    UChartsPolarAreaWidget* Polar = NewObject<UChartsPolarAreaWidget>();
    Polar->SetConfig(UChartsExampleLibrary::MakeExamplePolarAreaConfig()); Add(Polar);
    UChartsRadarWidget* Radar = NewObject<UChartsRadarWidget>();
    Radar->SetConfig(UChartsExampleLibrary::MakeExampleRadarConfig()); Add(Radar);
    TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(6));
    for (int32 I = 0; I < Widgets.Num(); ++I)
        Grid->AddSlot(I % 2, I / 2)[SNew(SBox).WidthOverride(560).HeightOverride(340)[Widgets[I]->TakeWidget()]];
    const int32 Width = 1144, Height = 1408;
    FWidgetRenderer* Renderer = new FWidgetRenderer(false);
    TStrongObjectPtr<UTextureRenderTarget2D> Target(NewObject<UTextureRenderTarget2D>());
    Target->ClearColor = FLinearColor::Transparent;
    Target->TargetGamma = 1.0f;
    Target->InitCustomFormat(Width, Height, PF_FloatRGBA, true);
    Target->UpdateResourceImmediate(true);
    Renderer->DrawWidget(Target.Get(), Grid, FVector2D(Width, Height), 0.0f);
    FlushRenderingCommands();
    TArray<FColor> Pixels;
    TArray<FLinearColor> LinearPixels;
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
    ReadFlags.SetLinearToGamma(false);
    TestTrue(TEXT("Rendered pixels read successfully"), Target->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(LinearPixels, ReadFlags));
    Pixels.Reserve(LinearPixels.Num());
    for (const FLinearColor& Pixel : LinearPixels) Pixels.Add(Pixel.ToFColor(true));
    TestEqual(TEXT("Expected render dimensions"), Pixels.Num(), Width * Height);
    if (Pixels.Num() == Width * Height)
    {
        const FColor Background = Pixels[20 * Width + 555];
        const FColor Expected = FChartsStyle().BackgroundColor.ToFColor(true);
        TestTrue(TEXT("Screenshot preserves the intended background color"), FMath::Abs(int32(Background.R) - Expected.R) < 5 && FMath::Abs(int32(Background.B) - Expected.B) < 5);
        for (int32 Tile = 0; Tile < 8; ++Tile)
        {
            int32 Colored = 0;
            for (int32 Y = (Tile / 2) * 352 + 45; Y < (Tile / 2) * 352 + 295; ++Y)
                for (int32 X = (Tile % 2) * 572 + 20; X < (Tile % 2) * 572 + 540; ++X)
                {
                    const FColor Pixel = Pixels[Y * Width + X];
                    if ((Pixel.B > Pixel.R + 45 && Pixel.B > 140) || (Pixel.G > Pixel.R + 45 && Pixel.G > 140)) ++Colored;
                }
            TestTrue(*FString::Printf(TEXT("Chart %d contains rendered data pixels"), Tile), Colored > 100);
        }
        FString Output = FPaths::ProjectSavedDir() / TEXT("RuntimeCharts");
        FParse::Value(FCommandLine::Get(), TEXT("ChartsOutput="), Output);
        IFileManager::Get().MakeDirectory(*Output, true);
        const FString Filename = Output / TEXT("RuntimeCharts-Gallery.png");
        TestTrue(TEXT("Native gallery screenshot saved"), FImageUtils::SaveImageByExtension(*Filename, FImageView(Pixels.GetData(), Width, Height)));
        AddInfo(FString::Printf(TEXT("Native UMG render: %s"), *Filename));
    }
    const TArray<FChartsModel> Models = ExampleModels();
    for (int32 I = 0; I < Widgets.Num(); ++I)
    {
        FChartsScene Scene;
        Scene.Build(Models[I], FChartsStyle(), {560, 340});
        const FChartsHitRegion& LastHit = Scene.Hits.Last();
        const FChartsHitRegion& Hit = LastHit.Shape == EChartsHitShape::Series ? LastHit.Regions.Last() : LastHit;
        FVector2f Position;
        if (Hit.Shape == EChartsHitShape::Sector) Position = SectorMiddle(Hit);
        else if (Hit.Shape == EChartsHitShape::Circle) Position = Hit.Center;
        else Position = {(Hit.Bounds.Left + Hit.Bounds.Right) / 2, (Hit.Bounds.Top + Hit.Bounds.Bottom) / 2};
        FChartsElement Element;
        TestTrue(TEXT("UMG local hit query uses its actual allotted size"), Widgets[I]->GetElementAtLocalPosition(FVector2D(Position), Element));
    }
    if (UWorld* World = GWorld)
    {
        TStrongObjectPtr<UChartsDemoWidget> Demo(CreateWidget<UChartsDemoWidget>(World));
        TestNotNull(TEXT("Demo can be created with CreateWidget"), Demo.Get());
        if (Demo.IsValid())
        {
            TStrongObjectPtr<UTextureRenderTarget2D> DemoTarget(Renderer->DrawWidget(Demo->TakeWidget(), FVector2D(1160, 850)));
            FlushRenderingCommands();
            TestNotNull(TEXT("Demo widget tree renders"), DemoTarget.Get());
        }
    }
    else AddError(TEXT("No world available for the demo creation test."));
    BeginCleanup(Renderer);
    FlushRenderingCommands();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChartsComboStyleRenderTest, "RuntimeCharts.Rendering.ComboStyleUpdates", Flags)
bool FChartsComboStyleRenderTest::RunTest(const FString& Parameters)
{
    if (!FApp::CanEverRender()) { AddError(TEXT("This test needs a rendering RHI; do not use -NullRHI.")); return false; }
    TStrongObjectPtr<UChartsComboWidget> Combo(NewObject<UChartsComboWidget>());
    FChartsComboConfig Config = UChartsExampleLibrary::MakeExampleComboConfig();
    Config.bIsSmooth = false;
    Config.bShowDataPoints = true;
    for (FChartsData& Row : Config.Data)
        for (int32 I = 0; I < Row.Values.Num(); ++I)
            Row.Values[I].SecondaryColor = FSlateColor(I == 0 ? FLinearColor::Yellow : FLinearColor::Red);
    Combo->SetConfig(Config);
    FChartsComboStyle Style;
    Style.bShowLegend = false;
    Style.LineThickness = 1.0f;
    Style.PointRadius = 2.0f;
    Combo->SetChartStyle(Style);
    FWidgetRenderer* Renderer = new FWidgetRenderer(false);
    TStrongObjectPtr<UTextureRenderTarget2D> Target(NewObject<UTextureRenderTarget2D>());
    Target->ClearColor = FLinearColor::Transparent;
    Target->TargetGamma = 1.0f;
    Target->InitCustomFormat(560, 340, PF_FloatRGBA, true);
    Target->UpdateResourceImmediate(true);
    auto DrawAndCount = [&]()
    {
        Renderer->DrawWidget(Target.Get(), Combo->TakeWidget(), FVector2D(560, 340), 0.0f);
        FlushRenderingCommands();
        TArray<FLinearColor> Pixels;
        FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
        ReadFlags.SetLinearToGamma(false);
        TestTrue(TEXT("Updated combo render can be read"), Target->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(Pixels, ReadFlags));
        int32 Count = 0;
        for (const FLinearColor& Pixel : Pixels)
        {
            const FColor Color = Pixel.ToFColor(true);
            if (Color.R > 220 && Color.B < 40 && (Color.G > 220 || Color.G < 40)) ++Count;
        }
        return Count;
    };
    const int32 ThinPixels = DrawAndCount();
    Style.LineThickness = 8.0f;
    Combo->SetChartStyle(Style);
    const int32 ThickPixels = DrawAndCount();
    TestTrue(TEXT("Runtime LineThickness update visibly increases line coverage"), ThinPixels > 0 && ThickPixels > ThinPixels * 1.5f);
    Style.PointRadius = 10.0f;
    Combo->SetChartStyle(Style);
    const int32 LargePointPixels = DrawAndCount();
    TestTrue(TEXT("Runtime PointRadius update visibly increases marker coverage"), LargePointPixels > ThickPixels * 1.1f);
    AddInfo(FString::Printf(TEXT("Secondary-color pixels: thin=%d, thick=%d, large points=%d"), ThinPixels, ThickPixels, LargePointPixels));
    BeginCleanup(Renderer);
    FlushRenderingCommands();
    return true;
}

#endif

