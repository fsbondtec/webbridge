<script lang="ts">
  import { onMount, onDestroy } from 'svelte';
  import { MyObject } from './generated/MyObject';
  import { TestObject } from './generated/TestObject';
  import { HelloWorld } from './generated/HelloWorld';
  import bildUrl from './assets/bild.jpg';

  // State
  let obj: MyObject | null = null;
  let logs: string[] = [];
  let errorModalOpen = false;
  let errorModalMessage = '';

  // Store subscriptions
  $: aBool = obj?.aBool;
  $: strProp = obj?.strProp;
  $: counter = obj?.counter;
  $: numbers = obj?.numbers;
  $: status = obj?.status;
  $: pod = obj?.pod;

  // Hello World state
  let helloObj: HelloWorld | null = null;
  let nameInput = 'World';

  // Hello World store subscriptions
  $: helloMessage = helloObj?.message;
  $: helloGreetCount = helloObj?.greetCount;

  function log(msg: string, type: 'info' | 'success' | 'error' = 'info') {
    const timestamp = new Date().toLocaleTimeString();
    const prefix = type === 'error' ? '❌' : type === 'success' ? '✅' : 'ℹ️';
    logs = [...logs, `[${timestamp}] ${prefix} ${msg}`];
    console.log(msg);
  }

  async function createObject() {
    try {
      log('Creating object...', 'info');

      log(`Static constant appversion: ${MyObject.appversion}`, 'info');
      const newObj = await MyObject.create('fab');
      log(`Object created: ${newObj.handle}`, 'success');
      newObj.aEvent.on((intVal, boolVal) => {
        log(`🔔 Event received: int=${intVal}, bool=${boolVal}`);
      });

      // Log constants (instance and static)
      log(`Instance constant aversion: ${newObj.version}`, 'info');
      log(`Instance constant CPP_VERSION: ${MyObject.CPP_VERSION}`, 'info');

      obj = newObj;
    } catch (error) {
      log(`Failed to create object: ${error}`, 'error');
    }
  }

  function destroyObject() {
    if (!obj) {
      log('No object to destroy!', 'error');
      return;
    }

    try {
      const objToDestroy = obj;
      obj = null;
      objToDestroy.destroy();
      log('Object destroyed', 'success');
    } catch (error) {
      log(`Failed to destroy object: ${error}`, 'error');
    }
  }

  onDestroy(() => {
    if (obj) {
      obj.destroy();
    }
    if (helloObj) {
      helloObj.destroy();
    }
  });

  async function createHelloWorld() {
    try {
      log('Creating HelloWorld...', 'info');

      const newObj = await HelloWorld.create('Demo Author');
      log(`HelloWorld created: ${newObj.handle}`, 'success');
      log(`Instance constant author: ${newObj.author}`, 'info');

      newObj.greeted.on((greetedName) => {
        log(`🔔 greeted event received: name=${greetedName}`);
      });

      helloObj = newObj;
    } catch (error) {
      log(`Failed to create HelloWorld: ${error}`, 'error');
    }
  }

  function destroyHelloWorld() {
    if (!helloObj) {
      log('No HelloWorld to destroy!', 'error');
      return;
    }

    try {
      const objToDestroy = helloObj;
      helloObj = null;
      objToDestroy.destroy();
      log('HelloWorld destroyed', 'success');
    } catch (error) {
      log(`Failed to destroy HelloWorld: ${error}`, 'error');
    }
  }

  async function callGreet() {
    if (!helloObj) {
      log('No HelloWorld object!', 'error');
      return;
    }

    try {
      log(`Calling greet('${nameInput}')...`, 'info');
      await helloObj.greet(nameInput);
      log('greet() completed', 'success');
    } catch (error) {
      log(`greet() failed: ${error}`, 'error');
    }
  }

  async function callSlowGreet() {
    if (!helloObj) {
      log('No HelloWorld object!', 'error');
      return;
    }

    try {
      log(`Calling slowGreet('${nameInput}') (async, ~1s on a worker thread)...`, 'info');
      await helloObj.slowGreet(nameInput);
      log('slowGreet() completed', 'success');
    } catch (error) {
      log(`slowGreet() failed: ${error}`, 'error');
    }
  }

  async function callBar() {
    if (!obj) {
      log('No object!', 'error');
      return;
    }

    try {
      log('Calling bar()...', 'info');
      const result = await obj.bar();
      log(`bar() returned: ${result}`, 'success');
    } catch (error) {
      log(`bar() failed: ${error}`, 'error');
    }
  }

  async function callFoo() {
    if (!obj) {
      log('No object!', 'error');
      return;
    }

    try {
      log('Calling foo() (async)...', 'info');
      await obj.foo('Hello from Svelte! 🎉');
      log('foo() completed', 'success');
    } catch (error) {
      log(`foo() failed: ${error}`, 'error');
    }
  }

  async function callFile() {
    if (!obj) {
      log('No object!', 'error');
      return;
    }

    try {
      log('Opening file dialog...', 'info');
      await obj.file();
      log('File dialog completed', 'success');
    } catch (error) {
      log(`file() failed: ${error}`, 'error');
    }
  }

  async function callTestVectors() {
    if (!obj) {
      log('No object!', 'error');
      return;
    }

    try {
      log('Calling testVectors()...', 'info');
      await obj.testVectors();
      log('testVectors() completed - check properties above!', 'success');
    } catch (error) {
      log(`testVectors() failed: ${error}`, 'error');
    }
  }

  async function callThrowError() {
    if (!obj) {
      log('No object!', 'error');
      return;
    }
    
    try {
      log('Calling throwError()...', 'info');
      await obj.throwError();
    } catch (error) {
      const errText =
        error instanceof Error ? error.message : (typeof error === 'string' ? error : JSON.stringify(error));
      errorModalMessage = errText;
      errorModalOpen = true;
      log(`throwError() failed (expected): ${JSON.stringify(error)}`, 'error');
    }
  }

  async function callMultiParamTest() {
    if (!obj) {
      log('No object!', 'error');
      return;
    }
    
    await obj.bar();
    
    try {
      log('Calling multiParamTest() with 6 different parameter types...', 'info');
      const result = await obj.multiParamTest(
        999,                           // int
        true,                          // bool
        'Test String from JS',         // string
        [10, 20, 30, 40, 50],         // vector<int>
        MyObject.Status.Running,      // Status enum
        { a: 777, b: 888888888 }      // Pod struct
      );
      log(`multiParamTest() returned: ${result}`, 'success');
      log('✨ Check properties above - they should all be updated!', 'info');
    } catch (error) {
      log(`multiParamTest() failed: ${JSON.stringify(error)}`, 'error');
    }
  }

  function clearLog() {
    logs = [];
    log('Log cleared', 'info');
  }

  // ============ BENCHMARK FUNCTIONALITY ============
  
  interface BenchmarkStats {
    min: number;
    max: number;
    avg: number;
    stdev: number;
    total: number;
  }

  function calculateStats(values: number[]): BenchmarkStats {
    if (values.length === 0) {
      return { min: 0, max: 0, avg: 0, stdev: 0, total: 0 };
    }

    const min = Math.min(...values);
    const max = Math.max(...values);
    const total = values.reduce((a, b) => a + b, 0);
    const avg = total / values.length;
    
    // Standard deviation
    const variance = values.reduce((sum, val) => sum + Math.pow(val - avg, 2), 0) / values.length;
    const stdev = Math.sqrt(variance);

    return { min, max, avg, stdev, total };
  }

  async function runBenchmark() {
    try {
      log('=== STARTING BENCHMARK ===', 'info');
      
      // Create TestObject
      log('Creating TestObject...', 'info');
      const testObj = await TestObject.create() as TestObject;
      log(`TestObject created: ${testObj.handle}`, 'success');

      const iterations = 1000;
      const warmupRuns = 50;
      testObj.asyncProcess('f.a.b.');

      // ===== SYNC BENCHMARK =====
      log(`\n--- Sync Benchmark (${iterations} iterations) ---`, 'info');
      
      // Warmup
      for (let i = 0; i < warmupRuns; i++) {
        await testObj.benchmarkSync(i);
      }

      const syncTimes: number[] = [];
      for (let i = 0; i < iterations; i++) {
        const start = performance.now();
        await testObj.benchmarkSync(i);
        const elapsed = performance.now() - start;
        syncTimes.push(elapsed);
      }

      const syncStats = calculateStats(syncTimes);
      log(`Sync Results (${iterations}x):`, 'success');
      log(`  Total: ${syncStats.total.toFixed(2)} ms`, 'info');
      log(`  Avg:   ${syncStats.avg.toFixed(3)} ms (${(syncStats.avg * 1000).toFixed(1)} µs)`, 'info');
      log(`  Min:   ${syncStats.min.toFixed(3)} ms (${(syncStats.min * 1000).toFixed(1)} µs)`, 'info');
      log(`  Max:   ${syncStats.max.toFixed(3)} ms (${(syncStats.max * 1000).toFixed(1)} µs)`, 'info');
      log(`  StdDev: ${syncStats.stdev.toFixed(3)} ms (${(syncStats.stdev * 1000).toFixed(1)} µs)`, 'info');

      // ===== ASYNC BENCHMARK =====
      log(`\n--- Async Benchmark (${iterations} iterations) ---`, 'info');
      
      // Warmup
      for (let i = 0; i < warmupRuns; i++) {
        await testObj.benchmarkAsync(i);
      }

      const asyncTimes: number[] = [];
      for (let i = 0; i < iterations; i++) {
        const start = performance.now();
        await testObj.benchmarkAsync(i);
        const elapsed = performance.now() - start;
        asyncTimes.push(elapsed);
      }

      const asyncStats = calculateStats(asyncTimes);
      log(`Async Results (${iterations}x):`, 'success');
      log(`  Total: ${asyncStats.total.toFixed(2)} ms`, 'info');
      log(`  Avg:   ${asyncStats.avg.toFixed(3)} ms (${(asyncStats.avg * 1000).toFixed(1)} µs)`, 'info');
      log(`  Min:   ${asyncStats.min.toFixed(3)} ms (${(asyncStats.min * 1000).toFixed(1)} µs)`, 'info');
      log(`  Max:   ${asyncStats.max.toFixed(3)} ms (${(asyncStats.max * 1000).toFixed(1)} µs)`, 'info');
      log(`  StdDev: ${asyncStats.stdev.toFixed(3)} ms (${(asyncStats.stdev * 1000).toFixed(1)} µs)`, 'info');

      // Comparison
      log(`\n--- Comparison ---`, 'info');
      log(`  Async overhead: ${(asyncStats.avg - syncStats.avg).toFixed(3)} ms (${((asyncStats.avg - syncStats.avg) * 1000).toFixed(1)} µs)`, 'info');
      log(`  Async is ${(asyncStats.avg / syncStats.avg).toFixed(2)}x slower than Sync`, 'info');

      // Cleanup
      log('\nDestroying TestObject...', 'info');
      testObj.destroy();
      log('TestObject destroyed', 'success');
      log('=== BENCHMARK COMPLETE ===', 'success');

    } catch (error) {
      log(`Benchmark failed: ${error}`, 'error');
    }
  }

  onMount(() => {
    log('WebBridge Demo initialized 🚀', 'success');
  });
