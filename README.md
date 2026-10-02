# Runtime Charts — UE 5.8.3 / C++20

A native UMG chart plugin. All geometry, text, and input use Unreal's Slate renderer. No web browser, JavaScript, third-party chart library, custom material, or asset import is required.

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

`FChartsValue` contains a `double Value` and `FSlateColor Color`.

`FChartsData` contains a `FString DataName` and `TArray<FChartsValue> Values`.

| Widget / config | Meaning of `Data` | Meaning of `Values[index]` |
| --- | --- | --- |
| `UChartsBarWidget` / `FChartsBarConfig` | Categories along the category axis | Grouped series |
| `UChartsLineWidget` / `FChartsLineConfig` | Categories from left to right | One sample per line series |
| `UChartsAreaWidget` / `FChartsAreaConfig` | Categories from left to right | One sample per filled series |
| `UChartsComboWidget` / `FChartsComboConfig` | Categories from left to right | Bar or line series |
| `UChartsPieWidget` / `FChartsPieConfig` | Slice categories | Concentric series rings, innermost first |
| `UChartsPolarAreaWidget` / `FChartsPolarAreaConfig` | Sector categories | Equal-angle subsectors within each category |
| `UChartsRadarWidget` / `FChartsRadarConfig` | Separate overlaid shapes | Spokes around the shape |

For example, `Jan.Values = [10, 100]` and `Feb.Values = [20, 200]` create two series with samples `[10, 20]` and `[100, 200]`. The X axis is categorical and evenly spaced. Values retain double precision until normalized into local drawing coordinates.

To color each series consistently, use the same color for that series index across categories. Each individual value's color is honored. Segment colors change at the midpoint between neighboring category positions. Legends use the first available color and optional `SeriesNames`; for radial charts they use data names.

## Axis rules

For bar, line, area, and combo charts:

- `Axis = []`: one shared automatic range across all finite values. Zero is included.
- `Axis = [(Min, Max)]`: one shared explicit range.
- `Axis = [(Min0, Max0), (Min1, Max1), ...]`: independent ranges for the corresponding `Values` index.
- If multiple axes are supplied but an index has no range, that series gets its own automatic range. Excess ranges without a data series are ignored.
- Reversed bounds are sorted. Equal nonzero bounds expand toward zero; `(0, 0)` becomes `(0, 1)`. Non-finite bounds fall back to automatic calculation.
- Values beyond an explicit range are clamped to its boundary for drawing. Events still return the original value.

All active Cartesian axes are labeled. Vertical charts place the first axis on the left and additional axes on the right; horizontal bars place value axes below the plot. Independent axes require additional space. When the axes cannot fit, the widget asks for a larger layout rather than producing invalid geometry.

Set `bIsHorizontal` on the bar config for horizontal bars. Standalone line, area, and combo charts run left to right.

## Chart-specific controls

**Line:** `bShowDataPoints` draws circles. `bIsSmooth` uses a monotone cubic Hermite curve, bounded by adjacent samples so it cannot introduce overshoot. False draws straight segments. A lone finite sample draws a point even when point markers are disabled, so it remains visible.

**Area:** inherits the line settings and adds `FillOpacity`. Each series fills between its curve and its own zero baseline, clamped to the configured range. Negative areas and baseline crossings are supported. Series are overlaid, not stacked.

**Combo:** inherits the line settings and adds `SeriesTypes`. Every bar series is drawn before every line series. A missing type defaults to Bar for index 0 and Line for later indices. Use separate axes for quantities such as unit counts and percentages.

**Pie:** each positive value receives its proportional angle within its series ring. One series produces a pie; additional series produce independently normalized concentric rings. `InnerRadiusRatio` creates a donut hole and `StartAngleDegrees` rotates the chart clockwise in screen coordinates. Zero, negative, missing, and non-finite values produce no slice. Ring gaps are not interactive.

**Polar area:** equal-angle sectors use `radius = chart radius × sqrt(value / maximum)`, so sector **area** is proportional to value. `MaxValue = 0` chooses the largest positive value. An explicit maximum clamps larger values visually. Multiple series subdivide each category's angle. Missing, zero, and negative values leave empty sectors.

