cd data
echo === Running cas on data ===
echo === Assembling data ===; ../cas -c math.asm data.asm kernel.asm stdio.asm ## >& Errs; cat Errs; rm Errs
echo === Linking data to d.hex ===; ../cas -o d.hex math.o data.o kernel.o stdio.o ## >& Errs; cat Errs; rm Errs
echo === Running reloc on d.hex ===; ../reloc 4000 d.hex ## >& Errs; cat Errs; rm Errs
echo === Testing reloc on d.hex ===; diff data.hx d.hx; rm d.hx
echo === Running and Testing ds on data ===
echo === ds: math.o ===; ../ds math.o >ds0; rm math.o; diff math.ds ds0; rm ds0
echo === ds: data.o ===; ../ds data.o >ds0; rm data.o; diff data.ds ds0; rm ds0
echo === ds: kernel.o ===; ../ds kernel.o >ds0; rm kernel.o; diff kernel.ds ds0; rm ds0
echo === ds: stdio.o ===; ../ds stdio.o >ds0; rm stdio.o; diff stdio.ds ds0; rm ds0
echo === Running das on d.hex ===
../das <d.hex >d.s
echo === Testing das on d.hex ===
diff data.s d.s; rm d.s
echo === Running dac on d.hex ===
../dac <d.hex >d.c
echo === Testing dac on d.hex ===
diff data.c d.c; rm d.c
echo === Testing cas on data: data.hex versus d.hex ===
diff data.hex d.hex; rm d.hex
cd ../hexs
echo === Running cas on hexs ===
echo === Assembling hexs ===; ../cas -c r.asm ## >& Errs; cat Errs; rm Errs
echo === Linking hexs to rr.hex ===; ../cas -o rr.hex r.o ## >& Errs; cat Errs; rm Errs
echo === Running reloc on rr.hex ===; ../reloc 4000 rr.hex ## >& Errs; cat Errs; rm Errs
echo === Testing reloc on rr.hex ===; diff r.hx rr.hx; rm rr.hx
echo === Running and Testing ds on hexs ===
echo === ds: r.o ===; ../ds r.o >ds0; rm r.o; diff r.ds ds0; rm ds0
echo === Running das on rr.hex ===
../das <rr.hex >rr.s
echo === Testing das on rr.hex ===
diff r.s rr.s; rm rr.s
echo === Running dac on rr.hex ===
../dac <rr.hex >rr.c
echo === Testing dac on rr.hex ===
diff r.c rr.c; rm rr.c
echo === Testing case on hexs: r.hex versus rr.hex ===
diff r.hex rr.hex; rm rr.hex
cd ..
