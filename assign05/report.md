

## Constant propagation

Looking at High Level IR, I checked for instructions like

```
HINS_LOAD_ICONST vrN, $lit  // vrN is a virtual register, $lit is a constant or integer literal
```

To optimize generated code, I replaced references to `vrN` directly with the literal value,
while in cases where `HINS_LOCALADDR` and `HINS_LOAD_INT` would reuse `vrN`, the replacement
would stop until a constant load was encountered again.

### Benefits
- Replacing `vrN` with literals removed some instructions (code size decrease) 
- Benchmark programs gained a little speed as some memory references are replaced with literals

For example, in benchmark program array20:

Unoptimized:
```asm
.L1:
	leaq 3216(%rsp), %r10       /* localaddr vr0, $3216 */
	movq %r10, 3256(%rsp)
	movq $10, 3264(%rsp)        /* ldci vr1, $10 */
	movq 3256(%rsp), %r11       /* ldi vr2, (vr0) */
	movq (%r11), %r11
	movq %r11, 3272(%rsp)
	movq 3272(%rsp), %r10       /* cmpi vr2, vr1 */
	movq 3264(%rsp), %r11
	cmpq %r11, %r10
	jl .L0                      /* jlt .L0 */
	leaq 3216(%rsp), %r10       /* localaddr vr3, $3216 */
	movq %r10, 3280(%rsp)
	movq $0, 3288(%rsp)         /* ldci vr4, $0 */
	movq 3288(%rsp), %r11       /* sti (vr3), vr4 */
	movq 3280(%rsp), %r10
	movq %r11, (%r10)
	jmp .L5                     /* jmp .L5 */
```

Optimized:
```asm
.L1:
	leaq 3216(%rsp), %r10       /* localaddr vr0, $3216 */
	movq %r10, 3256(%rsp)
	movq 3256(%rsp), %r11       /* ldi vr2, (vr0) */
	movq (%r11), %r11
	movq %r11, 3272(%rsp)
	movq 3272(%rsp), %r10       /* cmpi vr2, $10 */
	movq $10, %r11
	cmpq %r11, %r10
	jl .L0                      /* jlt .L0 */
	leaq 3216(%rsp), %r10       /* localaddr vr3, $3216 */
	movq %r10, 3280(%rsp)
	movq $0, %r11               /* sti (vr3), $0 */
	movq 3280(%rsp), %r10
	movq %r11, (%r10)
	jmp .L5                     /* jmp .L5 */
```

Virtual registers `vr1` and `vr4` are replaced directly with integer literals `$10` and `$0` repectively.

### Benchmarks

```
./build.rb array20

time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.210s
user	0m2.183s
sys	0m0.004s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.205s
user	0m2.184s
sys	0m0.000s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.214s
user	0m2.1892
sys	0m0.000s

./build.rb -o array20

time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.082s
user	0m2.060s
sys	0m0.000s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.088s
user	0m2.070s
sys	0m0.000s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.095s
user	0m2.073s
sys 0m0.000s
```

The speed improvement for constant folding improved running time by about 5%, 
a small but still noticeable speedup.


## Register Allocation

In the high-level IR, I set scalar variables (operands) from vr0-vr5 to indicate 
to the low-level generator that they should be replaced by machine registers
instead of by memory.  

So the algorithm naively replaces the following registers in assembly:

```
Unoptimized
    vr0 -> 0(rsp)
    vr1 -> 8(rsp)
    ...

Optimized
    vr0 -> rbx
    vr1 -> r12
    ...
    vr4 -> r15
```

Any virtual registers above vr4 are mapped to memory like normal.  I implmented
register allocation this way because specifically in the benchmark program, I 
noticed that many of the scalar variables were used as loop variables, which 
should speed up the program by a good amount.

### Benefits
- Benchmark programs gained a lot of speed as many memory operations are now done
with machine registers

### Benchmarks


```
./build.rb array20

time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.210s
user	0m2.183s
sys	0m0.004s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.205s
user	0m2.184s
sys	0m0.000s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m2.214s
user	0m2.1892
sys	0m0.000s

./build.rb -o array20

time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m1.151s
user	0m1.127s
sys	0m0.000s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m1.152s
user	0m1.126s
sys	0m0.000s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m1.160s
user	0m1.137s
sys 0m0.000s
```

The speed improvement for register allocation improved running time by about 
48%, nearly halving the running time for the benchmark program!
