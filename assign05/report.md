

## Constant Folding

### Algorithm
- removing HINS_LOAD_ICONST vreg, $lit
- replacing references to vreg in basic block with $lit
- 

### Benefits
- Code size is smaller
- Number of vregs reduced

### Challenges