**Radar:** each `Data` item produces another shape, in array order. At least three spokes are required. Set `bIsFillInside` for a translucent interior; otherwise only the perimeter and optional points are drawn and interactive. `AxisLabels` names spokes. Optional `Axis` ranges follow the shared/per-spoke rules above. With automatic/shared ranges, numeric ring labels are shown. An incomplete shape (missing/non-finite spoke values) is skipped instead of inventing values. Later shapes have input priority where they overlap.

## Input and dispatchers

The primary dispatchers are:

- `OnElementHovered(DataName, Value, Color)` — when the pointer enters a different element.
- `OnElementClicked(DataName, Value, Color)` — left-button press and release on the same element, or a touch tap.

Additional dispatchers are `OnElementHoveredDetailed`, `OnElementClickedDetailed`, and `OnElementHoverEnded`. Detailed events pass `FChartsElement`, which also includes the original zero-based `DataIndex` and `ValueIndex` for unambiguous identification.

Hit tests use the actual geometry: rectangles for bars, line distance and point circles for line charts, polygons for filled areas/radar, and radius plus angular interval for pie/polar segments. Line and area interactions select a sample according to its half of the category interval. Radar edges and filled wedges map to their adjoining spoke samples. The last painted overlapping element takes priority.

Pie clicks outside a circle, inside a donut hole, or in another segment never trigger that segment's event. These are mathematical input masks; no rectangular child button is used. The outer UMG widget still occupies its normal rectangular Slate hit-test slot. Returning an unhandled click allows parent handling; it does not promise click-through to unrelated widgets behind the chart's rectangle.

`GetElementAtLocalPosition` exposes the same geometry test to Blueprint after layout, using local Slate units. Use UMG's geometry conversion nodes for screen positions; do not apply DPI scaling twice. `GetHoveredElement` returns the current hover snapshot.

Charts default to visible and hit-testable. Do not set the chart or its parent to a visibility mode that disables child hit testing. Disabled charts ignore input.

## Updating and styling

Call `SetConfig` to replace data and redraw, and `SetChartStyle` to change presentation. Both also update their exposed properties. When editing nested structs or arrays directly from C++, call `RefreshChart` afterward. The same rule applies to Blueprint operations that edit nested members without calling the property setter. Use these APIs on the game thread.

`FChartsStyle` controls desired size, background/text/grid colors, font size, grid density, line thickness, point size, hit tolerance, bar gaps, legend visibility, automatic tooltips, and hover highlighting. Runtime values are clamped to sensible finite limits for layout dimensions and numeric style controls. Long labels and overflowing legends are shortened; detailed event payloads retain original names.

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
Units.Color = FSlateColor(FLinearColor(0.1f, 0.5f, 1.0f));
FChartsValue Target;
Target.Value = 60.0;
Target.Color = FSlateColor(FLinearColor(0.1f, 0.8f, 0.5f));
January.Values = {Units, Target};
Config.Data.Add(January);
BarChart->SetConfig(Config);
```

Use `WidgetTree->ConstructWidget<UChartsBarWidget>()` to create a chart inside a C++ UUserWidget. Chart controls derive from `UWidget`; the ready-made `UChartsDemoWidget` derives from `UUserWidget` and works with `CreateWidget`.

## Tests

The plugin includes native Unreal Automation tests under **RuntimeCharts**. Run them through **Tools → Test Automation** (Session Frontend) or an unattended editor instance. The rendering test requires a real rendering RHI; do not pass `-NullRHI`.

The rendering test saves `RuntimeCharts-Gallery.png` to the project's `Saved/RuntimeCharts` directory. Override with `-ChartsOutput="<absolute directory>"`. Test coverage includes range mapping, degenerate/extreme values, signed bars, missing samples, smoothing bounds, radial masks, radar overlays, DPI-transformed input, touch-handler behavior, Blueprint reflection, widget reconstruction, and actual UMG render-target pixels.

See the accompanying validation report for the tested platform and build results. Cross-platform compatibility beyond those results remains unverified.

## Engine API reference

The rendering path uses Unreal's native [`FSlateDrawElement`](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/SlateCore/FSlateDrawElement), with custom vertices for filled geometry and standard Slate line/text elements. API signatures were checked against the installed UE 5.8.3 headers.
