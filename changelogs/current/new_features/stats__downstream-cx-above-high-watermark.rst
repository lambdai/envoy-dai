Added ``downstream_cx_above_high_watermark`` gauges to per-worker listener stats and the
``envoy.filters.network.downstream_watermark_stats`` network filter to track active downstream
connections currently above the write buffer high watermark.
