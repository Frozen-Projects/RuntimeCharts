# Runtime Charts — UE 5.8.3 / C++20

A native UMG chart plugin. All geometry, text, and input use Unreal's Slate renderer. No web browser, JavaScript, third-party chart library, custom material, or asset import is required.

**v2.1.0:** configurable axis-label and legend padding, plus whole-series area hover targets that follow the rendered irregular shape. Retains grouped combo bars, primary/secondary colors, chart-specific style structs, and the configured-axis count fix.

## Install

1. Close your Unreal project.
2. Copy the `RuntimeCharts` folder into `<YourProject>/Plugins/RuntimeCharts`.
3. Regenerate project files and build your project's Editor target.
4. Enable **Runtime Charts** in the Plugins window if necessary.
5. In a Widget Blueprint, find the widgets under **Runtime Charts** in the Palette.

This is a source plugin. The module explicitly selects C++20. For C++ consumers, add `"RuntimeCharts"` to your module dependencies and include `RuntimeChartsWidgets.h`.

The descriptor uses Unreal's `5.8.0` compatibility version so UE 5.8 hotfix installations load it without a version prompt. Development and validation target the installed UE **5.8.3** build.

## First chart in Blueprint

1. Drag **Bar Chart** into a Widget Blueprint. Give it space using a Size Box or a filling layout slot.
2. Select the chart and edit **Charts → Config** in Details, or call **Set Config** at runtime.
3. For a working starting point, use **Make Example Bar Config** from **Charts → Examples** and pass its result to **Set Config**.
4. Bind **On Element Hovered** and **On Element Clicked**. Both provide `DataName`, `Value` (double), and `Color` (`FSlateColor`).

For a complete gallery, use **Create Widget → Charts Demo Widget → Add to Viewport**. Set the player controller's input mode to UI Only or Game and UI and enable the mouse cursor. The gallery contains all seven chart types plus horizontal bars and reports hover/click values at the top. It uses two 560 × 340 columns; allow about 1150 Slate units of width, with vertical scrolling for smaller heights.

Example configuration functions exist for every chart type. The widgets are also usable in Widget Components and Retainer Boxes through the usual UMG rendering path; these integrations have not been device-tested.

## Data layout

`FChartsValue` contains `double Value`, `FSlateColor PrimaryColor`, and `FSlateColor SecondaryColor`. All chart types use `PrimaryColor`; combo lines and their point markers use `SecondaryColor`.

`FChartsData` contains a `FString DataName` and `TArray<FChartsValue> Values`.

| Widget / config | Meaning of `Data` | Meaning of `Values[index]` |
| --- | --- | --- |
| `UChartsBarWidget` / `FChartsBarConfig` | Categories along the category axis | Grouped series |
| `UChartsLineWidget` / `FChartsLineConfig` | Categories from left to right | One sample per line series |
| `UChartsAreaWidget` / `FChartsAreaConfig` | Categories from left to right | One sample per filled series |
| `UChartsComboWidget` / `FChartsComboConfig` | Categories, like bar charts | A grouped bar and its matching line sample |
| `UChartsPieWidget` / `FChartsPieConfig` | Slice categories | Concentric series rings, innermost first |
| `UChartsPolarAreaWidget` / `FChartsPolarAreaConfig` | Sector categories | Equal-angle subsectors within each category |
| `UChartsRadarWidget` / `FChartsRadarConfig` | Separate overlaid shapes | Spokes around the shape |

For example, `Jan.Values = [10, 100]` and `Feb.Values = [20, 200]` create two series with samples `[10, 20]` and `[100, 200]`. The X axis is categorical and evenly spaced. Values retain double precision until normalized into local drawing coordinates.

To color each series consistently, use the same colors for that series index across categories. Each individual value's colors are honored. Segment colors change at the midpoint between neighboring category positions. Legends use the first available color and optional `SeriesNames`; for radial charts they use data names. Combo legends show both the bar color and the line color.

## Axis rules

For bar, line, area, and combo charts:

- `Axis = []`: one shared automatic range across all finite values. Zero is included.
- `Axis = [(Min, Max)]`: one shared explicit range.
- `Axis = [(Min0, Max0), (Min1, Max1), ...]`: independent ranges for the corresponding `Values` index.
- If multiple axes are supplied but a series index has no range, that series shares the first axis. Extra series never create extra value axes. Excess ranges without a data series are ignored.
- Reversed bounds are sorted. Equal nonzero bounds expand toward zero; `(0, 0)` becomes `(0, 1)`. Non-finite bounds fall back to automatic calculation.
- Values beyond an explicit range are clamped to its boundary for drawing. Events still return the original value.

