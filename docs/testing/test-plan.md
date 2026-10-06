# PCV system test plan

Status: proposed procedure derived from the [historical context](../LS_PCV_SYSTEM_CONTEXT.md). No test results or validated operating limits are recorded yet.

## Objective and conventions

Compare restrictors using repeatable measured crankcase pressure, oil carryover, and operating behavior. Use `inH2O` relative to atmosphere, with negative values indicating vacuum. The 3 mm restrictor and roughly -3 to -8 inH2O hot-idle/cruise range are working assumptions, not pass/fail limits.

Record configuration and evidence using the [data conventions](../../data/README.md). Do not interpret the relief valve's published cracking pressure as measured behavior or proof of sufficient flow capacity.

## 1. Document the configuration

- Record hose routing, fresh-air path, catch-can port assignments, pressure tap, restrictor measured bore, and relief orientation.
- Identify actual sensor, connector terminals, power supply, ADC/reference, and firmware revision when present.
- Measure crankcase/catch-can pressure upstream of the restrictor; protect the sensor from direct liquid oil exposure.
- Inspect hoses, retention, mounting, wiring, and catch-can condition before testing.

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
