# Circom

## Features

|                           | Tachyon   | [Circom-compat] | [Rapidsnark]                      |
| ------------------------- | --------- | --------------- | --------------------------------- |
| Language                  | C++       | Rust            | C++                               |
| Witness Generator         | C(Fast)   | WASM(Slow)      | -                                 |
| Flexible Witness Handling | Yes       | Yes             | No                                |
| Field Support             | All       | Bn128           | Bn128                             |
| [FFIASM] Generated Field  | Yes(Fast) | No(Slow)        | Yes(Fast)                         |
| Gpu Support               | Yes       | No              | Yes([Orbiter-Finance/Rapidsnark]) |

1. Speed: Tachyon runs approximately 10 times faster than [Rapidsnark]! The [benchmark] was conducted using [sha256_512.circom] with a degree of 2¹⁶.
2. Seamless Workflow: Tachyon doesn't require you to run any additional programs just to generate a circom proof. No need for [snarkjs] for witness file generation.
3. Flexible Witness Handling: With Tachyon, you can modify the witness directly in your program. No need to create a separate JSON file with [snarkjs].
4. Integrated Build System: Tachyon seamlessly integrates circom compilation into the build system, specifically bazel. When you make changes to a circom file, it's compiled accordingly and built in parallel, ensuring a safe and efficient process.
5. Field Support: Tachyon isn't limited to bn128; it easily supports all fields!
6. Gpu Support: Both Tachyon and [Orbiter-Finance/Rapidsnark] support MSM / NTT using [icicle].

[Circom-compat]: https://github.com/arkworks-rs/circom-compat
[Rapidsnark]: https://github.com/iden3/rapidsnark
[FFIASM]: https://github.com/iden3/ffiasm
[Orbiter-Finance/Rapidsnark]: https://github.com/Orbiter-Finance/rapidsnark
[benchmark]: /vendors/circom/benchmark/README.md
[sha256_512.circom]: /vendors/circom/benchmark/sha256_512.circom
[snarkjs]: https://github.com/iden3/snarkjs
[icicle]: https://github.com/ingonyama-zk/icicle

## How to build

Go to [prerequisites](../../docs/how_to_use/how_to_build.md#Prerequisites) and follow the instructions.

### With CUDA

```shell
bazel build --@kroma_network_tachyon//:has_openmp --config maxopt --config cuda //:prover_main
```

### Without CUDA

```shell
bazel build --@kroma_network_tachyon//:has_openmp --config maxopt //:prover_main
```

### With Docker

For the CUDA build, you need to [setup the NVIDIA Container toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html).

```shell
# These commands should be run in the base folder
cd ../..
# Build without CUDA
docker build -f docker/Dockerfile.circom.jammy -t tachyon:cpu  --target builder .
docker run -it tachyon:cpu
# Build with CUDA
docker build -f docker/Dockerfile.circom.jammy -t tachyon:cuda  --target builder-cuda .
docker run -it --runtime nvidia --gpus all tachyon:cuda
```

NOTE: To use external files for proving or compiling, you need to mount or copy them to the tachyon folder to use it within docker.

## How to run

```shell
bazel-bin/prover_main -h
Usage:

bazel-bin/prover_main zkey wtns proof public [OPTIONS]

Positional arguments:

zkey                The path to zkey file
wtns                The path to wtns file
proof               The path to proof json
public              The path to public json

Optional arguments:

--trace_path        The path to generate trace file
--curve             The curve type among ('bn254', 'bls12_381'), by default 'bn254'
--no_zk             Create proof without zk. By default zk is enabled. Use this flag in case you want to compare the proof with rapidsnark.
--no_use_mmap       Create proof without mmap(2). By default, mmap(2) is enabled, offering faster proof generation at the cost of increased memory usage due to the memory mapped file. Use this flag if you want to use less memory.
--verify            Verify the proof. By default verify is disabled. Use this flag to verify the proof with the public inputs.
-n, --num_runs      The number of times to run the proof generation
--disable_fast_twiddles_mode
                    Disables fast twiddle mode on Icicle NTT domain initialization.
```

## How to compile witness generator

This creates a witness_generator binary similar to when circom is used, but compiles much faster in comparison.

```shell
bazel build --config opt //circomlib/build:compile_witness_generator

bazel-bin/circomlib/build/compile_witness_generator --help
Usage:
bazel-bin/circomlib/build/compile_witness_generator [OPTION]...

-h, --help  help
--cpp,      A path to circuit .cpp file
-f, --field A field to use. default: bn128, supported: bls12381 bn128 goldilocks grumpkin pallas secq256r1 vesta
```