All active Cartesian axes are labeled. Vertical charts place the first axis on the left and additional axes on the right; horizontal bars place value axes below the plot. Independent axes require additional space. When the axes cannot fit, the widget asks for a larger layout rather than producing invalid geometry.

Set `bIsHorizontal` on the bar or combo config for horizontal bars. Standalone line and area charts run left to right.

## Chart-specific controls

**Line:** `bShowDataPoints` draws circles. `bIsSmooth` uses a monotone cubic Hermite curve, bounded by adjacent samples so it cannot introduce overshoot. False draws straight segments. A lone finite sample draws a point even when point markers are disabled, so it remains visible.

**Area:** inherits the line settings and adds `FillOpacity`. Each series fills between its curve and its own zero baseline, clamped to the configured range. Negative areas and baseline crossings are supported. Series are overlaid, not stacked.

**Combo:** inherits the bar settings, including `bIsHorizontal`, and adds `bShowDataPoints` and `bIsSmooth`. Every `Values[index]` creates a grouped bar and a sample on that index's line. For example, two values in each of three categories produce six bars and two lines. Each line passes through the centers of its corresponding bar tips, using the same value and axis range. All bars are painted before the lines. Bars use `PrimaryColor`; lines and points use `SecondaryColor`. Negative values, missing samples, independent axes, and horizontal orientation are supported. `FChartsComboStyle.LineThickness` controls the lines, and `PointRadius` controls visible point markers; enable `bShowDataPoints` to display all markers.

**Pie:** each positive value receives its proportional angle within its series ring. One series produces a pie; additional series produce independently normalized concentric rings. `InnerRadiusRatio` creates a donut hole and `StartAngleDegrees` rotates the chart clockwise in screen coordinates. Zero, negative, missing, and non-finite values produce no slice. Ring gaps are not interactive.

**Polar area:** equal-angle sectors use `radius = chart radius × sqrt(value / maximum)`, so sector **area** is proportional to value. `MaxValue = 0` chooses the largest positive value. An explicit maximum clamps larger values visually. Multiple series subdivide each category's angle. Missing, zero, and negative values leave empty sectors.

**Radar:** each `Data` item produces another shape, in array order. At least three spokes are required. Set `bIsFillInside` for a translucent interior; otherwise only the perimeter and optional points are drawn and interactive. `AxisLabels` names spokes. Optional `Axis` ranges can be shared or supplied per spoke; a missing range in a per-spoke list is calculated automatically for that spoke. With automatic/shared ranges, numeric ring labels are shown. An incomplete shape (missing/non-finite spoke values) is skipped instead of inventing values. Later shapes have input priority where they overlap.

## Input and dispatchers

The primary dispatchers are:

- `OnElementHovered(DataName, Value, Color)` — when the pointer enters a different element.
- `OnElementClicked(DataName, Value, Color)` — left-button press and release on the same element, or a touch tap.

Additional dispatchers are `OnElementHoveredDetailed`, `OnElementClickedDetailed`, and `OnElementHoverEnded`. Detailed events pass `FChartsElement`, which includes the original zero-based `DataIndex` and `ValueIndex`, `SeriesName`, both input colors, and `Part` (`Data`, `Bar`, `Line`, or `Area`). Event `Color` is the color of the interacted part: a combo bar reports `PrimaryColor`, while its line or marker reports `SecondaryColor`. Moving from a bar to its matching line produces a new hover event even though the data indices match. A click requires press and release on the same target; an area series is one target across all its samples.

Hit tests use the actual geometry: rectangles for bars, line distance and point circles for lines, rendered fill triangles for areas, polygons for radar, and radius plus angular interval for pie/polar segments. Radar edges and filled wedges map to their adjoining spoke samples. The last painted overlapping element takes priority.

**Area interaction:** all values at the same `Values[index]` belong to one series target. Its complete fill, outline, and markers highlight together, including separated visible runs. Concave notches, smooth boundaries, negative lobes, and gaps from missing values follow the actual rendered geometry; a rectangular bounding box is never used as the area mask. Moving between samples inside one series does not fire repeated hover-entry events. Leaving the visible shape clears hover, and entering another series changes the target. In overlaps, the later-painted series wins. With `FillOpacity = 0`, only the outline/markers remain interactive.

For area events, `Part = Area`, `ValueIndex` identifies the series, and `SeriesName` supplies its configured name (or `Series N`). The existing `DataName`, `Value`, `DataIndex`, and colors retain the nearby sample at the pointer's category position. `GetHoveredElement` and the tooltip update that sample context as the pointer moves, without retriggering the series hover event. A click can start and finish at different samples in the same series; it reports the release sample. This preserves useful sample values without treating the area as separate hover objects.

