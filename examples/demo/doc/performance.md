# Benchmark Results

## Type Registration Performance


The first object registration includes one-time initialization:

| Component                | Time (ms) | Details                                         |
|--------------------------|-----------|-------------------------------------------------|
| **First Object**    | ~15       | WebView context initialization, global registry setup |
| **next Object(s)**  | ~0.5        | Create sync/async dispatchers, bind core functions    |


---

## Method Call Performance (1000 iterations)

| Metric | Sync Calls | Async Calls | Difference |
|--------|-----------|-------------|------------|
| **Total Time** | 243.0 ms | 272.1 ms | +29.1 ms |
| **Average** | 0.243 ms (243µs) | 0.272 ms (272µs) | +29µs |
| **Minimum** | 0.000 ms | 0.100 ms | +100µs |
| **Maximum** | 1.900 ms | 2.000 ms | +100µs |
| **Std Deviation** | 0.135 ms | 0.144 ms | +9µs |
| **Async Overhead** | - | 1.12x | - |

**Key Findings:**
- Synchronous calls average **243 microseconds** per invocation
- Asynchronous calls add only **~30µs overhead** (12% slower)
- Both approaches show **low standard deviation**, indicating consistent performance
- Maximum latency remains under **2ms** even for outliers

#### JSON Processing Performance

nlohmann/json is convenient and user-friendly, but slower than RapidJSON.
- Array deserialization: ~7µs per operation (1000 iterations)
- JavaScript marshalling overhead: ~1ms per 1000 operations (~1µs per call)


#### Design Philosophy

> **"Make the common case fast, and the developer experience excellent."**

In WebBridge:
- JSON parsing takes **7µs** - this is already **fast enough** for GUI applications
- The total call overhead (~240µs) is dominated by **WebView bridging**, not JSON
- Switching to RapidJSON would save ~4µs per call (**1.6% improvement**)
- This does **not justify** the significant **usability cost**

**For 99% of use cases, nlohmann/json's performance is more than adequate.**

---

## References

- [WebBridge Type Registration Template](../tools/templates/registration.h.j2)
- [V8 Blog: Object and Array Optimizations](https://v8.dev/blog/fast-properties)
- [V8 Blog: Function Inlining](https://v8.dev/blog/function-optimizations)
- [nlohmann/json GitHub](https://github.com/nlohmann/json)
- [RapidJSON GitHub](https://github.com/Tencent/rapidjson)
- [simdjson GitHub](https://github.com/simdjson/simdjson)
