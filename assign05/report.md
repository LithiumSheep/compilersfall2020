

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

Any virtual registers above vr4 are mapped to memory like normal.  I implemented
register allocation this way because specifically in the benchmark program, I 
noticed that many of the scalar variables were used as loop variables, which 
should speed up the program by a good amount.  In x86_64, there are also 5 caller-owned 
registers (excluding %rbp) that could be used as machine registers, and there
are 5 scalar variables in the benchmark program.  

### Benefits
- The henchmark program gained a lot of speed as many memory operations are now done
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


### Optimizations combined

Benchmarks

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
real	0m0.918s
user	0m0.883s
sys	0m0.012s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m0.926s
user	0m0.895s
sys	0m0.008s
time ./out/array20 < data/array20.in > actual_output/array20.out
real	0m0.918s
user	0m0.893s
sys 0m0.004s
```

With both constant propagation and register allocation enabled, running time 
of the benchmark program was cut by about 59% from the unoptimized version.

```
baseline            ~2.210s
constant-propgation ~2.209  (~5% improvement)
register allocation ~1.154s (~48% improvement)
both                ~0.921s (~58% improvement)
```

### Conclusions

It appears that some optimizations may have compounding effects, as the time 
improvement of combining both optimizations ended up better than 
multiplicatively applying the improvements.

Some inefficiencies still remain in the code, however.  

One optimization that could certainly help is during array address calculations, algebraic identities
could be discovered, such as when calculating the address of arr[0].  In the current
implementation, we get the address of arr, multiply the type size by the index, then add it to the address.

```asm
localaddr vr0, $0
muli vr2, vr1, $8   // if vr1 = 0, this instruction can be skipped
addi vr3, vr0, vr2  // once again, if vr1 = 0, this instruction can also be skipped
```

Another optimization opportunity will be in register allocation.  Currently, 
a maximum of 5 scalar variables are mapped to machine registers in optimized code.
A more general optimization strategy would be to perform live register analysis 
on virtual registers and do better register allocation for basic blocks based on 
currently live registers, returned used machine registers to a pool when 
a virtual register is marked dead.  

Finally, peephole optimizations in x86_64 code to eliminate unnecessary instructions
could reduce code size and improve running time.  For example,

```asm
movq %rbx, %r11             /* addi vr7, vr0, $1 */
movq $1, %r10               // could be removed
addq %r11, %r10             // replaceable with addq $1, %r10
```

Many algebraic instructions had code that moved instructions into %r10 and %r11 before
calculating, and they could be simplified by moving one value to a destination register
and use a memory reference or literal directly in the left hand side argument.