Pie clicks outside a circle, inside a donut hole, or in another segment never trigger that segment's event. These are mathematical input masks; no rectangular child button is used. The outer UMG widget still occupies its normal rectangular Slate hit-test slot. Returning an unhandled click allows parent handling; it does not promise click-through to unrelated widgets behind the chart's rectangle.

`GetElementAtLocalPosition` exposes the same geometry test to Blueprint after layout, using local Slate units. Use UMG's geometry conversion nodes for screen positions; do not apply DPI scaling twice. `GetHoveredElement` returns the current hover snapshot.

Charts default to visible and hit-testable. Do not set the chart or its parent to a visibility mode that disables child hit testing. Disabled charts ignore input.

## Updating and styling

Call `SetConfig` to replace data and redraw, and `SetChartStyle` to change presentation. Both also update their exposed properties. When editing nested structs or arrays directly from C++, call `RefreshChart` afterward. The same rule applies to Blueprint operations that edit nested members without calling the property setter. Use these APIs on the game thread.

Each widget exposes its own `ChartStyle` type, accepted by its `SetChartStyle` function. Only options relevant to that chart type appear in Blueprint and Details:

| Style type | Controls beyond the common settings |
| --- | --- |
| `FChartsBarStyle` | Grid settings, `AxisLabelPadding`, `BarGapRatio` |
| `FChartsLineStyle` | Grid settings, `AxisLabelPadding`, `LineThickness`, `PointRadius`, `HitTolerance` |
| `FChartsAreaStyle` | Grid settings, `AxisLabelPadding`, `LineThickness`, `PointRadius`, `HitTolerance` |
| `FChartsComboStyle` | Grid settings, `AxisLabelPadding`, `BarGapRatio`, `LineThickness`, `PointRadius`, `HitTolerance` |
| `FChartsPieStyle` | Common settings only |
| `FChartsPolarAreaStyle` | Grid settings |
| `FChartsRadarStyle` | Grid settings, `AxisLabelPadding`, `LineThickness`, `PointRadius`, `HitTolerance` |

The common base `FChartsStyle` contains `DesiredSize`, `BackgroundColor`, `TextColor`, `FontSize`, `bShowLegend`, `LegendPadding`, `bShowTooltips`, and `bHighlightHovered`. `FChartsGridStyle` adds `GridColor`, `GridDivisions`, and `bShowGrid`. `FChartsAxisStyle` adds `AxisLabelPadding` and is the base for bar/line styles. Point radius applies when markers are drawn; grid controls apply where grids are drawn. Standalone bars have no line thickness or point radius fields, and pie charts have no grid fields.

- **Axis Label Padding** (default **8** Slate units): distance from the plot to Cartesian value ticks, category names, and axis headers; also separates radar spoke names from the shape. Available on bar, line, area, combo, and radar styles. Text is measured with Slate's font service, and plot space is reserved for the requested gap. Applies to all configured axes and both bar/combo orientations.
- **Legend Padding** (default **12** Slate units): space between the plot's lower layout boundary (including Cartesian axis labels) and the series legend. Available on every style. It reserves no space when `bShowLegend` is false.

Both padding values accept 0–200 Slate units. A larger padding reduces the available plot inside the widget; increase `DesiredSize` or its layout slot when more space is needed. Radial charts also retain their normal circular layout margins. For example, set `ChartStyle.AxisLabelPadding = 24` and `ChartStyle.LegendPadding = 32`, then call `SetChartStyle` or `RefreshChart` after direct C++ edits.

Runtime values are clamped to sensible finite limits for layout dimensions and numeric style controls. Long labels and overflowing legends are shortened; detailed event payloads retain original names.

The chart has no per-frame tick. Local geometry is cached until its config, style, or allotted size changes. Adjacent filled primitives share vertex/index batches, with a conservative 60,000-vertex limit per batch. Lines and text use Slate's native draw elements. Large datasets are not silently downsampled: aggregate or window live telemetry before passing very large arrays.

## C++ example

