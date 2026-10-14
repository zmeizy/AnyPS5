# GPU probe tools (not part of AnyPS5, never for a pull request)

Tools used to measure RDNA instruction semantics on a local AMD GPU and turn the results into AnyPS5 pull requests.

| File | Purpose |
| --- | --- |
| `anyps5_gpu_probes.py` | Runs the probes (`PROBES` list) with the hardware oracle's kernel template. Linux: `/dev/kfd` (tools/hw-oracle). Windows: the AMD driver's HIP runtime (`amdhip64_*.dll`), using kernels embedded in `PREBUILT`. `--check` assembles or lists kernels; `--build-prebuilt gfx1030,...` re-embeds kernels (needs clang/ld.lld with the AMDGPU target, e.g. from WSL Ubuntu `apt install clang lld llvm`). Writes `anyps5-gpu-probes-v<N>.json.gz`. |
| `anyps5-gpu-probes-v1-gfx1030.json.gz` | Results of probe set v1 on the owner's gfx1030 (Windows, HIP). |
| `load.py` | Loads a results file (`ANYPS5_PROBE_RESULTS`, default `results.json.gz`) and yields rows per probe/body/mode. |
| `f16ref.py`, `mullit_model.py` | Exact reference models used to fit the measurements. |
| `ZzProbeHarness.cpp` | Untracked scratch execution test: copy into `core/libs/prx/libSceAgcDriver/tests/execution/`, build `agc_driver_zz_probe_harness_tests`, never commit it. Runs any code through the recompiler in given float modes. |
| `compare.py`, `baseline.py` | Run a probe body through the harness (`ANYPS5_PROBE_HARNESS` = path of the built harness) and diff against the GPU rows per mode: `python3 baseline.py <probe name>`. |
| `gentest.py` | Assembles a one-instruction program in the execution-test layout (`program(asm)`) and looks up measured rows (`gpu_table`, `lookup`) to generate an execution test. |
| `newconflicts.sh` | Reports conflicts that a branch adds against open upstream PRs (merge refs fetched into `refs/prm/<n>`). |
