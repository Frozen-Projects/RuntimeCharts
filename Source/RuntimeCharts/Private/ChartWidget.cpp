#include "ChartWidget.h"
#include "SRuntimeChart.h"

UChartWidget::UChartWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SetVisibility(ESlateVisibility::Visible);
}

void UChartWidget::RefreshChart()
{
    if (!MyChart.IsValid()) return;
    FChartsModel Model;
    FChartsRenderStyle Style;
    BuildModel(Model);
    BuildStyle(Style);
    MyChart->SetModel(MoveTemp(Model), Style);
}

void UChartWidget::BuildModel(FChartsModel& OutModel) const { OutModel = FChartsModel(); }
void UChartWidget::BuildStyle(FChartsRenderStyle& OutStyle) const { OutStyle = FChartsRenderStyle(); }

TSharedRef<SWidget> UChartWidget::RebuildWidget()
{
    SAssignNew(MyChart, SRuntimeChart)
        .OnHovered(FOnRuntimeChartElement::CreateUObject(this, &UChartWidget::HandleHover))
        .OnClicked(FOnRuntimeChartElement::CreateUObject(this, &UChartWidget::HandleClick))
        .OnHoverEnded(FSimpleDelegate::CreateUObject(this, &UChartWidget::HandleHoverEnded));
    RefreshChart();
    return MyChart.ToSharedRef();
}

void UChartWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();
    RefreshChart();
}

void UChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    MyChart.Reset();
}

bool UChartWidget::GetElementAtLocalPosition(FVector2D LocalPosition, FChartsElement& OutElement) const
{
    OutElement = FChartsElement();
    return MyChart.IsValid() && MyChart->HitTest(FVector2f(LocalPosition), OutElement);
}

bool UChartWidget::GetHoveredElement(FChartsElement& OutElement) const
{
    OutElement = FChartsElement();
    return MyChart.IsValid() && MyChart->GetHovered(OutElement);
}

void UChartWidget::HandleHover(const FChartsElement& Element)
{
    OnElementHovered.Broadcast(Element.DataName, Element.Value, Element.Color);
    OnElementHoveredDetailed.Broadcast(Element);
}

void UChartWidget::HandleClick(const FChartsElement& Element)
{
    OnElementClicked.Broadcast(Element.DataName, Element.Value, Element.Color);
    OnElementClickedDetailed.Broadcast(Element);
}

void UChartWidget::HandleHoverEnded() { OnElementHoverEnded.Broadcast(); }

#if WITH_EDITOR
const FText UChartWidget::GetPaletteCategory() { return NSLOCTEXT("RuntimeCharts", "Palette", "Runtime Charts"); }
#endif