</script>

<div class="min-h-screen bg-gradient-to-br from-purple-600 to-indigo-800 p-5">
  <div class="max-w-5xl mx-auto">
    <!-- Hero Section -->
    <div class="card bg-base-100 shadow-2xl mb-6">
      <div class="card-body">
        <h1 class="card-title text-4xl font-bold text-center justify-center mb-4">
          🌉 WebBridge Svelte Demo
        </h1>
        
        <!-- Status Badge -->
        <div class="flex justify-center mb-4">
          {#if obj !== null}
            <div class="badge badge-success badge-lg gap-2">
              <div class="w-3 h-3 rounded-full bg-green-500 animate-pulse"></div>
              Object active
            </div>
          {:else}
            <div class="badge badge-neutral badge-lg gap-2">
              <div class="w-3 h-3 rounded-full bg-gray-400"></div>
              No object
            </div>
          {/if}
        </div>

        <!-- Control Buttons -->
        <div class="flex flex-wrap gap-2 justify-center">
          <button class="btn btn-primary" on:click={createObject} disabled={obj !== null}>
            Create Object
          </button>
          <button class="btn btn-secondary" on:click={destroyObject} disabled={obj === null}>
            Destroy Object
          </button>
          <button class="btn btn-accent" on:click={callBar} disabled={obj === null}>
            Call bar()
          </button>
          <button class="btn btn-accent" on:click={callFoo} disabled={obj === null}>
            Call foo()
          </button>
          <button class="btn btn-accent" on:click={callFile} disabled={obj === null}>
            Call file()
          </button>
          <button class="btn btn-accent" on:click={callTestVectors} disabled={obj === null}>
            Test Vectors
          </button>
          <button class="btn btn-error" on:click={callThrowError} disabled={obj === null}>
            Throw Error 💥
          </button>
          <button class="btn btn-info" on:click={callMultiParamTest} disabled={obj === null}>
            Test Multi-Params 🎯
          </button>
          <button class="btn btn-ghost" on:click={clearLog}>
            Clear Log
          </button>
          <button class="btn btn-warning font-bold" on:click={runBenchmark}>
            🚀 Run Benchmark (TestObject)
          </button>
        </div>
      </div>
    </div>

    <!-- Hello World Section -->
    <div class="card bg-base-100 shadow-xl mb-6">
      <div class="card-body">
        <h2 class="card-title text-2xl">👋 Hello World (learning example)</h2>

        <div class="flex flex-wrap items-center gap-2 mb-4">
          <input
            type="text"
            class="input input-bordered w-full max-w-xs"
            placeholder="Name to greet"
            bind:value={nameInput}
          />
          <button class="btn btn-primary" on:click={createHelloWorld} disabled={helloObj !== null}>
            Create
          </button>
          <button class="btn btn-accent" on:click={callGreet} disabled={helloObj === null}>
            Greet
          </button>
          <button class="btn btn-accent" on:click={callSlowGreet} disabled={helloObj === null}>
            Slow Greet (async)
          </button>
          <button class="btn btn-secondary" on:click={destroyHelloWorld} disabled={helloObj === null}>
            Destroy
          </button>
        </div>

        {#if helloObj && helloMessage && helloGreetCount}
          <div class="grid gap-3">
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">message</div>
                <div class="stat-value text-primary text-2xl font-mono">{$helloMessage}</div>
              </div>
            </div>

            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">greetCount</div>
                <div class="stat-value text-accent text-2xl">{$helloGreetCount}</div>
              </div>
            </div>
          </div>
        {:else}
          <div class="alert">
            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" class="stroke-info shrink-0 w-6 h-6">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path>
            </svg>
            <span>No HelloWorld object created yet.</span>
          </div>
        {/if}
      </div>
    </div>

    <!-- Properties Section -->
    <div class="card bg-base-100 shadow-xl mb-6">
      <div class="card-body">
        <h2 class="card-title text-2xl">Properties (auto-synced via WebBridge)</h2>
        
        {#if obj && aBool && strProp && counter && numbers && status && pod}
          <div class="grid gap-3">
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">aBool</div>
                <div class="stat-value text-primary text-2xl">{$aBool}</div>
              </div>
            </div>
            
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">strProp</div>
                <div class="stat-value text-secondary text-2xl font-mono">{$strProp || '(empty)'}</div>
              </div>
            </div>
            
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">counter</div>
                <div class="stat-value text-accent text-2xl">{$counter}</div>
              </div>
            </div>
            
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">numbers</div>
                <div class="stat-value text-xl font-mono">[{$numbers ? $numbers.join(', ') : ''}]</div>
              </div>
            </div>
            
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">status</div>
                <div class="stat-value text-xl">
                  {#if $status === 'Idle'}
                    <span class="text-gray-500">{$status}</span>
                  {:else if $status === 'Running'}
                    <span class="text-warning">{$status}</span>
                  {:else if $status === 'Completed'}
                    <span class="text-success">{$status}</span>
                  {:else if $status === 'Error'}
                    <span class="text-error">{$status}</span>
                  {:else}
                    <span>{$status || 'Idle'}</span>
                  {/if}
                </div>
              </div>
            </div>
            
            {#if $pod}
            <div class="stats shadow">
              <div class="stat">
                <div class="stat-title">pod</div>
                <div class="stat-value text-xl font-mono">{`{ a: ${$pod.a}, b: ${$pod.b} }`}</div>
              </div>
            </div>
            {/if}
          </div>
        {:else}
          <div class="alert">
            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" class="stroke-info shrink-0 w-6 h-6">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path>
            </svg>
            <span>No object created yet.</span>
          </div>
        {/if}
      </div>
    </div>

    <!-- Image Section -->
    <div class="card bg-base-100 shadow-xl mb-6">
      <div class="card-body">
        <h2 class="card-title text-2xl">🖼️ Embedded Image</h2>
        <figure>
          <img src={bildUrl} alt="Beispielbild" class="rounded-lg shadow-lg" />
        </figure>
        <div class="alert alert-info mt-4">
          <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" class="stroke-current shrink-0 w-6 h-6">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path>
          </svg>
          <span>This image is bundled with the app and served from embedded resources.</span>
        </div>
      </div>
    </div>

    <!-- Log Section -->
    <div class="card bg-base-100 shadow-xl">
      <div class="card-body">
        <h2 class="card-title text-2xl">Console Log</h2>
        <div class="mockup-code max-h-96 overflow-y-auto">
          {#each logs as logEntry}
            <pre data-prefix=">" class="text-sm"><code>{logEntry}</code></pre>
          {/each}
        </div>
      </div>
    </div>
  </div>
</div>

<!-- Error Modal -->
{#if errorModalOpen}
  <div class="modal modal-open">
    <div class="modal-box">
      <h3 class="font-bold text-lg text-error">❌ Fehler</h3>
      <p class="py-4">{errorModalMessage}</p>
      <div class="modal-action">
        <button class="btn btn-primary" on:click={() => errorModalOpen = false}>OK</button>
      </div>
    </div>
  </div>
{/if}