```cpp
#include "RuntimeChartsWidgets.h"

FChartsBarConfig Config;
Config.ChartName = TEXT("Monthly output");
Config.SeriesNames = {TEXT("Units"), TEXT("Target")};
Config.Axis = {FVector2D(0.0, 100.0)};

FChartsData January;
January.DataName = TEXT("January");
FChartsValue Units;
Units.Value = 42.0;
Units.PrimaryColor = FSlateColor(FLinearColor(0.1f, 0.5f, 1.0f));
Units.SecondaryColor = FSlateColor(FLinearColor(1.0f, 0.7f, 0.1f));
FChartsValue Target;
Target.Value = 60.0;
Target.PrimaryColor = FSlateColor(FLinearColor(0.1f, 0.8f, 0.5f));
Target.SecondaryColor = FSlateColor(FLinearColor(1.0f, 0.2f, 0.5f));
January.Values = {Units, Target};
Config.Data.Add(January);
FChartsData February = January;
February.DataName = TEXT("February");
February.Values[0].Value = 55.0;
February.Values[1].Value = 70.0;
Config.Data.Add(February);
BarChart->SetConfig(Config);

FChartsComboConfig ComboConfig;
static_cast<FChartsBarConfig&>(ComboConfig) = Config;
ComboConfig.bShowDataPoints = true;
ComboConfig.bIsSmooth = true;
ComboChart->SetConfig(ComboConfig);

FChartsComboStyle ComboStyle;
ComboStyle.LineThickness = 4.0f;
ComboStyle.PointRadius = 6.0f;
ComboChart->SetChartStyle(ComboStyle);
```

Use `WidgetTree->ConstructWidget<UChartsBarWidget>()` to create a chart inside a C++ UUserWidget. Chart controls derive from `UWidget`; the ready-made `UChartsDemoWidget` derives from `UUserWidget` and works with `CreateWidget`.

## Migrating from v1.x

This version changes the public API. Rebuild C++ and refresh affected Blueprint nodes after replacing the plugin.

- Replace `FChartsValue.Color` with `PrimaryColor` in C++. Set `SecondaryColor` for each combo sample's line; it defaults to amber.
- A [Core Property Redirect](https://dev.epicgames.com/documentation/unreal-engine/core-redirects-in-unreal-engine) for saved `ChartsValue.Color` is included in `Config/DefaultRuntimeCharts.ini`. Keep that file with the plugin. The redirect registration is tested; migration of an existing authored Widget Blueprint is not. Refresh or recreate affected Make/Break Charts Value nodes and verify saved colors before resaving assets.
- Replace old Make Charts Style nodes and style variables with the relevant type, such as **Charts Bar Style** or **Charts Combo Style**, then reconnect **Set Chart Style**. Reapply any saved custom styling; the old generic style property is now a typed property on each concrete widget. C++ calls through `UChartWidget*` must use the appropriate concrete widget type for style access.
- `SeriesTypes` and `EChartsSeriesType` are removed. A combo now draws both a bar and a line sample for every value. Existing combo data therefore gains bars for all values and lines for all series.
- Hover/click dispatcher signatures are unchanged. Their `Color` output now distinguishes combo bars from lines; detailed events additionally expose both colors and `Part`.

### Updating from v2.0 to v2.1

The typed style names and setter signatures remain the same. Rebuild the plugin and refresh Blueprint struct nodes to expose the new padding fields and `SeriesName` result. Area hover now represents the entire series: use `ValueIndex` as its identity and `GetHoveredElement` when you need the current nearby sample while moving inside the same shape. Existing simple dispatcher signatures are unchanged.

## Tests

The plugin includes native Unreal Automation tests under **RuntimeCharts**. Run them through **Tools → Test Automation** (Session Frontend) or an unattended editor instance. The rendering test requires a real rendering RHI; do not pass `-NullRHI`.

Rendering tests save `RuntimeCharts-Gallery.png` and `RuntimeCharts-AreaHoverAndPadding.png` to the project's `Saved/RuntimeCharts` directory. Override with `-ChartsOutput="<absolute directory>"`. Test coverage includes range mapping, degenerate/extreme values, signed bars, missing samples, smoothing bounds, radial masks, radar overlays, DPI-transformed input, touch-handler behavior, Blueprint reflection, widget reconstruction, and actual UMG render-target pixels. Combo regressions compare bar geometry with standalone bars, verify each line's endpoints and point radius, exercise both orientations with multiple gap/thickness settings, check part-specific event colors, and verify that unrelated style fields are absent. Area regressions cover full-series hover identity, exact irregular fill masks, overlaps, gaps, negative values, and highlighting across distant parts of a shape. Padding tests measure the gaps on vertical/horizontal axes, multiple scales, radar names, and legends, including updates after UMG layout.

See the accompanying validation report for the tested platform and build results. Cross-platform compatibility beyond those results remains unverified.

## Engine API reference

The rendering path uses Unreal's native [`FSlateDrawElement`](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/SlateCore/FSlateDrawElement), with custom vertices for filled geometry and standard Slate line/text elements. API signatures were checked against the installed UE 5.8.3 headers.
