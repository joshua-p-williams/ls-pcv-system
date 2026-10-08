# PCV system test plan

This proposed test procedure covers the current component selections. No physical test results or validated operating limits are recorded yet. Earlier rationale is preserved in the [historical context](../LS_PCV_SYSTEM_CONTEXT.md).

Use [A04: PCV system overview](../../hardware/architecture/pcv-system.md), [A01: overall gauge architecture](../../hardware/architecture/gauge-system.md) and [A02: power architecture](../../hardware/architecture/power-system.md) as conceptual configuration references. Record the actual test setup and any deviations; these drafts do not establish installed or verified connections.

Use [A03: pressure measurement signal chain](../../hardware/architecture/measurement-system.md) when defining calibration, stale/fault, zero/reset and peak-response checks. Its behavior is proposed, not a record of completed tests.

## Objective and conventions

Compare restrictors using repeatable measured crankcase pressure, oil carryover, and operating behavior. Use `inH2O` relative to atmosphere, with negative values indicating vacuum. The 3 mm restrictor and roughly -3 to -8 inH2O hot-idle/cruise range are working assumptions, not pass/fail limits.

Record configuration and evidence using the [data conventions](../../data/README.md). Do not interpret the relief valve's published cracking pressure as measured behavior or proof of sufficient flow capacity.

## 1. Document the configuration

- Record hose routing, fresh-air path, catch-can port assignments, pressure tap, restrictor measured bore, and relief orientation.
- Identify actual sensor, connector terminals, power supply, ADC/reference, and firmware revision when present.
- Measure crankcase/catch-can pressure upstream of the restrictor; protect the sensor from direct liquid oil exposure.
- Inspect hoses, retention, mounting, wiring, and catch-can condition before testing.

For gauge bench work, start with the documented [power integration checks](../../hardware/components/gauge-power-supply/README.md), [P04 sensor terminal/fit checks](../../hardware/pinouts/ftp-sensor.md), and [ADS1115 setup](../../hardware/components/adc/README.md). Record the actual conditioning components, gain, conversion rate, channel/address, and supply arrangement with the calibration. The former divider is superseded; the initial signal filter is 470 ohm / 1 uF, with stock parts and response to record/validate; fuse values remain design work. Verify the shared 5 V branch and separate I2C voltage domains before integrating the sensor and ADC.

[W01's phased assembly guide](../../hardware/wiring/bench/README.md#phased-assembly-and-verification) adds configuration-specific prerequisites and section views for the N16R8 USB bench setup. Resolve each phase's marked interfaces before connection or power; converter checks apply only when that source is introduced. Q30/Q31 define native USB, IN-OUT closed, USB-OTG open, nominal USB-fed 5 V and 3.3 V display power. Include actual diode/cable drop and source/load margin in phase checks. The harness draft and rendering checks are not bench-test results.

## 2. Characterize the sensor on the bench

1. Verify terminal positions and electrical compatibility before applying power.
2. Equalize the pressure port to atmosphere and record raw zero, supply/reference, and setup conditions.
3. Use a water manometer to apply known positive and negative differential pressures within the verified sensor range. Read total water-level difference, not just one leg's displacement; prevent water entering the sensor.
4. Record multiple points in both directions and repeat zero. Points such as +5 and -5 inH2O are examples only, pending range verification.
5. Derive calibration from readings; check residuals, sign, repeatability, and usable range. Preserve raw points and calculation method.
6. Check zero/reset controls, sensor fault indication, and min/max capture against known pressure changes. Record sample rate and filtering so missed or smoothed peaks can be assessed.

Do not proceed to tuning on an unverified sensor equation or unexplained zero drift.

## 3. Characterize the relief path on the bench

1. Confirm flow orientation from catch can to atmosphere; crankcase vacuum should close the valve.
2. Measure opening and reseating pressure over repeated slow pressure ramps using a suitable reference.
3. Check reverse leakage under vacuum and effects of installation orientation.
4. If modifying the spring, record original/replacement dimensions and results for each configuration. Do not infer the outcome from spring cutting alone.
5. Evaluate relief flow behavior separately from cracking pressure; document the setup and limitations of the test.

Approximately 0.08-0.12 PSI (2.2-3.3 inH2O) is the desired initial opening range. The published 0.5 PSI setting is approximately 13.8 inH2O and has not been accepted for the intended relief function.

## 4. Vehicle comparison

Before increasing load, establish and record configuration-specific stop criteria and an independently checked pressure indication. Stop for unexplained readings, excessive pressure/vacuum relative to those criteria, leaks, hose movement, or abnormal oil carryover. Investigate before continuing; this document does not yet establish validated numeric limits.

Start with the 3 mm baseline. Change one variable at a time and record a separate run for each configuration. Progress through engine-off atmospheric verification, cold idle, warm idle, light/moderate cruise, closed-throttle deceleration, moderate acceleration, and hot restart. High-load/WOT tests follow only after earlier checks support proceeding, in a controlled appropriate setting.

Use logging or stored peaks; do not require the driver to watch the gauge during a pull. Record RPM, MAP, and throttle position when available; label absent channels rather than estimating them.

For each run capture positive/negative peaks, steady-state pressure, engine temperature/operating state, ambient conditions, idle quality, vapor/leaks, catch-can contents, and oil-consumption observations over a stated interval. Compare 2 and 4 mm variants under similar conditions; repeat the baseline to assess variability.

## Results and decisions

Link raw logs, calibration, configuration revision, observations, and analysis. Record what the evidence supports, uncertainties, and the next action. Update component documents and [project status](../status.md) when the evidence changes a design decision. Document review or a successful firmware build does not validate physical hardware.
